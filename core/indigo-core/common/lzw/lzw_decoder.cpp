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

#include "lzw/lzw_decoder.h"
#include "base_cpp/scanner.h"
#include "lzw/lzw_dictionary.h"

using namespace indigo;

IMPL_ERROR(LzwDecoder, "LZW decoder");

CP_DEF(LzwDecoder);

LzwDecoder::LzwDecoder(LzwDict& NewDict, Scanner& NewIn) : _dict(NewDict), _bitin(_dict.getBitCodeSize(), NewIn), CP_INIT, TL_CP_GET(_symbolsBuf)
{
}

bool LzwDecoder::isEOF(void)
{
    if (_bitin.isEOF())
    {
        if (_symbolsBuf.size())
            return false;
        else
            return true;
    }

    return false;
}

int LzwDecoder::get(void)
{
    if (_symbolsBuf.size())
        return _symbolsBuf.pop();

    int NextCode;

    if (_bitin.isEOF())
        throw Error("end of stream");

    _bitin.readBits(NextCode);

    while (NextCode > _dict.getAlphabetSize())
    {
        _symbolsBuf.push(_dict.getChar(NextCode));
        NextCode = _dict.getPrefix(NextCode);
    }

    return NextCode;
}

//
// LzwScanner
//

IMPL_ERROR(LzwScanner, "LZW scanner");

LzwScanner::LzwScanner(LzwDecoder& decoder) : _decoder(decoder)
{
}

void LzwScanner::read(int length, void* res)
{
    char* data = (char*)res;
    for (int i = 0; i < length; i++)
        data[i] = _decoder.get();
}

void LzwScanner::skip(int n)
{
    throw Error("skip is not implemented");
}

bool LzwScanner::isEOF()
{
    return _decoder.isEOF();
}

int LzwScanner::lookNext()
{
    throw Error("lookNext is not implemented");
}

void LzwScanner::seek(long long pos, int from)
{
    throw Error("seek is not implemented");
}

long long LzwScanner::length()
{
    throw Error("length is not implemented");
}

long long LzwScanner::tell()
{
    throw Error("tell is not implemented");
}

byte LzwScanner::readByte()
{
    return _decoder.get();
}
