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
#include <string>

#include "base_cpp/properties_map.h"

using namespace indigo;

IMPL_ERROR(PropertiesMap, "properties map");

void PropertiesMap::copy(RedBlackStringObjMap<Array<char>>& other)
{
    clear();
    for (int i = other.begin(); i != other.end(); i = other.next(i))
    {
        insert(other.key(i), other.value(i).ptr());
    }
}
void PropertiesMap::copy(PropertiesMap& other)
{
    clear();
    for (auto p : other.elements())
    {
        insert(other.key(p), other.value(p));
    }
}

void PropertiesMap::merge(PropertiesMap& other)
{
    for (auto p : other.elements())
        insert(other.key(p), other.value(p));
}

void PropertiesMap::insert(const char* key, const std::string& value)
{
    insert(key, value.c_str());
}

void PropertiesMap::insert(const char* key, const char* value)
{
    if (_properties.find(key))
    {
        auto& val = _properties.at(key);
        if (value != 0)
            val.readString(value, true);
    }
    else
    {
        auto& name = _propertyNames.push();
        name.readString(key, true);
        int k = _properties.insert(key);
        if (value != 0)
            _properties.value(k).readString(value, true);
    }
}
Array<char>& PropertiesMap::insert(const char* key)
{
    insert(key, 0);
    return valueBuf(key);
}
const char* PropertiesMap::key(int i)
{
    return _propertyNames.at(i).ptr();
}

const char* PropertiesMap::value(int i)
{
    auto& buf = valueBuf(_propertyNames.at(i).ptr());
    if (buf.size() > 0)
    {
        return buf.ptr();
    }
    else
    {
        return "";
    }
}

Array<char>& PropertiesMap::valueBuf(const char* key)
{
    return _properties.at(key);
}

void PropertiesMap::clear()
{
    _properties.clear();
    _propertyNames.clear();
}

bool PropertiesMap::is_empty()
{
    return _properties.size() == 0;
}

bool PropertiesMap::contains(const char* key) const
{
    return _properties.find(key);
}

const char* PropertiesMap::at(const char* key) const
{
    return _properties.at(key).ptr();
}

void PropertiesMap::remove(const char* key)
{
    if (_properties.find(key))
    {
        _properties.remove(key);
        int to_remove = -1;
        for (auto i = 0; i < _propertyNames.size(); i++)
        {
            if (strcmp(_propertyNames.at(i).ptr(), key) == 0)
            {
                to_remove = i;
                break;
            }
        }
        if (to_remove >= 0)
        {
            _propertyNames.remove(to_remove);
        }
        else
        {
            throw Error("internal error with properties");
        }
    }
}

PropertiesMap::PrAuto PropertiesMap::elements()
{
    return PrAuto(*this);
}

PropertiesMap::PrIter::PrIter(PropertiesMap& owner, int idx) : AutoIterator(idx), _owner(owner)
{
}

PropertiesMap::PrIter& PropertiesMap::PrIter::operator++()
{
    _idx += 1;
    return *this;
}

PropertiesMap::PrIter PropertiesMap::PrAuto::begin()
{
    return PropertiesMap::PrIter(_owner, 0);
}

int PropertiesMap::PrAuto::next(int k)
{
    return k + 1;
}
PropertiesMap::PrIter PropertiesMap::PrAuto::end()
{
    return PropertiesMap::PrIter(_owner, _owner._propertyNames.size());
}