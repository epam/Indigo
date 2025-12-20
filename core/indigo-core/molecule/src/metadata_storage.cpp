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

#include "molecule/metadata_storage.h"
#include "base_c/defs.h"
#include "molecule/meta_commons.h"

using namespace indigo;

IMPL_ERROR(MetaDataStorage, "metadata storage");

bool isReactionObject(uint32_t class_id)
{
    return class_id == ReactionArrowObject::CID || class_id == ReactionPlusObject::CID || class_id == ReactionMultitailArrowObject::CID;
}

int MetaDataStorage::addMetaObject(MetaObject* pobj, bool explicit_reaction_object)
{
    int index = _meta_data.add(pobj);

    switch (pobj->_class_id)
    {
    case SimpleTextObject::CID:
        _text_object_indexes.push() = index;
        break;
    case SimpleGraphicsObject::CID:
        _simple_object_indexes.push() = index;
        break;
    case ReactionPlusObject::CID:
        _plus_indexes.push() = index;
        break;
    case ReactionArrowObject::CID:
        _arrow_indexes.push() = index;
        break;
    case EmbeddedImageObject::CID:
        _image_indexes.push() = index;
        break;
    case ReactionMultitailArrowObject::CID:
        _multi_tail_indexes.push() = index;
        break;
    default:
        break;
    }
    if (explicit_reaction_object && !isReactionObject(pobj->_class_id))
        _explicit_reaction_object_indexes.find_or_insert(index);
    return index;
}

void MetaDataStorage::append(const MetaDataStorage& other)
{
    const auto& meta = other.metaData();
    for (int i = 0; i < meta.size(); i++)
        addMetaObject(meta[i]->clone());
    for (auto it = other._explicit_reaction_object_indexes.begin(); it != other._explicit_reaction_object_indexes.end();
         it = other._explicit_reaction_object_indexes.next(it))
    {
        _explicit_reaction_object_indexes.insert(other._explicit_reaction_object_indexes.key(it));
    }
}

void MetaDataStorage::clone(const MetaDataStorage& other)
{
    resetMetaData();
    append(other);
}

int MetaDataStorage::getMetaObjectIndex(uint32_t meta_type, int index) const
{
    switch (meta_type)
    {
    case SimpleTextObject::CID:
        return _text_object_indexes[index];
        break;
    case SimpleGraphicsObject::CID:
        return _simple_object_indexes[index];
        break;
    case ReactionPlusObject::CID:
        return _plus_indexes[index];
        break;
    case ReactionArrowObject::CID:
        return _arrow_indexes[index];
        break;
    case EmbeddedImageObject::CID:
        return _image_indexes[index];
        break;
    case ReactionMultitailArrowObject::CID:
        return _multi_tail_indexes[index];
    default:
        throw Error("Unknown meta type");
        break;
    }
}

void MetaDataStorage::addExplicitReactionObjectIndex(int index)
{
    _explicit_reaction_object_indexes.find_or_insert(index);
}

const MetaObject& MetaDataStorage::getMetaObject(uint32_t meta_type, int index) const
{
    return *_meta_data[getMetaObjectIndex(meta_type, index)];
}

int MetaDataStorage::getNonChemicalMetaCount() const
{
    return getMetaCount(SimpleTextObject::CID) + getMetaCount(SimpleGraphicsObject::CID) + getMetaCount(EmbeddedImageObject::CID);
}

int MetaDataStorage::getMetaCount(uint32_t meta_type) const
{
    switch (meta_type)
    {
    case SimpleTextObject::CID:
        return _text_object_indexes.size();
        break;
    case SimpleGraphicsObject::CID:
        return _simple_object_indexes.size();
        break;
    case ReactionPlusObject::CID:
        return _plus_indexes.size();
        break;
    case ReactionArrowObject::CID:
        return _arrow_indexes.size();
        break;
    case EmbeddedImageObject::CID:
        return _image_indexes.size();
        break;
    case ReactionMultitailArrowObject::CID:
        return _multi_tail_indexes.size();
        break;
    default:
        break;
    }
    return 0;
}

void MetaDataStorage::resetReactionData()
{
    _plus_indexes.clear();
    _arrow_indexes.clear();
    _multi_tail_indexes.clear();
    for (int i = _meta_data.size() - 1; i >= 0; i--)
        if (isReactionObject(_meta_data[i]->_class_id))
            _meta_data.remove(i);

    for (auto it = _explicit_reaction_object_indexes.begin(); it != _explicit_reaction_object_indexes.end(); it = _explicit_reaction_object_indexes.next(it))
        _meta_data.remove(_explicit_reaction_object_indexes.key(it));

    _explicit_reaction_object_indexes.clear();
}
