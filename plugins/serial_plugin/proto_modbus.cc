/***************************************************************************
 * Copyright (C) GFZ Potsdam                                               *
 * All rights reserved.                                                    *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 ***************************************************************************/

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "qtime.h"

#include "utils.h"
#include "cppstreams.h"
#include "serial_plugin.h"
#include "plugin_channel.h"
#include "diag.h"

#include "big-endian.h"

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {

const int NUNIT             = 256;
const int NADDR             = 256;
const int SAMPLE_PERIOD     = 10;
const int MAX_TIME_ERROR    = 1000000;

//*****************************************************************************
// Data Structures
//*****************************************************************************

struct ModbusRequest
  {
    be_u_int16_t transaction_id;
    be_u_int16_t protocol_id;
    be_u_int16_t length;
    u_int8_t unit_id;
    u_int8_t function;
    be_u_int16_t reference;
    be_u_int16_t word_count;
  } PACKED;

struct ModbusResponse
  {
    be_u_int16_t transaction_id;
    be_u_int16_t protocol_id;
    be_u_int16_t length;
    u_int8_t unit_id;
    u_int8_t function;
    u_int8_t bc_ex;
    be_int16_t data[NADDR];
  } PACKED;

//*****************************************************************************
// ModbusProtocol
//*****************************************************************************

class OutputChannelEx: public OutputChannel
  {
  public:
    const double realscale;
    const double realoffset;
    const string realunit;
    const int precision;

    OutputChannelEx(const string &channel_name, const string &station_name,
      int max_zeros, double scale, double realscale_init,
      double realoffset_init, const string &realunit_init, int precision_init):
      OutputChannel(channel_name, station_name, max_zeros, scale),
      realscale(realscale_init), realoffset(realoffset_init),
      realunit(realunit_init), precision(precision_init) {}
  };

class ModbusProtocol: public Proto
  {
  private:
    int fd;
    vector<vector<rc_ptr<OutputChannelEx> > > modbus_channels;
    unsigned int minaddr[NUNIT], maxaddr[NUNIT];
    unsigned int curunit;
    unsigned int tran_count;
    bool startup_message;
    bool soh_message;
    int last_day;
    int last_soh;
    ModbusResponse resp;

    static ModbusProtocol *obj;
    static void alarm_handler(int sig);

    ssize_t writen(int fd, const void *vptr, size_t n);
    void handle_response();
    void send_request();
    void do_start();

  public:
    ModbusProtocol(const string &myname): modbus_channels(NUNIT),
      curunit(NUNIT), tran_count(0), startup_message(true),
      soh_message(true), last_day(-1), last_soh(-1)
      {
        obj = this;

        for(int i = 0; i < NUNIT; ++i)
          {
            minaddr[i] = NADDR;
            maxaddr[i] = 0;
          }
      }

    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision) override;
    void flush_channels() override;
    void start() override;
  };

ModbusProtocol *ModbusProtocol::obj;

void ModbusProtocol::alarm_handler(int sig)
  {
    obj->send_request();
  }

void ModbusProtocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    unsigned int unit_id, addr;

    if(sscanf(source_id.c_str(), "%u.%u", &unit_id, &addr) != 2 ||
      unit_id >= NUNIT || addr > NADDR)
        throw PluginADInvalid(source_id, channel_name);

    if(addr == NADDR)
        return;

    if(!modbus_channels[unit_id].size())
        modbus_channels[unit_id] = vector<rc_ptr<OutputChannelEx> >(NADDR);

    if(modbus_channels[unit_id][addr] != NULL)
        throw PluginADInUse(source_id, modbus_channels[unit_id][addr]->channel_name);

    modbus_channels[unit_id][addr] = new OutputChannelEx(channel_name, station_name,
      dconf.zero_sample_limit, scale, realscale, realoffset, realunit,
      precision);

    if(addr < minaddr[unit_id]) minaddr[unit_id] = addr;

    if(addr > maxaddr[unit_id]) maxaddr[unit_id] = addr;

    if(unit_id < curunit) curunit = unit_id;
  }

void ModbusProtocol::flush_channels()
  {
    for(unsigned int unit_id = 0; unit_id < NUNIT; ++unit_id)
      {
        for(unsigned int addr = 0; addr < modbus_channels[unit_id].size(); ++addr)
          {
            if(modbus_channels[unit_id][addr] != NULL)
                modbus_channels[unit_id][addr]->flush_streams();
          }
      }
  }

void ModbusProtocol::start()
  {
    if(curunit == NUNIT) throw PluginError("no channels defined");

    fd = open_port(O_RDWR);

    try
      {
        do_start();
      }
    catch(PluginError &e)
      {
        seed_log << e.what() << endl;
        seed_log << "closing device" << endl;
        close(fd);
        throw;
      }

    seed_log << "closing device" << endl;
    close(fd);
  }

ssize_t ModbusProtocol::writen(int fd, const void *vptr, size_t n)
  {
    ssize_t nwritten;
    size_t nleft = n;
    const char *ptr = (const char *) vptr;

    while (nleft > 0)
      {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
          {
            if(nwritten < 0)
              {
                if(errno == EAGAIN)
                  {
                    if(!terminate_proc)
                      {
                        usleep(100000);
                        continue;
                      }
                  }
                else if(errno == EBADF)  // fail silently if no TCP connection
                  {
                    return 0;
                  }
              }

            return(nwritten);
          }

        nleft -= nwritten;
        ptr += nwritten;
      }

    return(n);
  }

