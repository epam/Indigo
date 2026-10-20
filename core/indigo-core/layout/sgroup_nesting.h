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

#ifndef __sgroup_nesting_h__
#define __sgroup_nesting_h__

#include <map>
#include <vector>

#include "base_c/defs.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;

    // The S-groups that get brackets and how they nest: the brackets of a parent enclose those of its children
    class DLLEXPORT SGroupNesting
    {
    public:
        explicit SGroupNesting(BaseMolecule& molecule);

        const std::vector<int>& bracketedDeepestFirst() const;
        std::vector<int> children(int sgroup_idx) const;

        static bool isBracketed(int sgroup_type);

    private:
        int _findParent(int sgroup_idx) const;
        bool _contains(int sgroup_idx, const std::vector<int>& sorted_atoms) const;
        int _depth(int sgroup_idx) const;

        BaseMolecule& _molecule;
        std::vector<int> _bracketed;
        std::map<int, int> _parents;
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
