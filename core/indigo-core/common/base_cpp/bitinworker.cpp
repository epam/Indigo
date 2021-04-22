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

#include "base_cpp/bitinworker.h"
#include "base_cpp/scanner.h"

using namespace indigo;

BitInWorker::BitInWorker(int StartBits, Scanner& NewIn) : _bits(StartBits), _scanner(NewIn)
{
    _bitBuffer = 0L;
    _bitBufferCount = 0;
}

bool BitInWorker::isEOF(void)
{
    if (_scanner.isEOF())
    {
        if (_bitBufferCount < _bits)
            return true;
        else
            return false;
    }

    return false;
}

bool BitInWorker::readBits(int& Code)
{
    unsigned int Res;

    if (_scanner.isEOF())
    {
        if (_bitBufferCount < _bits)
            return false;
        else
        {
            Res = (_bitBuffer >> (sizeof(int) * 8 - _bits));
            _bitBuffer <<= _bits;
            _bitBufferCount -= _bits;

            Code = Res;

            return true;
        }
    }

    while (_bitBufferCount < _bits)
    {
        int offset = (sizeof(int) - 1) * 8 - _bitBufferCount;

        _bitBuffer |= (byte)(_scanner.readByte()) << offset;

        _bitBufferCount += 8;

        if (_scanner.isEOF())
            break;
    }

    Res = (_bitBuffer >> (sizeof(int) * 8 - _bits));
    _bitBuffer <<= _bits;
    _bitBufferCount -= _bits;

    Code = Res;

    return true;
}

BitInWorker::~BitInWorker(void)
{
}

/* END OF 'BITINWORKER.CPP' FILE */
