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

#ifndef __gzip_output__
#define __gzip_output__

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

#include <zlib.h>

namespace indigo
{

    class GZipOutput : public Output
    {
    public:
        enum
        {
            CHUNK_SIZE = 32768
        };

        explicit GZipOutput(Output& dest, int level);
        ~GZipOutput() override;

        void write(const void* data, int size) override;
        long long tell() const noexcept override;
        void flush() override;

        DECL_ERROR;

    protected:
        Output& _dest;
        z_stream _zstream;
        int _total_written;

        int _deflate(int flush);

        CP_DECL;
        TL_CP_DECL(Array<Bytef>, _outbuf);
        TL_CP_DECL(Array<Bytef>, _inbuf);
    };

} // namespace indigo

#endif
