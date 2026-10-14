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

#include "render_svg_ids.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdio>
#include <string>

#include <tinyxml2.h>

#include "base_cpp/crc32.h"
#include "base_cpp/exception.h"

using namespace indigo;
using namespace tinyxml2;

std::string indigo::makeSvgIdPrefix(const std::string& svg)
{
    static std::atomic<unsigned long long> counter{0};

    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::string moment = std::to_string(now) + "/" + std::to_string(counter.fetch_add(1));

    const unsigned svg_crc = CRC32::get(svg.data(), static_cast<int>(std::min<size_t>(svg.size(), INT_MAX)));
    const unsigned time_crc = CRC32::get(moment.data(), static_cast<int>(moment.size()));

    char hex[16];
    std::snprintf(hex, sizeof(hex), "%08x", svg_crc + time_crc);
    // A leading letter keeps the id a valid XML name and CSS selector (an id may not start with a digit).
    return std::string("i") + hex + "-";
}

namespace
{
    // Prefixes every "url(#id)" inside an attribute value.
    std::string prefixUrlReferences(const std::string& value, const std::string& prefix)
    {
        static const std::string open = "url(#";
        std::string result;
        size_t pos = 0;
        for (size_t found; (found = value.find(open, pos)) != std::string::npos; pos = found + open.size())
        {
            result.append(value, pos, found + open.size() - pos);
            result.append(prefix);
        }
        result.append(value, pos, std::string::npos);
        return result;
    }

    bool isLocalHref(const std::string& name, const std::string& value)
    {
        return (name == "href" || name == "xlink:href") && !value.empty() && value[0] == '#';
    }

    void prefixElement(XMLElement* element, const std::string& prefix)
    {
        for (const XMLAttribute* attr = element->FirstAttribute(); attr != nullptr; attr = attr->Next())
        {
            const std::string name = attr->Name();
            const std::string value = attr->Value();
            if (name == "id")
                element->SetAttribute(name.c_str(), (prefix + value).c_str());
            else if (isLocalHref(name, value))
                element->SetAttribute(name.c_str(), ("#" + prefix + value.substr(1)).c_str());
            else if (value.find("url(#") != std::string::npos)
                element->SetAttribute(name.c_str(), prefixUrlReferences(value, prefix).c_str());
        }
        for (XMLElement* child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
            prefixElement(child, prefix);
    }
}

std::string indigo::prefixSvgIds(const std::string& svg, const std::string& prefix)
{
    XMLDocument doc;
    if (doc.Parse(svg.c_str(), svg.size()) != XML_SUCCESS)
        throw Exception("SVG parsing error: %s", doc.ErrorStr());

    if (XMLElement* root = doc.RootElement())
        prefixElement(root, prefix);

    XMLPrinter printer;
    doc.Print(&printer);
    return printer.CStr();
}
