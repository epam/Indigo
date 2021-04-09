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

#ifndef __indigo_properties__
#define __indigo_properties__

#include "indigo_internal.h"
namespace indigo
{
    class PropertiesMap;
}

class DLLEXPORT IndigoProperty : public IndigoObject
{
public:
    IndigoProperty(indigo::PropertiesMap& props, int idx);
    ~IndigoProperty() override;

    const char* getName() override;
    int getIndex() override;

    const char* getValue();

protected:
    indigo::PropertiesMap& _props;
    int _idx;
};

class IndigoPropertiesIter : public IndigoObject
{
public:
    IndigoPropertiesIter(indigo::PropertiesMap& props);
    ~IndigoPropertiesIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    indigo::PropertiesMap& _props;
    int _idx;
};

#endif
