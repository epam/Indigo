/****************************************************************************
 * Copyright (C) 2010-2013 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "indigo_abbreviations.h"
#include "indigo_internal.h"

#include "tinyxml.h"

#include "resources/abbreviations.inc"

namespace indigo
{
namespace abbreviations
{

static void readXmlIntoArray (TiXmlElement *root, const char *node_name, std::vector<std::string> &dest)
{
   TiXmlNode *child = root->FirstChild(node_name);
   for (; child; child = child->NextSiblingElement(node_name))
      dest.push_back(child->FirstChild()->ToText()->Value());
}

//
// IndigoAbbreviations
//

IndigoAbbreviations::IndigoAbbreviations ()
{
   clear();
}

void IndigoAbbreviations::clear ()
{
   loadDefault();
}

void IndigoAbbreviations::loadDefault ()
{
   TiXmlDocument xml;
   xml.Parse(default_abbreviations_xml);
   if (xml.Error())
      throw IndigoError("XML parsing error: %s", xml.ErrorDesc());
   TiXmlHandle hxml(&xml);
   TiXmlHandle handle = hxml.FirstChild("abbreviations");

   TiXmlElement *elem = handle.FirstChild("item").ToElement();
   for (; elem; elem = elem->NextSiblingElement())
   {
      Abbreviation &abbr = abbreviations.push();

      const char *name = elem->Attribute("name");
      if (name)
         abbr.name = name;

      const char *expansion = elem->Attribute("expansion");
      if (expansion)
         abbr.expansion = expansion;

      readXmlIntoArray(elem, "right", abbr.right_aliases);
      readXmlIntoArray(elem, "left", abbr.left_aliases);
      readXmlIntoArray(elem, "right2", abbr.right_aliases2);
      readXmlIntoArray(elem, "left2", abbr.left_aliases2);
      // Read "alias" into both arrays
      readXmlIntoArray(elem, "alias", abbr.right_aliases);
      readXmlIntoArray(elem, "alias", abbr.left_aliases);
      // Add name if alias is empty
      if (abbr.right_aliases.size() == 0)
         abbr.right_aliases.push_back(name);
      if (abbr.left_aliases.size() == 0)
         abbr.left_aliases.push_back(name);

      if (abbr.expansion.find("*:4") != std::string::npos)
         abbr.connections = 4;
      else if (abbr.expansion.find("*:3") != std::string::npos)
         abbr.connections = 3;
      else if (abbr.expansion.find("*:2") != std::string::npos)
         abbr.connections = 2;
      else
         abbr.connections = 1;
   }
}

//
// Session IndigoName instance
//
_SessionLocalContainer<IndigoAbbreviations> indigo_abbreviations_self;

IndigoAbbreviations& indigoGetAbbreviationsInstance ()
{
   return indigo_abbreviations_self.getLocalCopy();
}

}} // namespace indigo::abbreviations
