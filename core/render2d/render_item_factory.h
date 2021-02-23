/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __render_item_factory_h__
#define __render_item_factory_h__

#include "render_context.h"
#include "render_item.h"
#include "render_item_aux.h"
#include "render_item_column.h"
#include "render_item_fragment.h"
#include "render_item_hline.h"
#include "render_item_molecule.h"
#include "render_item_reaction.h"

namespace indigo
{

#define GET_ITEM(name)                                                                                                                                         \
    case TYPE_##name:                                                                                                                                          \
        return _item##name[item.id];

#define IMPL_ITEM(name)                                                                                                                                        \
    int addItem##name()                                                                                                                                        \
    {                                                                                                                                                          \
        int id = _item##name.add(*this);                                                                                                                       \
        int num = _items.add(id);                                                                                                                              \
        _items[num].type = TYPE_##name;                                                                                                                        \
        return num;                                                                                                                                            \
    }                                                                                                                                                          \
    RenderItem##name& getItem##name(int i)                                                                                                                     \
    {                                                                                                                                                          \
        Item& item = _items[i];                                                                                                                                \
        if (item.type != TYPE_##name)                                                                                                                          \
            throw Error("Item type mismatch");                                                                                                                 \
        return _item##name[item.id];                                                                                                                           \
    }                                                                                                                                                          \
    bool isItem##name(int i)                                                                                                                                   \
    {                                                                                                                                                          \
        Item& item = _items[i];                                                                                                                                \
        return item.type == TYPE_##name;                                                                                                                       \
    }

#define DEF_POOL(name) ObjPool<RenderItem##name> _item##name;

    class RenderItemFactory
    {
    public:
        RenderItemFactory(RenderContext& rc_) : rc(rc_)
        {
        }
        virtual ~RenderItemFactory()
        {
        }

        DECL_ERROR;

        enum TYPE
        {
            TYPE_Fragment,
            TYPE_Auxiliary,
            TYPE_HLine,
            TYPE_Column,
            TYPE_Molecule,
            TYPE_Reaction,
            TYPE_Comment,
        };

        RenderItemBase& getItem(int i)
        {
            Item& item = _items[i];
            switch (item.type)
            {
                GET_ITEM(Fragment);
                GET_ITEM(Auxiliary);
                GET_ITEM(HLine);
                GET_ITEM(Column);
                GET_ITEM(Molecule);
                GET_ITEM(Reaction);
            default:
                throw Error("Item type unrecognized");
            }
        }

        IMPL_ITEM(Fragment);
        IMPL_ITEM(Auxiliary);
        IMPL_ITEM(HLine);
        IMPL_ITEM(Column);
        IMPL_ITEM(Molecule);
        IMPL_ITEM(Reaction);

        RenderContext& rc;

    private:
        struct Item
        {
            Item(int id_) : id(id_)
            {
            }
            int type;
            int id;
        };

        DEF_POOL(Fragment);
        DEF_POOL(Auxiliary);
        DEF_POOL(HLine);
        DEF_POOL(Column);
        DEF_POOL(Molecule);
        DEF_POOL(Reaction);

        ObjPool<Item> _items;
    };

} // namespace indigo

#endif //__render_item_factory_h__
