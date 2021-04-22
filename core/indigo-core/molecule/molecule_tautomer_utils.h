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

#ifndef __molecule_tautomer_utils__
#define __molecule_tautomer_utils__

#include "molecule/elements.h"
#include "molecule/molecule.h"

namespace indigo
{

    class Graph;
    class Molecule;

    class MoleculeTautomerUtils
    {
    public:
        static void countHReplacements(BaseMolecule& g, Array<int>& h_rep_count);

        static void highlightChains(BaseMolecule& g1, BaseMolecule& g2, const Array<int>& chains_2, const int* core_2);

    private:
        static bool _isRepMetal(int elem);
    };

} // namespace indigo

#endif
