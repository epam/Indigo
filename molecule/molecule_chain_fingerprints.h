/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __molecule_chain_fingerprints__
#define __molecule_chain_fingerprints__

#include "base_cpp/tlscont.h"
#include "graph/graph_subchain_enumerator.h"

namespace indigo {

struct MoleculeChainFingerprintParameters
{
   MoleculeChainFingerprintParameters ()
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
   MoleculeChainFingerprintBuilder (Molecule &mol, const MoleculeChainFingerprintParameters &parameters);

   void process ();

   const byte * get ();

   DECL_ERROR;
protected:

   static void _handleChain (Graph &graph, int size, const int *vertices, const int *edges, void *context);

   Molecule &_mol;
   const MoleculeChainFingerprintParameters &_parameters;

   TL_CP_DECL(Array<byte>, _fingerprint);

private:
   MoleculeChainFingerprintBuilder (const MoleculeChainFingerprintBuilder &); // no implicit copy
};

}

#endif
