/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "indigo_abbreviations.h"
#include "indigo_internal.h"

#include "tinyxml.h"

#include "resources/abbreviations.inc"

namespace indigo
{
    namespace abbreviations
    {

        static void readXmlIntoArray(TiXmlElement* root, const char* node_name, std::vector<std::string>& dest)
        {
            TiXmlNode* child = root->FirstChild(node_name);
            for (; child; child = child->NextSiblingElement(node_name))
                dest.push_back(child->FirstChild()->ToText()->Value());
        }

        //
        // IndigoAbbreviations
        //

        IndigoAbbreviations::IndigoAbbreviations()
        {
            clear();
        }

        void IndigoAbbreviations::clear()
        {
            loadDefault();
        }

        void IndigoAbbreviations::loadDefault()
        {
            TiXmlDocument xml;
            xml.Parse(default_abbreviations_xml);
            if (xml.Error())
                throw IndigoError("XML parsing error: %s", xml.ErrorDesc());
            TiXmlHandle hxml(&xml);
            TiXmlHandle handle = hxml.FirstChild("abbreviations");

            TiXmlElement* elem = handle.FirstChild("item").ToElement();
            for (; elem; elem = elem->NextSiblingElement())
            {
                Abbreviation& abbr = abbreviations.add(new Abbreviation);

                const char* name = elem->Attribute("name");
                if (name)
                    abbr.name = name;

                const char* expansion = elem->Attribute("expansion");
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

        IndigoAbbreviations& indigoGetAbbreviationsInstance()
        {
            return indigo_abbreviations_self.getLocalCopy();
        }

    } // namespace abbreviations
} // namespace indigo
