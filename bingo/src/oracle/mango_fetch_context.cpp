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

#include "oracle/mango_fetch_context.h"
#include "core/mango_matchers.h"
#include "core/bingo_context.h"
#include "oracle/mango_shadow_fetch.h"
#include "oracle/bingo_oracle_context.h"

TL_DEF(MangoFetchContext, PtrArray<MangoFetchContext>, _instances);
OsLock MangoFetchContext::_instances_lock;


MangoFetchContext::MangoFetchContext (int id_, MangoOracleContext &context,
                                      const Array<char> &query_id) :
substructure(context.context()),
similarity(context.context()),
exact(context.context()),
tautomer(context.context()),
gross(context.context()),
_context(context)
{
   id = id_;
   context_id = context.context().id;
   _query_id.copy(query_id);
   fresh = false;
   fetch_engine = 0;

   shadow_fetch.reset(new MangoShadowFetch(*this));
   fast_index.reset(new MangoFastIndex(*this));
}

MangoFetchContext & MangoFetchContext::get (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoFetchContext>, _instances);

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->id == id)
         return *_instances[i];

   throw Error("context #%d not found", id);
}

MangoFetchContext & MangoFetchContext::create (MangoOracleContext &context,
                                               const Array<char> &query_id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoFetchContext>, _instances);

   int id = 1;

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->id >= id)
         id = _instances[i]->id + 1;

   AutoPtr<MangoFetchContext> new_context(new MangoFetchContext(id, context, query_id));

   const BingoOracleContext &boc = context.context();

   new_context->id = id;

   _instances.add(new_context.release());
   return *_instances.top();
}

MangoFetchContext * MangoFetchContext::findFresh (int context_id,
                                                  const Array<char> &query_id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoFetchContext>, _instances);

   int i;

   for (i = 0; i < _instances.size(); i++)
   {
      MangoFetchContext *instance = _instances[i];

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

void MangoFetchContext::remove (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoFetchContext>, _instances);
   int i;

   for (i = 0; i < _instances.size(); i++)
      if (_instances[i]->id == id)
         break;

   if (i == _instances.size())
      throw Error("remove(): context #%d not found", id);

   _instances.remove(i);
}

void MangoFetchContext::removeByContextID (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoFetchContext>, _instances);
   int i;

   for (i = _instances.size() - 1; i >= 0; i--)
      if (_instances[i]->context_id == id)
         _instances.remove(i);
}
