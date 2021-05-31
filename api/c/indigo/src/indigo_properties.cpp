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

#include "indigo_properties.h"
#include "base_cpp/properties_map.h"

CEXPORT int indigoHasProperty(int handle, const char* prop)
{
    INDIGO_BEGIN
    {
        if (prop == 0 || *prop == 0)
            throw IndigoError("indigoHasProperty(): null or empty property given");

        IndigoObject& obj = self.getObject(handle);

        auto& props = obj.getProperties();

        return props.contains(prop);
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoGetProperty(int handle, const char* prop)
{
    INDIGO_BEGIN
    {
        if (prop == 0 || *prop == 0)
            throw IndigoError("indigoGetProperty(): null or empty property given");

        IndigoObject& obj = self.getObject(handle);
        auto& props = obj.getProperties();

        auto& tmp = self.getThreadTmpData();
        tmp.string = props.at(prop);
        return tmp.string.c_str();
    }
    INDIGO_END(0);
}

CEXPORT int indigoSetProperty(int handle, const char* prop, const char* value)
{
    INDIGO_BEGIN
    {
        if (prop == 0 || *prop == 0)
            throw IndigoError("indigoSetProperty(): null or empty property given");

        if (value == 0)
            value = "";

        IndigoObject& obj = self.getObject(handle);
        auto& props = obj.getProperties();
        props.insert(prop, value);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRemoveProperty(int handle, const char* prop)
{
    INDIGO_BEGIN
    {
        if (prop == 0 || *prop == 0)
            throw IndigoError("indigoRemoveProperty(): null or empty property given");

        IndigoObject& obj = self.getObject(handle);
        auto& props = obj.getProperties();
        props.remove(prop);
        return 1;
    }
    INDIGO_END(-1);
}

IndigoPropertiesIter::IndigoPropertiesIter(indigo::PropertiesMap& props) : IndigoObject(PROPERTIES_ITER), _props(props)
{
    _idx = -1;
}

IndigoPropertiesIter::~IndigoPropertiesIter()
{
}

IndigoProperty::IndigoProperty(indigo::PropertiesMap& props, int idx) : IndigoObject(PROPERTY), _props(props), _idx(idx)
{
}

IndigoProperty::~IndigoProperty()
{
}

const char* IndigoProperty::getName()
{
    return _props.key(_idx);
}

const char* IndigoProperty::getValue()
{
    return _props.value(_idx);
}

int IndigoProperty::getIndex()
{
    return _idx;
}

bool IndigoPropertiesIter::hasNext()
{
    if (_idx == -1)
        return *_props.elements().begin() != *_props.elements().end();

    return _props.elements().next(_idx) != *_props.elements().end();
}

IndigoObject* IndigoPropertiesIter::next()
{
    if (_idx == -1)
        _idx = *_props.elements().begin();
    else if (_idx != *_props.elements().end())
        _idx = _props.elements().next(_idx);

    if (_idx == *_props.elements().end())
        return 0;

    return new IndigoProperty(_props, _idx);
}

CEXPORT int indigoIterateProperties(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);
        auto& props = obj.getProperties();

        return self.addObject(new IndigoPropertiesIter(props));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClearProperties(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);
        auto& props = obj.getProperties();

        props.clear();
        return 0;
    }
    INDIGO_END(-1);
}
