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

#include <boost/bind.hpp>

#define SEISCOMP_COMPONENT GDRT
#include <seiscomp/logging/log.h>

#include "udpclient.h"
#include "settings.h"


using namespace std;
using namespace Seiscomp;
using boost::asio::ip::udp;

namespace Seiscomp {
namespace Applications {
namespace GDRT {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void UDPClient::wait() {
	_socket.async_receive_from(boost::asio::buffer(_recvBuffer),
				   _remoteEndpoint,
				   boost::bind(&UDPClient::handleReceive,
					       this,
					       boost::asio::placeholders::error,
					       boost::asio::placeholders::bytes_transferred));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void UDPClient::handleReceive(const boost::system::error_code& error,
			      size_t bytesTransferred) {
	if (error) {
	    SEISCOMP_ERROR("Receive failed: %s", error.message().c_str());
	    return;
	}

	string msg(_recvBuffer.begin(), bytesTransferred);
	char sta[10];

	if ( sscanf(msg.c_str(), "> %9s ", sta) == 1 ) {
		std::map<std::string, StationPtr>::iterator it = _stations.find(sta);
		if ( it != _stations.end() )
			it->second->parse(msg);
		else
			SEISCOMP_INFO("Ignoring unconfigured station %s", sta);
	}
	else {
		SEISCOMP_ERROR("Invalid message: %s", msg.c_str());
	}

	wait();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void UDPClient::addStation(const string &key,
			   const string &networkCode,
			   const string &stationCode,
			   const string &locationCode,
			   double sampleRate) {
	_stations.insert(pair<string, StationPtr>(key,
						  new Station(networkCode,
							      stationCode,
							      locationCode,
							      sampleRate)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void UDPClient::run() {
	_socket.open(udp::v6());
	_socket.set_option(boost::asio::ip::v6_only(false));
	_socket.bind(udp::endpoint(udp::v6(), global.plugins.gdrt.udpport));

	wait();

	SEISCOMP_DEBUG("Starting I/O service");
	_ioService.run();
	SEISCOMP_DEBUG("I/O service finished");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void UDPClient::stop() {
	SEISCOMP_DEBUG("Stopping I/O service");
	_ioService.stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}
