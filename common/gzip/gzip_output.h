/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __gzip_output__
#define __gzip_output__

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

#include <zlib.h>

namespace indigo {

class GZipOutput : public Output
{
public:
   enum { CHUNK_SIZE = 32768 };

   explicit GZipOutput (Output &dest, int level);
   virtual ~GZipOutput ();

   virtual void write (const void *data, int size);
   virtual void seek  (off_t_type offset, int from);
   virtual off_t_type tell  ();
   virtual void flush ();

   DECL_ERROR;
   
protected:
   Output  &_dest;
   z_stream _zstream;
   int _total_written;

   int _deflate (int flush);

   CP_DECL;
   TL_CP_DECL(Array<Bytef>, _outbuf);
   TL_CP_DECL(Array<Bytef>, _inbuf);
};

}

#endif
