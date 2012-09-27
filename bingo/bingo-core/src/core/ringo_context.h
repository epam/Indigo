/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#ifndef __ringo_context__
#define __ringo_context__

#include "core/ringo_matchers.h"
#include "core/ringo_index.h"
#include "base_cpp/tlscont.h"

using namespace indigo;

namespace ingido
{
   class BingoContext;
}

class RingoContext
{
public:
   explicit RingoContext (BingoContext &context);
   virtual ~RingoContext ();

   RingoSubstructure substructure;
   RingoExact exact;
   RingoAAM ringoAAM;

   DECL_ERROR;

   static int begin ();
   static int end ();
   static int next (int k);

   static void remove (int id);

   static RingoContext * existing (int id);
   static RingoContext * get (int id);

protected:
   static RingoContext * _get (int id, BingoContext &context);

   TL_DECL(PtrArray<RingoContext>, _instances);
   static OsLock _instances_lock;

   BingoContext &_context;
};

#endif
