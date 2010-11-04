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

#ifndef __ringo_fetch_context__
#define __ringo_fetch_context__

#include "core/ringo_matchers.h"
#include "base_cpp/auto_ptr.h"
#include "oracle/ringo_fast_index.h"
#include "oracle/ringo_shadow_fetch.h"
#include "oracle/ringo_oracle.h"

class RingoShadowFetch;

class RingoFetchContext
{
public:
   RingoFetchContext (int id, RingoOracleContext &context, const Array<char> &query_id);

   AutoPtr<RingoFastIndex>  fast_index;
   AutoPtr<RingoShadowFetch> shadow_fetch;

   BingoFetchEngine *fetch_engine;

   RingoSubstructure substructure;

   int         id;
   int         context_id;
   bool        fresh; // 'true' after selectivity calculation and before index start

   static RingoFetchContext & create (RingoOracleContext &context, const Array<char> &query_id);
   static RingoFetchContext & get (int id);
   static RingoFetchContext * findFresh (int context_id, const Array<char> &query_id);

   static void remove (int id);

   inline RingoOracleContext & context () {return _context;}

   DEF_ERROR("ringo fetch context");

protected:
   Array<char> _query_id;
   RingoOracleContext & _context;

   TL_DECL(PtrArray<RingoFetchContext>, _instances);
};


#endif
