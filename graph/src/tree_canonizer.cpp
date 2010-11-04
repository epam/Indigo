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

#include "base_cpp/tlscont.h"
#include "base_cpp/output.h"
#include "graph/tree_canonizer.h"

int TreeCanonizer::_selectCenter (const Graph &tree, bool &bicenter)
{
   QS_DEF(Array<int>, degrees1);
   QS_DEF(Array<int>, degrees2);
   int i, j;

   degrees1.clear_resize(tree.vertexEnd());
   degrees2.clear_resize(tree.vertexEnd());

   int *degrees_ptr = degrees1.ptr(), *degrees_ptr_2 = degrees2.ptr(), *ptrtmp;

   for (i = tree.vertexBegin(); i < tree.vertexEnd(); i = tree.vertexNext(i))
      degrees_ptr[i] = tree.getVertex(i).degree();

   int nonzero_degree = tree.vertexCount();
   int last_removed_vertex = tree.vertexBegin();

   while (nonzero_degree > 2)
   {
      int old_nonzero_degree = nonzero_degree;

      memcpy(degrees_ptr_2, degrees_ptr, tree.vertexEnd() * sizeof(int));
      
      for (i = tree.vertexBegin(); i != tree.vertexEnd(); i = tree.vertexNext(i))
         if (degrees_ptr[i] == 1)
         {
            const Vertex &vertex = tree.getVertex(i);

            degrees_ptr_2[i] = 0;
            nonzero_degree--;
            for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
            {
               int nei = vertex.neiVertex(j);
               if (degrees_ptr[nei] > 0)
               {
                  degrees_ptr_2[nei]--;
                  last_removed_vertex = nei;
                  break;
               }
            }
            if (j == vertex.neiEnd())
               throw Error("internal error in _selectCenter(): no edge removed after vertex #%d", i);
         }

      if (nonzero_degree == old_nonzero_degree)
         throw Error("internal error in _selectCenter(): nonzero_degree %d did not decrease", nonzero_degree);

      __swap(degrees_ptr, degrees_ptr_2, ptrtmp);
   }

   if (nonzero_degree == 1)
   {
      bicenter = false;
      return last_removed_vertex;
   }
   else if (nonzero_degree == 2)
   {
      int i1 = -1;
      int i2 = -1;

      bicenter  = true;

      for (i = tree.vertexBegin(); i != tree.vertexEnd(); i = tree.vertexNext(i))
      {
         if (degrees_ptr[i] == 1)
         {
            if (i1 == -1)
               i1 = i;
            else if (i2 == -1)
               i2 = i;
            else
               throw Error("internal error in _selectCenter(): i1 = %d, i2 = %d, i = %d", i1, i2, i);
         }
      }
      return tree.findEdgeIndex(i1, i2);
   }

   throw Error("internal error in _selectCenter(): nonzero_degree = %d", nonzero_degree);
}

