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

#include "molecule/ket_annotation.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;

IMPL_ERROR(KetObjectAnnotation, "Ket Object Annotation")

const std::map<std::string, int>& KetObjectAnnotation::getStringPropStrToIdx() const
{
    static std::map<std::string, int> str_to_idx{
        {"text", toUType(StringProps::text)},
    };
    return str_to_idx;
}

IMPL_ERROR(KetAnnotation, "Ket Annotation")

void KetAnnotation::copy(const KetAnnotation& other)
{
    KetObjWithProps::copy(other);
    if (other._extended.has_value())
        setExtended(*other._extended);
}

void KetAnnotation::setExtended(const rapidjson::Document& extended)
{
    _extended.emplace();
    _extended->CopyFrom(extended, _extended->GetAllocator());
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif