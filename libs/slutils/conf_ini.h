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

#ifndef CONF_INI_H
#define CONF_INI_H

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

class CfgCannotFindSection: public CfgError
  {
  public:
    CfgCannotFindSection(const string &name, const string &file):
      CfgError("section '" + name + "' doesn't exist in file '" + file + "'") {}
  };

//*****************************************************************************
// CfgSectionElement
//*****************************************************************************

class CfgSectionElement: public CfgElement
  {
  private:
    const string want;
    const rc_ptr<CfgAttributeMap> attributes;
    const rc_ptr<CfgElementMap> children;
    bool &found;

  public:
    CfgSectionElement(const string &name, rc_ptr<CfgAttributeMap> attributes_init,
      rc_ptr<CfgElementMap> children_init, bool &found_init):
      CfgElement("section"), want(name), attributes(attributes_init),
      children(children_init), found(found_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &name) override;
    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &name) override;
  };

//*****************************************************************************
// Entry points
//*****************************************************************************

void read_config_ini_helper(const string &file_name,
  rc_ptr<CfgElementMap> root);

template<class _RootElem>
inline void read_config_ini(const string &file_name,
  const _RootElem &root_element)
  {
    rc_ptr<CfgElementMap> root = new CfgElementMap;
    root->add_item(root_element);
    read_config_ini_helper(file_name, root);
  }

inline void read_config_ini(const string &file_name, const string &section_name,
  rc_ptr<CfgAttributeMap> attributes, rc_ptr<CfgElementMap> elements)
  {
    bool found = false;
    read_config_ini(file_name,
      CfgSectionElement(section_name, attributes, elements, found));

    if(!found) throw CfgCannotFindSection(section_name, file_name);
  }

} // namespace CfgParser_private

namespace CfgParser {

using CfgParser_private::CfgCannotFindSection;
using CfgParser_private::read_config_ini;

} // namespace CfgParser

#endif // CONF_INI_H

