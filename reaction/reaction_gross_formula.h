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

#ifndef __reaction_gross_formula__
#define __reaction_gross_formula__

#include <utility>

#include "base_cpp/array.h"
#include "base_cpp/ptr_array.h"
#include "molecule/molecule_gross_formula.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseReaction;

    class DLLEXPORT ReactionGrossFormula
    {
    public:
        static std::unique_ptr<std::pair<PtrArray<GROSS_UNITS>, PtrArray<GROSS_UNITS>>> collect(BaseReaction& rxn, bool add_isotopes = false);
        static void toString_Hill(std::pair<PtrArray<GROSS_UNITS>, PtrArray<GROSS_UNITS>>& gross, Array<char>& str, bool add_rsites);
    };

} // namespace indigo

#endif
