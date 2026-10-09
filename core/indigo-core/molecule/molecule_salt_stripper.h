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

#ifndef __molecule_salt_stripper_h__
#define __molecule_salt_stripper_h__

#include "base_cpp/ptr_array.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_neighbourhood_counters.h"
#include "molecule/query_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class Molecule;

    // Finds and removes the salts of a molecule (#3927): its components that are a lone
    // ion or a small inorganic species. Components are those of
    // BaseMolecule::moleculeComponents(), so nothing bonds a salt to the rest. A
    // component that has haptic bonds is a coordination compound and never a salt: the
    // patterns count ordinary bonds only, and to them its metal is a lone ion.
    class DLLEXPORT MoleculeSaltStripper
    {
    public:
        explicit MoleculeSaltStripper(const AromaticityOptions& arom_options);

        bool hasSalts(Molecule& mol);
        // Returns whether an atom was removed.
        bool strip(Molecule& mol);

    private:
        // Every atom of a pattern has all its bonds counted by X, so a match is a whole
        // connected piece of the graph, never a part of a larger molecule.
        static constexpr const char* SALT_PATTERNS[] = {
            // inorganics with quaternary atom
            "[X4&!#1&!#6](~[X1&!#6])(~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
            "[X4&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])(~[X2H&!#6])~[X1&!#6]",
            "[X4&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])(~[X1&!#6])~[X1&!#6]",
            "[X4&!#1&!#6](~[X2H&!#6])(~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
            // inorganics with tertiary atom
            "[X3&!#1&!#6](~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
            "[X3&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])~[X1&!#6]",
            "[X3&!#1&!#6](~[X2H&!#6])(~[X1&!#6])~[X1&!#6]",
            // inorganics with secondary atom
            "[X2&!#1&!#6](~[X1&!#6])~[X1&!#6]",
            "[X2&!#1&!#6](~[X2H&!#6])~[X1&!#6]",
            // inorganics with primary atom
            "[X1&!#1&!#6]~[X1&!#6]",
            // single-element ions
            "[X0+1]",
            "[X0+2]",
            "[X0+3]",
            "[X0+4]",
            "[X0-1]",
            "[X0-2]",
        };

        struct Pattern
        {
            QueryMolecule query;
            MoleculeAtomNeighbourhoodCounters counters;
        };

        bool _isSalt(Molecule& mol, int component);

        const AromaticityOptions _arom_options;
        PtrArray<Pattern> _patterns;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
