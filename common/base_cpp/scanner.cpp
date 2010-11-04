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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "base_c/defs.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"

Scanner::~Scanner ()
{
}

int Scanner::readIntFix (int digits)
{
   int result;

   QS_DEF(Array<char>, buf);

   buf.clear_resize(digits + 1);
   read(digits, buf.ptr());
   buf[digits] = 0;

   if (sscanf(buf.ptr(), "%d", &result) != 1)
      throw Error("readIntFix()");

   return result;
}

int Scanner::readInt1 (void)
{
   QS_DEF(Array<char>, buf);
   char c;
   int result;

   buf.clear();

   while (!isEOF())
   {
      c = readChar();
      if (!isdigit(c) && c != '-' && c != '+')
         break;
      buf.push(c);
   }

   buf.push(0);
   
   if (sscanf(buf.ptr(), "%d", &result) < 1)
      throw Error("readInt(): error parsing %s", buf.ptr());

   return result;
}

int Scanner::readInt (void)
{
   QS_DEF(Array<char>, buf);
   char c;
   int result;

   buf.clear();

   c = readChar();

   if (c == '+' || c == '-' || isdigit(c))
      buf.push(c);

   while (isdigit(lookNext()))
      buf.push(readChar());

   buf.push(0);

   if (sscanf(buf.ptr(), "%d", &result) < 1)
      throw Error("readInt(): error parsing %s", buf.ptr());

   return result;
}

int Scanner::readUnsigned ()
{
   int result = 0;
   bool was_digit = false;

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
      throw Error("readUnsigned(): no digits");

   return result;
}

float Scanner::readFloat (void)
{
   QS_DEF(Array<char>, buf);
   float result;

   buf.clear();

   while (!isEOF())
   {
      char c = readChar();

      if (!isdigit(c) && c != '-' && c != '+' && c != '.')
         break;
      buf.push(c);
   }

   buf.push(0);
   
   if (sscanf(buf.ptr(), "%f", &result) < 1)
      throw Error("readFloat(): error parsing %s", buf.ptr());

   return result;
}

bool Scanner::tryReadFloat (float &value)
{
   QS_DEF(Array<char>, buf);

   int pos = tell();

   buf.clear();

   while (!isEOF())
   {
      char c = readChar();

      if (!isdigit(c) && c != '-' && c != '+' && c != '.')
         break;
      buf.push(c);
   }

   buf.push(0);
   
   if (sscanf(buf.ptr(), "%f", &value) < 1)
   {
      seek(pos, SEEK_SET);
      return false;
   }

   return true;
}

void Scanner::readWord (Array<char> &word, const char *delimiters)
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
   } while (!isEOF());

   word.push(0);
}


float Scanner::readFloatFix (int digits)
{
   QS_DEF(Array<char>, buf);
   float result;

   buf.clear_resize(digits + 1);
   read(digits, buf.ptr());
   buf[digits] = 0;

   if (sscanf(buf.ptr(), "%f", &result) != 1)
      throw Error("readFloatFix()");

   return result;
}

char Scanner::readChar ()
{
   char c;

   read(sizeof(char), &c);
   return c;
}

byte Scanner::readByte ()
{
   byte c;

   read(1, &c);
   return c;
}

bool Scanner::skipString ()
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

void Scanner::skipSpace ()
{
   while (isspace(lookNext()))
      skip(1);
}

void Scanner::readString (Array<char> &out, bool append_zero)
{
   char c;

   out.clear();

   if (isEOF())
      throw Error("readString(): end of stream");

   do
   {  
      c = readChar();

      if (c == '\r')
      {
         if (lookNext() == '\n')
            continue;
         break;
      }
      if (c == '\n')
         break;

      out.push(c);
   } while (!isEOF());

   if (append_zero)
      out.push(0);
}

void Scanner::readCharsFix (int n, char *chars_out)
{
   read(n, chars_out);
}

word Scanner::readBinaryWord ()
{
   word res;

   read(sizeof(word), &res);

   return res;
}

dword Scanner::readBinaryDword ()
{
   dword res;

   read(sizeof(dword), &res);

   return res;
}

int Scanner::readBinaryInt ()
{
   int res;

   read(sizeof(int), &res);

   return res;
   //*res = ntohl(*res);
}

float Scanner::readBinaryFloat ()
{
   float res;

   read(sizeof(float), &res);

   return res;
}

