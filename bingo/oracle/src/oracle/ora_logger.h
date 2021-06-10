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

#ifndef __ora_logger__
#define __ora_logger__

#include <stdarg.h>
#include <stdio.h>

namespace indigo
{

    class OracleLogger
    {
    public:
        explicit OracleLogger();
        virtual ~OracleLogger();

        bool init(const char* filename);
        bool initIfClosed(const char* filename);
        void close();

        bool isInited();

        int dbgPrintf(const char* format, ...);

        int dbgPrintfV(const char* format, va_list args);

        void dbgPrintfTS(const char* format, ...);

        void dbgPrintfVTS(const char* format, va_list args);

    private:
        FILE* _file;
    };

} // namespace indigo

#endif
