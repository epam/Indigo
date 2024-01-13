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

#include <algorithm>
#include <ctype.h>
#include <errno.h>
#include <limits>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <array>
#include <cppcodec/base64_default_rfc4648.hpp>

#include "base_c/defs.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "reusable_obj_array.h"

using namespace indigo;

enum
{
    MAX_LINE_LENGTH = 1048576
};

IMPL_ERROR(Scanner, "scanner");

Scanner::~Scanner()
{
}

int Scanner::readIntFix(int digits)
{
    int result;

    char buf[20];
    if (digits >= NELEM(buf) - 1)
        throw Error("readIntFix(): digits = %d", digits);

    read(digits, buf);
    buf[digits] = 0;

    char* end;
    result = strtol(buf, &end, 10);
    // Check that some digits were read
    if (buf == end)
        throw Error("readIntFix(%d): invalid number representation: \"%s\"", digits, buf);

    // Check that the unread part contains only spaces
    while (end != buf + digits)
    {
        if (!isspace(*end))
            throw Error("readIntFix(%d): invalid number representation: \"%s\"", digits, buf);
        end++;
    }

    return result;
}

int Scanner::readInt1(void)
{
    QS_DEF(Array<char>, buf);
    char c;
    int result;

    buf.clear();

    skipSpace();

    while (!isEOF())
    {
        c = readChar();
        if (!isdigit(c) && c != '-' && c != '+')
            break;
        buf.push(c);
        if (buf.size() > MAX_LINE_LENGTH)
            throw Error("Line length is too long. Probably the file format is not correct.");
    }

    buf.push(0);

    if (sscanf(buf.ptr(), "%d", &result) < 1)
        throw Error("readInt(): error parsing %s", buf.ptr());

    return result;
}

int Scanner::readInt(void)
{
    QS_DEF(Array<char>, buf);
    char c;
    int result;

    buf.clear();

    c = readChar();

    if (c == '+' || c == '-' || isdigit(c))
        buf.push(c);

    while (isdigit(lookNext()))
    {
        buf.push(readChar());
        if (buf.size() > MAX_LINE_LENGTH)
            throw Error("Line length is too long. Probably the file format is not correct.");
    }

    buf.push(0);

    if (sscanf(buf.ptr(), "%d", &result) < 1)
        throw Error("readInt(): error parsing %s", buf.ptr());

    return result;
}

// Try to read unsigned int. Return readed value, on error return -1 and restore position
int Scanner::tryReadUnsigned()
{
    int result = 0;
    bool was_digit = false;
    long long pos = tell();

    while (!isEOF())
    {
        char c = readChar();
        if (isdigit(c))
        {
            was_digit = true;
            result = (int)(c - '0') + result * 10;
        }
        else
        {
            seek(-1, SEEK_CUR);
            break;
        }
    }
    if (!was_digit)
    {
        seek(pos, SEEK_SET);
        return -1;
    }

    return result;
}

int Scanner::readUnsigned()
{
    int result = tryReadUnsigned();
    if (result < 0)
        throw Error("readUnsigned(): no digits");

    return result;
}

// This very basic floating-point number parser was written
// to avoid locale problems on various platforms.
bool Scanner::_readDouble(double& res, int max)
{
    res = 0;

    bool plus = false;
    bool minus = false;
    bool digit = false;
    bool e = false;
    double denom = 0;
    int cnt = 0;

    while (1)
    {
        if (max > 0 && cnt == max)
            break;

        char c = (char)lookNext();

        if (c == -1) // EOF
            break;
        if (c == '+')
        {
            if (plus || minus || digit || denom > 1)
                return false;
            plus = true;
        }
        else if (c == '-')
        {
            if (plus || minus || digit || denom > 1)
                return false;
            minus = true;
        }
        else if (isdigit(c))
        {
            if (denom > 1)
            {
                res += (c - '0') / (double)denom;
                denom *= 10;
            }
            else
                res = res * 10 + (c - '0');
            digit = true;
        }
        else if (c == '.')
        {
            if (denom > 1)
                return false;
            denom = 10;
        }
        else if (c == 'E' || c == 'e')
        {
            skip(1);
            e = true;
            break;
        }
        else if (isspace(c))
        {
            if (plus || minus || digit || denom > 1)
                break;
        }
        else
            break;
        skip(1);
        cnt++;
    }

    if (minus)
        res *= -1;

    if (e)
    {
        int exponent = readInt();

        if (exponent > 0)
        {
            while (exponent-- > 0)
                res *= 10;
        }
        while (exponent++ < 0)
            res /= 10;
    }

    return digit;
}

