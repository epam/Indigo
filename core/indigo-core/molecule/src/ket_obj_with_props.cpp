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

#include "molecule/ket_obj_with_props.h"
#include "molecule/json_writer.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(KetObjWithProps, "Ket Options")

static std::map<std::string, int> empty_str_to_idx;

const std::map<std::string, int>& KetObjWithProps::getBoolPropStrToIdx() const
{
    return empty_str_to_idx;
};

const std::map<std::string, int>& KetObjWithProps::getIntPropStrToIdx() const
{
    return empty_str_to_idx;
};

const std::map<std::string, int>& KetObjWithProps::getStringPropStrToIdx() const
{
    return empty_str_to_idx;
};

bool KetObjWithProps::getBoolProp(int idx) const
{
    auto it = _bool_props.find(idx);
    if (it == _bool_props.end())
        throw Error("Option %d not found", idx);
    return it->second;
};

int KetObjWithProps::getIntProp(int idx) const
{
    auto it = _int_props.find(idx);
    if (it == _int_props.end())
        throw Error("Option %d not found", idx);
    return it->second;
};

const std::string& KetObjWithProps::getStringProp(int idx) const
{
    auto it = _string_props.find(idx);
    if (it == _string_props.end())
        throw Error("Option %d not found", idx);
    return it->second;
};

static std::pair<bool, int> find_prop_idx(const std::map<std::string, int>& map, const std::string& name)
{
    auto it = map.find(name);
    if (it == map.end())
        return std::make_pair(false, -1);
    return std::make_pair(true, it->second);
}

std::pair<bool, int> KetObjWithProps::getBoolPropIdx(const std::string& name) const
{
    return find_prop_idx(getBoolPropStrToIdx(), name);
}

std::pair<bool, int> KetObjWithProps::getIntPropIdx(const std::string& name) const
{
    return find_prop_idx(getIntPropStrToIdx(), name);
}

std::pair<bool, int> KetObjWithProps::getStringPropIdx(const std::string& name) const
{
    return find_prop_idx(getStringPropStrToIdx(), name);
}

void KetObjWithProps::parseOptsFromKet(const rapidjson::Value& json)
{
    // Parse bool props
    for (auto it : getBoolPropStrToIdx())
    {
        if (json.HasMember(it.first.c_str()))
            setBoolProp(it.second, json[it.first.c_str()].GetBool());
    }
    // Parse int props
    for (auto it : getIntPropStrToIdx())
    {
        if (json.HasMember(it.first.c_str()))
            setIntProp(it.second, json[it.first.c_str()].GetInt());
    }
    // Parse string props
    for (auto it : getStringPropStrToIdx())
    {
        if (json.HasMember(it.first.c_str()))
            setStringProp(it.second, json[it.first.c_str()].GetString());
    }
};

void KetObjWithProps::saveOptsToKet(IJsonWriter& writer) const
{
    // Parse bool props
    std::map<int, std::string> boolPropIdxTostr;
    for (auto it : getBoolPropStrToIdx())
    {
        boolPropIdxTostr.emplace(it.second, it.first);
    }
    for (auto it : boolPropIdxTostr)
    {
        if (hasBoolProp(it.first))
        {
            writer.Key(it.second);
            writer.Bool(getBoolProp(it.first));
        }
    }

    // Parse int props
    std::map<int, std::string> intPropIdxTostr;
    for (auto it : getIntPropStrToIdx())
    {
        intPropIdxTostr.emplace(it.second, it.first);
    }
    for (auto it : intPropIdxTostr)
    {
        if (hasIntProp(it.first))
        {
            writer.Key(it.second);
            writer.Int(getIntProp(it.first));
        }
    }
    // Parse string props
    std::map<int, std::string> strPropIdxTostr;
    for (auto it : getStringPropStrToIdx())
    {
        strPropIdxTostr.emplace(it.second, it.first);
    }
    for (auto it : strPropIdxTostr)
    {
        if (hasStringProp(it.first))
        {
            writer.Key(it.second);
            writer.String(getStringProp(it.first));
        }
    }
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif