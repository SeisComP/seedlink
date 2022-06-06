/***************************************************************************
 * (C) 2021 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ *
 * All rights reserved.                                                    *
 *                                                                         *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 ***************************************************************************/

#ifndef SEISCOMP_APPS_GDRT_STATION_H__
#define SEISCOMP_APPS_GDRT_STATION_H__

#include <string>
#include <seiscomp/core/baseobject.h>

#include "station.h"
#include "channel.h"


namespace Seiscomp {
namespace Applications {
namespace GDRT {

DEFINE_SMARTPOINTER(Station);
class Station : public Core::BaseObject {
	public:
		Station(const std::string &networkCode,
			const std::string &stationCode,
			const std::string &locationCode,
			double sampleRate);

		void parse(const std::string &msg);

	private:
		Channel<std::int32_t> _lbs;
		Channel<std::int32_t> _lbo;
		Channel<std::int32_t> _lbp;
		Channel<std::int32_t> _lbt;
		Channel<std::int32_t> _lbr;
		Channel<std::int32_t> _lbc;
		Channel<double> _lbx;
		Channel<double> _lby;
		Channel<double> _lbz;
		Channel<double> _lb1;
		Channel<double> _lb2;
		Channel<double> _lb3;
		Channel<double> _lb4;
		Channel<double> _lb5;
		Channel<double> _lb6;
};


}
}
}


#endif
