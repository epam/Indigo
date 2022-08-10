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

#ifndef __properties_map_h__
#define __properties_map_h__

#include <map>
#include <string>
#include <vector>

#include "base_cpp/auto_iter.h"
#include "base_cpp/exception.h"
#include "base_cpp/red_black.h"


namespace indigo
{

    class DLLEXPORT PropertiesMap
    {
    public:
        DECL_ERROR;

        explicit PropertiesMap()
        {
        }

        void copy(const RedBlackStringObjMap<Array<char>>& properties);
        void copy(PropertiesMap&);
        void insert(const char* key, const char* value);
        std::string& insert(const char* key);

        const char* key(int) const;
        const char* value(int) const;
        std::string& valueBuf(const char* key);

        bool contains(const char* key) const;
        const char* at(const char* key) const;
        void remove(const char* key);
        void clear();

        class PrIter : public AutoIterator
        {
        public:
            PrIter(PropertiesMap& owner, int idx);
            PrIter& operator++();

        private:
            PropertiesMap& _owner;
        };

        class PrAuto
        {
        public:
            PrAuto(PropertiesMap& owner) : _owner(owner)
            {
            }
            PrIter begin();
            int next(int);
            PrIter end();

        private:
            PropertiesMap& _owner;
        };

        PrAuto elements();

    private:
        // std::less<...> is used to avoid creation of temp string when searching by const char* index
        using StorageType = std::map<std::string, std::string, std::less<std::string>>;

        PropertiesMap(const PropertiesMap&);

        StorageType _properties;
        std::vector<StorageType::iterator> _propertiesOrdered;
    };

} // namespace indigo

#endif // __auto_iter_h__
