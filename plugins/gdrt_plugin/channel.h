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

#ifndef SEISCOMP_APPS_GDRT_CHANNEL_H__
#define SEISCOMP_APPS_GDRT_CHANNEL_H__

#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <cstdint>
#include <boost/type_traits/is_same.hpp>
#include <seiscomp/core/datetime.h>
#include <libmseed.h>
#include <plugin.h>


namespace Seiscomp {
namespace Applications {
namespace GDRT {


template<typename T>
class Channel {
	public:
		Channel(const std::string &networkCode,
			const std::string &stationCode,
		       	const std::string &locationCode,
		       	const std::string &channelCode,
		       	double sampleRate);

		~Channel();

		void put(Core::Time t, T value);
	
	private:
		const std::string _stationId;
		MS3Record* _msr;
		Core::Time _time;
		std::vector<T> _data;

		void send(const char *record, size_t len);
		bool flush(bool force);
};


#include "channel.ipp"


}
}
}


#endif
