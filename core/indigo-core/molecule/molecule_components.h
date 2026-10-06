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

#ifndef __molecule_components__
#define __molecule_components__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

// The components the public API reports for a molecule with haptic bonds: #3927.

namespace indigo
{
    class BaseMolecule;

    // Components in which a haptic bond joins atoms the way an edge does: a ferrocene
    // is one here and three to Graph::countComponents(). S-groups join nothing, and
    // neither does an attachment group no bond refers to. Components are numbered by
    // their lowest atom, as the graph numbers its own.
    class DLLEXPORT MoleculeComponents
    {
    public:
        DECL_ERROR;

        MoleculeComponents() = default;

        MoleculeComponents(const MoleculeComponents&) = delete;
        MoleculeComponents& operator=(const MoleculeComponents&) = delete;

        void build(const BaseMolecule& mol);

        int count() const;
        int componentOf(int atom) const;
        int atomCount(int component) const;
        int bondCount(int component) const; // edges; haptic bonds are counted apart
        bool isLoneAtom(int atom) const;
        void collectAtoms(int component, Array<int>& atoms) const; // in ascending order

    private:
        void _checkComponent(int component) const;

        Array<int> _component_of; // by atom index; -1 where there is no atom
        Array<int> _atom_count;
        Array<int> _bond_count;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
