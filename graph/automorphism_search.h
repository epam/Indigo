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

#ifndef __automorphism_search__
#define __automorphism_search__

#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"

namespace indigo {

class AutomorphismSearch
{
public:
   explicit AutomorphismSearch ();
   virtual ~AutomorphismSearch ();

   // obtain return canonical ordering
   bool getcanon;
   // vertex compare method compares vertices degree first by default
   // if compare_vertex_degree_first is false then vertex degree is 
   // compared in vertex compare method after cb_vertex_cmp.
   bool compare_vertex_degree_first;
   // Reverse degree order in refine the refine method
   // By default nontrivial cell refines cell by degree from lowest to highest.
   // If reverse_degree_in_refine is true then this degree is reversed.
   // Trival cell always refines cell by degree from highest to lowest, but if
   // nesessary for this flag can be added.
   bool refine_reverse_degree;
   // With this flag during refinement cells are refined and sorted by
   // sorted neighbourhood of cell indices.
   // By default this flag is disabled.
   bool refine_by_sorted_neighbourhood;

   int  worksize;

   void *context;

   const int *ignored_vertices;

   int (*cb_vertex_cmp) (Graph &graph, int idx1, int idx2, const void *context);
   int (*cb_vertex_rank) (Graph &graph, int vertex_idx, const void *context);

   int (*cb_edge_rank) (Graph &graph, int edge_idx, const void *context);

   bool (*cb_check_automorphism) (Graph &graph, const Array<int> &mapping, const void *context);
   int  (*cb_compare_mapped) (Graph &graph, const Array<int> &mapping1, const Array<int> &mapping2, const void *context);

   void *context_automorphism;

   void (*cb_automorphism) (const int *automorphism, void *context);

   void process (Graph &graph);

   void getCanonicalNumbering (Array<int> &numbering);

   void getOrbits                   (Array<int> &orbits) const;
   void getCanonicallyOrderedOrbits (Array<int> &orbits) const;

   enum
   {
      INFINITY = 0x7FFF
   };

   DECL_ERROR;

protected:

   enum
   {
      _INITIAL = 1,
      _FIRST_LOOP,
      _OTHER_LOOP,
      _FIRST_TO_FIRST,
      _FIRST_TO_OTHER,
      _OTHER_TO_OTHER
   };

   struct _Call
   {
      int level;
      int numcells;
      int k;
      int tc;
      int tv1;
      int place; // _INITIAL, _FIRST_TO_FIRST, etc.
   };

   TL_CP_DECL(Array<_Call>, _call_stack);

   TL_CP_DECL(Array<int>, _lab);
   TL_CP_DECL(Array<int>, _ptn);
   TL_CP_DECL(Graph,      _graph);

   TL_CP_DECL(Array<int>, _mapping);
   TL_CP_DECL(Array<int>, _inv_mapping);
   TL_CP_DECL(Array<int>, _degree);

   TL_CP_DECL(ReusableObjArray< Array<int> >, _tcells);

   TL_CP_DECL(ReusableObjArray< Array<int> >, _fix);
   TL_CP_DECL(ReusableObjArray< Array<int> >, _mcr);

   TL_CP_DECL(Array<int>, _active);
   TL_CP_DECL(Array<int>, _workperm);
   TL_CP_DECL(Array<int>, _workperm2);
   TL_CP_DECL(Array<int>, _bucket);
   TL_CP_DECL(Array<int>, _count);
   TL_CP_DECL(Array<int>, _firstlab);
   TL_CP_DECL(Array<int>, _canonlab);
   TL_CP_DECL(Array<int>, _orbits);
   TL_CP_DECL(Array<int>, _fixedpts);
   TL_CP_DECL(Array<int[2]>, _work_active_cells);
   TL_CP_DECL(Array<int>, _edge_ranks_in_refine);

   int _n;
   Graph *_given_graph;

   int _gca_first;
   int _canonlevel, _gca_canon;
   int _cosetindex;
   bool _needshortprune;
   int _orbits_num;

   void _prepareGraph (Graph &graph);

   int _firstNode (int level, int numcells);
   int _otherNode (int level, int numcells);
   void _refine (int level, int &numcells);
   void _refineOriginal (int level, int &numcells);
   void _refineBySortingNeighbourhood (int level, int &numcells);
   void _refineByCell (int split1, int split2, int level, int &numcells, int &hint, int target_edge_rank);
   int _targetcell (int level, Array<int> &cell);
   void _breakout (int level, int tc, int tv);
   int _shortPrune (Array<int> &tcell, Array<int> &mcr, int idx);
   int _longPrune (Array<int> &tcell, Array<int> &fix, int idx);
   void _recover (int level);
   int _processNode (int level, int numcells);
   bool _isAutomorphism (Array<int> &perm);
   int _compareCanon ();
   void _buildFixMcr (const Array<int> &perm, Array<int> &fix, Array<int> &mcr);
   void _joinOrbits (const Array<int> &perm);
   void _handleAutomorphism (const Array<int> &perm);
   bool _hasEdgeWithRank (int from, int to, int target_edge_rank);

   static int _cmp_vertices (int idx1, int idx2, void *context);
};

}

#endif
