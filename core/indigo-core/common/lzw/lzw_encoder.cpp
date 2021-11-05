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

#include "lzw/lzw_encoder.h"
#include "base_cpp/bitoutworker.h"
#include "base_cpp/output.h"

using namespace indigo;

IMPL_ERROR(LzwEncoder, "LZW encoder");

LzwEncoder::LzwEncoder(LzwDict& NewDict, Output& NewOut) : _dict(NewDict), _bitout(_dict.getBitCodeSize(), NewOut)
{
    _string = -1;
    _char = 0;
    _isFinished = false;
}

void LzwEncoder::start(void)
{
    _isFinished = false;
}

void LzwEncoder::send(int NextSymbol)
{
    int Index, HashIndex;

    if (_string < 0)
    {
        _string = NextSymbol;
        return;
    }

    _char = NextSymbol;

    HashIndex = _dict.hashFunction(_string, _char);

    /* 'string++char' is in dictionary */
    if ((Index = _dict.dictSearch(_string, _char, HashIndex)) != -1)
    {
        _string = Index;
        return;
    }

    _dict.addElem(_string, _char, HashIndex);

    _bitout.writeBits(_string);

    _string = _char;
}

void LzwEncoder::finish(void)
{
    if (!_isFinished)
    {
        _bitout.writeBits(_string);
        _bitout.close();
        _string = -1;
        _isFinished = true;
    }
}

LzwEncoder::~LzwEncoder(void)
{
    finish();
}

//
// LzwOutput
//

LzwOutput::LzwOutput(LzwEncoder& encoder) : _encoder(encoder)
{
}

void LzwOutput::writeByte(byte value)
{
    _encoder.send(value);
}

void LzwOutput::write(const void* data_, int size)
{
    const char* data = (const char*)data_;
    for (int i = 0; i < size; i++)
        _encoder.send(data[i]);
}

void LzwOutput::flush()
{
}