short Scanner::readPackedShort ()
{
   byte high = readByte();

   if (high < 128)
      return high;

   byte low = readByte();

   high -= 128;

   return high * (short)256 + low;
}

void Scanner::readAll (Array<char> &arr)
{
   arr.clear_resize(length());

   read(arr.size(), arr.ptr());
}

bool Scanner::isSingleLine (Scanner &scanner)
{
   int pos = scanner.tell();

   scanner.skipString();

   bool res = scanner.isEOF();

   scanner.seek(pos, SEEK_SET);
   return res;
}


FileScanner::FileScanner (const char *format, ...) : Scanner ()
{
   char filename[1024];

   va_list args;

   va_start(args, format);
   vsnprintf(filename, sizeof(filename), format, args);
   va_end(args);

   _file_len = 0;
   _file = fopen(filename, "rb");

   if (_file == NULL)
      throw Error("can't open file %s", filename);

   fseek(_file, 0, SEEK_END);
   _file_len = ftell(_file);
   fseek(_file, 0, SEEK_SET);
}

int FileScanner::lookNext ()
{
   char res;
   size_t nread = fread(&res, 1, 1, _file);

   if (nread == 0)
      return -1;

   fseek(_file, -1, SEEK_CUR);

   return res;
}

int FileScanner::tell ()
{
   return ftell(_file);
}

void FileScanner::read (int length, void *res)
{
   size_t nread = fread(res, 1, length, _file);

   if (nread != (size_t)length)
      throw Error("FileScanner::read() error");
}

void BufferScanner::skip (int n)
{
   _offset += n;

   if (_size >= 0 && _offset > _size)
      throw Error("skip() passes after end of buffer");
}

void BufferScanner::seek (int pos, int from)
{
   if (from == SEEK_SET)
      _offset = pos;
   else if (from == SEEK_CUR)
      _offset += pos;
   else // SEEK_END
   {
      if (_size < 0)
         throw Error("can not seek from end: buffer is unlimited");
      _offset = _size - pos;
   }

   if ((_size >= 0 && _offset > _size) || _offset < 0)
      throw Error("size = %d, offset = %d after seek()", _size, _offset);
}

byte BufferScanner::readByte ()
{
   if (_size >= 0 && _offset >= _size)
      throw Error("readByte(): end of buffer");

   return _buffer[_offset++];
}

bool FileScanner::isEOF ()
{
   if (_file == NULL)
      return true;

   return ftell(_file) == _file_len;
}

void FileScanner::skip (int n)
{
   int res = fseek(_file, n, SEEK_CUR);

   if (res != 0)
      throw Error("skip() passes after end of file");
}

void FileScanner::seek (int pos, int from)
{
   fseek(_file, pos, from);
}

int FileScanner::length ()
{
   return _file_len;
}

FileScanner::~FileScanner ()
{
   if (_file != NULL)
      fclose(_file);
}

void BufferScanner::_init (const char *buffer, int size)
{
   if (size < -1 || (size > 0 && buffer == 0))
      throw Error("incorrect parameters in BufferScanner constructor");

   _buffer = buffer;
   _size = size;
   _offset = 0;
}

BufferScanner::BufferScanner (const char *buffer, int buffer_size)
{
   _init(buffer, buffer_size);
}

BufferScanner::BufferScanner (const byte *buffer, int buffer_size)
{
   _init((const char *)buffer, buffer_size);
}

BufferScanner::BufferScanner (const char *str)
{
   _init(str, (int)strlen(str));
}

BufferScanner::BufferScanner (const Array<char> &arr)
{
   _init(arr.ptr(), arr.size());
}

bool BufferScanner::isEOF ()
{
   if (_size < 0)
      throw Error("isEOF() called to unlimited buffer");
   return _offset >= _size;
}

void BufferScanner::read (int length, void *res)
{
   if (_size >= 0 && _offset + length > _size)
      throw Error("BufferScanner::read() error");

   memcpy(res, &_buffer[_offset], length);
   _offset += length;
}


int BufferScanner::lookNext ()
{
   if (_size >= 0 && _offset >= _size)
      return -1;

   return _buffer[_offset];
}

int BufferScanner::length ()
{
   return _size;
}

int BufferScanner::tell ()
{
   return _offset;
}

const void * BufferScanner::curptr ()
{
   return _buffer + _offset;
}
