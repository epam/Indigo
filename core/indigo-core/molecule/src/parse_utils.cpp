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
        for (auto ch : data)
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
}
