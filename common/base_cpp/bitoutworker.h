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

#ifndef __bitoutworker_h__
#define __bitoutworker_h__

#include "base_c/defs.h"

namespace indigo
{

    class Output;

    class BitOutWorker
    {
    public:
        BitOutWorker(int StartBits, Output& NewOut);

        bool writeBits(int Code);

        void close(void);

        ~BitOutWorker(void);

    private:
        int _bits; /* Code size */

        int _bitBufferCount;

        dword _bitBuffer;

        Output& _output;

        BitOutWorker(const BitOutWorker&);
    };

} // namespace indigo

#endif /* __bitoutworker_h__ */

/* END OF 'BITOUTWORKER.H' FILE */
