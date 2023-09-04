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

#ifndef __parse_utils__
#define __parse_utils__

#include <string>
#include <regex>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    std::string latin1_to_utf8(const std::string& src);
    bool is_valid_utf8(const std::string& data);

    inline bool validate_base64(const std::string& str)
    {
        if (str.size() & 3) // check for padding
            return false;
        std::regex base64reg_exp("^[a-zA-Z0-9\\+/]*={0,3}$");
        return std::regex_match(str, base64reg_exp);
    }

    inline std::vector<std::string> split(const std::string& str, char delim)
    {
        std::vector<std::string> strings;
        size_t start;
        size_t end = 0;
        while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
        {
            end = str.find(delim, start);
            strings.push_back(str.substr(start, end - start));
        }
        return strings;
    }

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
