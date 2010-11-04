#ifndef __gzip_output__
#define __gzip_output__

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

#include <zlib.h>

class GZipOutput : public Output
{
public:
   enum { CHUNK_SIZE = 32768 };

   explicit GZipOutput (Output &dest, int level);
   virtual ~GZipOutput ();

   virtual void write (const void *data, int size);
   virtual void seek  (int offset, int from);
   virtual int  tell  ();
   virtual void flush ();

   DEF_ERROR("GZip output");
   
protected:
   Output  &_dest;
   z_stream _zstream;
   int _total_written;

   int _deflate (int flush);

   TL_CP_DECL(Array<Bytef>, _outbuf);
   TL_CP_DECL(Array<Bytef>, _inbuf);
};

#endif