void ModbusProtocol::handle_response()
  {
    struct timeval tv;
    N(gettimeofday(&tv, NULL));
    time_t t = tv.tv_sec;
    tm* ptm = gmtime(&t);

    EXT_TIME et;
    et.year = ptm->tm_year + 1900;
    et.month = ptm->tm_mon + 1;
    et.day = ptm->tm_mday;
    et.hour = ptm->tm_hour;
    et.minute = ptm->tm_min;
    et.second = ptm->tm_sec;
    et.usec = tv.tv_usec;
    et.doy = mdy_to_doy(et.month, et.day, et.year);

    INT_TIME it = ext_to_int(et);

    if(!digitime.valid)
      {
        digitime.it = it;
        digitime.valid = true;
        digitime.exact = true;
      }
    else
      {
        double time_diff = tdiff(it, digitime.it);

        DEBUG_MSG("time_diff = " << time_diff << endl);

        if(time_diff < -MAX_TIME_ERROR || time_diff > MAX_TIME_ERROR)
          {
            logs(LOG_WARNING) << "time diff. " << time_diff / 1000000.0 << " sec" << endl;
            digitime.it = it;
          }
      }

    if(digitime.it.second / (24 * 60 * 60) != last_day)
      {
        startup_message = true;
        soh_message = true;
        last_day = digitime.it.second / (24 * 60 * 60);
      }

    if(dconf.statusinterval &&
      digitime.it.second / (dconf.statusinterval * 60) != last_soh)
      {
        soh_message = true;
        last_soh = digitime.it.second / (dconf.statusinterval * 60);
      }

    if(et.hour != 0 || et.minute != 0 || et.second != 0)
      {
        if(startup_message)
          {
            startup_message = false;
            seed_log << ident_str << SEED_NEWLINE
                     << "protocol: " << dconf.proto_name << SEED_NEWLINE
                     << "sample rate: " << (1 / (double(SAMPLE_PERIOD)))
                     << " Hz" << endl;
          }

        if(soh_message)
          {
            std::ios_base::fmtflags ff(seed_log.flags());
            char tbuf[50];
            strftime(tbuf, 50, "%Y-%m-%d;%H:%M:%S", ptm);
            seed_log << fixed << "status: " << tbuf << ";" << curunit;

            for(unsigned int addr = minaddr[curunit]; addr <= maxaddr[curunit]; ++addr)
              {
                if(modbus_channels[curunit][addr] == NULL) continue;

                seed_log << ";" << setprecision(modbus_channels[curunit][addr]->precision);

                if(modbus_channels[curunit][addr]->realscale < 0)
                    seed_log << (resp.data[addr - minaddr[curunit]] - modbus_channels[curunit][addr]->realoffset);
                else
                    seed_log << (resp.data[addr - minaddr[curunit]] * modbus_channels[curunit][addr]->realscale - modbus_channels[curunit][addr]->realoffset);
                seed_log << modbus_channels[curunit][addr]->realunit;
              }

            seed_log << endl;
	    seed_log.flags(ff);
          }
      }

    for(unsigned int addr = minaddr[curunit]; addr <= maxaddr[curunit]; ++addr)
      {
        if(modbus_channels[curunit][addr] == NULL) continue;

        modbus_channels[curunit][addr]->set_timemark(digitime.it, 0, digitime.quality);
        modbus_channels[curunit][addr]->put_sample(resp.data[addr - minaddr[curunit]]);
      }

    while((curunit = (curunit + 1) % NUNIT))
      {
        if(modbus_channels[curunit].size())
          {
            send_request();
            return;
          }
      }

    digitime.it = add_time(digitime.it, SAMPLE_PERIOD, 0);
    soh_message = false;

    do
      {
        if(modbus_channels[curunit].size()) break;
      }
    while((curunit = (curunit + 1) % NUNIT));
  }

void ModbusProtocol::send_request()
  {
    DEBUG_MSG("request unit " << curunit << endl);

    ModbusRequest req;
    req.transaction_id = tran_count++;
    req.protocol_id = 0;
    req.length = 6;
    req.unit_id = curunit;
    req.function = 4;
    req.reference = dconf.baseaddr + minaddr[curunit];
    req.word_count = maxaddr[curunit] - minaddr[curunit] + 1;

    if(writen(fd, &req, sizeof(ModbusRequest)) < 0)
        throw PluginLibraryError("error writing to " + dconf.port_name);
  }

void ModbusProtocol::do_start()
  {
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGALRM, &sa, NULL));

    struct itimerval itv;
    itv.it_interval.tv_sec = SAMPLE_PERIOD;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;

    N(setitimer(ITIMER_REAL, &itv, NULL));

    while(!terminate_proc)
      {
        if(read_port(fd, &resp, 11 + (maxaddr[curunit] - minaddr[curunit]) * 2) == 0)
            continue;

        if(resp.function == 0x04)
          {
            if(resp.bc_ex != (maxaddr[curunit] - minaddr[curunit] + 1) * 2)
                throw PluginError("unexpected byte count " + to_string(resp.bc_ex));

            handle_response();
          }
        else if(resp.function == 0x84)
          {
            throw PluginError("Modbus exception " + to_string(resp.bc_ex));
          }
        else
          {
            throw PluginError("unexpected function code " + to_string(resp.function));
          }
      }
  }

RegisterProto<ModbusProtocol> proto("modbus");

} // unnamed namespace

