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

#ifndef DIAG_H
#define DIAG_H

#include <iostream>
#include <cstdlib>

#define internal_check(expr) do { if(!(expr)) { \
  std::clog << "internal error (" __FILE__  ":" << __LINE__ << "); " \
  "failed expression was: " #expr << endl; std::exit(1); } } while(0)

#define N(expr) do { if((expr) < 0) { \
  std::clog << __FILE__ ":" << __LINE__ << ": " << strerror(errno) << endl; \
  std::exit(1); } } while(0)

#define P(expr) do { if((expr) == NULL) { \
  std::clog << __FILE__ ":" << __LINE__ << ": " << strerror(errno) << endl; \
  std::exit(1); } } while(0)

#ifdef DEBUG_MESSAGES
#define DEBUG_MSG(s) do { logs(LOG_DEBUG) << s; } while(0)
#else
#define DEBUG_MSG(s) do { } while(0)
#endif

#endif // DIAG_H

