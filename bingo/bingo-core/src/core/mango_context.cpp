/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#include "core/mango_context.h"
#include "core/bingo_context.h"

TL_DEF(MangoContext, PtrArray<MangoContext>, _instances);

OsLock MangoContext::_instances_lock;

IMPL_ERROR(MangoContext, "mango context");

MangoContext::MangoContext (BingoContext &context) :
substructure(context),
similarity(context),
exact(context),
tautomer(context),
gross(context),
_context(context)
{
}

MangoContext::~MangoContext ()
{
}

MangoContext * MangoContext::_get (int id, BingoContext &context)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoContext>, _instances);

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->_context.id == id)
         return _instances[i];

   return 0;
}

MangoContext * MangoContext::existing (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoContext>, _instances);

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->_context.id == id)
         return _instances[i];

   throw Error("context #%d not found", id);
}

MangoContext * MangoContext::get (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoContext>, _instances);

   for (int i = 0; i < _instances.size(); i++)
      if (_instances[i]->_context.id == id)
         return _instances[i];

   BingoContext *bingo_context = BingoContext::get(id);

   return &_instances.add(new MangoContext(*bingo_context));
}

void MangoContext::remove (int id)
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoContext>, _instances);
   int i;

   for (i = 0; i < _instances.size(); i++)
      if (_instances[i]->_context.id == id)
         break;

   //if (i == _instances.size())
   //   throw Error("remove(): context #%d not found", id);

   if (i != _instances.size())
      _instances.remove(i);
}

int MangoContext::begin ()
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoContext>, _instances);

   if (_instances.size() < 1)
      return 0;

   int i, min_id = _instances[0]->_context.id;

   for (i = 1; i < _instances.size(); i++)
      if (_instances[i]->_context.id < min_id)
         min_id = _instances[i]->_context.id;

   return min_id;
}

int MangoContext::end ()
{
   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoContext>, _instances);

   if (_instances.size() < 1)
      return 0;

   int i, max_id = _instances[0]->_context.id;

   for (i = 1; i < _instances.size(); i++)
      if (_instances[i]->_context.id > max_id)
         max_id = _instances[i]->_context.id;

   return max_id + 1;
}

int MangoContext::next (int k)
{
   int i, next_id = end();

   OsLocker locker(_instances_lock);
   TL_GET(PtrArray<MangoContext>, _instances);

   for (i = 0; i < _instances.size(); i++)
      if (_instances[i]->_context.id > k && _instances[i]->_context.id < next_id)
         next_id = _instances[i]->_context.id;

   return next_id;
}
