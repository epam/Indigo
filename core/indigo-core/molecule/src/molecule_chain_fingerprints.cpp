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

#include "molecule/molecule_chain_fingerprints.h"

#include "base_cpp/crc32.h"
#include "graph/graph_subchain_enumerator.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_fingerprint.h"

using namespace indigo;

IMPL_ERROR(MoleculeChainFingerprintBuilder, "chain fingerprint builder");

CP_DEF(MoleculeChainFingerprintBuilder);

MoleculeChainFingerprintBuilder::MoleculeChainFingerprintBuilder(Molecule& mol, const MoleculeChainFingerprintParameters& parameters)
    : _mol(mol), _parameters(parameters), CP_INIT, TL_CP_GET(_fingerprint)
{
}

void MoleculeChainFingerprintBuilder::process()
{
    QS_DEF(Molecule, copy_without_h);
    QS_DEF(Array<int>, vertices);
    int i;

    vertices.clear();

    // remove hydrogens
    for (i = _mol.vertexBegin(); i < _mol.vertexEnd(); i = _mol.vertexNext(i))
        if (_mol.getAtomNumber(i) != ELEM_H)
            vertices.push(i);

    copy_without_h.makeSubmolecule(_mol, vertices, 0);

    GraphSubchainEnumerator gse(copy_without_h, _parameters.min_edges, _parameters.max_edges, _parameters.mode);

    gse.context = this;
    gse.cb_handle_chain = _handleChain;

    _fingerprint.clear_resize(_parameters.size_qwords * 8);

    _fingerprint.zerofill();
    gse.processChains();
}

void MoleculeChainFingerprintBuilder::_handleChain(Graph& graph, int size, const int* vertices, const int* edges, void* context)
{
    MoleculeChainFingerprintBuilder* self = (MoleculeChainFingerprintBuilder*)context;
    QS_DEF(Array<char>, str);
    QS_DEF(Array<char>, rev_str);
    int i;

    str.clear();
    rev_str.clear();
    for (i = 0; i <= size; i++)
    {
        int number = self->_mol.getAtomNumber(vertices[i]);

        str.push(number & 0xFF);

        if (i == size)
            break;

        int order = self->_mol.getBondOrder(edges[i]);

        switch (order)
        {
        case BOND_SINGLE:
            str.push('-');
            break;
        case BOND_DOUBLE:
            str.push('=');
            break;
        case BOND_TRIPLE:
            str.push('#');
            break;
        default:
            str.push(':');
        }
    }

    int len = str.size();

    for (i = 0; i < len; i++)
        rev_str.push(str[len - i - 1]);

    str.push(0);
    rev_str.push(0);

    unsigned seed;

    unsigned seed1 = CRC32::get(str.ptr());
    unsigned seed2 = CRC32::get(rev_str.ptr());

    seed = std::min(seed1, seed2);

    for (int i = 0; i < self->_parameters.bits_per_chain; i++)
    {
        seed = seed * 0x8088405 + 1;

        int k = seed % (self->_fingerprint.size() * 8);
        int nbyte = k / 8;
        int nbit = k % 8;

        self->_fingerprint[nbyte] |= (1 << nbit);
    }
}

const byte* MoleculeChainFingerprintBuilder::get()
{
    return _fingerprint.ptr();
}
