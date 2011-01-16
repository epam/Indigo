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

#ifndef __indigo_fingerprints__
#define __indigo_fingerprints__

#include "indigo_internal.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

class DLLEXPORT IndigoFingerprint : public IndigoObject
{
public:
   IndigoFingerprint ();
   virtual ~IndigoFingerprint ();

   virtual void toString (Array<char> &str);
   virtual void toBuffer (Array<char> &buf);

   static IndigoFingerprint & cast (IndigoObject &obj);
   
   Array<byte> bytes;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