TreeCanonizer::_Tree::_Tree (const Graph &tree, int root, int separating_edge, const TreeCanonizer &canonizer, const void *context) :
_tree(tree),
_canonizer(canonizer),
TL_CP_GET(_pool),
TL_CP_GET(_levels),
TL_CP_GET(_nodes)
{
   QS_DEF(Array<int>, stack);
   int i, j;

   stack.clear();

   for (int i = 0; i < _levels.size(); i++)
      _levels[i].clearAndResetPool(_pool);
   if (_levels.size() == 0)
      _levels.push(_pool);

   _nodes.clear();

   _root = _nodes.add();
   _Node &root_node = _nodes[_root];

   stack.push(_root);
   _levels[0].list.add(_root);

   _max_depth = 0;

   root_node.depth = 0;
   root_node.parent = -1;
   root_node.v_idx = root;

   while (stack.size() > 0)
   {
      int node_idx = stack.pop();
      const Vertex &vertex = tree.getVertex(_nodes[node_idx].v_idx);

      for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
      {
         const _Node &node = _nodes[node_idx];

         int edge_idx = vertex.neiEdge(i);
         int nei_idx = vertex.neiVertex(i);

         if (node.parent != -1 && nei_idx == _nodes[node.parent].v_idx)
            continue;

         if (edge_idx == separating_edge)
            continue;

         int new_depth = node.depth + 1;

         if (_levels.size() <= new_depth)
            _levels.push(_pool);

         int child_node_idx = _nodes.add();
         _Node &child_node = _nodes[child_node_idx];

         _levels[new_depth].list.add(child_node_idx);

         child_node.depth = new_depth;
         child_node.v_idx = nei_idx;
         child_node.e_idx = edge_idx;
         child_node.parent = node_idx;

         if (canonizer.cb_edge_rank == 0)
            child_node.e_rank = 0;
         else
         {
            child_node.e_rank = canonizer.cb_edge_rank(tree, edge_idx, context);
            if (child_node.e_rank < 0)
               throw Error("negative edge rank %d not allowed", child_node.e_rank);
         }

         if (canonizer.cb_vertex_rank == 0)
            child_node.v_rank = 0;
         else
         {
            child_node.v_rank = canonizer.cb_vertex_rank(tree, nei_idx, context);
            if (child_node.v_rank < 0)
               throw Error("negative vertex rank %d not allowed", child_node.v_rank);
         }
         
         if (_max_depth < new_depth)
            _max_depth = new_depth;

         stack.push(child_node_idx);
      }
   }

   for (i = 0; i <= _max_depth; i++)
   {
      _Level &level = _levels[i];

      int ranks_start = 0;
      int children_start = 0;

      for (j = level.list.begin(); j < level.list.end(); j = level.list.next(j))
      {
         _Node &xnode = _nodes[level.list.at(j)];
         int nchildren = _tree.getVertex(xnode.v_idx).degree();
         
         if (i != 0 || separating_edge != -1)
            nchildren--; // -1 parent or seaparating edge

         // + 1 for vertex rank + 1 for edge rank
         int nranks =  nchildren + 2;

         xnode.ranks_start = ranks_start;
         xnode.n_ranks = nranks;
         ranks_start += nranks;

         xnode.children_start = children_start;
         xnode.n_children = nchildren;
         children_start += nchildren;
      }

      level.nranks = ranks_start;
      level.ordered_children.clear_resize(children_start);
   }
}

TreeCanonizer::_Tree::~_Tree ()
{
   _levels.clearObjects();
}

