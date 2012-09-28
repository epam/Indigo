/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "gzip/gzip_output.h"

using namespace indigo;

IMPL_ERROR(GZipOutput, "GZip output");

GZipOutput::GZipOutput (Output &dest, int level) :
_dest(dest),
TL_CP_GET(_outbuf),
TL_CP_GET(_inbuf)
{
   _zstream.zalloc = Z_NULL;
   _zstream.zfree = Z_NULL;
   _zstream.opaque = Z_NULL;
   _zstream.next_in = Z_NULL;
   _zstream.avail_in = 0;

   int rc = deflateInit2(&_zstream, level, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

   if (rc == Z_VERSION_ERROR)
      throw Error("zlib version incompatible");
   if (rc == Z_MEM_ERROR)
      throw Error("not enough memory for zlib");
   if (rc == Z_STREAM_ERROR)
      throw Error("invalid parameter given to zlib");
   if (rc != Z_OK)
      throw Error("unknown zlib error code: %d", rc);

   _outbuf.clear_resize(CHUNK_SIZE);
   _inbuf.clear_resize(CHUNK_SIZE);
   _total_written = 0;
}

GZipOutput::~GZipOutput ()
{
   _zstream.avail_in = 0;
   _zstream.next_in = Z_NULL;
   
   int rc = _deflate(Z_FINISH);

   if (rc != Z_STREAM_END)
      return;//throw Error("unexpected zlib error (%d)", rc);
   
   deflateEnd(&_zstream);
}

void GZipOutput::write (const void *data, int size)
{
   if (size < 1)
      return;

   _zstream.avail_in = size;
   _zstream.next_in = (Bytef *)data;

   do
   {
      _deflate(Z_NO_FLUSH);
   } while (_zstream.avail_out == 0);

   if (_zstream.avail_in != 0)
      throw Error("some data left uncompressed unexpectedly");
}

void GZipOutput::flush ()
{
   _zstream.avail_in = 0;
   _zstream.next_in = Z_NULL;
   _deflate(Z_FULL_FLUSH);
   _dest.flush();
}

int GZipOutput::_deflate (int flush)
{
   _zstream.avail_out = _outbuf.size();
   _zstream.next_out = _outbuf.ptr();

   int rc = deflate(&_zstream, flush);

   if (rc == Z_STREAM_ERROR)
      throw Error("inconsistent zlib stream state");

   if (rc == Z_BUF_ERROR)
      throw Error("Z_BUF_ERROR (workaround not implemented)");

   if (rc != Z_STREAM_END && rc != Z_OK)
      throw Error("unexpected zlib error (%d)", rc);

   int n = _outbuf.size() - _zstream.avail_out;

   if (n > 0)
   {
      _dest.write(_outbuf.ptr(), n);
      _total_written += n;
   }

   return rc;
}

int GZipOutput::tell ()
{
   return _total_written;
}

void GZipOutput::seek (int offset, int from)
{
   throw Error("not imlemented");
}
