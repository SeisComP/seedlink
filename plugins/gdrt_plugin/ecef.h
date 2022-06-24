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


#ifndef SEISCOMP_APPS_GDRT_ECEF_H__
#define SEISCOMP_APPS_GDRT_ECEF_H__

#include <seiscomp/math/matrix3.h>
#include <seiscomp/math/vector3.h>

#include <string>

namespace Gempa {

/**
 * @brief Convert Earth Centered Earth Fixed coordinates x, y, z
          to local tangent plane coordinates East, North, Up
          WGS 84 ellipsoid used
 */
class Ecef2Enu {
	public:
		/**
		 * @brief Reference point
		 * @param lat in degrees
		 * @param lon in degrees
		 * @param h in meters above surface
		 */
		Ecef2Enu(double lat, double lon, double h);

		/**
		 * @brief Convert ECEF to ENU
		 * @param x ECEF x-coordinate in meters (points to lat=0, lon=0)
		 * @param y ECEF y-coordinate in meters (points to lat=0, lon=90)
		 * @param z ECEF z-coordinate in meters (points to lat=90)
		 * @return East, North, Up in meters
		 */
		Seiscomp::Math::Vector3d convert(double x, double y, double z)  const;

	protected:
		Seiscomp::Math::Matrix3d _m;
		Seiscomp::Math::Vector3d _x0;
};


/**
 * @brief Convert local tangent plane coordinates East, North, Up
          to Earth Centered Earth Fixed coordinates x, y, z
          WGS 84 ellipsoid used
 */
class Enu2Ecef {
	public:
		/**
		 * @brief Reference point
		 * @param lat in degrees
		 * @param lon in degrees
		 * @param h in meters above surface
		 */
		Enu2Ecef(double lat, double lon, double h);

		/**
		 * @brief Convert ENU to ECEF
		 * @param e East in meters
		 * @param n North in meters
		 * @param u Up in meters
		 * @return x, y, z in meters
		 */
		Seiscomp::Math::Vector3d convert(double e, double n, double u) const;

	protected:
		Seiscomp::Math::Matrix3d _m;
		Seiscomp::Math::Vector3d _x0;
};


/**
 * @brief Convert geodetic coordinates lat, lon, h to
          Earth Centered Earth Fixed coordinates x, y, z
          WGS 84 ellipsoid used
 * @param lat in degrees
 * @param lon in degrees
 * @param h in meters above surface
 * @return x, y, z in meters
 */
Seiscomp::Math::Vector3d geodetic2ecef(double lat, double lon, double h);

}

#endif
