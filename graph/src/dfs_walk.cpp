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

#include "graph/dfs_walk.h"
#include "graph/graph.h"
#include "base_cpp/array.h"

using namespace indigo;

IMPL_ERROR(DfsWalk, "DFS walk");

DfsWalk::DfsWalk (const Graph &graph) :
_graph(graph),
TL_CP_GET(_vertices),
TL_CP_GET(_edges),
TL_CP_GET(_v_seq),
TL_CP_GET(_root_vertices),
TL_CP_GET(_closures),
TL_CP_GET(_class_dist_from_exit)
{
   ignored_vertices = 0;
   vertex_ranks = 0;
   vertex_classes = 0;
   _root_vertices.resize(graph.vertexEnd());
   _root_vertices.zerofill();
}

DfsWalk::~DfsWalk ()
{
}

void DfsWalk::mustBeRootVertex (int v_idx)
{
   _root_vertices[v_idx] = 1;
}

void DfsWalk::walk ()
{
   QS_DEF(Array<int>, v_stack);
   int i, j;

   if (vertex_ranks != 0 && vertex_classes != 0)
      throw Error("you can not specify both vertex_ranks and vertex_classes");

   _vertices.clear_resize(_graph.vertexEnd());
   _edges.clear_resize(_graph.edgeEnd());
   _vertices.zerofill();
   _edges.zerofill();
   _closures.clear();

   v_stack.clear();
   _v_seq.clear();
   
   while (1)
   {
      if (v_stack.size() < 1)
      {
         int selected = -1;

         for (i = _graph.vertexBegin(); i != _graph.vertexEnd(); i = _graph.vertexNext(i))
         {
            if (ignored_vertices != 0 && ignored_vertices[i] != 0)
               continue;
            if (_vertices[i].dfs_state != 0)
               continue;
            if (vertex_classes != 0 && vertex_classes[i] >= 0)
               continue;
            if (vertex_ranks == 0)
            {
               selected = i;
               break;
            }
            if (selected == -1 || vertex_ranks[i] < vertex_ranks[selected])
               selected = i;
         }
         if (selected == -1)
            break;
         _vertices[selected].parent_vertex = -1;
         _vertices[selected].parent_edge = -1;
         v_stack.push(selected);
      }
      
      int v_idx = v_stack.pop();
      int parent_vertex = _vertices[v_idx].parent_vertex;

      {
         SeqElem &seq_elem = _v_seq.push();

         seq_elem.idx = v_idx;
         seq_elem.parent_vertex = parent_vertex;
         seq_elem.parent_edge = _vertices[v_idx].parent_edge;
      }
   
      _vertices[v_idx].dfs_state = 2;

      const Vertex &vertex = _graph.getVertex(v_idx);
      QS_DEF(Array<VertexEdge>, nei);

      nei.clear();
      
      for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
      {
         int nei_v = vertex.neiVertex(i);

         if (ignored_vertices != 0 && ignored_vertices[nei_v] != 0)
            continue;
         if (_root_vertices[nei_v] == 1 && _vertices[nei_v].dfs_state == 0)
            continue;

         VertexEdge &ve = nei.push();

         ve.e = vertex.neiEdge(i);
         ve.v = nei_v;
      }

      if (vertex_ranks != 0)
         nei.qsort(_cmp_ranks, vertex_ranks);

      if (vertex_classes != 0 && vertex_classes[v_idx] >= 0)
      {
         // prefer not to leave the class if possible
         _current_class = vertex_classes[v_idx];
         nei.qsort(_cmp_classes, this);
      }

      for (i = 0; i < nei.size(); i++)
      {
         int edge_idx = nei[i].e;
         int nei_idx = nei[i].v;

         if (nei_idx == parent_vertex)
            continue;
         
         if (_vertices[nei_idx].dfs_state == 2)
         {
            _edges[edge_idx].closing_cycle = 1;
            Edge &e = _closures.push();
            e.beg = v_idx;
            e.end = nei_idx;

            _vertices[nei_idx].openings++;
            _vertices[v_idx].branches++;

            SeqElem &seq_elem = _v_seq.push();
            
            seq_elem.idx = nei_idx;
            seq_elem.parent_vertex = v_idx;
            seq_elem.parent_edge = edge_idx;
         }
         else
         {
            if (_vertices[nei_idx].dfs_state == 1) 
            {
               j = v_stack.find(nei_idx);

               if (j == -1) 
                  throw Error("internal: removing vertex from stack");

               v_stack.remove(j);

               int parent = _vertices[nei_idx].parent_vertex;

               if (parent >= 0)
                  _vertices[parent].branches--;
            }

            _vertices[v_idx].branches++;
            _vertices[nei_idx].parent_vertex = v_idx;
            _vertices[nei_idx].parent_edge = edge_idx;
            _vertices[nei_idx].dfs_state = 1;
            v_stack.push(nei_idx);
         }
      }
   }
}

const Array<DfsWalk::SeqElem> & DfsWalk::getSequence () const
{
   return _v_seq;
}

bool DfsWalk::isClosure (int e_idx) const
{
   return _edges[e_idx].closing_cycle != 0;
}

int DfsWalk::numBranches (int v_idx) const
{
   return _vertices[v_idx].branches;
}

int DfsWalk::numOpenings (int v_idx) const
{
   return _vertices[v_idx].openings;
}

void DfsWalk::calcMapping (Array<int> &mapping) const
{
   int i, counter = 0;
   
   mapping.clear_resize(_graph.vertexEnd());
   mapping.fffill();

   for (i = 0; i < _v_seq.size(); i++)
   {
      if (mapping[_v_seq[i].idx] == -1)
         mapping[_v_seq[i].idx] = counter++;
   }
}

int DfsWalk::_cmp_ranks (VertexEdge &ve1, VertexEdge &ve2, void *context)
{
   int *ranks = (int *)context;

   return ranks[ve2.v] - ranks[ve1.v];
}

int DfsWalk::_cmp_classes (VertexEdge &ve1, VertexEdge &ve2, void *context)
{
   DfsWalk *self = (DfsWalk *)context;

   if (self->vertex_classes[ve1.v] == self->_current_class)
      return 1;
   if (self->vertex_classes[ve2.v] == self->_current_class)
      return -1;
   return 0;
}

void DfsWalk::getNeighborsClosing (int v_idx, Array<int> &res)
{
   int i;

   res.clear();
   for (i = 0; i < _closures.size(); i++)
   {
      if (_closures[i].end == v_idx)
         res.push(_closures[i].beg);
   }
}
