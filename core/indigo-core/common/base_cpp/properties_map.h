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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/array.h"
#include "base_cpp/auto_iter.h"
#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"
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
        ~PropertiesMap()
        {
        }
        //   inline RedBlackStringObjMap< Array<char> >& getProperties() {
        //      return _properties;
        //   }
        void copy(RedBlackStringObjMap<Array<char>>& properties);
        void copy(PropertiesMap&);
        void merge(PropertiesMap&);
        void insert(const char* key, const char* value);
        void insert(const char* key, const std::string& value);

        Array<char>& insert(const char* key);

        const char* key(int);
        const char* value(int);
        Array<char>& valueBuf(const char* key);

        bool contains(const char* key) const;
        const char* at(const char* key) const;
        void remove(const char* key);
        void clear();
        bool is_empty();

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
        PropertiesMap(const PropertiesMap&);
        RedBlackStringObjMap<Array<char>> _properties;
        ObjArray<Array<char>> _propertyNames;
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // __auto_iter_h__