float Scanner::readFloat(void)
{
    double res;

    if (!_readDouble(res, 0))
        throw Error("readFloat(): error parsing");

    return (float)res;
}

bool Scanner::tryReadFloat(float& value)
{
    long long pos = tell();
    double res;

    if (!_readDouble(res, 0))
    {
        seek(pos, SEEK_SET);
        return false;
    }

    value = (float)res;
    return true;
}

void Scanner::readWord(Array<char>& word, const char* delimiters)
{
    word.clear();

    if (isEOF())
        throw Error("readWord(): end of stream");

    do
    {
        int next = lookNext();

        if (next == -1)
            break;

        if (delimiters == 0 && isspace((char)next))
            break;

        if (delimiters != 0 && strchr(delimiters, (char)next) != NULL)
            break;

        word.push(readChar());

        if (word.size() > MAX_LINE_LENGTH)
            throw Error("Line length is too long. Probably the file format is not correct.");
    } while (!isEOF());

    word.push(0);
}

float Scanner::readFloatFix(int digits)
{
    long long pos = tell();
    double res;

    if (!_readDouble(res, digits))
        throw Error("readFloatFix(): error parsing");

    long long rest = tell() - pos - digits;

    // Check that the unread part contains only spaces
    while (rest-- > 0LL)
    {
        if (!isspace(readChar()))
            throw Error("readFloatFix(): garbage after the number");
    }

    return (float)res;
}

char Scanner::readChar()
{
    char c;

    read(sizeof(char), &c);
    return c;
}

byte Scanner::readByte()
{
    byte c;

    read(1, &c);
    return c;
}

bool Scanner::skipLine()
{
    char c;

    if (isEOF())
        return false;

    while (!isEOF())
    {
        c = readChar();
        if (c == '\n')
        {
            if (lookNext() == '\r')
                skip(1);
            return true;
        }
        if (c == '\r')
        {
            if (lookNext() == '\n')
                skip(1);
            return true;
        }
    }

    return false;
}

void Scanner::read(int length, Array<char>& buf)
{
    buf.resize(length);
    read(length, buf.ptr());
}

void Scanner::skipSpace()
{
    while (isspace(lookNext()))
        skip(1);
}

void Scanner::skipBom()
{
    long long pos = tell();
    const int kBOMSize = 3;
    const std::array<unsigned char, kBOMSize> kBOM = {0xEF, 0xBB, 0xBF};
    if (length() >= kBOMSize)
    {
        std::array<unsigned char, kBOMSize> bom;
        readCharsFix(kBOMSize, (char*)bom.data());
        if (bom != kBOM)
            seek(pos, SEEK_SET);
    }
}

void Scanner::skipUntil(const char* delimiters)
{
    while (strchr(delimiters, lookNext()) == nullptr)
        skip(1);
}

void Scanner::appendLine(Array<char>& out, bool append_zero)
{
    if (isEOF())
        throw Error("appendLine(): end of stream");

    if (out.size() > 0)
        while (out.top() == 0)
            out.pop();

    do
    {
        char c = readChar();

        if (c == '\r')
        {
            if (lookNext() == '\n')
                continue;
            break;
        }
        if (c == '\n')
            break;

        out.push(c);

        if (out.size() > MAX_LINE_LENGTH)
            throw Error("Line length is too long. Probably the file format is not correct.");
    } while (!isEOF());

    if (append_zero)
        out.push(0);
}

