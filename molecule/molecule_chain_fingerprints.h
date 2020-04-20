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

#ifndef __molecule_chain_fingerprints__
#define __molecule_chain_fingerprints__

#include "base_cpp/tlscont.h"
#include "graph/graph_subchain_enumerator.h"

namespace indigo
{
    struct MoleculeChainFingerprintParameters
    {
        MoleculeChainFingerprintParameters()
        {
            size_qwords = 128;
            min_edges = 1;
            max_edges = 7;
            bits_per_chain = 4;
            mode = GraphSubchainEnumerator::MODE_NO_DUPLICATE_VERTICES;
        }

        int size_qwords; // size in bytes = size_qwords * 8
        int min_edges;
        int max_edges;
        int bits_per_chain;
        int mode; // one of GraphSubchainEnumerator::MODE_XXX
    };

    class Molecule;
    class Graph;

    class MoleculeChainFingerprintBuilder
    {
    public:
        MoleculeChainFingerprintBuilder(Molecule& mol, const MoleculeChainFingerprintParameters& parameters);

        void process();

        const byte* get();

        DECL_ERROR;

    protected:
        static void _handleChain(Graph& graph, int size, const int* vertices, const int* edges, void* context);

        Molecule& _mol;
        const MoleculeChainFingerprintParameters& _parameters;

        CP_DECL;
        TL_CP_DECL(Array<byte>, _fingerprint);

    private:
        MoleculeChainFingerprintBuilder(const MoleculeChainFingerprintBuilder&); // no implicit copy
    };

} // namespace indigo

#endif
