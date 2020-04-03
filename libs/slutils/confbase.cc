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

#include <iostream>
#include <string>
#include <map>
#include <list>
#include <stack>
#include <cstring>

#include "confbase.h"
#include "utils.h"
#include "diag.h"

namespace CfgParser_private {

using namespace std;
using namespace Utilities;

//*****************************************************************************
// CfgStack
//*****************************************************************************

void CfgStack::init(rc_ptr<CfgElementMap> root)
  {
    maps.push(root);
  }

void CfgStack::push(ostream &cfglog, rc_ptr<CfgElement> el,
  const string &name)
  {
    if(el == NULL)
      {
        elements.push(NULL);
        maps.push(NULL);
      }
    else
      {
        elements.push(el);
        maps.push(el->start_children(cfglog, name));
      }
  }

void CfgStack::pop(ostream &cfglog)
  {
    internal_check(!elements.empty());
    
    rc_ptr<CfgElement> el = elements.top();
    if(el != NULL && maps.top() != NULL) el->end_children(cfglog);
    maps.pop();
    elements.pop();
  }

rc_ptr<CfgElementMap> CfgStack::get_elements()
  {
    return maps.top();
  }

} // namespace CfgParser_private

