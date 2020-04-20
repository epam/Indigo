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

#ifndef __gray_codes_h__
#define __gray_codes_h__

#include "base_c/defs.h"
#include "base_cpp/tlscont.h"

namespace indigo
{

    // Enumerate all gray codes starting with zero without loops
    class GrayCodesEnumerator
    {
    public:
        GrayCodesEnumerator(int length, bool needFullCode = false);

        void next(void);
        bool isDone(void);

        enum
        {
            START = -1,
            END = -2
        };

        // Ret: START for the call before first next
        int getBitChangeIndex(void);

        // Use bitarray.h functions
        const byte* getCode(void);

    private:
        CP_DECL;
        TL_CP_DECL(Array<int>, _indices);
        TL_CP_DECL(Array<byte>, _code);
        bool _needFullCode;
        int _bitChangeIndex;
    };

} // namespace indigo

#endif
