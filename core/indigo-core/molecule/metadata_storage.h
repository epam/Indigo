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
#ifndef __metadata_storage__
#define __metadata_storage__

#include <cstdint>

#include "base_cpp/ptr_array.h"
#include "base_cpp/red_black.h"
#include "common/math/algebra.h"

#include <memory>

namespace indigo
{
    // A meta object is constructed complete: its payload is fixed by the
    // constructor and the subclasses offer no in-place fill. It is therefore
    // never reset and not Reusable, and its owner is a plain owning array.
    class MetaObject
    {
    public:
        explicit MetaObject(uint32_t class_id) : _class_id(class_id)
        {
        }
        uint32_t _class_id;
        virtual MetaObject* clone() const = 0;
        virtual void getBoundingBox(Rect2f& bbox) const = 0;
        virtual void offset(const Vec2f& offset) = 0;
        virtual ~MetaObject() = default;
    };

    // Stays dense: objects are appended and a removal compacts the array, so
    // size() is the object count and every consumer can walk it by index. The
    // per-kind index arrays below are a derived lookup cache and are rebuilt
    // after a compaction rather than constraining the layout.
    using MetaObjectStore = PtrArray<MetaObject>;

    class MetaDataStorage
    {
    public:
        DECL_ERROR;
        void clone(const MetaDataStorage& other);
        void append(const MetaDataStorage& other);

        virtual ~MetaDataStorage()
        {
        }

        int addMetaObject(MetaObject* pobj, bool explicit_reaction_object = false);

        void resetMetaData()
        {
            _meta_data.clear();
            _clearIndexes();
            _explicit_reaction_object_indexes.clear();
        }

        void resetReactionData();

        const MetaObjectStore& metaData() const
        {
            return _meta_data;
        }

        int getMetaCount(uint32_t meta_type) const;
        int getNonChemicalMetaCount() const;

        const MetaObject& getMetaObject(uint32_t meta_type, int index) const;
        int getMetaObjectIndex(uint32_t meta_type, int index) const;
        void addExplicitReactionObjectIndex(int index);

    protected:
        void _clearIndexes();
        // Rebuilds every per-kind index from the current contents of the store.
        void _reindex();
        void _indexMetaObject(uint32_t class_id, int index);

        MetaObjectStore _meta_data;
        Array<int> _plus_indexes;
        Array<int> _arrow_indexes;
        Array<int> _multi_tail_indexes;
        Array<int> _simple_object_indexes;
        Array<int> _text_object_indexes;
        Array<int> _image_indexes;
        RedBlackSet<int> _explicit_reaction_object_indexes;
    };
}
#endif