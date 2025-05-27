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

#ifndef MSEED_H
#define MSEED_H

#include <string>

#include "qtime.h"

#include "buffer.h"
#include "utils.h"
#include "format.h"

namespace SProc_private {

using namespace std;
using namespace Utilities;

class MSEEDFormat: public Format
  {
  private:
    const rc_ptr<BufferStore> bufs;
    const int rec_length;
    const PacketType packtype;
    const string station_id;
    const string network_id;
    const string location_id;
    const string stream_name;
    int sample_rate_factor;
    int sample_rate_multiplier;
    int sequence;

    Buffer *get_buffer(const INT_TIME &it, int usec_correction,
      int timing_quality, void *&dataptr, int &datalen) override;
    void queue_buffer(Buffer *buf, int samples, int frames) override;

  public:
    MSEEDFormat(rc_ptr<BufferStore> bufs_init, int rec_length_init,
      PacketType packtype_init, const string &station_id_init,
      const string &network_id_init, const string &location_id_init,
      const string &stream_name_init, int freqn, int freqd):
      bufs(bufs_init), rec_length(rec_length_init), packtype(packtype_init),
      station_id(station_id_init), network_id(network_id_init),
      location_id(location_id_init), stream_name(stream_name_init), sequence(0)
      {
        if(freqn == 0 || freqd == 0)
          {
            sample_rate_factor = 0;
            sample_rate_multiplier = 0;
          }
        else if(!(freqn % freqd))
          {
            sample_rate_factor = freqn / freqd;
            sample_rate_multiplier = 1;
          }
        else if(!(freqd % freqn))
          {
            sample_rate_factor = -freqd / freqn;
            sample_rate_multiplier = 1;
          }
        else
          {
            sample_rate_factor = -freqd;
            sample_rate_multiplier = freqn;
          }
      }
  };

} // namespace SProc_private

namespace SProc {

using SProc_private::MSEEDFormat;

} // namespace SProc

#endif // MSEED_H

