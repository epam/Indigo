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

#include <cstdint>
#include <regex>
#include <string>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    std::string latin1_to_utf8(const std::string& src);
    bool is_valid_utf8(const std::string& data);

    // Reads the code point of the UTF-8 sequence at pos and moves pos past it. A byte that does not start a complete
    // sequence is read as the code point with its value and moves pos by one byte.
    uint32_t utf8_next_code_point(const std::string& str, size_t& pos);
    void utf8_append_code_point(std::string& str, uint32_t code_point);
    // The length of UTF-8 text in UTF-16 code units: the unit of the offsets in KET text (JavaScript strings).
    size_t utf8_length_in_utf16(const std::string& str, size_t pos = 0, size_t count = std::string::npos);
    // The byte offset in UTF-8 text of an offset in UTF-16 code units, or the text size when it is past the end.
    size_t utf16_offset_to_utf8(const std::string& str, size_t utf16_offset);

    bool validate_base64(const std::string& str);

    std::vector<std::string> split_with_empty(const std::string& str, char delim);
    std::vector<std::string> split(const std::string& str, char delim);
    std::vector<std::string> split_spaces(const std::string& str);
    std::vector<std::string> split_to_lines(const std::string& str);

    inline bool is_lower_case(const std::string& str)
    {
        for (auto c : str)
            if (!std::islower(c))
                return false;
        return true;
    }

    inline bool is_upper_case(const std::string& str)
    {
        for (auto c : str)
            if (!std::isupper(c))
                return false;
        return true;
    }

    inline int extract_id(const std::string& str, const std::string& start)
    {
        if (str.find(start) == 0)
        {
            auto ss_id = str.substr(start.size());
            if (ss_id.size())
                return std::stoi(ss_id);
        }
        return -1;
    }

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