void TreeCanonizer::_Tree::canonCode (Output &output)
{
   int i, j, depth;

   QS_DEF(Array<int>, rank_lists_1);
   QS_DEF(Array<int>, rank_lists_2);

   Array<int> *rank_lists = &rank_lists_1,
              *rank_lists_next = &rank_lists_2, *rank_lists_tmp;

   rank_lists->clear_resize(_levels[_max_depth].nranks);

   for (depth = _max_depth; depth > 0; depth--)
   {
      QS_DEF(Array<int>, sorted_nodes);
      _Level &level = _levels[depth];
      _Level &next_level = _levels[depth - 1];

      // prepare array of ranks for the next level
      rank_lists_next->clear_resize(next_level.nranks);

      for (i = next_level.list.begin(); i != next_level.list.end();
           i = next_level.list.next(i))
      {
         _nodes[next_level.list.at(i)].children_iter = 0;
         // set to 2 because 0 and 1 are for edge and vertex rank
         _nodes[next_level.list.at(i)].rank_iter = 2;
      }

      sorted_nodes.clear();

      for (i = level.list.begin(); i != level.list.end(); i = level.list.next(i))
      {
         _Node &xnode = _nodes[level.list.at(i)];

         sorted_nodes.push(level.list.at(i));

         rank_lists->at(xnode.ranks_start) = xnode.e_rank;
         rank_lists->at(xnode.ranks_start + 1) = xnode.v_rank;
      }

      // 'Depth-first' digital sorting
      QS_DEF(Array<_SElem>, stack);
      QS_DEF(PtrArray< Array<int> >, lists_by_rank);

      _SElem &first = stack.push();

      first.left = 0;
      first.right = sorted_nodes.size();
      first.depth = 0;

      int rank_on_level = 0;

      while (stack.size() > 0)
      {
         _SElem top = stack.pop();

         for (i = 0; i < lists_by_rank.size(); i++)
            if (lists_by_rank[i] != 0)
               lists_by_rank[i]->clear();

         if (top.right - top.left == 1)
         {
            // 1 node; sorting not needed
            _Node &xnode = _nodes[sorted_nodes[top.left]];
            _Node &parent = _nodes[xnode.parent];

            rank_lists_next->at(parent.ranks_start + parent.rank_iter) = rank_on_level;
            next_level.ordered_children[parent.children_start + parent.children_iter] = sorted_nodes[top.left];
            parent.rank_iter++;
            parent.children_iter++;
            rank_on_level++;
            continue;
         }

         int max_rank = 0;
         for (i = top.left; i < top.right; i++)
         {
            _Node &xnode = _nodes[sorted_nodes[i]];
            int rank = 0;

            if (top.depth < xnode.n_ranks)
               // "+1" because zero means 'no rank'
               rank = rank_lists->at(xnode.ranks_start + top.depth) + 1;
            else if (top.depth > xnode.n_ranks)
               throw Error("internal error: top.depth > xnode.n_ranks");

            lists_by_rank.expand(rank + 1);
            if (lists_by_rank[rank] == 0)
               lists_by_rank[rank] = new Array<int>();

            lists_by_rank[rank]->push(sorted_nodes[i]);
            if (rank > max_rank)
               max_rank = rank;
         }

         int idx = top.left;

         // reorder nodes
         for (i = 0; i <= max_rank; i++)
         {
            const Array<int> *arr = lists_by_rank[i];

            if (arr == 0)
               continue;

            for (j = 0; j < arr->size(); j++)
               sorted_nodes[idx++] = arr->at(j);
         }

         if (idx != top.right)
            throw Error("internal error: idx != top.right");

         // assign ranks to nodes having order 0
         if (lists_by_rank[0] != 0 && lists_by_rank[0]->size() > 0)
         {
            for (j = 0; j < lists_by_rank[0]->size(); j++)
            {
               int node_idx = lists_by_rank[0]->at(j);
               _Node &parent = _nodes[_nodes[node_idx].parent];

               rank_lists_next->at(parent.ranks_start + parent.rank_iter) = rank_on_level;
               next_level.ordered_children[parent.children_start + parent.children_iter] = node_idx;
               parent.rank_iter++;
               parent.children_iter++;
            }
            rank_on_level++;
         }

         idx = top.right;

         // push others on stack in reverse order
         for (i = max_rank; i > 0; i--)
         {
            if (lists_by_rank[i] == 0)
               continue;

            int n = lists_by_rank[i]->size();

            if (n < 1)
               continue;

            _SElem &elem = stack.push();

            elem.depth = top.depth + 1;
            elem.left = idx - n;
            elem.right = idx;
            idx = elem.left;
         }
      }

      __swap(rank_lists, rank_lists_next, rank_lists_tmp);
   }

   // now the children of each node are sorted
   // perform DFS walk
   {
      QS_DEF(Array<int>, stack);

      stack.push(_root);

      while (stack.size() > 0)
      {
         int idx = stack.pop();

         depth--;
         if (idx == -1)
         {
            output.writeChar(')');
            continue;
         }

         _Node &node = _nodes[idx];

         if (idx != _root && _canonizer.cb_edge_code != 0)
            _canonizer.cb_edge_code(_tree, node.e_idx, output);

         if (_canonizer.cb_vertex_code != 0)
            _canonizer.cb_vertex_code(_tree, node.v_idx, output);
         else
            output.writeChar('.');

         if (node.n_children > 0)
         {
            output.writeChar('(');
            stack.push(-1);
         }

         for (i = node.n_children - 1; i >= 0; i--)
            stack.push(_levels[node.depth].ordered_children[node.children_start + i]);
      }
   }

   output.writeChar(0);
}

void TreeCanonizer::perform (Output &output, const void *context)
{
   if (!skip_tree_check)
      if (!Graph::isTree(_tree))
         throw Error("non-tree given");

   bool bicenter;

   int center = _selectCenter(_tree, bicenter);

   if (bicenter)
   {
      _Tree xtree1(_tree, _tree.getEdge(center).beg, center, *this, context);
      _Tree xtree2(_tree, _tree.getEdge(center).end, center, *this, context);

      QS_DEF(Array<char>, code1);
      QS_DEF(Array<char>, code2);

      code1.clear();
      code2.clear();

      ArrayOutput output1(code1);
      ArrayOutput output2(code2);

      xtree1.canonCode(output1);
      xtree2.canonCode(output2);

      Array<char> *pcode1, *pcode2;

      if (strcmp(code1.ptr(), code2.ptr()) > 0)
      {
         pcode1 = &code1;
         pcode2 = &code2;
      }
      else
      {
         pcode2 = &code1;
         pcode1 = &code2;
      }

      output.writeString(pcode1->ptr());
      if (cb_edge_code != 0)
         cb_edge_code(_tree, center, output);
      output.writeString(pcode2->ptr());
      output.writeChar(0);
   }
   else
   {
      _Tree xtree(_tree, center, -1, *this, context);

      xtree.canonCode(output);
      output.writeChar(0);
   }
}
