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

#include "molecule/molecule_hash.h"

#include "graph/subgraph_hash.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"

namespace indigo
{
    dword MoleculeHash::calculate(Molecule& mol)
    {
        QS_DEF(Molecule, mol_without_h);
        QS_DEF(Array<int>, vertices);
        int i;

        vertices.clear();

        for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            if (mol.getAtomNumber(i) != ELEM_H)
                vertices.push(i);

        mol_without_h.makeSubmolecule(mol, vertices, 0);

        QS_DEF(Array<int>, vertex_codes);
        vertex_codes.clear_resize(mol_without_h.vertexEnd());

        SubgraphHash hh(mol_without_h);

        for (int v = mol_without_h.vertexBegin(); v != mol_without_h.vertexEnd(); v = mol_without_h.vertexNext(v))
            vertex_codes[v] = mol_without_h.atomCode(v);
        hh.vertex_codes = &vertex_codes;
        hh.max_iterations = (mol_without_h.edgeCount() + 1) / 2;

        return hh.getHash();
    }
}
