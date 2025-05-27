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

#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <cerrno>
#include <cstdio>

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

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {

const int SAMPLE_PERIOD     = 1;
const int RECVBUFSIZE       = 256;
const int READ_TIMEOUT      = 15;
const int MAX_TIME_ERROR    = 100000;

//*****************************************************************************
// VaisalaProtocol
//*****************************************************************************

class VaisalaProtocol: public Proto
  {
  private:
    int fd;
    char recvbuf[RECVBUFSIZE + 1];
    map<string, rc_ptr<OutputChannel> > vaisala_channels;
    int wp, rp;
    bool startup_message;
    bool soh_message;
    int last_day;
    int last_soh;

    static VaisalaProtocol *obj;
    static void alarm_handler(int sig);

    ssize_t writen(int fd, const void *vptr, size_t n);
    void decode_message(const char *msg);
    void handle_response(const char *msg);
    void measure_request();
    void do_start();

  public:
    VaisalaProtocol(const string &myname):
      wp(0), rp(0), startup_message(true), soh_message(true),
      last_day(-1), last_soh(-1)
      {
        obj = this;
      }

    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision) override;
    void flush_channels() override;
    void start() override;
  };

VaisalaProtocol *VaisalaProtocol::obj;

void VaisalaProtocol::alarm_handler(int sig)
  {
    obj->measure_request();
  }

void VaisalaProtocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    map<string, rc_ptr<OutputChannel> >::iterator it;
    if((it = vaisala_channels.find(source_id)) != vaisala_channels.end())
        throw PluginADInUse(source_id, it->second->channel_name);

    vaisala_channels[source_id] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void VaisalaProtocol::flush_channels()
  {
    map<string, rc_ptr<OutputChannel> >::iterator it;
    for(it = vaisala_channels.begin(); it != vaisala_channels.end(); ++it)
      {
        if(it->second != NULL)
            it->second->flush_streams();
      }
  }

void VaisalaProtocol::start()
  {
    fd = open_port(O_RDWR);

    try
      {
        do_start();
      }
    catch(PluginError &e)
      {
        seed_log << "closing device" << endl;
        close(fd);
        throw;
      }

    seed_log << "closing device" << endl;
    close(fd);
  }

ssize_t VaisalaProtocol::writen(int fd, const void *vptr, size_t n)
  {
    ssize_t nwritten;
    size_t nleft = n;
    const char *ptr = (const char *) vptr;

    while (nleft > 0)
      {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
            return(nwritten);

        nleft -= nwritten;
        ptr += nwritten;
      }

    return(n);
  }

void VaisalaProtocol::decode_message(const char *msg)
  {
    DEBUG_MSG(msg << endl);

    int n = 0, rp = 0, toklen, seplen;
    while((toklen = strcspn(msg + rp, ",")))
      {
        seplen = strspn(msg + rp + toklen, ",");

        if(++n < 2)
          {
            rp += (toklen + seplen);
            continue;
          }

        char source_id[3];
        double val;
        char status;
        if(sscanf(msg + rp, "%2s=%lf%c", source_id, &val, &status) != 3)
          {
            logs(LOG_WARNING) << "error parsing '" << msg << "' at: " << (msg + rp) << endl;
            return;
          }

        if(status == '#')  // invalid measurement
          {
            rp += (toklen + seplen);
            continue;
          }

        map<string, rc_ptr<OutputChannel> >::iterator it;
        if((it = vaisala_channels.find(source_id)) != vaisala_channels.end())
          {
            it->second->set_timemark(digitime.it, 0, digitime.quality);
            it->second->put_sample(val);
          }

        rp += (toklen + seplen);
      }

    if(toklen)
        logs(LOG_WARNING) << "unused data at the end of message: " << (msg + rp) << endl;
  }

void VaisalaProtocol::handle_response(const char *msg)
  {
    if(strncmp(msg, "0R", 2))
      {
        logs(LOG_WARNING) << "invalid message: " << msg << endl;
        return;
      }

    if(msg[2] == '0')  // automatic message
        return;

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
    else if(!strncmp(msg, "0R1", 3))
      {
        digitime.it = add_time(digitime.it, SAMPLE_PERIOD, 0);
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
            char tbuf[50];
            strftime(tbuf, 50, "%Y-%m-%d;%H:%M:%S;", ptm);
            seed_log << "status: " << tbuf << msg << endl;

            if(!strncmp(msg, "0R5", 3))
                soh_message = false;
          }
      }

    decode_message(msg);
  }

void VaisalaProtocol::measure_request()
  {
    DEBUG_MSG("request" << endl);

    if(writen(fd, "0R\r\n", 4) < 0)
        throw PluginLibraryError("error writing to " + dconf.port_name);
  }

void VaisalaProtocol::do_start()
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
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);

        struct timeval tv;
        tv.tv_sec = READ_TIMEOUT;
        tv.tv_usec = 0;

        int r;
        if((r = select(fd + 1, &read_set, NULL, NULL, &tv)) < 0)
          {
            if(errno == EINTR) continue;
            throw PluginLibraryError("select error");
          }

        if(r == 0)
            throw PluginError("timeout");

        if(FD_ISSET(fd, &read_set))
          {
            int bytes_read;
            if((bytes_read = read(fd, &recvbuf[wp], RECVBUFSIZE - wp)) < 0)
                throw PluginLibraryError("error reading from " + dconf.port_name);

            if(bytes_read == 0)
                throw PluginError("EOF reading " + dconf.port_name);

            wp += bytes_read;
            recvbuf[wp] = 0;

            int rp = 0, msglen, seplen;
            while(msglen = strcspn(recvbuf + rp, "\r\n"),
              seplen = strspn(recvbuf + rp + msglen, "\r\n"))
              {
                if(msglen > 0)
                  {
                    recvbuf[rp + msglen] = 0;
                    handle_response(recvbuf + rp);
                  }

                rp += (msglen + seplen);
              }

            if(msglen >= RECVBUFSIZE)
              {
                logs(LOG_WARNING) << "receive buffer overflow" << endl;
                wp = rp = 0;
                continue;
              }

            memmove(recvbuf, recvbuf + rp, msglen);
            wp -= rp;
            rp = 0;
          }
      }
  }

RegisterProto<VaisalaProtocol> proto("vaisala");

} // unnamed namespace

