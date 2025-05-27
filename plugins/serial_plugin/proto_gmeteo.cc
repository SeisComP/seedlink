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
const int READ_TIMEOUT      = 30;
const int MAX_TIME_ERROR    = 1000000;

//*****************************************************************************
// MeteoProtocol
//*****************************************************************************

class MeteoProtocol: public Proto
  {
  private:
    int fd;
    char recvbuf[RECVBUFSIZE + 1];
    map<string, rc_ptr<OutputChannel> > meteo_channels;
    int wp, rp;
    bool startup_message;
    bool soh_message;
    int last_day;
    int last_soh;

    ssize_t writen(int fd, const void *vptr, size_t n);
    void decode_message(const char *msg);
    void handle_response(const char *msg);
    void do_start();

  public:
    MeteoProtocol(const string &myname):
      wp(0), rp(0), startup_message(true), soh_message(true),
      last_day(-1), last_soh(-1) {}

    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision) override;
    void flush_channels() override;
    void start() override;
  };

void MeteoProtocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    map<string, rc_ptr<OutputChannel> >::iterator it;
    if((it = meteo_channels.find(source_id)) != meteo_channels.end())
        throw PluginADInUse(source_id, it->second->channel_name);

    meteo_channels[source_id] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

void MeteoProtocol::flush_channels()
  {
    map<string, rc_ptr<OutputChannel> >::iterator it;
    for(it = meteo_channels.begin(); it != meteo_channels.end(); ++it)
      {
        if(it->second != NULL)
            it->second->flush_streams();
      }
  }

void MeteoProtocol::start()
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

void MeteoProtocol::decode_message(const char *msg)
  {
    int rp = 0, toklen, seplen;
    while(toklen = strcspn(msg + rp, ","),
      seplen = strspn(msg + rp + toklen, ","))
      {
        char source_id[3];
        double val;
        if(sscanf(msg + rp, " %2s=%lf,", source_id, &val) != 2)
          {
            logs(LOG_WARNING) << "error parsing '" << msg << "' at: " << (msg + rp) << endl;
            return;
          }

        map<string, rc_ptr<OutputChannel> >::iterator it;
        if((it = meteo_channels.find(source_id)) != meteo_channels.end())
          {
            it->second->set_timemark(digitime.it, 0, digitime.quality);
            it->second->put_sample(val);
          }

        rp += (toklen + seplen);
      }

    if(!strcmp(msg + rp, " OK"))
        return;

    if(toklen)
        logs(LOG_WARNING) << "unused data at the end of message: " << (msg + rp) << endl;
    else
        logs(LOG_WARNING) << "missing OK" << endl;
  }

void MeteoProtocol::handle_response(const char *msg)
  {
    DEBUG_MSG(msg << endl);

    time_t t;
    if(sscanf(msg, "%ld:", &t) != 1)
      {
        logs(LOG_WARNING) << "invalid message: " << msg << endl;
        return;
      }

    tm* ptm = gmtime(&t);

    EXT_TIME et;
    et.year = ptm->tm_year + 1900;
    et.month = ptm->tm_mon + 1;
    et.day = ptm->tm_mday;
    et.hour = ptm->tm_hour;
    et.minute = ptm->tm_min;
    et.second = ptm->tm_sec;
    et.usec = 0;
    et.doy = mdy_to_doy(et.month, et.day, et.year);

    digitime.it = ext_to_int(et);
    digitime.valid = true;
    digitime.exact = true;

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
            soh_message = false;
            seed_log << "weather data: " << msg << endl;
          }
      }

    if((msg = strchr(msg, ':')) != NULL)
        decode_message(msg + 1);
  }

void MeteoProtocol::do_start()
  {
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

RegisterProto<MeteoProtocol> proto("gmeteo");

} // unnamed namespace