void Scanner::readLine(Array<char>& out, bool append_zero)
{
    out.clear();
    appendLine(out, append_zero);
}

void Scanner::readCharsFix(int n, char* chars_out)
{
    read(n, chars_out);
}

int Scanner::readCharsFlexible(int n, char* chars_out)
{
    int i = 0;
    while ((i < n) && !isEOF())
    {
        chars_out[i++] = readChar();
    }
    return i;
}

word Scanner::readBinaryWord()
{
    word res;

    read(sizeof(word), &res);

    return res;
}

dword Scanner::readBinaryDword()
{
    dword res;

    read(sizeof(dword), &res);

    return res;
}

int Scanner::readBinaryInt()
{
    int res;

    read(sizeof(int), &res);

    return res;
    //*res = ntohl(*res);
}

float Scanner::readBinaryFloat()
{
    float res;

    read(sizeof(float), &res);

    return res;
}

short Scanner::readPackedShort()
{
    byte high = readByte();

    if (high < 128)
        return high;

    byte low = readByte();

    high -= 128;

    return high * (short)256 + low;
}

unsigned int Scanner::readPackedUInt()
{
    unsigned int value = 0;

    int shift = 0;
    while (true)
    {
        byte cur = readByte();
        value |= (cur & 0x7F) << shift;

        if (!(cur & 0x80))
            return value;
        shift += 7;
    }
}

void Scanner::readAll(std::string& str)
{
    const long long size = length() - tell();
    const int max_int = std::numeric_limits<int>::max();
    if (size > max_int)
    {
        throw Error("Cannot read more than %d into memory", max_int);
    }
    str.resize(static_cast<size_t>(size));
    read(static_cast<int>(str.size()), &str[0]);
}

void Scanner::readAll(Array<char>& arr)
{
    const long long size = length() - tell();
    const int max_int = std::numeric_limits<int>::max();
    if (size > max_int)
    {
        throw Error("Cannot read more than %d into memory", max_int);
    }

    arr.clear_resize(static_cast<int>(size));

    read(arr.size(), arr.ptr());
}

bool Scanner::isSingleLine(Scanner& scanner)
{
    long long pos = scanner.tell();

    scanner.skipLine();

    bool res = scanner.isEOF();

    scanner.seek(pos, SEEK_SET);
    return res;
}

//
// FileScanner
//

FileScanner::FileScanner(Encoding filename_encoding, const char* filename)
{
    _init(filename_encoding, filename);
}

FileScanner::FileScanner(const char* format, ...)
{
    char filename[1024];

    va_list args;

    va_start(args, format);
    vsnprintf(filename, sizeof(filename), format, args);
    va_end(args);

    _init(ENCODING_ASCII, filename);
}

void FileScanner::_init(Encoding filename_encoding, const char* filename)
{
    _file = 0;
    _file_len = 0LL;

    if (filename == 0)
        throw Error("null filename");

    _file = openFile(filename_encoding, filename, "rb");

    if (_file == NULL)
        throw Error("can't open file %s. Error: %s", filename, strerror(errno));

#ifdef _WIN32
    _fseeki64(_file, 0LL, SEEK_END);
    _file_len = _ftelli64(_file);
    _fseeki64(_file, 0LL, SEEK_SET);
#else
    fseeko(_file, 0LL, SEEK_END);
    _file_len = ftello(_file);
    fseeko(_file, 0LL, SEEK_SET);
#endif
    _invalidateCache();
}

int FileScanner::lookNext()
{
    _validateCache();
    if (_cache_pos == _max_cache)
        return -1;

    return _cache[_cache_pos];
}

void FileScanner::_invalidateCache()
{
    _max_cache = 0;
    _cache_pos = 0;
}

void FileScanner::_validateCache()
{
    if (_cache_pos < _max_cache)
        return;

    size_t nread = fread(_cache, 1, NELEM(_cache), _file);
    _max_cache = static_cast<int>(nread);
    _cache_pos = 0;
}

