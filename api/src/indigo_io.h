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

#ifndef __indigo_io__
#define __indigo_io__

#include "indigo_internal.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

class DLLEXPORT IndigoScanner : public IndigoObject
{
public:
   IndigoScanner (Scanner *scanner);
   IndigoScanner (const char *str);
   IndigoScanner (const char *buf, int size);

   static Scanner & get (IndigoObject &obj);

   virtual ~IndigoScanner ();

   Scanner *ptr;
protected:
   Array<char> _buf;
};

class DLLEXPORT IndigoOutput : public IndigoObject
{
public:
   IndigoOutput ();
   IndigoOutput (Output *output);
   virtual ~IndigoOutput ();

   virtual void toString (Array<char> &str);

   static Output & get (IndigoObject &obj);

   Output *ptr;
protected:
   bool        _own_buf;
   Array<char> _buf;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
