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

#ifndef SEISCOMP_APPS_GDRT_APP_H__
#define SEISCOMP_APPS_GDRT_APP_H__

#include <seiscomp/client/application.h>

#include "udpclient.h"
#include "version.h"


namespace Seiscomp {
namespace Applications {
namespace GDRT {


class Application : public Client::Application {
	public:
		Application(int argc, char** argv);


	protected:
		bool init() override;
		bool run() override;
		void done() override;
		void exit(int returnCode) override;

		const char *version() override {
			return GDRT_VERSION_NAME;
		}

	private:
		UDPClient _client;
};


}
}
}


#endif
