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

#include <string>
#include <cstdio>

#define SEISCOMP_COMPONENT GDRT
#include <seiscomp/logging/log.h>

#include "station.h"


using namespace std;
using namespace Seiscomp;


namespace Seiscomp {
namespace Applications {
namespace GDRT {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station::Station(const std::string &networkCode,
		 const std::string &stationCode,
		 const std::string &locationCode,
		 double sampleRate,
		 Gempa::Ecef2Enu *ecef2enu)
: _ecef2enu(ecef2enu)
, _lbs(networkCode, stationCode, locationCode, "LBS", sampleRate)
, _lbo(networkCode, stationCode, locationCode, "LBO", sampleRate)
, _lbp(networkCode, stationCode, locationCode, "LBP", sampleRate)
, _lbt(networkCode, stationCode, locationCode, "LBT", sampleRate)
, _lbr(networkCode, stationCode, locationCode, "LBR", sampleRate)
, _lbc(networkCode, stationCode, locationCode, "LBC", sampleRate)
, _lbx(networkCode, stationCode, locationCode, "LBX", sampleRate)
, _lby(networkCode, stationCode, locationCode, "LBY", sampleRate)
, _lbz(networkCode, stationCode, locationCode, "LBZ", sampleRate)
, _lb1(networkCode, stationCode, locationCode, "LB1", sampleRate)
, _lb2(networkCode, stationCode, locationCode, "LB2", sampleRate)
, _lb3(networkCode, stationCode, locationCode, "LB3", sampleRate)
, _lb4(networkCode, stationCode, locationCode, "LB4", sampleRate)
, _lb5(networkCode, stationCode, locationCode, "LB5", sampleRate)
, _lb6(networkCode, stationCode, locationCode, "LB6", sampleRate)
, _lbe(networkCode, stationCode, locationCode, "LBE", sampleRate)
, _lbn(networkCode, stationCode, locationCode, "LBN", sampleRate)
, _lbu(networkCode, stationCode, locationCode, "LBU", sampleRate) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Station::parse(const string &msg) {
	int yr, mo, dt, hh, mm, s, o, p, t, c;
	double ss, r, x, y, z, xx, yy, zz, xy, xz, yz;

	if ( sscanf(msg.c_str(),
		    "> %*s %4d%2d%2d%2d%2d%lf %d %d %d %d %lf %d"
		    "%lf %lf %lf %lf %lf %lf %lf %lf %lf",
		    &yr, &mo, &dt, &hh, &mm, &ss, &s, &o, &p, &t, &r, &c,
		    &x, &y, &z, &xx, &yy, &zz, &xy, &xz, &yz) != 21 ) {
		SEISCOMP_ERROR("Invalid message: %s", msg.c_str());
		return;
	}

	Core::Time tm = Core::Time(yr, mo, dt, hh, mm) + Core::TimeSpan(ss);

	if ( !tm.valid() ) {
		SEISCOMP_ERROR("Invalid time: %d%d%d%d%d%lf", yr, mo, dt, hh, mm, ss);
		return;
	}

	_lbs.put(tm, s);
	_lbo.put(tm, o);
	_lbp.put(tm, p);
	_lbt.put(tm, t);
	_lbr.put(tm, r*100);
	_lbc.put(tm, c%100 + c/100%100 + c/10000%100 + c/1000000%100);
	_lbx.put(tm, x);
	_lby.put(tm, y);
	_lbz.put(tm, z);
	_lb1.put(tm, xx);
	_lb2.put(tm, yy);
	_lb3.put(tm, zz);
	_lb4.put(tm, xy);
	_lb5.put(tm, xz);
	_lb6.put(tm, yz);

	if ( _ecef2enu ) {
		auto v = _ecef2enu->convert(x, y, z);
		_lbe.put(tm, v.x);
		_lbn.put(tm, v.y);
		_lbu.put(tm, v.z);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}
