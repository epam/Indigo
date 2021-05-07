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

#include "base_cpp/exception.h"
#include "indigo_internal.h"
#include "molecule/structure_checker.h"
#include <string>
#include <vector>

namespace indigo
{

    class DLLEXPORT IndigoStructureChecker : public StructureChecker
    {
    public:
        IndigoStructureChecker();

        CheckResult check(const char* item, const char* check_types = 0, const char* load_params = 0);

        CheckResult check(int item, const char* check_types_and_selections);
        CheckResult check(int item, const char* check_types, const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds);
        CheckResult check(int item, const std::vector<CheckTypeCode>& check_types = std::vector<CheckTypeCode>(),
                          const std::vector<int>& selected_atoms = std::vector<int>(), const std::vector<int>& selected_bonds = std::vector<int>());

        CheckResult check(const IndigoObject& item, const std::vector<CheckTypeCode>& check_types = std::vector<CheckTypeCode>(),
                          const std::vector<int>& selected_atoms = std::vector<int>(), const std::vector<int>& selected_bonds = std::vector<int>());

        std::string toJson(const StructureChecker::CheckResult& res);
        
        DECL_ERROR;

    private:
        IndigoStructureChecker(const IndigoStructureChecker&); // no implicit copy
    };                                                         // namespace indigo

} // namespace indigo

#endif
