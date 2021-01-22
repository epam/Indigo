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
#include <regex>
#include <string>

using namespace indigo;

struct CheckParams
{
    int check_flags = StructureChecker2::CHECK_ALL;
    std::vector<int> selected_atoms;
    std::vector<int> selected_bonds;
};

static constexpr const char* checkTypeName[] = {
    IndigoStructureChecker::CHECK_NONE_TXT,      IndigoStructureChecker::CHECK_LOAD_TXT,         IndigoStructureChecker::CHECK_VALENCE_TXT,
    IndigoStructureChecker::CHECK_RADICAL_TXT,   IndigoStructureChecker::CHECK_PSEUDOATOM_TXT,   IndigoStructureChecker::CHECK_STEREO_TXT,
    IndigoStructureChecker::CHECK_QUERY_TXT,     IndigoStructureChecker::CHECK_OVERLAP_ATOM_TXT, IndigoStructureChecker::CHECK_OVERLAP_BOND_TXT,
    IndigoStructureChecker::CHECK_RGROUP_TXT,    IndigoStructureChecker::CHECK_SGROUP_TXT,       IndigoStructureChecker::CHECK_TGROUP_TXT,
    IndigoStructureChecker::CHECK_CHIRALITY_TXT, IndigoStructureChecker::CHECK_CHIRAL_FLAG_TXT,  IndigoStructureChecker::CHECK_3D_COORD_TXT,
    IndigoStructureChecker::CHECK_CHARGE_TXT,    IndigoStructureChecker::CHECK_SALT_TXT,         IndigoStructureChecker::CHECK_AMBIGUOUS_H_TXT,
    IndigoStructureChecker::CHECK_COORD_TXT,     IndigoStructureChecker::CHECK_V3000_TXT,        IndigoStructureChecker::CHECK_ALL_TXT};

static CheckParams check_params_from_string(const char* params)
{
    CheckParams r;
    size_t len = sizeof(checkTypeName) / sizeof(*checkTypeName);
    if (params)
    {
        std::smatch sm1;
        std::unordered_set<std::string> words;
        std::string s(params);
        std::regex rx1(R"(\b(\w+)\b)", std::regex_constants::icase);
        while (std::regex_search(s, sm1, rx1))
        {
            words.insert(sm1[1]);
            s = sm1.suffix();
        }
        r.check_flags = StructureChecker2::CHECK_NONE;
        for (int i = 0; i < len; i++)
        {
            if (words.find(std::string(checkTypeName[i])) != words.end())
            {
                r.check_flags |= 1 << i;
            }
        }
        r.check_flags = !r.check_flags || r.check_flags & 1 << (len - 1) ? StructureChecker2::CHECK_ALL
                                                                         : (r.check_flags == 1 ? StructureChecker2::CHECK_NONE : r.check_flags >> 1);

        std::smatch sm2;
        s = params;
        std::regex rx2(R"(\b(atoms|bonds)\b((?:\W+\b\d+\b\W*?)+))", std::regex_constants::icase);
        std::regex rx3(R"(\b(\d+)\b)");
        std::smatch sm3;
        while (std::regex_search(s, sm2, rx2))
        {
            std::vector<int>& vec = std::tolower(sm2[1].str()[0]) == 'a' ? r.selected_atoms : r.selected_bonds;
            std::string a = sm2[2];
            while (std::regex_search(a, sm3, rx3))
            {
                vec.push_back(atoi(sm3[1].str().c_str()));
                a = sm3.suffix();
            }
            s = sm2.suffix();
        }
    }
    return r;
}

IndigoStructureChecker::IndigoStructureChecker()
{
}

StructureChecker2::CheckResult IndigoStructureChecker::check(const char* item, const char* check_flags, const char* load_params)
{
    std::string lp = std::string(load_params ? load_params : "");

    int it = indigoLoadStructureFromString(item, lp.c_str());
    if (it < 0)
    {
        it = indigoLoadStructureFromString(item, (lp + " query").c_str()); //##!!!PATCH
    }
    auto r = check(it, check_flags);
    indigoFree(it);
    return r;
}

StructureChecker2::CheckResult IndigoStructureChecker::check(int item, const char* check_flags)
{
    auto p = check_params_from_string(check_flags);
    return check(item, p.check_flags, p.selected_atoms, p.selected_bonds);
}

StructureChecker2::CheckResult IndigoStructureChecker::check(int item, int check_types, const std::vector<int>& selected_atoms,
                                                             const std::vector<int>& selected_bonds)
{
    if (item < 0)
    {
        StructureChecker2::CheckMessage msg;
        msg.code = StructureChecker2::CheckMessageCode::CHECK_MSG_LOAD;
        StructureChecker2::CheckResult cr;
        cr.messages.push_back(msg);
        return cr;
    }
    else
    {
        return check(indigoGetInstance().getObject(item), check_types, selected_atoms, selected_bonds);
    }
}

StructureChecker2::CheckResult IndigoStructureChecker::check(const IndigoObject& item, int check_types, const std::vector<int>& selected_atoms,
                                                             const std::vector<int>& selected_bonds)
{
    CheckResult r;
    if (IndigoBaseMolecule::is((IndigoObject&)item))
    {
        r = checkMolecule(((IndigoObject&)item).getBaseMolecule(), check_types, selected_atoms, selected_bonds);
    }
    else if (IndigoBaseReaction::is((IndigoObject&)item))
    {
        r = checkReaction(((IndigoObject&)item).getBaseReaction(), check_types);
    }
    else if (IndigoAtom::is((IndigoObject&)item))
    {
        IndigoAtom& ia = IndigoAtom::cast((IndigoObject&)item);
        std::vector<int> atoms = {ia.getIndex() + 1};
        r = checkMolecule(ia.mol, check_types, atoms);
    }
    else if (IndigoBond::is((IndigoObject&)item))
    {
        IndigoBond& ib = IndigoBond::cast((IndigoObject&)item);
        std::vector<int> bonds = {ib.getIndex() + 1};
        r = checkMolecule(ib.mol, check_types, std::vector<int>(), bonds);
    }
    return r;
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
