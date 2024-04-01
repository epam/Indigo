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

#include <algorithm>

#include "base_cpp/properties_map.h"

using namespace indigo;

IMPL_ERROR(PropertiesMap, "properties map");

void PropertiesMap::copy(const RedBlackStringObjMap<Array<char>>& other)
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
    const auto it = _properties.find(key);
    if (it != _properties.end())
    {
        if (value != 0)
            _properties.at(key) = value;
    }
    else
    {
        auto newIt = _properties.emplace(key, value).first;
        _propertiesOrdered.push_back(std::move(newIt));
    }
}
std::string& PropertiesMap::insert(const char* key)
{
    insert(key, "");
    return valueBuf(key);
}
const char* PropertiesMap::key(int i) const
{
    return _propertiesOrdered.at(i)->first.c_str();
}

const char* PropertiesMap::value(int i) const
{
    return _propertiesOrdered.at(i)->second.c_str();
}

std::string& PropertiesMap::valueBuf(const char* key)
{
    return _properties.at(key);
}

void PropertiesMap::clear()
{
    _properties.clear();
    _propertiesOrdered.clear();
}

bool PropertiesMap::is_empty()
{
    return _properties.size() == 0;
}

bool PropertiesMap::contains(const char* key) const
{
    return _properties.find(key) != _properties.end();
}

const char* PropertiesMap::at(const char* key) const
{
    return _properties.at(key).c_str();
}

void PropertiesMap::remove(const char* key)
{
    const auto it = _properties.find(key);
    if (it != _properties.end())
    {
        _properties.erase(it);
        const auto orderedIt = std::find(_propertiesOrdered.begin(), _propertiesOrdered.end(), it);
        if (orderedIt != _propertiesOrdered.end())
        {
            _propertiesOrdered.erase(orderedIt);
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
    return PropertiesMap::PrIter(_owner, static_cast<int>(_owner._propertiesOrdered.size()));
}
