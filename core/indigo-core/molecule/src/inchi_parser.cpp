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

#include <string>

#include "molecule/inchi_parser.h"

using namespace indigo;

IMPL_EXCEPTION(indigo, InchiParserError, "InchiParser");

InChICodeParser::InChICodeParser(const char* inchi_code) : _mobileCount(0)
{
    std::string str(inchi_code);

    size_t pos = str.find("/h");
    // assert(pos != npos)
    int num = 0, from = -1;
    bool isValid;
    int type = STATIC;
    for (pos += 2; pos < str.size() && str[pos] != '/'; ++pos)
    {
        isValid = false;
        switch (str[pos])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            num = num * 10 + (str[pos] - '0');
            break;
        case ',':
            if (type == 0)
            {
                type = MOBILE;
                _mobileCount += (num ? num : 1);
            }
            else
            {
                if (from == -1)
                {
                    _hydrogens.push(num - 1);
                    _types.push(type);
                }
                else
                {
                    for (auto i = from; i <= num; ++i)
                    {
                        _hydrogens.push(i - 1);
                        _types.push(type);
                    }
                    from = -1;
                }
            }
            num = 0;
            break;
        case '-':
            if (from != -1)
            {
                throw InchiParserError("Dash without left boundary defined");
            }
            from = num;
            num = 0;
            break;
        case '(':
            if (pos < str.size() && str[pos + 1] == 'H')
            {
                ++pos;
                type = 0;
            }
            else
            {
                throw InchiParserError("Invalid InChI format: \"%s\"", inchi_code);
            }
            break;
        case ')':
            if (from == -1)
            {
                _hydrogens.push(num - 1);
                _types.push(type);
            }
            else
            {
                for (auto i = from; i <= num; ++i)
                {
                    _hydrogens.push(i - 1);
                    _types.push(type);
                }
                from = -1;
            }
            num = 0;
            type = STATIC;
            isValid = true;
            break;
        case 'H':
            if (from == -1)
            {
                _hydrogens.push(num - 1);
                _types.push(type);
            }
            else
            {
                for (auto i = from; i <= num; ++i)
                {
                    _hydrogens.push(i - 1);
                    _types.push(type);
                }
                from = -1;
            }
            num = 0;
            if (pos < str.size() && str[pos + 1] == ',')
            {
                ++pos;
            }
            else
            {
                isValid = true;
            }
            break;
        default:;
        }
    }
    if (!isValid)
    {
        // throw InchiParserError("Invalid InChI format: \"%s\"", inchi_code);
    }
}

int InChICodeParser::_nextElement(int type, int index)
{
    if (index == -1)
        index = 0;
    else
        ++index;

    for (; index != _hydrogens.size(); ++index)
    {
        if (_types[index] & type)
            break;
    }
    return index;
}
