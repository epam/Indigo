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

namespace indigo
{
    class BaseMolecule;
    class Filter;

    // The pieces a molecule falls into when a haptic bond holds atoms together the
    // way an ordinary bond does (#3927). Graph::countComponents() follows edges only,
    // and a haptic bond is not an edge, so a ferrocene is three pieces to the graph
    // and one here. S-groups join nothing: they annotate atoms, they do not bond them.
    //
    // Numbering matches the graph's: components are ordered by their lowest atom
    // index, so a molecule without haptic bonds decomposes exactly as its graph.
    class DLLEXPORT MoleculeComponents
    {
    public:
        DECL_ERROR;

        explicit MoleculeComponents(BaseMolecule& mol);

        MoleculeComponents(const MoleculeComponents&) = delete;
        MoleculeComponents& operator=(const MoleculeComponents&) = delete;

        int count() const;
        int componentOf(int atom) const;
        int atomCount(int component) const;

        // Ordinary bonds only, as BaseMolecule::edgeCount() counts them; the haptic
        // bonds of a component are not among them.
        int bondCount(int component) const;

        // Points `filter` at the atoms of `component`, for makeSubmolecule(). The
        // filter reads this object, so it is valid only until the molecule is edited.
        void selectAtoms(int component, Filter& filter) const;

    private:
        void _checkComponent(int component) const;

        Array<int> _component_of; // per atom index; -1 where no atom is
        Array<int> _atom_count;
        Array<int> _bond_count;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
