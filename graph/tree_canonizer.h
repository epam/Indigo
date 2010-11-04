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

#ifndef __tree_canonizer__
#define __tree_canonizer__

#include "graph/graph.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/list.h"
#include "base_cpp/tlscont.h"

class Output;

class TreeCanonizer
{
public:
   TreeCanonizer (const Graph &tree) :
   cb_vertex_rank(0),
   cb_edge_rank(0),
   cb_vertex_code(0),
   cb_edge_code(0),
   _tree(tree)
   {
      skip_tree_check = false;
   }

   bool skip_tree_check;
   
   void perform (Output &output, const void *context);

   int (*cb_vertex_rank) (const Graph &tree, int vertex_idx, const void *context);
   int (*cb_edge_rank)   (const Graph &tree, int edge_idx, const void *context);

   void (*cb_vertex_code) (const Graph &tree, int vertex_idx, Output &output);
   void (*cb_edge_code)   (const Graph &tree, int edge_idx, Output &output);

   DEF_ERROR("tree canonizer");
protected:

   const Graph &_tree;
   
   int _selectCenter (const Graph &tree, bool &bicenter);
   
   struct _Node
   {
      int parent;  // parent node
      int v_idx;   // index in tree
      int e_idx;   // index of edge (to parent)
      int v_rank;  // global rank of the vertex
      int e_rank;  // global rank of the edge (to parent)
      int depth;   // level number
      int n_ranks;
      int ranks_start; 
      int rank_iter;

      int n_children;
      int children_start;
      int children_iter;
   };

   class _Level
   {
   public:
      _Level (Pool<List<int>::Elem> &pool) : list(pool), nranks(0) {}

      void clear ()
      {
         list.clear();
         ordered_children.clear();
      }

      void clearAndResetPool (Pool<List<int>::Elem> &pool)
      {
         list.clearAndResetPool(pool);
         ordered_children.clear();
      }

      List<int>  list;
      Array<int> ordered_children;
      int nranks;
   };

   class _Tree
   {
   public:
      _Tree (const Graph &tree, int root, int separating_edge, const TreeCanonizer &canonizer, const void *context);
      ~_Tree ();

      void canonCode (Output &output);

   protected:

      struct _SElem
      {
         int left;
         int right;
         int depth;
      };

      const Graph &_tree;
      const TreeCanonizer &_canonizer;
      TL_CP_DECL(Pool<List<int>::Elem>, _pool);
      TL_CP_DECL(ObjArray<_Level>, _levels);
      TL_CP_DECL(Pool<_Node>, _nodes);

      int _root;
      int _max_depth;
   };
};

#endif