long long FileScanner::tell()
{
    _validateCache();
#ifdef _WIN32
    return _ftelli64(_file) - _max_cache + _cache_pos;
#else
    return ftello(_file) - _max_cache + _cache_pos;
#endif
}

void FileScanner::read(int length, void* res)
{
    int to_read_from_cache = std::min(length, _max_cache - _cache_pos);
    memcpy(res, _cache + _cache_pos, to_read_from_cache);
    _cache_pos += to_read_from_cache;

    if (to_read_from_cache != length)
    {
        int left = length - to_read_from_cache;
        size_t nread = fread((char*)res + to_read_from_cache, 1, left, _file);

        if (nread != (size_t)left)
            throw Error("FileScanner::read() error");
    }
}

bool FileScanner::isEOF()
{
    if (_file == NULL)
        return true;
    if (_cache_pos < _max_cache)
        return false;

    return tell() == _file_len;
}

void FileScanner::skip(int n)
{
    _validateCache();
    _cache_pos += n;

    if (_cache_pos > _max_cache)
    {
        int delta = _cache_pos - _max_cache;
#ifdef _WIN32
        long long res = _fseeki64(_file, delta, SEEK_CUR);
#else
        long long res = fseeko(_file, delta, SEEK_CUR);
#endif
        _invalidateCache();

        if (res != 0LL)
            throw Error("skip() passes after end of file");
    }
}

void FileScanner::seek(long long pos, int from)
{
#ifdef _WIN32
    if (from == SEEK_CUR)
        _fseeki64(_file, pos - _max_cache + _cache_pos, from);
    else
        _fseeki64(_file, pos, from);
#else
    if (from == SEEK_CUR)
        fseeko(_file, pos - _max_cache + _cache_pos, from);
    else
        fseeko(_file, pos, from);
#endif
    _invalidateCache();
}

long long FileScanner::length()
{
    return _file_len;
}

char FileScanner::readChar()
{
    _validateCache();
    if (_cache_pos == _max_cache)
        throw Error("readChar() passes after end of file");
    return _cache[_cache_pos++];
}

FileScanner::~FileScanner()
{
    if (_file != NULL)
        fclose(_file);
}

//
// BufferScanner
//

void BufferScanner::_init(const char* buffer, int size)
{
    if (size < -1 || (size > 0 && buffer == 0))
        throw Error("incorrect parameters in BufferScanner constructor");
    if (_is_base64)
    {
        std::string encoded(buffer, size);
        auto decoded = base64::decode(encoded.c_str(), encoded.size());
        _base64_buffer.copy(reinterpret_cast<const char*>(decoded.data()), static_cast<int>(decoded.size()));
        _buffer = _base64_buffer.ptr();
        _size = _base64_buffer.size();
    }
    else
    {
        _buffer = buffer;
        _size = size;
    }
    _offset = 0;
}

BufferScanner::BufferScanner(const char* buffer, int buffer_size, bool is_base64) : _is_base64(is_base64)
{
    _init(buffer, buffer_size);
}

BufferScanner::BufferScanner(const byte* buffer, int buffer_size, bool is_base64) : _is_base64(is_base64)
{
    _init((const char*)buffer, buffer_size);
}

BufferScanner::BufferScanner(const char* str, bool is_base64) : _is_base64(is_base64)
{
    if (str == 0)
        throw Error("null input");
    _init(str, (int)strlen(str));
}

BufferScanner::BufferScanner(const Array<char>& arr, bool is_base64) : _is_base64(is_base64)
{
    _init(arr.ptr(), arr.size());
}

BufferScanner::~BufferScanner()
{
}

bool BufferScanner::isEOF()
{
    if (_size < 0)
        throw Error("isEOF() called to unlimited buffer");
    return _offset >= _size;
}

void BufferScanner::read(int length, void* res)
{
    if (_size >= 0 && _offset + length > _size)
        throw Error("BufferScanner::read() error");

    memcpy(res, &_buffer[_offset], length);
    _offset += length;
}

int BufferScanner::lookNext()
{
    if (_size >= 0 && _offset >= _size)
        return -1;

    return _buffer[_offset];
}

