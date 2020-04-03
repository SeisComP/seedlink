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

#ifndef FILTER_H
#define FILTER_H

#include <string>

#include "cbuf.h"

namespace SProc_private {

using namespace std;

class Filter
  {
  protected:
    const int dec;
    int len;
    
    Filter(const string &name_init, int dec_init): dec(dec_init), name(name_init) {}

  public:
    const string name;

    int length()
      {
        return len;
      }

    int decimation()
      {
        return dec;
      }

    virtual double apply(CircularBuffer<double>::iterator p) =0;
    virtual double shift() =0;
    virtual ~Filter() {}
  };

} // namespace SProc_private

namespace SProc {

using SProc_private::Filter;

} // namespace SProc

#endif // FILTER_H

