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

#ifndef __lzw_encoder_h__
#define __lzw_encoder_h__

#include "base_cpp/bitoutworker.h"
#include "base_cpp/output.h"
#include "lzw/lzw_dictionary.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT LzwEncoder
    {
    public:
        DECL_ERROR;

        LzwEncoder(LzwDict& NewDict, Output& NewOut);

        void start(void);

        void send(int NextSymbol);

        void finish(void);

        ~LzwEncoder(void);

    private:
        LzwDict& _dict;

        BitOutWorker _bitout;

        int _string;

        byte _char;

        bool _isFinished;

        // no implicit copy
        LzwEncoder(const LzwEncoder&);
    };

    class LzwOutput : public Output
    {
    public:
        LzwOutput(LzwEncoder& encoder);

        void write(const void* data, int size) override;
        void writeByte(byte value) override;
        void flush() override;

    private:
        LzwEncoder& _encoder;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
