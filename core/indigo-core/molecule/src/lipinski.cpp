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

#include "molecule/lipinski.h"

#include "molecule/elements.h"
#include "molecule/molecule.h"

namespace indigo
{
    int Lipinski::getNumRotatableBonds(Molecule& molecule)
    {
        int result = 0;
        for (auto i = molecule.edgeBegin(); i < molecule.edgeEnd(); i = molecule.edgeNext(i))
        {
            if (molecule.getBondOrder(i) == 1 && molecule.getEdgeTopology(i) != TOPOLOGY_RING && !molecule.isTerminalEdge(i))
            {
                ++result;
            }
        }
        return result;
    }

    int Lipinski::getNumHydrogenBondAcceptors(Molecule& molecule)
    {
        int result = 0;
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            if (molecule.getAtomNumber(i) == ELEM_N || molecule.getAtomNumber(i) == ELEM_O)
            {
                ++result;
            }
        }
        return result;
    }

    int Lipinski::getNumHydrogenBondDonors(Molecule& molecule)
    {
        int result = 0;
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            if (molecule.getAtomNumber(i) == ELEM_N || molecule.getAtomNumber(i) == ELEM_O)
            {
                result += molecule.getAtomTotalH(i);
            }
        }
        return result;
    }
}
