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

#ifndef __indigo_structure_checker__
#define __indigo_structure_checker__

#include "base_c/defs.h"
#include "base_cpp/exception.h"
#include "indigo_internal.h"
#include "molecule/structure_checker2.h"
#include <string>
#include <unordered_set>
#include <vector>


namespace indigo
{


    class DLLEXPORT IndigoStructureChecker : public StructureChecker2
    {
    public:
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

        IndigoStructureChecker();

        CheckResult check(const char* item, const char* check_flags = 0, const char* load_params = 0);
        CheckResult check(int item, const char* check_flags);
        CheckResult check(int item, int check_flags = StructureChecker2::CHECK_ALL, const std::vector<int>& selected_atoms = std::vector<int>(),
                          const std::vector<int>& selected_bonds = std::vector<int>());
        CheckResult check(const IndigoObject& item, int check_flags = StructureChecker2::CHECK_ALL, const std::vector<int>& selected_atoms = std::vector<int>(),
                          const std::vector<int>& selected_bonds = std::vector<int>());
        std::string toJson(const StructureChecker2::CheckResult& res);

        DECL_ERROR;

    private:
        IndigoStructureChecker(const IndigoStructureChecker&); // no implicit copy
    };                                               // namespace indigo

} // namespace indigo

#endif
