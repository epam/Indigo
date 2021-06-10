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

//
// Gray code enumertator without loops
//

#include "base_cpp/gray_codes.h"
#include "base_c/bitarray.h"

using namespace indigo;

CP_DEF(GrayCodesEnumerator);

GrayCodesEnumerator::GrayCodesEnumerator(int length, bool needFullCode) : CP_INIT, TL_CP_GET(_indices), TL_CP_GET(_code)

{
    _bitChangeIndex = START;
    _needFullCode = needFullCode;
    _indices.resize(length + 1);
    for (int i = 0; i <= length; i++)
        _indices[i] = i;

    if (needFullCode)
    {
        _code.resize(bitGetSize(length));
        _code.zerofill();
    }
}

void GrayCodesEnumerator::next(void)
{
    if (_indices.size() - 1 == 0)
    {
        _bitChangeIndex = END;
        return;
    }
    _bitChangeIndex = _indices[0];
    if (_bitChangeIndex == _indices.size() - 1)
        _bitChangeIndex = END;
    else
    {
        _indices[0] = 0;
        _indices[_bitChangeIndex] = _indices[_bitChangeIndex + 1];
        _indices[_bitChangeIndex + 1] = _bitChangeIndex + 1;
        if (_needFullCode)
            bitFlipBit(_code.ptr(), _bitChangeIndex);
    }
}

bool GrayCodesEnumerator::isDone(void)
{
    return _bitChangeIndex == END;
}

int GrayCodesEnumerator::getBitChangeIndex(void)
{
    return _bitChangeIndex;
}

const byte* GrayCodesEnumerator::getCode(void)
{
    return _code.ptr();
}
