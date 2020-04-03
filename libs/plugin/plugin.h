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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <sys/types.h>

#ifdef PLUGIN_COMPATIBILITY
#include "qtime.h"
#endif

#define PLUGIN_INTERFACE_VERSION  3
#define PLUGIN_FD                 63
#define PLUGIN_MAX_DATA_BYTES     4000
#define PLUGIN_MAX_MSG_SIZE       448
#define PLUGIN_MSEED_SIZE         512
#define PLUGIN_SIDLEN             10      /* length of station ID */
#define PLUGIN_CIDLEN             10      /* length of channel ID */

enum PluginPacketType
  {
    PluginRawDataTimePacket = 8,
    PluginRawDataPacket,
    PluginRawDataGapPacket,
    PluginRawDataFlushPacket,
    PluginLogPacket,
    PluginMSEEDPacket
  };

struct ptime
  {
    int year;      /* year, eg. 2003                   */
    int yday;      /* day of year (1-366)              */
    int hour;      /* hour (0-23)                      */
    int minute;    /* minute (0-59)                    */
    int second;    /* second (0-59), 60 if leap second */
    int usec;      /* microsecond (0-999999)           */
  };
    
struct PluginPacketHeader
  {
    enum PluginPacketType packtype;
    char station[PLUGIN_SIDLEN];
    char channel[PLUGIN_CIDLEN];
    struct ptime pt;
    int usec_correction;
    int timing_quality;
    int data_size;
  };
    
#ifdef __cplusplus
extern "C" {
#endif

int send_raw3(const char *station, const char *channel, const struct ptime *pt,
  int usec_correction, int timing_quality, const int32_t *dataptr,
  int number_of_samples);
int send_flush3(const char *station, const char *channel);
int send_log3(const char *station, const struct ptime *pt, const char *fmt,
  ...);
int send_mseed(const char *station, const void *dataptr, int packet_size);
int send_mseed2(const char *station, const char *channel, int seq,
  const void *dataptr, int packet_size);
int send_raw_depoch(const char *station, const char *channel, double depoch,
  int usec_correction, int timing_quality, const int32_t *dataptr,
  int number_of_samples);

#ifdef PLUGIN_COMPATIBILITY
int send_raw(const char *station, const char *channel, const INT_TIME *it,
  int usec_correction, const int32_t *dataptr, int number_of_samples);
int send_raw2(const char *station, const char *channel, const INT_TIME *it,
  int usec_correction, int timing_quality, const int32_t *dataptr,
  int number_of_samples);
int send_log(const char *station, const INT_TIME *it, const char *fmt, ...);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_H */

