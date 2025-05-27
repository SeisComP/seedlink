/***************************************************************************
 * Copyright (C) Tristan DIDIER, IPGP/OVSG, 2020                           *
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


#ifndef DEF_SERIAL_STRING_H
#define DEF_SERIAL_STRING_H

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
#include <sys/wait.h>

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

//****************************************************************************
// CONSTANTES
//****************************************************************************

const int MAX_TIME_ERROR    = 1000000; //copied from proto_modbus

//*****************************************************************************
// Data Structures
//*****************************************************************************


//*****************************************************************************
// CLASS miscString_Protocol
//*****************************************************************************


class miscStringProtocol: public Proto
  {
    protected:
    int NCHAN; //configurable with scconfig
    float SAMPLE_PERIOD;//configurable with scconfig
    int FLUSH_PERIOD;//configurable with scconfig
    vector<rc_ptr<OutputChannel> > miscString_channels;//size depending on NCHAN
    int MAXFRAMELENGTH;//depends on NCHAN

    bool startup_message, soh_message;
    int last_day, last_soh;

    time_t nextFlush; //next forced flush date if FLUSH_PERIOD != 0

    //Protected methods
    static void alarm_handler(int sig);
    void handle_response(char* frame);
    void decode_message(char* data);

    //Public methods
    public:
    //Constructeur
    miscStringProtocol(const string &myname);

    //Implementation of virtual methods from Proto class (cf serial_plugin.h)
    void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision) override;
    void flush_channels() override;
};

#endif
