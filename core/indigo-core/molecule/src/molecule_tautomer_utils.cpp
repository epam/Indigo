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

#include "molecule/molecule_tautomer_utils.h"

using namespace indigo;

bool MoleculeTautomerUtils::_isRepMetal(int elem)
{
    static const int list[] = {ELEM_Li, ELEM_Na, ELEM_K, ELEM_Rb, ELEM_Cs, ELEM_Be, ELEM_Mg, ELEM_Ca, ELEM_Sr, ELEM_Ba};

    for (int i = 0; i < NELEM(list); i++)
        if (elem == list[i])
            return true;

    return false;
}

// Count potential hydrogens (+, - charges or metal bonds)
void MoleculeTautomerUtils::countHReplacements(BaseMolecule& m, Array<int>& h_rep_count)
{

    h_rep_count.clear_resize(m.vertexEnd());

    for (int i : m.vertices())
    {
        const Vertex& vertex = m.getVertex(i);

        h_rep_count[i] = 0;

        for (int bond_idx : vertex.neighbors())
        {
            if (_isRepMetal(m.getAtomNumber(vertex.neiVertex(bond_idx))))
            {
                int bond_type = m.getBondOrder(vertex.neiEdge(bond_idx));

                if (bond_type != BOND_AROMATIC)
                    h_rep_count[i] += bond_type;
            }
        }

        // + or - charge also count as potential hydrogens
        int charge = m.getAtomCharge(i);

        if (charge != CHARGE_UNKNOWN)
            h_rep_count[i] += abs(charge);
    }
}

// If core_2 != 0 highlights g1 too
void MoleculeTautomerUtils::highlightChains(BaseMolecule& g1, BaseMolecule& g2, const Array<int>& chains_2, const int* core_2)
{
    int i;

    for (i = g2.vertexBegin(); i < g2.vertexEnd(); i = g2.vertexNext(i))
    {
        if (chains_2[i] > 0 || (core_2 != 0 && core_2[i] >= 0))
            g2.highlightAtom(i);
    }

    for (i = g2.edgeBegin(); i < g2.edgeEnd(); i = g2.edgeNext(i))
    {
        const Edge& edge = g2.getEdge(i);

        // zeroed bond?
        if (g2.getBondOrder(i) == -1 && g2.possibleBondOrder(i, BOND_SINGLE))
            continue;

        if (chains_2[edge.beg] > 0 && chains_2[edge.end] > 0 && abs(chains_2[edge.beg] - chains_2[edge.end]) == 1)
            g2.highlightBond(i);
        else if (core_2 != 0 && core_2[edge.beg] >= 0 && core_2[edge.end] >= 0)
        {
            if (g1.findEdgeIndex(core_2[edge.beg], core_2[edge.end]) != -1)
                g2.highlightBond(i);
        }
    }
}
