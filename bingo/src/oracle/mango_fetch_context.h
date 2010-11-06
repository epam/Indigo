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

#ifndef __mango_fetch_context__
#define __mango_fetch_context__

#include "core/mango_matchers.h"
#include "base_cpp/auto_ptr.h"
#include "oracle/mango_fast_index.h"
#include "oracle/mango_shadow_fetch.h"
#include "oracle/mango_oracle.h"

using namespace indigo;

class MangoShadowFetch;

class MangoFetchContext
{
public:
   MangoFetchContext (int id, MangoOracleContext &context, const Array<char> &query_id);

   AutoPtr<MangoFastIndex>   fast_index;
   AutoPtr<MangoShadowFetch> shadow_fetch;

   BingoFetchEngine *fetch_engine;

   MangoSubstructure substructure;
   MangoSimilarity   similarity;
   MangoExact        exact;
   MangoTautomer     tautomer;
   MangoGross        gross;
   MangoMass         mass;

   int         id;
   int         context_id;
   bool        fresh; // 'true' after selectivity calculation and before index start

   static MangoFetchContext & create (MangoOracleContext &context, const Array<char> &query_id);
   static MangoFetchContext & get (int id);
   static MangoFetchContext * findFresh (int context_id, const Array<char> &query_id);
   static void remove (int id);

   inline MangoOracleContext & context () {return _context;}

   DEF_ERROR("mango fetch context");

protected:
   Array<char> _query_id;
   MangoOracleContext & _context;

   TL_DECL(PtrArray<MangoFetchContext>, _instances);
};


#endif
