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

#ifndef FORMAT_H
#define FORMAT_H

#include <string>

#include "buffer.h"

namespace SProc_private {

using namespace std;

template<class T>
class Packet
  {
  friend class Format;
  private:
    Buffer *buf;

  public:
    T *const data;
    const int datalen;
    
    Packet(): buf(NULL), data(NULL), datalen(0) {}
    Packet(Buffer *buf_init, void *data_init, int datalen_init):
      buf(buf_init), data(static_cast<T *>(data_init)), datalen(datalen_init) {}

    Packet &operator=(const Packet &p)
      {
        if(this != &p)
          {
            this->~Packet();
            new(this) Packet(p);
          }

        return *this;
      }

    bool valid()
      {
        return buf != NULL;
      }
  };

//*****************************************************************************
// PacketType
//*****************************************************************************

enum PacketType { Steim1Packet, Steim2Packet, LogPacket };

//*****************************************************************************
// Format
//*****************************************************************************

class Format
  {
  protected:
    virtual Buffer *get_buffer(const INT_TIME &it, int usec_correction,
      int timing_quality, void *&dataptr, int &datalen) =0;
    virtual void queue_buffer(Buffer *buf, int samples, int frames) =0;

  public:
    virtual ~Format() {}
    
    template<class T>
    Packet<T> get_packet(const INT_TIME &it, int usec_correction,
      int timing_quality)
      {
        void *dataptr = NULL;
        int datalen = 0;
        Buffer* buf = get_buffer(it, usec_correction, timing_quality,
          dataptr, datalen);

        return Packet<T>(buf, dataptr, datalen);
      }
    
    template<class T>
    void queue_packet(Packet<T> &pckt, int samples, int frames)
      {
        queue_buffer(pckt.buf, samples, frames);
        pckt.buf = NULL;
      }
  };

} // namespace SProc_private

namespace SProc {

using SProc_private::Packet;
using SProc_private::Steim1Packet;
using SProc_private::Steim2Packet;
using SProc_private::LogPacket;
using SProc_private::Format;

} // namespace SProc_private

#endif // FORMAT_H

