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

#ifndef __mango_context__
#define __mango_context__

#include "core/mango_matchers.h"
#include "core/mango_index.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/ptr.h"

class MangoContext
{
public:

   explicit MangoContext (BingoContext &context);
   virtual ~MangoContext ();

   MangoSubstructure substructure;
   MangoSimilarity   similarity;
   MangoExact        exact;
   MangoTautomer     tautomer;
   MangoGross        gross;

   static int begin ();
   static int end ();
   static int next (int k);

   DEF_ERROR("mango context");

   static void remove (int id);

   static MangoContext * get (int id);
   static MangoContext * existing (int id);

protected:
   static MangoContext * _get (int id, BingoContext &context);

   TL_DECL(PtrArray<MangoContext>, _instances);

   BingoContext &_context;
};

#endif
