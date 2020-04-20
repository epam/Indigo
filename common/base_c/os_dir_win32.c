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

#include "base_c/os_dir.h"

#include <stdio.h>
#include <windows.h>

int osDirExists(const char* dirname)
{
    DWORD attr = GetFileAttributesA((LPCSTR)dirname);

    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            return OS_DIR_NOTFOUND;
        return OS_DIR_OTHER;
    }

    if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
    {
        SetLastError(ERROR_DIRECTORY);
        return OS_DIR_NOTDIR;
    }

    return OS_DIR_OK;
}

int osDirCreate(const char* dirname)
{
    if (CreateDirectoryA((LPCSTR)dirname, NULL))
        return OS_DIR_OK;

    if (GetLastError() == ERROR_PATH_NOT_FOUND)
        return OS_DIR_NOTFOUND;

    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return OS_DIR_EXISTS;

    return OS_DIR_OTHER;
}

int osDirSearch(const char* dirname, const char* pattern, OsDirIter* iter)
{
    WIN32_FIND_DATAA data;
    char full_pattern[1024];
    int n;
    HANDLE handle;

    if (pattern == 0)
        pattern = "*";

    n = _snprintf(full_pattern, sizeof(full_pattern), "%s\\%s", dirname, pattern);

    if (n >= sizeof(full_pattern))
    {
        SetLastError(ERROR_FILENAME_EXCED_RANGE);
        return OS_DIR_OTHER;
    }

    handle = FindFirstFileA((LPCSTR)full_pattern, &data);

    if (handle == INVALID_HANDLE_VALUE)
    {
        int err = GetLastError();
        if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)
            return OS_DIR_NOTFOUND;
        if (GetLastError() == ERROR_DIRECTORY)
            return OS_DIR_NOTDIR;
        return OS_DIR_OTHER;
    }

    SetLastError(0);
    iter->dirname = dirname;
    iter->dirstream = handle;
    strncpy(iter->first, data.cFileName, sizeof(iter->first));
    return OS_DIR_OK;
}

int osDirNext(OsDirIter* iter)
{
    WIN32_FIND_DATAA data;

    while (*iter->first || FindNextFileA((HANDLE)iter->dirstream, &data))
    {
        DWORD attr;
        int n;

        if (*iter->first)
            n = _snprintf(iter->path, sizeof(iter->path), "%s\\%s", iter->dirname, iter->first);
        else
            n = _snprintf(iter->path, sizeof(iter->path), "%s\\%s", iter->dirname, data.cFileName);

        *iter->first = 0;

        if (n >= sizeof(iter->path))
        {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            osDirClose(iter);
            return OS_DIR_OTHER;
        }

        attr = GetFileAttributesA((LPCSTR)iter->path);

        if (attr == INVALID_FILE_ATTRIBUTES)
        {
            osDirClose(iter);
            return OS_DIR_OTHER;
        }

        if (attr & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        return OS_DIR_OK;
    }

    if (GetLastError() == ERROR_NO_MORE_FILES)
    {
        osDirClose(iter);
        return OS_DIR_END;
    }
    return OS_DIR_OTHER;
}

void osDirClose(OsDirIter* iter)
{
    if (iter->dirstream != NULL)
    {
        FindClose((HANDLE)iter->dirstream);
        iter->dirstream = NULL;
    }
}

const char* osDirLastError(char* buf, int max_size)
{
    int err = GetLastError();
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, (LPSTR)buf, max_size, NULL);
    if (buf[strlen(buf) - 1] == '\n')
        buf[strlen(buf) - 1] = 0;
    if (buf[strlen(buf) - 1] == '\r')
        buf[strlen(buf) - 1] = 0;

    return buf;
}
