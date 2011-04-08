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

#define WRAPPER_LOAD_FROM_STRING(name)            \
CEXPORT int name##FromString (const char *string) \
{                                                 \
   int source = indigoReadString(string);         \
   int result;                                    \
                                                  \
   if (source <= 0)                               \
      return -1;                                  \
                                                  \
   result = name(source);                         \
   indigoFree(source);                            \
   return result;                                 \
}

#define WRAPPER_LOAD_FROM_FILE(name)              \
CEXPORT int name##FromFile(const char *filename)  \
{                                                 \
   int source = indigoReadFile(filename);         \
   int result;                                    \
                                                  \
   if (source <= 0)                               \
      return -1;                                  \
                                                  \
   result = name(source);                         \
   indigoFree(source);                            \
   return result;                                 \
}

#define WRAPPER_LOAD_FROM_BUFFER(name)            \
CEXPORT int name##FromBuffer(const char *buf, int size) \
{                                                 \
   int source = indigoReadBuffer(buf, size);      \
   int result;                                    \
                                                  \
   if (source <= 0)                               \
      return -1;                                  \
                                                  \
   result = name(source);                         \
   indigoFree(source);                            \
   return result;                                 \
}

WRAPPER_LOAD_FROM_STRING(indigoLoadMolecule)
WRAPPER_LOAD_FROM_FILE(indigoLoadMolecule)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadMolecule)

WRAPPER_LOAD_FROM_STRING(indigoLoadQueryMolecule)
WRAPPER_LOAD_FROM_FILE(indigoLoadQueryMolecule)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadQueryMolecule)

WRAPPER_LOAD_FROM_STRING(indigoLoadSmarts)
WRAPPER_LOAD_FROM_FILE(indigoLoadSmarts)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadSmarts)

WRAPPER_LOAD_FROM_STRING(indigoLoadReaction)
WRAPPER_LOAD_FROM_FILE(indigoLoadReaction)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadReaction)

WRAPPER_LOAD_FROM_STRING(indigoLoadQueryReaction)
WRAPPER_LOAD_FROM_FILE(indigoLoadQueryReaction)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadQueryReaction)

WRAPPER_LOAD_FROM_STRING(indigoLoadReactionSmarts)
WRAPPER_LOAD_FROM_FILE(indigoLoadReactionSmarts)
WRAPPER_LOAD_FROM_BUFFER(indigoLoadReactionSmarts)

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
