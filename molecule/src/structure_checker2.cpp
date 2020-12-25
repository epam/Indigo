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

#include "molecule/structure_checker2.h"
#include "api/indigo.h"
#include "api/src/indigo_molecule.h"
#include <map>
#include <regex>
#include <string>

using namespace indigo;

namespace indigo
{
    static int check_type_from_string(const char* check);
}

StructureChecker2::StructureChecker2()
{
}

StructureChecker2::CheckResult StructureChecker2::check(const char* item, const char* params)
{
    return check(indigoLoadStructureFromString(item, params), params);
}

StructureChecker2::CheckResult StructureChecker2::check(int item, const char* check_types)
{
    return check(indigoGetInstance().getObject(item), check_types);
}

StructureChecker2::CheckResult StructureChecker2::check(const IndigoObject& item, const char* check_types)
{
    return check(item, check_type_from_string(check_types));
}

StructureChecker2::CheckResult StructureChecker2::check(const IndigoObject& item, int check_types)
{
    CheckResult r;
    if (IndigoBaseMolecule::is((IndigoObject&)item))
    {
        checkMolecule(((IndigoObject&)item).getBaseMolecule(), r);
    }
    // else if (IndigoBaseReaction::is((IndigoObject&)item)){}
    return r;
}

const char* StructureChecker2::CheckResult::toJson()
{
    return "CHECKER2";
}

const char* StructureChecker2::CheckResult::toYaml()
{
    return "CHECKER2";
}

/**************************************************/
void StructureChecker2::checkMolecule(const BaseMolecule& mol, CheckResult& result)
{
}

