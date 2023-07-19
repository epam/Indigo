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

#ifndef __structure_checker__
#define __structure_checker__

#include "base_cpp/exception.h"
#include <string>
#include <vector>

namespace indigo
{

    class BaseMolecule;
    class BaseReaction;

    class DLLEXPORT StructureChecker
    {
    public:
        enum class CheckTypeCode
        {
            CHECK_NONE,         // Check nothing
            CHECK_LOAD,         // Check loading (correspondence some known format)
            CHECK_VALENCE,      // Check valence correctness
            CHECK_RADICAL,      // Check radicals existence
            CHECK_PSEUDOATOM,   // Check pseudoatoms existence
            CHECK_STEREO,       // Check stereochemistry description correctness
            CHECK_QUERY,        // Check query features existence
            CHECK_OVERLAP_ATOM, // Check overlapping atoms existence
            CHECK_OVERLAP_BOND, // Check overlapping bonds existence
            CHECK_RGROUP,       // Check R-groups existence
            CHECK_SGROUP,       // Check S-groups existence
            CHECK_TGROUP,       // Check T-groups existence (SCSR features)
            CHECK_CHIRALITY,    // Check chirality feature correctness (including 3D source)
            CHECK_CHIRAL_FLAG,  // Check chiral flag existence (MOLFILE format)
            CHECK_3D_COORD,     // Check 3D coordinates existence
            CHECK_CHARGE,       // Check charged structure
            CHECK_SALT,         // Check possible salt structure
            CHECK_AMBIGUOUS_H,  // Check ambiguous H existence
            CHECK_COORD,        // Check coordinates existence
            CHECK_V3000         // Check v3000 format
        };

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
            CHECK_MSG_QUERY,
            CHECK_MSG_QUERY_ATOM,
            CHECK_MSG_QUERY_BOND,
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
            CHECK_MSG_SALT_NOT_IMPL,
            CHECK_MSG_V3000
        };

        struct DLLEXPORT CheckMessage;

        class DLLEXPORT CheckResult
        {
        public:
            bool isEmpty() const;
            std::vector<CheckMessage> messages;
        };

        struct DLLEXPORT CheckMessage
        {
            CheckMessage();
            CheckMessage(CheckMessageCode _code, int _index, const std::vector<int>& _ids, const CheckResult& _subresult);
            CheckMessageCode code = StructureChecker::CheckMessageCode::CHECK_MSG_NONE;
            std::string message();
            std::string prefix;
            int index = -1;
            std::vector<int> ids;
            CheckResult subresult;
        };

        StructureChecker();

        CheckResult checkMolecule(const BaseMolecule& item, const std::string& check_types_and_selections = "");
        CheckResult checkMolecule(const BaseMolecule& item, const std::string& check_types, const std::vector<int>& selected_atoms,
                                  const std::vector<int>& selected_bonds);
        CheckResult checkMolecule(const BaseMolecule& item, const std::vector<CheckTypeCode>& check_types = std::vector<CheckTypeCode>(),
                                  const std::vector<int>& selected_atoms = std::vector<int>(), const std::vector<int>& selected_bonds = std::vector<int>());

        CheckResult checkReaction(const BaseReaction& reaction, const std::string& check_types = "");
        CheckResult checkReaction(const BaseReaction& reaction, const std::vector<CheckTypeCode>& check_types = std::vector<CheckTypeCode>());

        static CheckTypeCode getCheckType(const std::string& type);
        static std::string getCheckType(StructureChecker::CheckTypeCode code);
        static std::string getCheckMessage(StructureChecker::CheckMessageCode code);
        static CheckTypeCode getCheckTypeByMsgCode(StructureChecker::CheckMessageCode code);

        DECL_ERROR;

    private:
        StructureChecker(const StructureChecker&); // no implicit copy
    };

} // namespace indigo

#endif
