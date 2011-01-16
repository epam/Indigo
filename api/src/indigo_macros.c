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

#include "indigo.h"

CEXPORT int indigoLoadMoleculeFromString (const char *string)
{
   int source = indigoReadString(string);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadMolecule(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadMoleculeFromFile (const char *filename)
{
   int source = indigoReadFile(filename);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadMolecule(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadMoleculeFromBuffer (const char *buf, int size)
{
   int source = indigoReadBuffer(buf, size);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadMolecule(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadQueryMoleculeFromString (const char *string)
{
   int source = indigoReadString(string);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadQueryMolecule(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadQueryMoleculeFromFile (const char *filename)
{
   int source = indigoReadFile(filename);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadQueryMolecule(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadQueryMoleculeFromBuffer (const char *buf, int size)
{
   int source = indigoReadBuffer(buf, size);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadQueryMolecule(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadSmartsFromString (const char *string)
{
   int source = indigoReadString(string);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadSmarts(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadSmartsFromFile (const char *filename)
{
   int source = indigoReadFile(filename);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadSmarts(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadSmartsFromBuffer (const char *buf, int size)
{
   int source = indigoReadBuffer(buf, size);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadSmarts(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadReactionFromString (const char *string)
{
   int source = indigoReadString(string);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadReaction(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadReactionFromFile (const char *filename)
{
   int source = indigoReadFile(filename);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadReaction(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadReactionFromBuffer (const char *buf, int size)
{
   int source = indigoReadBuffer(buf, size);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadReaction(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadQueryReactionFromString (const char *string)
{
   int source = indigoReadString(string);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadQueryReaction(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadQueryReactionFromFile (const char *filename)
{
   int source = indigoReadFile(filename);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadQueryReaction(source);
   indigoFree(source);
   return result;
}

CEXPORT int indigoLoadQueryReactionFromBuffer (const char *buf, int size)
{
   int source = indigoReadBuffer(buf, size);
   int result;

   if (source <= 0)
      return -1;

   result = indigoLoadQueryReaction(source);
   indigoFree(source);
   return result;
}


CEXPORT int indigoSaveMolfileToFile (int molecule, const char *filename)
{
   int f = indigoWriteFile(filename);
   int res;

   if (f == -1)
      return -1;

   res = indigoSaveMolfile(molecule, f);

   indigoFree(f);
   return res;
}

CEXPORT int indigoSaveCmlToFile (int molecule, const char *filename)
{
   int f = indigoWriteFile(filename);
   int res;

   if (f == -1)
      return -1;

   res = indigoSaveCml(molecule, f);

   indigoFree(f);
   return res;
}

CEXPORT const char * indigoMolfile (int molecule)
{
   int b = indigoWriteBuffer();
   const char *res;

   if (b == -1)
      return 0;

   if (indigoSaveMolfile(molecule, b) == -1)
      return 0;

   res = indigoToString(b);
   indigoFree(b);
   return res;
}

CEXPORT const char * indigoCml (int molecule)
{
   int b = indigoWriteBuffer();
   const char *res;

   if (b == -1)
      return 0;

   if (indigoSaveCml(molecule, b) == -1)
      return 0;

   res = indigoToString(b);
   indigoFree(b);
   return res;
}

CEXPORT int indigoSaveRxnfileToFile (int reaction, const char *filename)
{
   int f = indigoWriteFile(filename);
   int res;

   if (f == -1)
      return -1;

   res = indigoSaveRxnfile(reaction, f);

   indigoFree(f);
   return res;
}

CEXPORT const char * indigoRxnfile (int molecule)
{
   int b = indigoWriteBuffer();
   const char *res;

   if (b == -1)
      return 0;

   if (indigoSaveRxnfile(molecule, b) == -1)
      return 0;

   res = indigoToString(b);
   indigoFree(b);
   return res;
}
