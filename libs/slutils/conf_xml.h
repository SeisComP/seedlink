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

#ifndef CONF_XML_H
#define CONF_XML_H

#include "utils.h"
#include "cppstreams.h"
#include "confbase.h"

namespace CfgParser_private {

using namespace std;
using namespace Utilities;
using namespace CPPStreams;

//*****************************************************************************
// Exceptions
//*****************************************************************************

class CfgCannotFindRoot: public CfgError
  {
  public:
    CfgCannotFindRoot(const string &name, const string &file):
      CfgError("root element '" + name + "' doesn't exist in file '" + file + "'") {}
  };

//*****************************************************************************
// CfgRootElement
//*****************************************************************************

class CfgRootElement: public CfgElement
  {
  private:
    const rc_ptr<CfgAttributeMap> attributes;
    const rc_ptr<CfgElementMap> children;
    bool &found;

  public:
    CfgRootElement(const string &name, rc_ptr<CfgAttributeMap> attributes_init,
      rc_ptr<CfgElementMap> children_init, bool &found_init):
      CfgElement(name), attributes(attributes_init), children(children_init),
      found(found_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &name) override
      {
        found = true;
        return attributes;
      }

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &name) override
      {
        return children;
      }
  };

//*****************************************************************************
// Entry points
//*****************************************************************************

void read_config_xml_helper(const string &file_name,
  rc_ptr<CfgElementMap> root);

template<class _RootElem>
inline void read_config_xml(const string &file_name,
  const _RootElem &root_element)
  {
    rc_ptr<CfgElementMap> root = new CfgElementMap;
    root->add_item(root_element);
    read_config_xml_helper(file_name, root);
  }

inline void read_config_xml(const string &file_name, const string &root_name,
  rc_ptr<CfgAttributeMap> attributes, rc_ptr<CfgElementMap> elements)
  {
    bool found = false;
    read_config_xml(file_name,
      CfgRootElement(root_name, attributes, elements, found));

    if(!found) throw CfgCannotFindRoot(root_name, file_name);
  }

} // namespace CfgParser_private

namespace CfgParser {

using CfgParser_private::read_config_xml;

} // namespace CfgParser

#endif // CONF_XML_H

