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

#ifndef __lzw_decoder_h__
#define __lzw_decoder_h__

#include "base_cpp/bitinworker.h"
#include "base_cpp/scanner.h"
#include "lzw/lzw_dictionary.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT LzwDecoder
    {
    public:
        DECL_ERROR;

        LzwDecoder(LzwDict& NewDict, Scanner& NewIn);

        bool isEOF(void);

        int get(void);

    private:
        LzwDict& _dict;

        BitInWorker _bitin;

        CP_DECL;
        TL_CP_DECL(Array<byte>, _symbolsBuf);

        // no implicit copy
        LzwDecoder(const LzwDecoder&);
    };

    class LzwScanner : public Scanner
    {
    public:
        LzwScanner(LzwDecoder& decoder);

        void read(int length, void* res) override;
        void skip(int n) override;
        bool isEOF() override;
        int lookNext() override;
        void seek(long long pos, int from) override;
        long long length() override;
        long long tell() override;

        byte readByte() override;

        DECL_ERROR;

    private:
        LzwDecoder& _decoder;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
