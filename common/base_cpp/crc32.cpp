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

#include <string.h>

#include "base_c/defs.h"
#include "base_cpp/crc32.h"

using namespace indigo;

CRC32 _crc; // singletone

unsigned _Reflect(unsigned ref, char ch)
{
    unsigned value = 0;

    // swap bits 0-7, 1-6, ...
    for (int i = 1; i <= ch; i++)
    {
        if (ref & 1)
            value |= 1 << (ch - i);
        ref >>= 1;
    }
    return value;
}

CRC32::CRC32()
{
    int i, polynom = 0x04c11db7; // used in PKZip, WinZip, and Ethernet.

    for (i = 0; i < 256; i++)
    {
        _table[i] = _Reflect(i, 8) << 24;
        for (int j = 0; j < 8; j++)
            _table[i] = (_table[i] << 1) ^ (_table[i] & (1 << 31) ? polynom : 0);
        _table[i] = _Reflect(_table[i], 32);
    }
}

CRC32::~CRC32()
{
}

unsigned CRC32::get(const char* text)
{
    unsigned code = 0xffffffff;

    while (*text != 0)
    {
        code = (code >> 8) ^ _crc._table[(code & 0xFF) ^ ((byte)*text)];
        text++;
    }

    return code ^ 0xffffffff;
}

unsigned CRC32::get(const char* text, int len)
{
    unsigned code = 0xffffffff;

    for (int i = 0; i < len; i++)
        code = (code >> 8) ^ _crc._table[(code & 0xFF) ^ ((byte)text[i])];

    return code ^ 0xffffffff;
}
