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
#include <unordered_set>
#include <vector>

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
            CHECK_RADICAL = 0x00000004,      // Check radicals existence
            CHECK_PSEUDOATOM = 0x00000008,   // Check pseudoatoms existence
            CHECK_STEREO = 0x00000010,       // Check stereochemistry description correctness
            CHECK_QUERY = 0x00000020,        // Check query features existence
            CHECK_OVERLAP_ATOM = 0x00000040, // Check overlapping atoms existence
            CHECK_OVERLAP_BOND = 0x00000080, // Check overlapping bonds existence
            CHECK_RGROUP = 0x00000100,       // Check R-groups existence
            CHECK_SGROUP = 0x00000200,       // Check S-groups existence
            CHECK_TGROUP = 0x00000400,       // Check T-groups existence (SCSR features)
            CHECK_CHIRALITY = 0x00000800,    // Check chirality feature correctness (including 3D source)
            CHECK_CHIRAL_FLAG = 0x00001000,  // Check chiral flag existence (MOLFILE format)
            CHECK_3D_COORD = 0x00002000,     // Check 3D coordinates existence
            CHECK_CHARGE = 0x00004000,       // Check charged structure
            CHECK_SALT = 0x00008000,         // Check possible salt structure
            CHECK_AMBIGUOUS_H = 0x00010000,  // Check ambiguous H existence
            CHECK_COORD = 0x00020000,        // Check coordinates existence
            CHECK_V3000 = 0x00040000,        // Check v3000 format
            CHECK_ALL = -1                   // Check all features (default)
        };

        /// <summary>
        /// Textual Check Type values for the corresponding CheckTypeCode
        /// to be used in host language calls (Java, Python etc.) as a comma-separated list
        /// See below check(char *, char *):
        /// check("molecule text", "load, valence, radical, atoms 1 2 3, bonds 4 5 6, tgroup"
        /// is equivalent to
        /// check(the_molecule, check_flags = CHECK_LOAD | CHECK_VALECE | CHECK_RADICAL | CHECK_TGROUP, selected_atoms=[1,2,3], selected_bonds=[4,5,6]);
        /// </summary>
        static constexpr char* CHECK_NONE_TXT = "none";                 // Check none
        static constexpr char* CHECK_LOAD_TXT = "load";                 // Check loading (correspondence some known format)
        static constexpr char* CHECK_VALENCE_TXT = "valence";           // Check valence correctness
        static constexpr char* CHECK_RADICAL_TXT = "radical";           // Check radicals existence
        static constexpr char* CHECK_PSEUDOATOM_TXT = "pseudoatom";     // Check pseudoatoms existence
        static constexpr char* CHECK_STEREO_TXT = "stereo";             // Check stereochemistry description correctness
        static constexpr char* CHECK_QUERY_TXT = "query";               // Check query features existence
        static constexpr char* CHECK_OVERLAP_ATOM_TXT = "overlap_atom"; // Check overlapping atoms existence
        static constexpr char* CHECK_OVERLAP_BOND_TXT = "overlap_bond"; // Check overlapping bonds existence
        static constexpr char* CHECK_RGROUP_TXT = "rgroup";             // Check R-groups existence
        static constexpr char* CHECK_SGROUP_TXT = "sgroup";             // Check S-groups existence
        static constexpr char* CHECK_TGROUP_TXT = "tgroup";             // Check T-groups existence (SCSR features)
        static constexpr char* CHECK_CHIRALITY_TXT = "chirality";       // Check chirality feature correctness (including 3D source)
        static constexpr char* CHECK_CHIRAL_FLAG_TXT = "chiral_flag";   // Check chiral flag existence (MOLFILE format)
        static constexpr char* CHECK_3D_COORD_TXT = "3d_coord";         // Check 3D coordinates existence
        static constexpr char* CHECK_CHARGE_TXT = "charge";             // Check charged structure
        static constexpr char* CHECK_SALT_TXT = "salt";                 // Check possible salt structure
        static constexpr char* CHECK_AMBIGUOUS_H_TXT = "ambigous_h";    // Check ambiguous H existence
        static constexpr char* CHECK_COORD_TXT = "coord";               // Check coordinates existence
        static constexpr char* CHECK_V3000_TXT = "v3000";               // Check v3000 format
        static constexpr char* CHECK_ALL_TXT = "all";                   // Check all features (default)

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
