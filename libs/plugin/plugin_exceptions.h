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

#ifndef PLUGIN_EXCEPTIONS_H
#define PLUGIN_EXCEPTIONS_H

#include <string>

#include "utils.h"

namespace SeedlinkPlugin_private {

using namespace std;
using namespace Utilities;

class PluginError: public GenericException
  {
  public:
    PluginError(const string &message):
      GenericException("Plugin", message) {}
  };

class PluginBrokenLink: public PluginError
  {
  public:
    PluginBrokenLink():
      PluginError("Error sending data to Seedlink server") {}
    PluginBrokenLink(const string &description):
      PluginError(string() + "error sending data to Seedlink server (" + description + ")") {}
  };

class PluginADInvalid: public PluginError
  {
  public:
    PluginADInvalid(const string &source_id, const string &channel_name):
      PluginError(string() + "invalid source ID given for channel " +
        channel_name) {}
  };

class PluginADInUse: public PluginError
  {
  public:
    PluginADInUse(const string &source_id, const string &channel_name):
      PluginError(string() + "source ID " + source_id + " is already in use "
        "by channel " + channel_name) {}
  };

class PluginLibraryError: public PluginError
  {
  public:
    PluginLibraryError(const string &message):
      PluginError(message + " (" + strerror(errno) + ")") {}
  };

} // namespace SeedlinkPlugin_private

namespace SeedlinkPlugin {
using SeedlinkPlugin_private::PluginError;
using SeedlinkPlugin_private::PluginBrokenLink;
using SeedlinkPlugin_private::PluginADInvalid;
using SeedlinkPlugin_private::PluginADInUse;
using SeedlinkPlugin_private::PluginLibraryError;

} // namespace SeedlinkPlugin

#endif // PLUGIN_EXCEPTIONS_H

