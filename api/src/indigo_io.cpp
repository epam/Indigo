/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

#include "indigo_internal.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "base_cpp/auto_ptr.h"
#include "molecule/gross_formula.h"

IndigoScanner::IndigoScanner (Scanner *scanner) : IndigoObject(SCANNER), ptr(scanner)
{
   _dbg_info.appendString("<scanner>", true);
}

IndigoScanner::IndigoScanner (const char *str) : IndigoObject(SCANNER)
{
   _buf.readString(str, false);
   ptr = new BufferScanner(_buf);
   _dbg_info.appendString("<scanner with stored string>", true);
}

IndigoScanner::IndigoScanner (const char *buf, int size) : IndigoObject(SCANNER)
{
   _buf.copy(buf, size);
   ptr = new BufferScanner(_buf);
   _dbg_info.appendString("<scanner with stored buffer>", true);
}

IndigoScanner::~IndigoScanner ()
{
   delete ptr;
}

IndigoOutput::IndigoOutput (Output *output) : IndigoObject(OUTPUT), ptr(output)
{
   _dbg_info.appendString("<output>", true);
   _own_buf = false;
}

IndigoOutput::IndigoOutput () : IndigoObject(OUTPUT)
{
   ptr = new ArrayOutput(_buf);
   _dbg_info.appendString("<output with stored buffer>", true);
   _own_buf = true;
}

void IndigoOutput::toString (Array<char> &str)
{
   if (_own_buf)
      str.copy(_buf);
   else
      throw IndigoError("can not convert %s to string", debugInfo());
}

Output & IndigoOutput::getOutput ()
{
   return *ptr;
}

IndigoOutput::~IndigoOutput ()
{
   delete ptr;
}

CEXPORT int indigoReadFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(new FileScanner(self.filename_encoding, filename)));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoReadString (const char *str)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(new BufferScanner(str)));
   }
   INDIGO_END(0, -1);
}

CEXPORT int indigoReadBuffer (const char *buffer, int size)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(new BufferScanner(buffer, size)));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoLoadString (const char *str)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(str));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoLoadBuffer (const char *buffer, int size)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoScanner(buffer, size));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoWriteFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoOutput(new FileOutput(filename)));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoWriteBuffer (void)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoOutput());
   }
   INDIGO_END(0, -1)
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
   INDIGO_END(0, 0);
}

CEXPORT int indigoToBuffer (int handle, char **buf, int *size)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      obj.toBuffer(self.tmp_string);

     *buf = self.tmp_string.ptr();
     *size = self.tmp_string.size();
   }
   INDIGO_END(1, 0);
}

CEXPORT int indigoIterateSDF (int reader)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(reader);
      
      return self.addObject(new IndigoSdfLoader(obj.getScanner()));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoIterateRDF (int reader)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(reader);

      return self.addObject(new IndigoRdfLoader(obj.getScanner()));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoIterateSmiles (int reader)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(reader);
      
      return self.addObject(new IndigoMultilineSmilesLoader(obj.getScanner()));
   }
   INDIGO_END(0, -1)
}


CEXPORT int indigoTell (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.type == IndigoObject::SDF_LOADER)
         return ((IndigoSdfLoader &)obj).tell();
      if (obj.type == IndigoObject::RDF_LOADER)
         return ((IndigoRdfLoader &)obj).tell();
      if (obj.type == IndigoObject::MULTILINE_SMILES_LOADER)
         return ((IndigoMultilineSmilesLoader &)obj).tell();
      if (obj.type == IndigoObject::RDF_MOLECULE ||
          obj.type == IndigoObject::RDF_REACTION ||
          obj.type == IndigoObject::SMILES_MOLECULE ||
          obj.type == IndigoObject::SMILES_REACTION)
         return ((IndigoRdfData &)obj).tell();

      throw IndigoError("indigoTell(): not applicable to %s", obj.debugInfo());
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoIterateSDFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoSdfLoader(filename));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoIterateRDFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoRdfLoader(filename));
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoIterateSmilesFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoMultilineSmilesLoader(filename));
   }
   INDIGO_END(0, -1)
}
