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

#ifndef ENCODER_H
#define ENCODER_H

#include <sys/types.h>

#include "spclock.h"

namespace SProc_private {

class Encoder
  {
  public:
    virtual void send_data(int32_t sample_val) =0;
    virtual void sync_time(const SPClock &clk) =0;
    virtual void flush() =0;
    virtual ~Encoder() {}
  };

} // namespace SProc_private

#endif // ENCODER_H

