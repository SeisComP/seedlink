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

#include <fstream>

#define SEISCOMP_COMPONENT GDRT
#include <seiscomp/logging/log.h>

#include "app.h"
#include "settings.h"


using namespace std;
using namespace Seiscomp;


namespace Seiscomp {
namespace Applications {
namespace GDRT {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Application(int argc, char** argv)
: Client::Application(argc, argv) {
	setMessagingEnabled(false);
	setDatabaseEnabled(true, false);
	setLoadStationsEnabled(true);
	bindSettings(&global);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::init() {
	if ( !Client::Application::init() )
		return false;

	ifstream ifs(global.plugins.gdrt.stationsFrom);
	if ( !ifs.is_open() ) {
		SEISCOMP_ERROR("Cannot open %s", global.plugins.gdrt.stationsFrom.c_str());
		return false;
	}

	string key, networkCode, stationCode, locationCode;
	double sampleRate;

	while ( ifs >> key >> networkCode >> stationCode >> locationCode >> sampleRate ) {
		SEISCOMP_INFO("Adding station %s (%s.%s.%s) @ %lf Hz",
			      key.c_str(),
			      networkCode.c_str(),
			      stationCode.c_str(),
			      locationCode.c_str(),
			      sampleRate);

		if ( locationCode == "--" )
			locationCode = "";

		Gempa::Ecef2Enu* ecef2enu = nullptr;

		DataModel::SensorLocation* loc = Client::Inventory::Instance()->getSensorLocation(
			networkCode,
			stationCode,
			locationCode,
			Core::Time::UTC());

		if ( loc ) {
			try {
				ecef2enu = new Gempa::Ecef2Enu(loc->latitude(),
				                               loc->longitude(),
				                               loc->elevation());
			}
			catch ( Core::ValueError & ) {
			}
		}

		if ( !ecef2enu )
			SEISCOMP_WARNING("Cannot find coordinates of %s.%s.%s, transformed streams will not be available",
					 networkCode.c_str(),
					 stationCode.c_str(),
					 locationCode.c_str());

		_client.addStation(key,
				   networkCode,
				   stationCode,
				   locationCode,
				   sampleRate,
				   ecef2enu);
	}

	// Inventory no longer needed
	Client::Inventory::Reset();

	if ( !ifs.eof() ) {
		SEISCOMP_ERROR("%s: invalid input", global.plugins.gdrt.stationsFrom.c_str());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::run() {
	SEISCOMP_INFO("Starting GDRT client");
	_client.run();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {
	Client::Application::done();
	SEISCOMP_INFO("Done");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exit(int returnCode) {
	Client::Application::exit(returnCode);
	SEISCOMP_INFO("Stopping GDRT client");
	_client.stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}
