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

#ifndef __molecule_mass_h__
#define __molecule_mass_h__
#include "base_cpp/red_black.h"
#include "molecule/molecule_mass_options.h"

#include <map>
#include <set>

namespace indigo
{

    class Molecule;

    // Molecular mass calculation
    class MoleculeMass
    {
        DECL_ERROR;

        const double* _relativeAtomicMassByNumber(int number) const;

    protected:
        struct _ElemCounter
        {
            int elem;
            double weight;
        };

        static int _cmp(_ElemCounter& ec1, _ElemCounter& ec2, void* context);

    public:
        MoleculeMass();
        MassOptions mass_options;

        const std::map<int, double>* relative_atomic_mass_map;

        /* Mass of a molecule calculated using the average mass of each
         * element weighted for its natural isotopic abundance
         */
        double molecularWeight(Molecule& mol);

        /* Mass of a molecule containing most likely
         * isotopic composition for a single random molecule.
         * Notes: in PubChem search engine it is called Exact Mass
         */
        double mostAbundantMass(Molecule& mol);

        /* Mass of a molecule calculated using the mass of
         * the most abundant isotope of each element.
         * Notes: in Marvin it is called Exact Mass
         */
        double monoisotopicMass(Molecule& mol);

        /* Sum of the mass numbers of all constituent atoms.
         */
        int nominalMass(Molecule& mol);

        /* Atom weight percentage like "C 77% H 13%"
         */
        void massComposition(Molecule& molecule, Array<char>& str);
    };

} // namespace indigo

#endif // __molecule_mass_h__