/**************************************************/
namespace indigo
{
    static const std::string check_type_names[] = {"LOAD",         "VALENCE",      "RADICAL", "PSEUDOATOM", "STEREO",      "QUERY",
                                                   "OVERLAP_ATOM", "OVERLAP_BOND", "RGROUP",  "SGROUP",     "TGROUP",      "CHIRALITY",
                                                   "CHIRAL_FLAG",  "3D_COORD",     "CHARGE",  "SALT",       "AMBIGUOUS_H", "COORD"};
    static const std::map<std::string, StructureChecker2::CheckTypeCode> check_type_map = {{"NONE", StructureChecker2::CHECK_NONE},
                                                                                           {check_type_names[0], StructureChecker2::CHECK_LOAD},
                                                                                           {check_type_names[1], StructureChecker2::CHECK_VALENCE},
                                                                                           {check_type_names[2], StructureChecker2::CHECK_RADICAL},
                                                                                           {check_type_names[3], StructureChecker2::CHECK_PSEUDOATOM},
                                                                                           {check_type_names[4], StructureChecker2::CHECK_STEREO},
                                                                                           {check_type_names[5], StructureChecker2::CHECK_QUERY},
                                                                                           {check_type_names[6], StructureChecker2::CHECK_OVERLAP_ATOM},
                                                                                           {check_type_names[7], StructureChecker2::CHECK_OVERLAP_BOND},
                                                                                           {check_type_names[8], StructureChecker2::CHECK_RGROUP},
                                                                                           {check_type_names[9], StructureChecker2::CHECK_SGROUP},
                                                                                           {check_type_names[10], StructureChecker2::CHECK_TGROUP},
                                                                                           {check_type_names[11], StructureChecker2::CHECK_CHIRALITY},
                                                                                           {check_type_names[12], StructureChecker2::CHECK_CHIRAL_FLAG},
                                                                                           {check_type_names[13], StructureChecker2::CHECK_3D_COORD},
                                                                                           {check_type_names[14], StructureChecker2::CHECK_CHARGE},
                                                                                           {check_type_names[15], StructureChecker2::CHECK_SALT},
                                                                                           {check_type_names[16], StructureChecker2::CHECK_AMBIGUOUS_H},
                                                                                           {check_type_names[17], StructureChecker2::CHECK_COORD},
                                                                                           {"ALL", StructureChecker2::CHECK_ALL}};
    static int check_type_from_string(const char* check)
    {
        int r = StructureChecker2::CHECK_NONE;
        bool def = true;
        if (check)
        {
            std::string s = check;
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            std::regex regex{R"(\w+)"};
            std::sregex_token_iterator it{s.begin(), s.end(), regex, -1};
            std::vector<std::string> words{it, {}};
            for (std::string n : words)
            {
                auto search = check_type_map.find(n);
                if (search != check_type_map.end())
                {
                    r |= search->second;
                    def = false;
                }
            }
        }
        return def ? StructureChecker2::CHECK_ALL : r;
    }
} // namespace indigo

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
/*
#ifndef __structure_checker2__
#define __structure_checker2__

#include <map>
#include <sstream>

#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "base_cpp/red_black.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/query_molecule.h"

#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"

namespace indigo
{
    using namespace std;

    class Molecule;
    class QueryMolecule;
    class BaseMolecule;
    class Scanner;
    class Output;

    class DLLEXPORT StructureChecker2
    {
    public:
        StructureChecker2();
        CheckResult check(const char* item, const char* params);
        CheckResult check(int item, const char* params);
        CheckResult check(IndigoObject item, const char* params);
        CheckResult check(IndigoObject item, int params);
        struct CheckType
        {
            enum CheckTypeCode
            {
                CHECK_NONE = 0x00000000,         // Check none
                CHECK_LOAD = 0x00000001,         // Check loading (correspondence some known format)
                CHECK_VALENCE = 0x00000002,      // Check valence correctness
                CHECK_RADICAL = 0x00000004,      // Check radicals existance
                CHECK_PSEUDOATOM = 0x00000008,   // Check pseudoatoms existance
                CHECK_STEREO = 0x00000010,       // Check strerochemistry description correctness
                CHECK_QUERY = 0x00000020,        // Check query fetaures existance
                CHECK_OVERLAP_ATOM = 0x00000040, // Check overlapping atoms existance
                CHECK_OVERLAP_BOND = 0x00000080, // Check overlapping bonds existance
                CHECK_RGROUP = 0x00000100,       // Check R-groups existance
                CHECK_SGROUP = 0x00000200,       // Check S-groups existance
                CHECK_TGROUP = 0x00000400,       // Check T-groups existance (SCSR features)
                CHECK_CHIRALITY = 0x00000800,    // Check chirality feature correctness (including 3D source)
                CHECK_CHIRAL_FLAG = 0x00001000,  // Check chiral flag existance (MOLFILE format)
                CHECK_3D_COORD = 0x00002000,     // Check 3D coordinates existance
                CHECK_CHARGE = 0x00004000,       // Check charged structure
                CHECK_SALT = 0x00008000,         // Check possible salt structure
                CHECK_AMBIGUOUS_H = 0x00010000,  // Check ambiguous H existance
                CHECK_COORD = 0x00020000,        // Check coordinates existance
                CHECK_V3000 = 0x00040000,        // Check v3000 format
                CHECK_ALL = -1                   // Check all features
            };

            const static char* names[] = {"LOAD",   "VALENCE", "RADICAL",   "PSEUDOATOM",  "STEREO",   "QUERY",  "OVERLAP_ATOM", "OVERLAP_BOND", "RGROUP",
                                          "SGROUP", "TGROUP",  "CHIRALITY", "CHIRAL_FLAG", "3D_COORD", "CHARGE", "SALT",         "AMBIGUOUS_H",  "COORD"};
            const static map<string, CheckTypeCode> checkTypeName = {{"NONE", CHECK_NONE},
                                                                     {names[0], StructureChecker::CHECK_LOAD},
                                                                     {names[1], StructureChecker::CHECK_VALENCE},
                                                                     {names[2], StructureChecker::CHECK_RADICAL},
                                                                     {names[3], StructureChecker::CHECK_PSEUDOATOM},
                                                                     {names[4], StructureChecker::CHECK_STEREO},
                                                                     {names[5], StructureChecker::CHECK_QUERY},
                                                                     {names[6], StructureChecker::CHECK_OVERLAP_ATOM},
                                                                     {names[7], StructureChecker::CHECK_OVERLAP_BOND},
                                                                     {names[8], StructureChecker::CHECK_RGROUP},
                                                                     {names[9], StructureChecker::CHECK_SGROUP},
                                                                     {names[10], StructureChecker::CHECK_TGROUP},
                                                                     {names[11], StructureChecker::CHECK_CHIRALITY},
                                                                     {names[12], StructureChecker::CHECK_CHIRAL_FLAG},
                                                                     {names[13], StructureChecker::CHECK_3D_COORD},
                                                                     {names[14], StructureChecker::CHECK_CHARGE},
                                                                     {names[15], StructureChecker::CHECK_SALT},
                                                                     {names[16], StructureChecker::CHECK_AMBIGUOUS_H},
                                                                     {names[17], StructureChecker::CHECK_COORD},
                                                                     {"ALL", StructureChecker::CHECK_ALL}};
            int fromString(string check);
            string toString(int check);
        };
    };

    struct CheckMessage
    {
        enum CheckMessageCode
        {
            CHECK_MSG_LOAD = 1,
            CHECK_MSG_VALENCE,
            CHECK_MSG_IGNORE_VALENCE_ERROR,
            CHECK_MSG_RADICAL,
            CHECK_MSG_PSEUDOATOM,
            CHECK_MSG_CHIRAL_FLAG,
            CHECK_MSG_WRONG_STEREO,
            CHECK_MSG_3D_STEREO,
            CHECK_MSG_UNDEFINED_STEREO,
            CHECK_MSG_IGNORE_STEREO_ERROR,
            CHECK_MSG_QUERY,
            CHECK_MSG_IGNORE_QUERY_FEATURE,
            CHECK_MSG_OVERLAP_ATOM,
            CHECK_MSG_OVERLAP_BOND,
            CHECK_MSG_RGROUP,
            CHECK_MSG_SGROUP,
            CHECK_MSG_TGROUP,
            CHECK_MSG_CHARGE,
            CHECK_MSG_SALT,
            CHECK_MSG_EMPTY,
            CHECK_MSG_AMBIGUOUS_H,
            CHECK_MSG_3D_COORD,
            CHECK_MSG_ZERO_COORD
        };
        static StructureChecker::CheckMessage message_list[] = {
            {StructureChecker::CHECK_MSG_LOAD, StructureChecker::CHECK_LOAD, "Error at loading structure, wrong format found"},
            {StructureChecker::CHECK_MSG_EMPTY, StructureChecker::CHECK_LOAD, "Input structure is empty"},
            {StructureChecker::CHECK_MSG_VALENCE, StructureChecker::CHECK_VALENCE, "Structure contains atoms with unusuall valence"},
            {StructureChecker::CHECK_MSG_IGNORE_VALENCE_ERROR, StructureChecker::CHECK_VALENCE, "IGNORE_BAD_VALENCE flag is active"},
            {StructureChecker::CHECK_MSG_RADICAL, StructureChecker::CHECK_RADICAL, "Structure contains radicals"},
            {StructureChecker::CHECK_MSG_PSEUDOATOM, StructureChecker::CHECK_PSEUDOATOM, "Structure contains pseudoatoms"},
            {StructureChecker::CHECK_MSG_WRONG_STEREO, StructureChecker::CHECK_STEREO, "Structure contains incorrect stereochemistry"},
            {StructureChecker::CHECK_MSG_QUERY, StructureChecker::CHECK_QUERY, "Structure contains query features"},
            {StructureChecker::CHECK_MSG_OVERLAP_ATOM, StructureChecker::CHECK_OVERLAP_ATOM, "Structure contains overlapping atoms"},
            {StructureChecker::CHECK_MSG_OVERLAP_BOND, StructureChecker::CHECK_OVERLAP_BOND, "Structure contains overlapping bonds."},
            {StructureChecker::CHECK_MSG_RGROUP, StructureChecker::CHECK_RGROUP, "Structure contains R-groups"},
            {StructureChecker::CHECK_MSG_SGROUP, StructureChecker::CHECK_SGROUP, "Structure contains S-groups"},
            {StructureChecker::CHECK_MSG_TGROUP, StructureChecker::CHECK_TGROUP, "Structure contains SCSR templates"},
            {StructureChecker::CHECK_MSG_3D_STEREO, StructureChecker::CHECK_STEREO, "Structure contains stereocenters defined by 3D coordinates"},
            {StructureChecker::CHECK_MSG_UNDEFINED_STEREO, StructureChecker::CHECK_STEREO,
             "Structure contains stereocenters with undefined stereo configuration"},
            {StructureChecker::CHECK_MSG_CHIRAL_FLAG, StructureChecker::CHECK_CHIRAL_FLAG, "Structure contains wrong chiral flag"},
            {StructureChecker::CHECK_MSG_3D_COORD, StructureChecker::CHECK_3D_COORD, "Structure contains 3D coordinates"},
            {StructureChecker::CHECK_MSG_CHARGE, StructureChecker::CHECK_CHARGE, "Structure has non-zero charge"},
            {StructureChecker::CHECK_MSG_SALT, StructureChecker::CHECK_SALT, "Structure contains charged fragments (possible salt)"},
            {StructureChecker::CHECK_MSG_AMBIGUOUS_H, StructureChecker::CHECK_AMBIGUOUS_H, "Structure contains ambiguous hydrogens"},
            {StructureChecker::CHECK_MSG_ZERO_COORD, StructureChecker::CHECK_COORD, "Structure has no atoms coordinates"}};
        toString(CheckMessageCode code);
    }

    struct CheckResult
    {
        int m_id;
        Array<int> atom_ids;
        Array<int> bond_ids;
        toJson(CheckResult result);
        toOutput(CheckResult result, Output& output)
    };

    void checkStructure(Scanner& scanner, const char* params);
    void checkBaseMolecule(BaseMolecule& mol);
    void checkMolecule(Molecule& mol);
    void checkQueryMolecule(QueryMolecule& mol);

    void checkMolecule(BaseMolecule& mol, bool query);

    void parseCheckTypes(const char* params);
    void addAtomSelection(Array<int>& atoms);
    void addBondSelection(Array<int>& bonds);

    void buildCheckResult();

    void clearCheckResult();

    static const char* typeToString(dword check_type);
    static dword getType(const char* check_type);

    dword check_flags;
    dword check_result;

    float mean_dist;

    DECL_ERROR;

protected:
    void _parseSelection(Scanner& sc, Array<int>& ids);
    void _checkAtom(BaseMolecule& mol, Molecule& target, int idx, bool query);
    void _checkBond(BaseMolecule& mol, Molecule& target, int idx, bool query);

    Array<int> _selected_atoms;
    Array<int> _selected_bonds;

    ObjArray<CheckResult> _results;

    Array<int> _bad_val_ids;
    Array<int> _rad_ids;
    Array<int> _atom_qf_ids;
    Array<int> _bond_qf_ids;
    Array<int> _pseudo_ids;
    Array<int> _sg_atom_ids;
    Array<int> _sg_bond_ids;
    Array<int> _atom_3d_ids;
    Array<int> _overlapped_atom_ids;
    Array<int> _overlapped_bond_ids;
    Array<int> _atom_amb_h_ids;
    Array<int> _atom_3d_stereo_ids;
    Array<int> _atom_wrong_stereo_ids;
    Array<int> _atom_undefined_stereo_ids;

    Output& _output;

private:
    StructureChecker(const StructureChecker&); // no implicit copy
};                                             // namespace indigo

} // namespace indigo

#endif
*/
