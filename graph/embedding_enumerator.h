/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __embedding_enumerator__
#define __embedding_enumerator__

#include "base_cpp/red_black.h"
#include "base_cpp/list.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/obj_array.h"

namespace indigo {

class Graph;
class GraphVertexEquivalence;

class EmbeddingEnumerator
{
public:
   enum
   {
      UNMAPPED = -1,
      TERM_OUT = -2,
      IGNORE   = -3
   };

   bool allow_many_to_one;

   DLLEXPORT EmbeddingEnumerator (Graph &supergraph);

   DLLEXPORT ~EmbeddingEnumerator ();

   // when cb_embedding returns zero, enumeration stops
   int  (*cb_embedding) (Graph &subgraph, Graph &supergraph,
                         int *core_sub, int *core_super, void *userdata);

   bool (*cb_match_vertex) (Graph &subgraph, Graph &supergraph,
                            const int *core_sub, int sub_idx, int super_idx, void *userdata);
   bool (*cb_match_edge)   (Graph &subgraph, Graph &supergraph,
                            int self_idx, int other_idx, void *userdata);

   void (*cb_vertex_remove) (Graph &subgraph, int sub_idx, void *userdata);
   void (*cb_edge_add)      (Graph &subgraph, Graph &supergraph,
                             int sub_idx, int super_idx, void *userdata);
   void (*cb_vertex_add)    (Graph &subgraph, Graph &supergraph,
                             int sub_idx, int super_idx, void *userdata);
   bool (*cb_allow_many_to_one) (Graph &subgraph, int sub_idx, void *userdata);

   void *userdata;

   DLLEXPORT void setSubgraph (Graph &subgraph);

   DLLEXPORT void ignoreSubgraphVertex (int idx);
   DLLEXPORT void ignoreSupergraphVertex (int idx);

   DLLEXPORT int countUnmappedSubgraphVertices ();
   DLLEXPORT int countUnmappedSupergraphVertices ();

   DLLEXPORT int countUnmappedSubgraphEdges ();
   DLLEXPORT int countUnmappedSupergraphEdges ();

   DLLEXPORT void setEquivalenceHandler (GraphVertexEquivalence *equivalence_handler);

   DLLEXPORT bool fix (int node1, int node2);
   DLLEXPORT bool unsafeFix (int node1, int node2);

   // returns 0 if cb_embedding returned 0, 1 otherwise
   DLLEXPORT int process ();
   DLLEXPORT void processStart ();
   DLLEXPORT bool processNext ();

   DLLEXPORT const int * getSubgraphMapping ();

   DLLEXPORT const int * getSupergraphMapping ();

   // Update internal structures to fit all target vertices that might be added
   DLLEXPORT void validate ();

   DEF_ERROR("embedding enumerator");

protected:

   // digit 1 relates to the subgraph, 2 relates to the supergraph.

   Graph *_g1;
   Graph *_g2;

   GraphVertexEquivalence *_equivalence_handler;

   TL_CP_DECL(Array<int>, _core_1);
   TL_CP_DECL(Array<int>, _core_2);

   TL_CP_DECL(Pool<RedBlackSet<int>::Node>, _s_pool);
   TL_CP_DECL(Pool<List<int>::Elem>,        _l_pool);

   void _terminatePreviousMatch ();

   enum
   {
      _RETURN0    = 0,
      _NOWAY      = 1,
      _ADD_PAIR   = 2
   };

   class _Enumerator
   {
   public:
      _Enumerator (EmbeddingEnumerator &context);
      _Enumerator (const _Enumerator &other);

      bool fix (int node1, int node2, bool safe);
      void setUseEquivalence (bool use);
      void reset ();

      int  nextPair ();
      void addPair  (int node1, int node2);
      void restore ();

      int  _current_node1, _current_node2;

   protected:

      EmbeddingEnumerator &_context;

      bool _use_equivalence;
      RedBlackSet<int>  _mapped_orbit_ids; 
      List<int>         _term1;
      List<int>         _term2;
      List<int>         _unterm2;

      int  _core_len;
      int  _t1_len, _t2_len;
      int  _selected_node1, _selected_node2;
      int  _node1_prev_value, _node2_prev_value;

      int  _current_node2_parent;
      int  _current_node2_nei_index;

      bool _checkNode1  (int node1);
      bool _checkNode2  (int node2, int for_node1);
      bool _checkPair (int node1, int node2);
   };

   TL_CP_DECL(ObjArray<_Enumerator>, _enumerators);
};

}

#endif