long long BufferScanner::length()
{
    return _size;
}

long long BufferScanner::tell()
{
    return _offset;
}

const void* BufferScanner::curptr()
{
    return _buffer + _offset;
}

void BufferScanner::skip(int n)
{
    _offset += n;

    if (_size >= 0 && _offset > _size)
        throw Error("skip() passes after end of buffer");
}

void BufferScanner::seek(long long pos, int from)
{
    if (from == SEEK_SET)
        _offset = static_cast<int>(pos);
    else if (from == SEEK_CUR)
        _offset += static_cast<int>(pos);
    else // SEEK_END
    {
        if (_size < 0)
            throw Error("can not seek from end: buffer is unlimited");
        _offset = _size - static_cast<int>(pos);
    }

    if ((_size >= 0 && _offset > _size) || _offset < 0)
        throw Error("size = %d, offset = %d after seek()", _size, _offset);
}

byte BufferScanner::readByte()
{
    if (_size >= 0 && _offset >= _size)
        throw Error("readByte(): end of buffer");

    return _buffer[_offset++];
}

void Scanner::_prefixFunction(Array<char>& str, Array<int>& prefix)
{
    prefix.clear();
    prefix.push(0);

    int i, k = 0;

    for (i = 1; i < str.size(); i++)
    {
        while ((k > 0) && (str[k] != str[i]))
            k = prefix[k - 1];
        if (str[k] == str[i])
            k++;
        prefix.push(k);
    }
}

bool Scanner::findWord(const char* word)
{
    QS_DEF(ReusableObjArray<Array<char>>, strs);

    strs.clear();
    Array<char>& str = strs.push();

    str.readString(word, false);
    return findWord(strs) == 0;
}

int Scanner::findWord(ReusableObjArray<Array<char>>& words)
{
    if (isEOF())
        return -1;

    QS_DEF(ReusableObjArray<Array<int>>, prefixes);
    QS_DEF(Array<int>, pos);
    int i;
    long long pos_saved = tell();

    prefixes.clear();
    pos.clear();

    for (i = 0; i < words.size(); i++)
    {
        _prefixFunction(words[i], prefixes.push());
        pos.push(0);
    }

    while (!isEOF())
    {
        int c = readChar();

        for (i = 0; i < words.size(); i++)
        {
            while (pos[i] > 0 && words[i][pos[i]] != c)
                pos[i] = prefixes[i][pos[i] - 1];
            if (words[i][pos[i]] == c)
                pos[i]++;

            if (pos[i] == words[i].size())
            {
                seek(-words[i].size(), SEEK_CUR);
                return i;
            }
        }
    }

    seek(pos_saved, SEEK_SET);
    return -1;
}

bool Scanner::findWordIgnoreCase(const char* word)
{
    QS_DEF(ReusableObjArray<Array<char>>, strs);

    strs.clear();
    Array<char>& str = strs.push();

    str.readString(word, false);
    return findWordIgnoreCase(strs) == 0;
}

int Scanner::findWordIgnoreCase(ReusableObjArray<Array<char>>& words)
{
    if (isEOF())
        return -1;

    QS_DEF(ReusableObjArray<Array<int>>, prefixes);
    QS_DEF(Array<int>, pos);
    int i;
    long long pos_saved = tell();

    prefixes.clear();
    pos.clear();

    for (i = 0; i < words.size(); i++)
    {
        _prefixFunction(words[i], prefixes.push());
        pos.push(0);
    }

    while (!isEOF())
    {
        int c = readChar();

        for (i = 0; i < words.size(); i++)
        {
            int c1 = ::tolower(words[i][pos[i]]);
            int c2 = ::tolower(c);

            while (pos[i] > 0 && c1 != c2)
                pos[i] = prefixes[i][pos[i] - 1];
            if (c1 == c2)
                pos[i]++;

            if (pos[i] == words[i].size())
            {
                seek(-words[i].size(), SEEK_CUR);
                return i;
            }
        }
    }

    seek(pos_saved, SEEK_SET);
    return -1;
}
