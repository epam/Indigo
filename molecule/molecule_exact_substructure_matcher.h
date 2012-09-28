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

#ifndef __molecule_exact_substructure_matcher__
#define __molecule_exact_substructure_matcher__

#include "base_cpp/obj.h"
#include "graph/embedding_enumerator.h"
#include "graph/graph_decomposer.h"

namespace indigo {

class Molecule;

class MoleculeExactSubstructureMatcher
{
public:
   MoleculeExactSubstructureMatcher (Molecule &query, Molecule &target);

   bool find ();

   dword flags;

   DECL_ERROR;
protected:
   Molecule &_query;
   Molecule &_target;
   EmbeddingEnumerator _ee;
   Obj<GraphDecomposer> _query_decomposer;
   Obj<GraphDecomposer> _target_decomposer;

   struct _MatchToken
   {
      bool compare (const char *text) const;

      const char *t_text;
      int         t_flag;
   };

   static bool _matchAtoms (Graph &subgraph, Graph &supergraph,
                            const int *core_sub, int sub_idx, int super_idx, void *userdata);
   static bool _matchBonds (Graph &subgraph, Graph &supergraph,
                            int sub_idx, int super_idx, void *userdata);
   static int _embedding (Graph &subgraph, Graph &supergraph,
                          int *core_sub, int *core_super, void *userdata);

   void _collectConnectedComponentsInfo ();
private:
   MoleculeExactSubstructureMatcher (const MoleculeExactSubstructureMatcher &);

};

}

#endif
