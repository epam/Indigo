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

#include "oracle/ringo_fetch_context.h"
#include "core/ringo_matchers.h"
#include "core/bingo_context.h"
#include "oracle/bingo_oracle_context.h"

TL_DEF(RingoFetchContext, PtrArray<RingoFetchContext>, _instances);

RingoFetchContext::RingoFetchContext (int id_, RingoOracleContext &context,
                                      const Array<char> &query_id) :
substructure(context.context()),
_context(context)
{
   id = id_;
   context_id = context.context().id;
   _query_id.copy(query_id);
   fresh = false;
   fetch_engine = 0;

   shadow_fetch.reset(new RingoShadowFetch(*this));
   fast_index.reset(new RingoFastIndex(*this));
}

RingoFetchContext & RingoFetchContext::get (int id)
{
   TL_GET(PtrArray<RingoFetchContext>, _instances);

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->id == id)
         return *_instances[i];

   throw Error("context #%d not found", id);
}

RingoFetchContext & RingoFetchContext::create (RingoOracleContext &context,
                                               const Array<char> &query_id)
{
   TL_GET(PtrArray<RingoFetchContext>, _instances);

   int id = 1;

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->id >= id)
         id = _instances[i]->id + 1;

   AutoPtr<RingoFetchContext> new_context(new RingoFetchContext(id, context, query_id));
   const BingoOracleContext &boc = context.context();

   new_context->id = id;
   new_context->substructure.treat_x_as_pseudoatom = boc.treat_x_as_pseudoatom;
   new_context->substructure.ignore_closing_bond_direction_mismatch =
           boc.ignore_closing_bond_direction_mismatch;

   _instances.add(new_context.release());
   return *_instances.top();
}

RingoFetchContext * RingoFetchContext::findFresh (int context_id,
                                                  const Array<char> &query_id)
{
   TL_GET(PtrArray<RingoFetchContext>, _instances);

   int i;

   for (i = 0; i < _instances.size(); i++)
   {
      RingoFetchContext *instance = _instances[i];

      if (!instance->fresh)
         continue;

      if (instance->context_id != context_id)
         continue;

      if (instance->_query_id.memcmp(query_id) != 0)
         continue;

      return instance;
   }

   return 0;
}

void RingoFetchContext::remove (int id)
{
   TL_GET(PtrArray<RingoFetchContext>, _instances);
   int i;

   for (i = 0; i < _instances.size(); i++)
      if (_instances[i]->id == id)
         break;

   if (i == _instances.size())
      throw Error("remove(): context #%d not found", id);

   _instances.remove(i);
}
