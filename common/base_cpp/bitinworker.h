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

#ifndef __bitinworker_h__
#define __bitinworker_h__

#include "base_c/defs.h"

namespace indigo
{

    class Scanner;

    class DLLEXPORT BitInWorker
    {
    public:
        BitInWorker(int StartBits, Scanner& NewIn);

        bool readBits(int& Code);

        bool isEOF(void);

        ~BitInWorker(void);

    private:
        int _bits; /* Code size */

        int _bitBufferCount;

        dword _bitBuffer;

        Scanner& _scanner;

        BitInWorker(const BitInWorker&);
    };

} // namespace indigo

#endif /* __bitinworker_h__ */

/* END OF 'BITINWORKER.H' FILE */
