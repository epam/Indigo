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
            CHECK_ALL = -1                   // Check all features
        };

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

        struct DLLEXPORT CheckMessage
        {
            CheckMessageCode code;
            std::string message;
        };

        struct DLLEXPORT CheckResult
        {
            int error;
            std::vector<CheckMessage> messages;
            const char* toJson();
            const char* toYaml();
        };

        StructureChecker2();

        CheckResult check(const char* item, const char* params);
        CheckResult check(int item, const char* params);
        CheckResult check(const IndigoObject& item, const char* params);
        CheckResult check(const IndigoObject& item, int params);

        void checkMolecule(const BaseMolecule& mol, CheckResult& result);

        DECL_ERROR;

    private:
        StructureChecker2(const StructureChecker2&); // no implicit copy
    };                                               // namespace indigo

} // namespace indigo

#endif
