#include "molecule/metadata_storage.h"
#include "base_c/defs.h"
#include "molecule/meta_commons.h"

using namespace indigo;

IMPL_ERROR(MetaDataStorage, "metadata storage");

bool isReactionObject(uint32_t class_id)
{
    return class_id == ReactionArrowObject::CID || class_id == ReactionPlusObject::CID || class_id == ReactionMultitailArrowObject::CID;
}

void MetaDataStorage::_clearIndexes()
{
    _plus_indexes.clear();
    _arrow_indexes.clear();
    _multi_tail_indexes.clear();
    _simple_object_indexes.clear();
    _text_object_indexes.clear();
    _image_indexes.clear();
}

void MetaDataStorage::_indexMetaObject(uint32_t class_id, int index)
{
    switch (class_id)
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
}

void MetaDataStorage::_reindex()
{
    _clearIndexes();
    for (int i = 0; i < _meta_data.size(); i++)
        _indexMetaObject(_meta_data[i]._class_id, i);
}

int MetaDataStorage::addMetaObject(MetaObject* pobj, bool explicit_reaction_object)
{
    const uint32_t class_id = pobj->_class_id;
    _meta_data.add(std::unique_ptr<MetaObject>(pobj));
    const int index = _meta_data.size() - 1;

    _indexMetaObject(class_id, index);
    if (explicit_reaction_object && !isReactionObject(class_id))
        _explicit_reaction_object_indexes.find_or_insert(index);
    return index;
}

void MetaDataStorage::append(const MetaDataStorage& other)
{
    const auto& meta = other.metaData();
    for (int i = 0; i < meta.size(); i++)
        addMetaObject(meta[i].clone());
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
    return _meta_data[getMetaObjectIndex(meta_type, index)];
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
    // Drops the reaction objects and the ones explicitly marked as such, keeping
    // the rest in order. Compacting keeps the store dense; the per-kind indexes
    // are rebuilt from the new contents.
    MetaObjectStore kept;
    for (int i = 0; i < _meta_data.size(); i++)
    {
        if (isReactionObject(_meta_data[i]._class_id) || _explicit_reaction_object_indexes.find(i))
            continue;
        kept.add(_meta_data.release(i));
    }
    _meta_data = std::move(kept);
    _reindex();
    _explicit_reaction_object_indexes.clear();
}
