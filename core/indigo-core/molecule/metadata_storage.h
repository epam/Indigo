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

#include <memory>
#include <vector>

#include "base_cpp/ptr_array.h"

namespace indigo
{
    class MetaObject
    {
    public:
        explicit MetaObject(uint32_t class_id) : _class_id(class_id)
        {
        }
        uint32_t _class_id;
        virtual MetaObject* clone() const = 0;
        virtual ~MetaObject(){};
    };

    class MetaDataStorage
    {
    public:
        DECL_ERROR;

        using MetadataVector = std::vector<std::shared_ptr<MetaObject>>;

        virtual ~MetaDataStorage() = default;

        void addMetaObject(MetaObject* pobj);

        void resetMetaData()
        {
            _meta_data.clear();
            _plus_indexes.clear();
            _arrow_indexes.clear();
            _simple_object_indexes.clear();
        }

        void resetReactionData();

        const MetadataVector& metaData() const
        {
            return _meta_data;
        }

        int getMetaCount(uint32_t meta_type) const;
        const MetaObject& getMetaObject(uint32_t meta_type, int index) const;

    protected:
        MetadataVector _meta_data;
        std::vector<int> _plus_indexes;
        std::vector<int> _arrow_indexes;
        std::vector<int> _simple_object_indexes;
        std::vector<int> _text_object_indexes;
    };
}
#endif