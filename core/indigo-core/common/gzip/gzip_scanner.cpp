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

#include "gzip/gzip_scanner.h"
#include <algorithm>

using namespace indigo;

IMPL_ERROR(GZipScanner, "GZip scanner");

CP_DEF(GZipScanner);

GZipScanner::GZipScanner(Scanner& source) : _source(source), CP_INIT, TL_CP_GET(_inbuf), TL_CP_GET(_outbuf)
{
    _zstream.zalloc = Z_NULL;
    _zstream.zfree = Z_NULL;
    _zstream.opaque = Z_NULL;
    _zstream.avail_in = 0;
    _zstream.next_in = Z_NULL;

    int rc = inflateInit2(&_zstream, 16 + MAX_WBITS);

    if (rc == Z_VERSION_ERROR)
        throw Error("zlib version incompatible");
    if (rc == Z_MEM_ERROR)
        throw Error("not enough memory for zlib");
    if (rc != Z_OK)
        throw Error("unknown zlib error code: %d", rc);

    _outbuf.clear_resize(CHUNK_SIZE);
    _inbuf.clear_resize(CHUNK_SIZE);
    _outbuf_start = 0;
    _inbuf_end = 0;
    _uncompressed_total = 0;
    _eof = false;

    _zstream.avail_out = _outbuf.size();
    _zstream.next_out = _outbuf.ptr();
}

GZipScanner::~GZipScanner()
{
    inflateEnd(&_zstream);
}

void GZipScanner::read(int length, void* res)
{
    if (res == 0)
        throw Error("zero pointer given");

    if (_outbuf_start < _outbuf.size() - (int)_zstream.avail_out)
    {
        int n = std::min(length, CHUNK_SIZE - (int)_zstream.avail_out - _outbuf_start);

        memcpy(res, _outbuf.ptr() + _outbuf_start, n);
        _outbuf_start += n;
        _uncompressed_total += n;
        length -= n;
        res = (char*)res + n;
    }

    if (!_read(length, res))
        throw Error("end of compressed data");
}

bool GZipScanner::_read(int length, void* res)
{
    while (length > 0)
    {
        if (_eof)
            return false;

        if (_zstream.avail_in == 0)
        {
            _inbuf_end = 0;

            if (_source.isEOF())
                throw Error("end of file in source stream");

            do
            {
                _inbuf[_inbuf_end++] = _source.readChar();
            } while (!_source.isEOF() && _inbuf_end < _inbuf.size());
            _zstream.avail_in = _inbuf_end;
            _zstream.next_in = _inbuf.ptr();
        }

        _zstream.avail_out = _outbuf.size();
        _zstream.next_out = _outbuf.ptr();
        _outbuf_start = 0;

        int rc = inflate(&_zstream, Z_NO_FLUSH);

        if (rc == Z_STREAM_ERROR)
            throw Error("inconsistent stream structure");

        if (rc == Z_NEED_DICT)
        {
            inflateEnd(&_zstream);
            throw Error("need a dictionary");
        }

        if (rc == Z_MEM_ERROR)
        {
            inflateEnd(&_zstream);
            throw Error("not enough memory");
        }

        if (rc == Z_DATA_ERROR)
        {
            inflateEnd(&_zstream);
            throw Error("corrupted input data");
        }

        if (rc == Z_BUF_ERROR)
            throw Error("Z_BUF_ERROR (workaround not implemented)");

        if (rc != Z_OK && rc != Z_STREAM_END)
            throw Error("unknown zlib error code: %d", rc);

        int n = std::min(length, CHUNK_SIZE - (int)_zstream.avail_out - _outbuf_start);

        if (n > 0 && res != 0)
        {
            memcpy(res, _outbuf.ptr(), n);
            _outbuf_start += n;
            _uncompressed_total += n;
            res = (char*)res + n;
        }

        length -= n;

        if (rc == Z_STREAM_END)
            _eof = true;
    }

    return true;
}

void GZipScanner::readAll(Array<char>& arr)
{
    arr.clear();

    while (!isEOF())
        arr.push(readByte());
}

void GZipScanner::skip(int length)
{
    QS_DEF(Array<char>, dummy);

    dummy.clear_resize(length);
    read(length, dummy.ptr());
}

long long GZipScanner::tell()
{
    return _uncompressed_total;
}

bool GZipScanner::isEOF()
{
    if ((unsigned)_outbuf_start + _zstream.avail_out == (unsigned)_outbuf.size())
    {
        if (_eof)
            return true;
        _read(1, 0);
    }
    return _eof && (unsigned)_outbuf_start + _zstream.avail_out == (unsigned)_outbuf.size();
}

void GZipScanner::seek(long long pos, int from)
{
    throw Error("not implemented");
}

int GZipScanner::lookNext()
{
    if (_outbuf_start < _outbuf.size() - (int)_zstream.avail_out)
        return _outbuf[_outbuf_start];

    if (_eof)
        return -1;

    _read(1, 0);

    if (_eof)
        return -1;

    if (_outbuf_start < _outbuf.size() - (int)_zstream.avail_out)
        return _outbuf[_outbuf_start];

    throw Error("internal");
}

long long GZipScanner::length()
{
    throw Error("not implemented");
}
