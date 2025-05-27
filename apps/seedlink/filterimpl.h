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

#ifndef FILTERIMPL_H
#define FILTERIMPL_H

#include <string>

#include "filter.h"

namespace SProc_private {

using namespace std;

class FilterImpl: public Filter
  {
  public:
    enum FilterType { ZeroPhase, MinimumPhase };

  private:
    const FilterType type;
    const double gain;
    const double *const points;

  public:
    FilterImpl(const string &name, FilterType type_init, int len_init, int dec,
      double gain_init, const double *points_init):
      Filter(name, dec), type(type_init), gain(gain_init), points(points_init)
      {
        len = len_init;
      }

    ~FilterImpl()
      {
        delete[] points;
      }

    double apply(CircularBuffer<double>::iterator p) override
      {
        double acc = 0;

        if(type == ZeroPhase)
          {
            for(int i = 0; i < len / 2; ++i)
                acc += *(p++) * points[i] * gain;

            if (len % 2)
                acc += *(p++) * points[len/2] * gain;

            for(int i = len / 2 - 1; i >= 0; --i)
                acc += *(p++) * points[i] * gain;
          }
        else
          {
            for(int i = 0; i < len; ++i)
                acc += *(p++) * points[i] * gain;
          }

        return acc;
      }

    double shift() override
      {
        return ((type == ZeroPhase) ? (double(len) / 2.0 - 0.5) : 0);
      }
  };

} // namespace SProc_private

namespace SProc {

using SProc_private::FilterImpl;

} // namespace SProc

#endif // FILTERIMPL_H

