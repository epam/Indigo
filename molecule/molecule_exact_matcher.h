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

#ifndef __molecule_exact_matcher__
#define __molecule_exact_matcher__

#include "base_cpp/obj.h"
#include "graph/embedding_enumerator.h"
#include "graph/graph_decomposer.h"

namespace indigo {

class Molecule;

class MoleculeExactMatcher
{
public:
   enum
   {
      // Conditions
      CONDITION_NONE      = 0x0000,
      CONDITION_ELECTRONS = 0x0001, // bond types, atom charges, valences, radicals must match
      CONDITION_ISOTOPE   = 0x0002, // atom isotopes must match
      CONDITION_STEREO    = 0x0004, // tetrahedral and cis-trans configurations must match
      CONDITION_FRAGMENTS = 0x0008, // query fragments count must be equal to target fragments count
      CONDITION_ALL       = 0x000F, // all but 3D
      CONDITION_3D        = 0x0010  // atom positions must match up to affine+scale transformation
   };

   MoleculeExactMatcher (BaseMolecule &query, BaseMolecule &target);

   bool find ();

   const int * getQueryMapping ();

   static void parseConditions (const char *params, int &flags, float &rms_threshold);

   static bool matchAtoms (BaseMolecule& query, BaseMolecule& target, int sub_idx, int super_idx, int flags);
   static bool matchBonds (BaseMolecule& query, BaseMolecule& target, int sub_idx, int super_idx, int flags);

   int   flags;
   float rms_threshold; // for affine match

   bool needCoords ();

   DECL_ERROR;
protected:
   BaseMolecule &_query;
   BaseMolecule &_target;
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
   MoleculeExactMatcher (const MoleculeExactMatcher &);

};

}

#endif
