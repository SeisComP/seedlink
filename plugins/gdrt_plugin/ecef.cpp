/***************************************************************************
 * Copyright (C) 2022 by gempa GmbH                                        *
 *                                                                         *
 * All Rights Reserved.                                                    *
 *                                                                         *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 ***************************************************************************/


#include "ecef.h"

#include <seiscomp/math/matrix3.h>

#include <cmath>
#include <iostream>

namespace {

inline double sinDeg(double value) {
	return std::sin(deg2rad(value));
}

inline double cosDeg(double value) {
	return std::cos(deg2rad(value));
}

constexpr double SemiMajorAxis = 6378.1370e3; // Earth semi-major axis
constexpr double SemiMinorAxis = 6356.7523e3; // Earth semi-minor axis

}


namespace Gempa {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Math::Vector3d geodetic2ecef(double lat, double lon, double h) {
	double e2 = 1 - std::pow(SemiMinorAxis / SemiMajorAxis, 2.0);
	double n = SemiMajorAxis / std::sqrt(1 - e2 * pow(sinDeg(lat), 2));
	double x = (n + h) * cosDeg(lat) * cosDeg(lon);
	double y = (n + h) * cosDeg(lat) * sinDeg(lon);
	double z = (pow(SemiMinorAxis / SemiMajorAxis, 2.0) * n + h) * sinDeg(lat);

	return Seiscomp::Math::Vector3d(x, y, z);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Ecef2Enu::Ecef2Enu(double lat, double lon, double h) {
	_m.setRow(0, Seiscomp::Math::Vector3d(-sinDeg(lon), cosDeg(lon), 0));
	_m.setRow(1, Seiscomp::Math::Vector3d(-sinDeg(lat) * cosDeg(lon),
	                                      -sinDeg(lat) * sinDeg(lon), cosDeg(lat)));
	_m.setRow(2, Seiscomp::Math::Vector3d(cosDeg(lat) * cosDeg(lon),
	                                      cosDeg(lat) * sinDeg(lon), sinDeg(lat)));

	_x0 = geodetic2ecef(lat, lon, h);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Math::Vector3d Ecef2Enu::convert(double x, double y, double z) const {
	Seiscomp::Math::Vector3d x1(x, y, z);

	Seiscomp::Math::Vector3d enu;
	return _m.transform(enu, x1 - _x0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Enu2Ecef::Enu2Ecef(double lat, double lon, double h) {
	_m.setRow(0, Seiscomp::Math::Vector3d(-sinDeg(lon),
	                                      -sinDeg(lat) * cosDeg(lon),
	                                      cosDeg(lat) * cosDeg(lon)));
	_m.setRow(1, Seiscomp::Math::Vector3d(cosDeg(lon),
	                                      -sinDeg(lat) * sinDeg(lon),
	                                      cosDeg(lat) * sinDeg(lon)));
	_m.setRow(2, Seiscomp::Math::Vector3d(0, cosDeg(lat), sinDeg(lat)));

	_x0 = geodetic2ecef(lat, lon, h);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Math::Vector3d Enu2Ecef::convert(double e, double n, double u) const {
	Seiscomp::Math::Vector3d enu(e, n, u);
	Seiscomp::Math::Vector3d ecef;
	return _m.transform(ecef, enu) + _x0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
