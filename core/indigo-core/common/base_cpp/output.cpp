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

#include "base_cpp/output.h"

#include <cerrno>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "base_cpp/tlscont.h"

#ifndef va_copy
#define va_copy(d, s) ((d) = (s))
#endif

using namespace indigo;

IMPL_ERROR(Output, "output");

void Output::writeBinaryInt(int value)
{
    // value = htonl(value);
    write(&value, sizeof(int));
}

void Output::writeBinaryFloat(float value)
{
    write(&value, sizeof(float));
}

void Output::writeByte(byte value)
{
    write(&value, 1);
}

void Output::writeChar(char value)
{
    write(&value, 1);
}

void Output::writeBinaryWord(word value)
{
    // value = htons(value);
    write(&value, sizeof(word));
}

void Output::writeBinaryUInt16(uint16_t value)
{
    // value = htons(value);
    write(&value, sizeof(value));
}

void Output::printf(const char* format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void Output::vprintf(const char* format, va_list args_orig)
{
    QS_DEF(Array<char>, str);
    if (str.size() < 2048)
        str.resize(2048);

    int n;
    while (true)
    {
        va_list args;

        // vsnprintf may change va_list argument that leads to segfault
        va_copy(args, args_orig);

#if defined(_WIN32) && !defined(__MINGW32__)
        n = _vsnprintf_l(str.ptr(), str.size(), format, getCLocale(), args);
#else
        n = vsnprintf(str.ptr(), str.size(), format, args);
#endif
        /* If that worked, return the string. */
        if (n > -1 && n < str.size())
            break;

        /* Else try again with more space. */
        int new_size;
        if (n > -1)                    /* glibc 2.1 */
            new_size = n + 1;          /* precisely what is needed */
        else                           /* glibc 2.0 */
            new_size = str.size() * 2; /* twice the old size */
        str.resize(new_size);
    }

    write(str.ptr(), n);
}

void Output::writeArray(const Array<char>& data)
{
    write(data.ptr(), data.size());
}

void Output::writeCR()
{
    writeChar('\n');
}

void Output::printfCR(const char* format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    writeCR();
}

void Output::writeString(const char* string)
{
    int n = (int)strlen(string);

    write(string, n);
}

void Output::writeStringCR(const char* string)
{
    writeString(string);
    writeCR();
}

void Output::writePackedShort(short value)
{
    byte low = value & 255;
    byte high = (value - low) >> 8;

    if (value < 128)
        writeByte(low);
    else
    {
        writeByte(high + 128);
        writeByte(low);
    }
}

void Output::writePackedUInt(unsigned int value)
{
    if (value == 0)
    {
        writeByte(0);
        return;
    }
    while (value > 0)
    {
        if (value >= 128)
            writeByte((value & 0x7F) | 0x80);
        else
            writeByte(value);
        value >>= 7;
    }
}

void OutputSeek::skip(int count)
{
    seek(count, SEEK_CUR);
}

FileOutput::FileOutput(Encoding filename_encoding, const char* filename)
{
    _file = openFile(filename_encoding, filename, "wb");

    if (_file == NULL)
        throw Error("can't open file %s. Error: %s", filename, strerror(errno));
}

FileOutput::FileOutput(const char* filename)
{
    _file = fopen(filename, "wb");

    if (_file == NULL)
        throw Error("can't open file %s. Error: %s", filename, strerror(errno));
}

FileOutput::FileOutput(bool append, const char* format, ...)
{
    char filename[1024];

    va_list args;

    va_start(args, format);
    vsnprintf(filename, sizeof(filename), format, args);
    va_end(args);

    if (append)
        _file = fopen(filename, "ab+");
    else
        _file = fopen(filename, "wb");

    if (_file == NULL)
        throw Error("can't open file %s. Error: %s", filename, strerror(errno));
}

FileOutput::~FileOutput()
{
    fclose(_file);
}

void FileOutput::write(const void* data, int size)
{
    if (size < 1)
        return;

    size_t res = fwrite(data, size, 1, _file);

    if (res != 1)
        throw Error("file write error in write()");
}

void FileOutput::flush()
{
    fflush(_file);
}

void FileOutput::seek(long long offset, int from)
{
#ifdef _WIN32
    _fseeki64(_file, offset, from);
#else
    fseeko(_file, offset, from);
#endif
}

long long FileOutput::tell() const noexcept
{
#ifdef _WIN32
    return _ftelli64(_file);
#else
    return ftello(_file);
#endif
}

ArrayOutput::ArrayOutput(Array<char>& arr) : _arr(arr)
{
    _arr.clear();
}

void ArrayOutput::write(const void* data, int size)
{
    int old_size = _arr.size();

    _arr.resize(old_size + size);
    memcpy(_arr.ptr() + old_size, data, size);
}

long long ArrayOutput::tell() const noexcept
{
    return _arr.size();
}

void ArrayOutput::flush()
{
}

void ArrayOutput::clear()
{
    _arr.clear();
}

StandardOutput::StandardOutput() : _count(0)
{
}

void StandardOutput::write(const void* data, int size)
{
    if (size == 0)
        return;

    size_t res = fwrite(data, size, 1, stdout);

    if (res != 1)
        throw Error("error writing to standard output");

    _count += size;
}

long long StandardOutput::tell() const noexcept
{
    return _count;
}

void StandardOutput::flush()
{
    fflush(stdout);
}

void NullOutput::write(const void* data, int size)
{
}

void NullOutput::flush()
{
}

StringOutput::StringOutput(std::string& str) : _str(str)
{
}

void StringOutput::write(const void* data, const int size)
{
    const std::string tmp_str(static_cast<const char*>(data), size);
    _str += tmp_str;
}

long long StringOutput::tell() const noexcept
{
    return static_cast<long long>(_str.size());
}

void StringOutput::flush()
{
}

void StringOutput::clear() noexcept
{
    _str.clear();
}

namespace indigo
{
    void bprintf(Array<char>& buf, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        ArrayOutput output(buf);
        output.vprintf(format, args);
        output.writeChar(0);
        va_end(args);
    }

} // namespace indigo