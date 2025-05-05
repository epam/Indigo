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

#include "molecule/parse_utils.h"

#include <cstdint>

namespace indigo
{
    const size_t kAsciiSize = 0x80;
    const uint8_t kUpper_2_bits_shift = 6;
    const uint8_t kLower6bitsMask = 0x3F;
    const uint8_t kUtf8Magic2bytes = 0b110;
    const uint8_t kUtf8Magic3bytes = 0b1110;
    const uint8_t kUtf8Magic4bytes = 0b11110;
    const uint8_t kUtf8MagicLowerByte = 0b10;
    const uint8_t kUtf8Check3HighBits = 5;
    const uint8_t kUtf8Check4HighBits = 4;
    const uint8_t kUtf8Check5HighBits = 3;
    const uint8_t kUtf8Check2HighBits = 6;

    const uint8_t kUtf8ExtraByteSz2 = 1;
    const uint8_t kUtf8ExtraByteSz3 = 2;
    const uint8_t kUtf8ExtraByteSz4 = 3;

    std::string latin1_to_utf8(const std::string& src)
    {
        std::string result;
        for (unsigned char ch : src)
        {
            if (ch < kAsciiSize)
            {
                result.push_back(ch);
            }
            else
            {
                result.push_back(0xC0 | (ch >> kUpper_2_bits_shift));
                result.push_back(kAsciiSize | (ch & kLower6bitsMask));
            }
        }
        return result;
    }

    bool is_valid_utf8(const std::string& data)
    {
        int cnt = 0;
        for (unsigned char ch : data)
        {
            if (cnt == 0)
            {
                if ((ch >> kUtf8Check3HighBits) == kUtf8Magic2bytes)
                    cnt = kUtf8ExtraByteSz2;
                else if ((ch >> kUtf8Check4HighBits) == kUtf8Magic3bytes)
                    cnt = kUtf8ExtraByteSz3;
                else if ((ch >> kUtf8Check5HighBits) == kUtf8Magic4bytes)
                    cnt = kUtf8ExtraByteSz4;
                else if (ch & kAsciiSize)
                    return false;
            }
            else
            {
                if ((ch >> kUtf8Check2HighBits) != kUtf8MagicLowerByte)
                    return false;
                cnt--;
            }
        }
        return cnt == 0;
    }

    bool validate_base64(const std::string& str)
    {
        const int kPadding = 3;
        if (str.size() & kPadding) // check for padding
            return false;
        int sz = static_cast<int>(str.size());
        for (int i = 0; i < sz; ++i)
        {
            auto ch = str[i];
            if ((ch >= 'a' && ch <= 'z') || ((ch >= 'A' && ch <= 'Z')) || ((ch >= '0' && ch <= '9')) || ch == '+' || ch == '/')
                continue;
            if (i > (sz - kPadding) && ch == '=')
                return ++i < sz ? str[i] == '=' : true; // check for 2nd '='
            return false;
        }
        return true;
    }

    std::vector<std::string> split_to_lines(const std::string& str)
    {
        std::vector<std::string> result;
        if (str.empty())
            return result;

        if (str.find_first_of("\r\n") == 0)
            result.emplace_back();

        size_t start = 0, end;
        while (start < str.size())
        {
            end = str.find_first_of("\r\n", start);
            result.emplace_back(str, start, (end == std::string::npos) ? std::string::npos : end - start);

            if (end == std::string::npos)
                break;
            start = (end + 1 < str.size() && ((str[end] == '\r' && str[end + 1] == '\n') || (str[end] == '\n' && str[end + 1] == '\r'))) ? end + 2 : end + 1;
        }

        if (!str.empty() && str.find_last_of("\r\n") == str.size() - 1)
            result.emplace_back();
        return result;
    }

    std::vector<std::string> split_spaces(const std::string& str)
    {
        std::vector<std::string> result;
        size_t i = 0, n = str.size();
        while (i < n)
        {
            size_t start = i;
            bool is_space = (str[i] == ' ');

            i = std::find_if(str.begin() + i, str.end(), [is_space](char c) { return (c == ' ') != is_space; }) - str.begin();

            result.emplace_back(str.substr(start, i - start));
        }
        return result;
    }

    std::vector<std::string> split_with_empty(const std::string& str, char delim)
    {
        std::vector<std::string> strings;
        if (str.size())
        {
            if (str.front() == delim)
            {
                strings.push_back("");
            }

            auto splitted = split(str, delim);
            strings.insert(strings.end(), splitted.begin(), splitted.end());

            if (!str.empty() && str.back() == delim)
            {
                strings.push_back("");
            }
        }
        return strings;
    }

    std::vector<std::string> split(const std::string& str, char delim)
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
}
