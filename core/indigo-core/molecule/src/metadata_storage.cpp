#include "molecule/metadata_storage.h"
#include "base_c/defs.h"
#include "molecule/ket_commons.h"

using namespace indigo;

IMPL_ERROR(MetaDataStorage, "metadata storage");

void MetaDataStorage::addMetaObject(MetaObject* pobj)
{
    int index = _meta_data.size();
    _meta_data.emplace_back(pobj);

    switch (pobj->_class_id)
    {
    case KETTextObject::CID:
        _text_object_indexes.push_back(index);
        break;
    case KETSimpleObject::CID:
        _simple_object_indexes.push_back(index);
        break;
    case KETReactionPlus::CID:
        _plus_indexes.push_back(index);
        break;
    case KETReactionArrow::CID:
        _arrow_indexes.push_back(index);
        break;
    default:
        break;
    }
}

const MetaObject& MetaDataStorage::getMetaObject(uint32_t meta_type, int index) const
{
    switch (meta_type)
    {
    case KETTextObject::CID:
        return *_meta_data[_text_object_indexes[index]];
        break;
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

int MetaDataStorage::getMetaCount(uint32_t meta_type) const
{
    switch (meta_type)
    {
    case KETTextObject::CID:
        return _text_object_indexes.size();
        break;
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

    std::vector<int> indexes_to_remove;
    for (int i = _meta_data.size() - 1; i >= 0; i--)
    {
        if (_meta_data[i]->_class_id == KETReactionArrow::CID || _meta_data[i]->_class_id == KETReactionPlus::CID)
        {
            indexes_to_remove.push_back(i);
        }
    }

    for (int i : indexes_to_remove)
    {
        _meta_data.erase(_meta_data.begin() + i);

        for (int& toi : _text_object_indexes)
        {
            if (toi > i)
                --toi;
        }

        for (int& soi : _simple_object_indexes)
        {
            if (soi > i)
                --soi;
        }
    }
}
