/***************************************************************************
 * (C) 2020 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ *
 * All rights reserved.                                                    *
 *                                                                         *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 ***************************************************************************/

#ifndef SEISCOMP_APPS_GDRT_SETTINGS_H__
#define SEISCOMP_APPS_GDRT_SETTINGS_H__

#include <seiscomp/system/application.h>


namespace Seiscomp {
namespace Applications {
namespace GDRT {


// Define default configuration
struct Settings : System::Application::AbstractSettings {
	struct Plugins {
		struct GDRT {
			GDRT(): udpport(9999), stationsFrom("stations.txt") {}

			int udpport;
			std::string stationsFrom;

			virtual void accept(System::Application::SettingsLinker &linker) {
				linker
				& cfg(udpport, "udpport")
				& cfgAsPath(stationsFrom, "stationsFrom");
			}
		} gdrt;

		virtual void accept(System::Application::SettingsLinker &linker) {
			linker
			& cfg(gdrt, "gdrt");
		}
	} plugins;

	virtual void accept(System::Application::SettingsLinker &linker) override {
		linker
		& cli(plugins.gdrt.udpport, "Plugin", "udpport",
		      "UDP port for receiving GDRT messages",
		      true)
		& cli(plugins.gdrt.stationsFrom, "Plugin", "stations-from",
		      "Location of station list file")
		& cfg(plugins, "plugins");
	}
};


extern Settings global;


}
}
}


#endif
