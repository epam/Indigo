/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __output_h__
#define __output_h__

#include <stdio.h>

#include "base_cpp/array.h"
#include "base_cpp/io_base.h"

namespace indigo
{

class Output
{
public:
   DEF_ERROR("output");

   explicit Output ();
   virtual ~Output ();

   virtual void write (const void *data, int size) = 0;
   virtual void seek  (int offset, int from) = 0;
   virtual int  tell  () = 0;
   virtual void flush () = 0;

   DLLEXPORT void writeByte        (byte value);
   DLLEXPORT void writeChar        (char value);
   DLLEXPORT void writeBinaryInt   (int   value);
   DLLEXPORT void writeBinaryDword (dword value);
   DLLEXPORT void writeBinaryWord  (word value);
   DLLEXPORT void writeBinaryFloat (float value);
   DLLEXPORT void writePackedShort (short value);
   DLLEXPORT void writeString      (const char *string);
   DLLEXPORT void writeStringCR    (const char *string);
   DLLEXPORT void writeCR          ();
   DLLEXPORT void writeArray       (const Array<char> &data);
   
   DLLEXPORT void skip             (int count);

   DLLEXPORT void printf   (const char *format, ...);
   DLLEXPORT void vprintf  (const char *format, va_list args);
   DLLEXPORT void printfCR (const char *format, ...);
};

class FileOutput : public Output
{
public:
   DLLEXPORT FileOutput (Encoding filename_encoding, const char *filename);
   DLLEXPORT explicit FileOutput (const char *name);
   //explicit FileOutput (const char *format, ...);
   DLLEXPORT explicit FileOutput (bool append, const char *format, ...);
   DLLEXPORT virtual ~FileOutput ();

   DLLEXPORT virtual void write (const void *data, int size);
   DLLEXPORT virtual void seek  (int offset, int from);
   DLLEXPORT virtual int  tell  ();
   DLLEXPORT virtual void flush ();
   
protected:
   FILE *_file;
};

class ArrayOutput : public Output
{
public:
   DLLEXPORT explicit ArrayOutput (Array<char> &arr);
   DLLEXPORT virtual ~ArrayOutput ();

   DLLEXPORT virtual void write (const void *data, int size);
   DLLEXPORT virtual void seek  (int offset, int from);
   DLLEXPORT virtual int  tell  ();
   DLLEXPORT virtual void flush ();

   DLLEXPORT void clear ();

protected:
   Array<char> &_arr;
};

class StandardOutput : public Output
{
public:
   explicit StandardOutput ();
   virtual ~StandardOutput ();

   DLLEXPORT virtual void write (const void *data, int size);
   DLLEXPORT virtual void seek  (int offset, int from);
   DLLEXPORT virtual int  tell  ();
   DLLEXPORT virtual void flush ();
protected:
   int _count;
};

DLLEXPORT void bprintf (Array<char>& buf, const char *format, ...);

}
 
#endif
