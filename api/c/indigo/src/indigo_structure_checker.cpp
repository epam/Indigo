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

#include <functional>
#include <string>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "indigo.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "indigo_structure_checker.h"

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

static StructureChecker::CheckResult _check(CheckMode mode, IndigoStructureChecker& thisPtr, int handleitem, const std::string& check_types_str,
                                            const IndigoObject* objitem, const std::vector<StructureChecker::CheckTypeCode>& check_types,
                                            const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds)
{
    StructureChecker::CheckResult r;
    if (handleitem < 0)
    {
        StructureChecker::CheckMessage msg;
        msg.code = StructureChecker::CheckMessageCode::CHECK_MSG_LOAD;
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

StructureChecker::CheckResult IndigoStructureChecker::check(const char* item, const char* check_types, const char* load_params)
{
    std::string lp = std::string(load_params ? load_params : "");

    int it = indigoLoadStructureFromString(item, lp.c_str());
    if (it < 0)
    {
        it = indigoLoadStructureFromString(item, (lp + " query").c_str()); // ##!!!PATCH
    }
    auto r = check(it, check_types);
    indigoFree(it);
    return r;
}

StructureChecker::CheckResult IndigoStructureChecker::check(int item, const char* check_types)
{
    return _check(CheckMode::STRING_ALL, *this, item, check_types, nullptr, {}, {}, {});
}

StructureChecker::CheckResult IndigoStructureChecker::check(int item, const char* check_types, const std::vector<int>& selected_atoms,
                                                            const std::vector<int>& selected_bonds)
{
    return _check(CheckMode::STRING_TYPES, *this, item, check_types, nullptr, {}, selected_atoms, selected_bonds);
}

StructureChecker::CheckResult IndigoStructureChecker::check(int item, const std::vector<CheckTypeCode>& check_types, const std::vector<int>& selected_atoms,
                                                            const std::vector<int>& selected_bonds)
{
    return _check(CheckMode::BIN_ALL, *this, item, "", nullptr, check_types, selected_atoms, selected_bonds);
}

StructureChecker::CheckResult IndigoStructureChecker::check(const IndigoObject& item, const std::vector<CheckTypeCode>& check_types,
                                                            const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds)
{
    return _check(CheckMode::BIN_ALL, *this, 0, "", &item, check_types, selected_atoms, selected_bonds);
}

using namespace rapidjson;

void dumpMessage(StructureChecker::CheckMessage& msg, std::string& out_str)
{
    if (!out_str.empty())
        out_str += ", ";
    if (!msg.prefix.empty())
        out_str += msg.prefix + ':';
    out_str += msg.message();
    if (!msg.ids.empty())
    {
        out_str += ": (";
        for (int i = 0; i < msg.ids.size(); ++i)
        {
            out_str += std::to_string(msg.ids[i]);
            if (i < msg.ids.size() - 1)
                out_str += ",";
        }
        out_str += ")";
    }

    if (!msg.subresult.isEmpty())
    {
        for (auto sub_msg : msg.subresult.messages)
            dumpMessage(sub_msg, out_str);
    }
}

std::string IndigoStructureChecker::toJson(const StructureChecker::CheckResult& res)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    for (auto msg : res.messages)
    {
        writer.Key(getCheckType(StructureChecker::getCheckTypeByMsgCode(msg.code)).c_str());
        std::string message;
        dumpMessage(msg, message);
        writer.String(message.c_str());
    }
    writer.EndObject();
    return std::string(s.GetString());
}
