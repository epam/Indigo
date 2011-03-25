/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#include "indigo_io.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "base_cpp/auto_ptr.h"
#include "molecule/gross_formula.h"
#include "indigo_savers.h"

IndigoScanner::IndigoScanner (Scanner *scanner) : IndigoObject(SCANNER), ptr(scanner)
{
}

IndigoScanner::IndigoScanner (const char *str) : IndigoObject(SCANNER)
{
   _buf.readString(str, false);
   ptr = new BufferScanner(_buf);
}

IndigoScanner::IndigoScanner (const char *buf, int size) : IndigoObject(SCANNER)
{
   _buf.copy(buf, size);
   ptr = new BufferScanner(_buf);
}

IndigoScanner::~IndigoScanner ()
{
   if (ptr != 0) // can be zero after indigoClose()
      delete ptr;
}

Scanner & IndigoScanner::get (IndigoObject &obj)
{
   if (obj.type == SCANNER)
      return *((IndigoScanner &)obj).ptr;
   throw IndigoError("%s is not a scanner", obj.debugInfo());
}

IndigoOutput::IndigoOutput (Output *output) : IndigoObject(OUTPUT), ptr(output)
{
   _own_buf = false;
}

IndigoOutput::IndigoOutput () : IndigoObject(OUTPUT)
{
   ptr = new ArrayOutput(_buf);
   _own_buf = true;
}

void IndigoOutput::toString (Array<char> &str)
{
   if (_own_buf)
      str.copy(_buf);
   else
      throw IndigoError("can not convert %s to string", debugInfo());
}

IndigoOutput::~IndigoOutput ()
{
   delete ptr;
}

Output & IndigoOutput::get (IndigoObject &obj)
{
   if (obj.type == OUTPUT)
   {
      Output *ptr = ((IndigoOutput &)obj).ptr;

      if (ptr == 0)
         throw IndigoError("output stream has been closed");
      return *ptr;
   }
   throw IndigoError("%s is not an output", obj.debugInfo());
}

CEXPORT int indigoReadFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(new FileScanner(self.filename_encoding, filename)));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoReadString (const char *str)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(new BufferScanner(str)));
   }
   INDIGO_END(-1);
}

CEXPORT int indigoReadBuffer (const char *buffer, int size)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(new BufferScanner(buffer, size)));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoLoadString (const char *str)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(str));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoLoadBuffer (const char *buffer, int size)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(buffer, size));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoWriteFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoOutput(new FileOutput(filename)));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoClose (int output)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(output);
      if (obj.type == IndigoObject::OUTPUT)
      {
         IndigoOutput &out = ((IndigoOutput &)obj);
         delete out.ptr;
         out.ptr = 0;
         return 1;
      }
      else if (obj.type == IndigoObject::SAVER)
      {
         IndigoSaver &saver = ((IndigoSaver &)obj);
         saver.close();
         return 1;
      }
      else
         throw IndigoError("indigoClose(): does not accept %s", obj.debugInfo());

   }
   INDIGO_END(-1)
}

CEXPORT int indigoWriteBuffer (void)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoOutput());
   }
   INDIGO_END(-1)
}

CEXPORT const char * indigoToString (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      obj.toString(self.tmp_string);
      self.tmp_string.push(0);

      return self.tmp_string.ptr();
   }
   INDIGO_END(0);
}

CEXPORT int indigoToBuffer (int handle, char **buf, int *size)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      obj.toBuffer(self.tmp_string);

     *buf = self.tmp_string.ptr();
     *size = self.tmp_string.size();
     return 1;
   }
   INDIGO_END(-1);
}
