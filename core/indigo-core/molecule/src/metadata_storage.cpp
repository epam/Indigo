#include "molecule/metadata_storage.h"
#include "base_c/defs.h"
#include "molecule/ket_commons.h"

using namespace indigo;

IMPL_ERROR(MetaDataStorage, "metadata storage");

void MetaDataStorage::addMetaObject(MetaObject* pobj)
{
    int index = _meta_data.size();
    _meta_data.expand(index + 1);
    _meta_data.set(index, pobj);

    switch (pobj->_class_id)
    {
    case KETSimpleObject::CID:
        _simple_object_indexes.push() = index;
        break;
    case KETReactionPlus::CID:
        _plus_indexes.push() = index;
        break;
    case KETReactionArrow::CID:
        _arrow_indexes.push() = index;
        break;
    default:
        break;
    }
}

void MetaDataStorage::clone(const MetaDataStorage& other)
{
    resetMetaData();
    const auto& meta = other.metaData();
    for (int i = 0; i < meta.size(); i++)
        addMetaObject(meta[i]->clone());
}

const MetaObject& MetaDataStorage::getMetaObject(std::uint32_t meta_type, int index) const
{
    switch (meta_type)
    {
    case KETSimpleObject::CID:
        return *_meta_data[_simple_object_indexes[index]];
        break;
    case KETReactionPlus::CID:
        return *_meta_data[_plus_indexes[index]];
        break;
    case KETReactionArrow::CID:
        return *_meta_data[_arrow_indexes[index]];
        break;
    default:
        throw Error("Unknown meta type");
        break;
    }
}

int MetaDataStorage::getMetaCount(std::uint32_t meta_type) const
{
    switch (meta_type)
    {
    case KETSimpleObject::CID:
        return _simple_object_indexes.size();
        break;
    case KETReactionPlus::CID:
        return _plus_indexes.size();
        break;
    case KETReactionArrow::CID:
        return _arrow_indexes.size();
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
    for (int i = _meta_data.size() - 1; i >= 0; i--)
    {
        if (_meta_data[i]->_class_id == KETReactionArrow::CID || _meta_data[i]->_class_id == KETReactionPlus::CID)
            _meta_data.remove(i);
    }
}
