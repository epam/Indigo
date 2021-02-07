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

#include "indigo_structure_checker.h"
#include "api/indigo.h"
#include "api/src/indigo_molecule.h"
#include "api/src/indigo_reaction.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule_automorphism_search.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"
#include <functional>
#include <string>

using namespace indigo;

IndigoStructureChecker::IndigoStructureChecker()
{
}

enum class CheckMode
{
    STRING_ALL,
    STRING_TYPES,
    BIN_ALL
};

static StructureChecker2::CheckResult _check(CheckMode mode, IndigoStructureChecker& thisPtr, int handleitem, const std::string& check_types_str,
                                             const IndigoObject* objitem, const std::vector<StructureChecker2::CheckTypeCode>& check_types,
                                             const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds)
{
    StructureChecker2::CheckResult r;
    if (handleitem < 0)
    {
        StructureChecker2::CheckMessage msg;
        msg.code = StructureChecker2::CheckMessageCode::CHECK_MSG_LOAD;
        r.messages.push_back(msg);
    }
    else
    {
        const IndigoObject& item = objitem ? *objitem : indigoGetInstance().getObject(handleitem);
        if (IndigoBaseMolecule::is((IndigoObject&)item))
        {
            switch (mode)
            {
            case CheckMode::STRING_ALL:
                r = thisPtr.checkMolecule(((IndigoObject&)item).getBaseMolecule(), check_types_str);
                break;
            case CheckMode::STRING_TYPES:
                r = thisPtr.checkMolecule(((IndigoObject&)item).getBaseMolecule(), check_types_str, selected_atoms, selected_bonds);
                break;
            case CheckMode::BIN_ALL:
                r = thisPtr.checkMolecule(((IndigoObject&)item).getBaseMolecule(), check_types, selected_atoms, selected_bonds);
                break;
            }
        }
        else if (IndigoBaseReaction::is((IndigoObject&)item))
        {
            switch (mode)
            {
            case CheckMode::STRING_ALL:
            case CheckMode::STRING_TYPES:
                r = thisPtr.checkReaction(((IndigoObject&)item).getBaseReaction(), check_types_str);
                break;
            case CheckMode::BIN_ALL:
                r = thisPtr.checkReaction(((IndigoObject&)item).getBaseReaction(), check_types);
                break;
            }
        }
        else if (IndigoAtom::is((IndigoObject&)item))
        {
            IndigoAtom& ia = IndigoAtom::cast((IndigoObject&)item);
            std::vector<int> atoms = {ia.getIndex() + 1};
            switch (mode)
            {
            case CheckMode::STRING_ALL:
            case CheckMode::STRING_TYPES:
                r = thisPtr.checkMolecule(ia.mol, check_types_str, atoms, {});
                break;
            case CheckMode::BIN_ALL:
                r = thisPtr.checkMolecule(ia.mol, check_types, atoms);
                break;
            }
        }
        else if (IndigoBond::is((IndigoObject&)item))
        {
            IndigoBond& ib = IndigoBond::cast((IndigoObject&)item);
            std::vector<int> bonds = {ib.getIndex() + 1};
            switch (mode)
            {
            case CheckMode::STRING_ALL:
            case CheckMode::STRING_TYPES:
                r = thisPtr.checkMolecule(ib.mol, check_types_str, std::vector<int>(), bonds);
                break;
            case CheckMode::BIN_ALL:
                r = thisPtr.checkMolecule(ib.mol, check_types, std::vector<int>(), bonds);
                break;
            }
        }
    }
    return r;
}

StructureChecker2::CheckResult IndigoStructureChecker::check(const char* item, const char* check_types, const char* load_params)
{
    std::string lp = std::string(load_params ? load_params : "");

    int it = indigoLoadStructureFromString(item, lp.c_str());
    if (it < 0)
    {
        it = indigoLoadStructureFromString(item, (lp + " query").c_str()); //##!!!PATCH
    }
    auto r = check(it, check_types);
    indigoFree(it);
    return r;
}

StructureChecker2::CheckResult IndigoStructureChecker::check(int item, const char* check_types)
{
    return _check(CheckMode::STRING_ALL, *this, item, check_types, nullptr, {}, {}, {});
}

StructureChecker2::CheckResult IndigoStructureChecker::check(int item, const char* check_types, const std::vector<int>& selected_atoms,
                                                             const std::vector<int>& selected_bonds)
{
    return _check(CheckMode::STRING_TYPES, *this, item, check_types, nullptr, {}, selected_atoms, selected_bonds);
}

StructureChecker2::CheckResult IndigoStructureChecker::check(int item, const std::vector<CheckTypeCode>& check_types, const std::vector<int>& selected_atoms,
                                                             const std::vector<int>& selected_bonds)
{
    return _check(CheckMode::BIN_ALL, *this, item, "", nullptr, check_types, selected_atoms, selected_bonds);
}

StructureChecker2::CheckResult IndigoStructureChecker::check(const IndigoObject& item, const std::vector<CheckTypeCode>& check_types,
                                                             const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds)
{
    return _check(CheckMode::BIN_ALL, *this, 0, "", &item, check_types, selected_atoms, selected_bonds);
}

using namespace rapidjson;
static void _toJson(const StructureChecker2::CheckResult& data, Writer<StringBuffer>& writer)
{
    writer.StartArray();
    for (auto msg : data.messages)
    {
        writer.StartObject();
        writer.Key("code");
        writer.Uint(static_cast<unsigned int>(msg.code));
        writer.Key("message");
        writer.String(msg.message().c_str());
        if (msg.index >= 0)
        {
            writer.Key("index");
            writer.Uint(msg.index);
        }
        if (!msg.ids.empty())
        {
            writer.Key("ids");
            writer.StartArray();
            for (auto i : msg.ids)
            {
                writer.Uint(i);
            }
            writer.EndArray();
        }
        if (!msg.subresult.isEmpty())
        {
            writer.Key("subresult");
            _toJson(msg.subresult, writer);
        }
        writer.EndObject();
    }
    writer.EndArray();
}
std::string IndigoStructureChecker::toJson(const StructureChecker2::CheckResult& res)
{
    std::stringstream result;
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    _toJson(res, writer);
    return std::string(s.GetString());
}
