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

#include "base_cpp/bitoutworker.h"
#include "base_cpp/output.h"

using namespace indigo;

BitOutWorker::BitOutWorker(int StartBits, Output& NewOut) : _bits(StartBits), _output(NewOut)
{
    _bitBuffer = 0;
    _bitBufferCount = 0;
};

bool BitOutWorker::writeBits(int Code)
{
    unsigned int offset = sizeof(int) * 8 - _bits - _bitBufferCount, UCode = (unsigned)Code;

    _bitBuffer |= (dword)UCode << offset;
    _bitBufferCount += _bits;

    while (_bitBufferCount >= 8)
    {
        unsigned int next_byte;

        next_byte = _bitBuffer >> ((sizeof(int) - sizeof(char)) * 8);
        _output.writeByte(next_byte);

        _bitBuffer <<= 8;

        _bitBufferCount -= 8;
    }

    return true;
}

void BitOutWorker::close(void)
{
    unsigned int last_byte;

    if (_bitBufferCount)
    {
        last_byte = _bitBuffer >> ((sizeof(int) - sizeof(char)) * 8);
        _output.writeByte(last_byte);
        _bitBufferCount = 0;
        _bitBuffer = 0L;
    }
}

BitOutWorker::~BitOutWorker(void)
{
    close();
}

/* END OF 'BITOUTWORKER.CPP' FILE */
