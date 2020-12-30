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

#ifndef __structure_checker2__
#define __structure_checker2__

#include "base_c/defs.h"
#include "base_cpp/exception.h"
#include <string>
#include <vector>
#include <unordered_set>

class IndigoObject;

namespace indigo
{

    class BaseMolecule;

    class DLLEXPORT StructureChecker2
    {
    public:
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
            CHECK_ALL = -1                   // Check all features (default)
        };

        static constexpr const char* checkTypeName[] = {"none",         "load",         "valence", "radical",    "pseudoatom", "stereo",    "query",
                                                        "overlap_atom", "overlap_bond", "rgroup",  "sgroup",     "tgroup",     "chirality", "chiral_flag",
                                                        "3d_coord",     "charge",       "salt",    "ambigous_h", "coord",      "v3000",     "all"};
        // See below check(char *, char *):
        // check("molecule text", "load, valence, radical, atoms 1 2 3, bonds 4 5 6, tgroup"
        // is equivalent to
        // check(the_molecule, check_flags = CHECK_LOAD | CHECK_VALECE | CHECK_RADICAL | CHECK_TGROUP, selected_atoms=[1,2,3], selected_bonds=[4,5,6]);

        enum class CheckMessageCode
        {
            CHECK_MSG_NONE,
            CHECK_MSG_LOAD,
            CHECK_MSG_VALENCE,
            CHECK_MSG_VALENCE_NOT_CHECKED_QUERY,
            CHECK_MSG_VALENCE_NOT_CHECKED_RGROUP,
            CHECK_MSG_IGNORE_VALENCE_ERROR,
            CHECK_MSG_RADICAL,
            CHECK_MSG_RADICAL_NOT_CHECKED_PSEUDO,
            CHECK_MSG_PSEUDOATOM,
            CHECK_MSG_CHIRAL_FLAG,
            CHECK_MSG_WRONG_STEREO,
            CHECK_MSG_3D_STEREO,
            CHECK_MSG_UNDEFINED_STEREO,
            CHECK_MSG_IGNORE_STEREO_ERROR,
            CHECK_MSG_QUERY,
            CHECK_MSG_QUERY_ATOM,
            CHECK_MSG_QUERY_BOND,
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
            CHECK_MSG_AMBIGUOUS_H_NOT_CHECKED_QUERY,
            CHECK_MSG_3D_COORD,
            CHECK_MSG_ZERO_COORD,
            CHECK_MSG_REACTION,
            CHECK_MSG_CHIRALITY,
            CHECK_MSG_CHARGE_NOT_IMPL,
            CHECK_MSG_SALT_NOT_IMPL,
            CHECK_MSG_V3000
        };

        struct DLLEXPORT CheckMessage;

        class DLLEXPORT CheckResult
        {
        public:
            bool isEmpty() const;
            void message(CheckMessageCode code, int index, const CheckResult& subresult);
            void message(CheckMessageCode code, const std::vector<int>& ids);
            void message(CheckMessageCode code, const std::unordered_set<int>& ids);
            void message(CheckMessageCode code);
            std::string toJson();
            std::vector<CheckMessage> messages;
        };

        struct DLLEXPORT CheckMessage
        {
            CheckMessageCode code = StructureChecker2::CheckMessageCode::CHECK_MSG_NONE;
            std::string message;
            int index = -1;
            std::vector<int> ids;
            CheckResult subresult;
        };

        StructureChecker2();

        CheckResult check(const char* item, const char* check_flags = 0, const char* load_params = 0);
        CheckResult check(int item, const char* check_flags);
        CheckResult check(int item, int check_flags = CHECK_ALL, const std::vector<int>& selected_atoms = std::vector<int>(),
                          const std::vector<int>& selected_bonds = std::vector<int>());
        CheckResult check(const IndigoObject& item, int check_flags = CHECK_ALL, const std::vector<int>& selected_atoms = std::vector<int>(),
                          const std::vector<int>& selected_bonds = std::vector<int>());

        DECL_ERROR;

    private:
        StructureChecker2(const StructureChecker2&); // no implicit copy
    };                                               // namespace indigo

} // namespace indigo

#endif
