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

#ifndef __gzip_scanner__
#define __gzip_scanner__

#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"

#include <zlib.h>

namespace indigo
{

    class GZipScanner : public Scanner
    {
    public:
        enum
        {
            CHUNK_SIZE = 32768
        };

        explicit GZipScanner(Scanner& source);
        ~GZipScanner() override;

        void read(int length, void* res) override;
        long long tell() override;
        bool isEOF() override;
        void seek(long long pos, int from) override;
        int lookNext() override;
        void skip(int length) override;
        long long length() override;
        void readAll(Array<char>& arr) override;

        DECL_ERROR;

    protected:
        Scanner& _source;
        z_stream _zstream;

        bool _read(int length, void* res);

        CP_DECL;
        TL_CP_DECL(Array<Bytef>, _inbuf);
        TL_CP_DECL(Array<Bytef>, _outbuf);
        int _outbuf_start;
        int _inbuf_end;
        int _uncompressed_total;
        bool _eof;
    };

} // namespace indigo

#endif
