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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "base_c/defs.h"
#include "oracle/ora_logger.h"

using namespace indigo;

int OracleLogger::dbgPrintfV(const char* format, va_list args)
{
    if (_file == NULL)
        return 0;

    int res = vfprintf(_file, format, args);

    fflush(_file);

    return res;
}

int OracleLogger::dbgPrintf(const char* format, ...)
{
    va_list args;
    int n;

    va_start(args, format);
    n = dbgPrintfV(format, args);
    va_end(args);

    return n;
}

OracleLogger::OracleLogger()
{
    _file = NULL;
}

OracleLogger::~OracleLogger()
{
    close();
}

void OracleLogger::close()
{
    if (_file != NULL)
    {
        fclose(_file);
        _file = NULL;
    }
}

bool OracleLogger::init(const char* filename)
{
    char full_name[1024];
#ifdef _WIN32
    char* tmp_dir = getenv("TEMP");
    char path[1024];
    if (tmp_dir == NULL)
        strcpy(path, "C:\\");
    else
        snprintf(path, sizeof(path), "%s\\", tmp_dir);
#else
    char path[] = "/tmp/";
#endif
    close();

    snprintf(full_name, sizeof(full_name), "%s%s", path, filename);

    _file = fopen(full_name, "a+t");

    if (_file == NULL)
        return false;

    return true;
}

bool OracleLogger::isInited()
{
    return _file != NULL;
}

bool OracleLogger::initIfClosed(const char* filename)
{
    if (isInited())
        return true;

    return init(filename);
}

void OracleLogger::dbgPrintfTS(const char* format, ...)
{
    va_list args;

    va_start(args, format);
    dbgPrintfVTS(format, args);
    va_end(args);
}

void OracleLogger::dbgPrintfVTS(const char* format, va_list args)
{
    time_t tm = time(NULL);
    const struct tm* lt = localtime(&tm);

    dbgPrintf("[%02d.%02d.%4d %02d:%02d:%02d] ", lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900, lt->tm_hour, lt->tm_min, lt->tm_sec);

    dbgPrintfV(format, args);
}
