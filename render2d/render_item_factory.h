/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef __render_item_factory_h__
#define __render_item_factory_h__

#include "render_context.h"
#include "render_item.h"
#include "render_item_fragment.h"
#include "render_item_aux.h"
#include "render_item_hline.h"
#include "render_item_molecule.h"
#include "render_item_reaction.h"

namespace indigo {


#define GET_ITEM(name) case TYPE_##name: return _item##name[item.id];

#define IMPL_ITEM(name) \
   int addItem##name() \
   { \
      int id = _item##name.add(*this); \
      int num = _items.add(id); \
      _items[num].type = TYPE_##name; \
         return num; \
   } \
   RenderItem##name& getItem##name (int i) \
   { \
      Item& item = _items[i]; \
      if (item.type != TYPE_##name) \
         throw Error("Item type mismatch"); \
      return _item##name[item.id]; \
   } \
   bool isItem##name (int i) \
   { \
      Item& item = _items[i]; \
      return item.type == TYPE_##name; \
   }

#define DEF_POOL(name) ObjPool<RenderItem##name> _item##name;

class RenderItemFactory {
public:
   RenderItemFactory (RenderContext& rc_) : rc(rc_)
   {
   }
   virtual ~RenderItemFactory ()
   {
   }

   DECL_ERROR;

   enum TYPE {
      TYPE_Fragment,
      TYPE_Auxiliary,
      TYPE_HLine,
      TYPE_Molecule,
      TYPE_Reaction,
      TYPE_Comment,
   };

   RenderItemBase& getItem (int i)
   {
      Item& item = _items[i];
      switch (item.type) {
         GET_ITEM(Fragment);
         GET_ITEM(Auxiliary);
         GET_ITEM(HLine);
         GET_ITEM(Molecule);
         GET_ITEM(Reaction);
         default: throw Error("Item type unrecognized");
      }
   }

   IMPL_ITEM(Fragment);
   IMPL_ITEM(Auxiliary);
   IMPL_ITEM(HLine);
   IMPL_ITEM(Molecule);
   IMPL_ITEM(Reaction);
      
   RenderContext& rc;
private:
   struct Item {
      Item (int id_) : id(id_) {
      }
      int type;
      int id;
   };

   DEF_POOL(Fragment);
   DEF_POOL(Auxiliary);
   DEF_POOL(HLine);
   DEF_POOL(Molecule);
   DEF_POOL(Reaction);

   ObjPool<Item> _items;
};

}

#endif //__render_item_factory_h__
