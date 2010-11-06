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
#include "graph/filter.h"
#include "graph/biconnected_decomposer.h"

using namespace indigo;

BiconnectedDecomposer::BiconnectedDecomposer (const Graph &graph) :
_graph(graph),
TL_CP_GET(_components),
TL_CP_GET(_dfs_order),
TL_CP_GET(_lowest_order),
TL_CP_GET(_component_lists),
TL_CP_GET(_component_ids),
TL_CP_GET(_edges_stack),
_cur_order(0)
{
   _components.clear();
   _component_lists.clear();
   _dfs_order.clear_resize(graph.vertexEnd());
   _dfs_order.zerofill();
   _lowest_order.clear_resize(graph.vertexEnd());
   _component_ids.clear_resize(graph.vertexEnd());
   _component_ids.zerofill();
}

BiconnectedDecomposer::~BiconnectedDecomposer ()
{
}

int BiconnectedDecomposer::decompose ()
{
   int i, j;

   j = -1;

   for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
      if (_dfs_order[i] == 0)
         _biconnect (i, j);

   return _components.size();
}

void BiconnectedDecomposer::_biconnect (int v, int u)
{
   int i, w;

   _cur_order++;	
   _dfs_order[v] = _lowest_order[v] = _cur_order;

   const Vertex &v_vert = _graph.getVertex(v);

   Edge new_edge;

   for (i = v_vert.neiBegin(); i < v_vert.neiEnd(); i = v_vert.neiNext(i))
   {
      w = v_vert.neiVertex(i);

      if (_dfs_order[w] == 0)
      {
         new_edge.beg = v;
         new_edge.end = w;

         _edges_stack.push(new_edge);

         _biconnect(w, v);

         if (_lowest_order[w] < _lowest_order[v])
            _lowest_order[v] = _lowest_order[w];

         if (_lowest_order[w] >= _dfs_order[v])
         {
            //v -articulation point in G;
            //start new BCcomp;
            Array<int> &new_comp = _components.add(new Array<int>());
            new_comp.clear_resize(_graph.vertexEnd());
            new_comp.zerofill();

            int cur_comp = _components.size() - 1;

            if (_component_ids[v] == 0)
               _component_ids[v] = &_component_lists.add(new Array<int>());

            _component_ids[v]->push(cur_comp);

            while (_dfs_order[_edges_stack.top().beg] >= _dfs_order[w])
            {
               _components[cur_comp]->at(_edges_stack.top().beg) = 1;
               _components[cur_comp]->at(_edges_stack.top().end) = 1;
               _edges_stack.pop();
            }

            _components[cur_comp]->at(v) = 1;
            _components[cur_comp]->at(w) = 1;
            _edges_stack.pop();
         }
      } else {
         if  (_dfs_order[w] < _dfs_order[v] && w != u)
         {
            new_edge.beg = v;
            new_edge.end = w;
            _edges_stack.push(new_edge);

            if  (_lowest_order[v] > _dfs_order[w])
               _lowest_order[v] = _dfs_order[w];
         }
      }
   }
}

void BiconnectedDecomposer::getComponent (int idx, Filter &filter) const
{
   filter.init(_components[idx]->ptr(), Filter::EQ, 1);
}

bool BiconnectedDecomposer::isArticulationPoint (int idx) const
{
   return _component_ids[idx] != 0;
}

const Array<int> & BiconnectedDecomposer::getIncomingComponents (int idx) const
{
   if (!isArticulationPoint(idx))
      throw Error("vertex %d is not articulation point");

   return *_component_ids[idx];
}

void BiconnectedDecomposer::getVertexComponents (int idx, Array<int> &components) const
{
   if (!isArticulationPoint(idx))
   {
      int i;

      components.clear();

      for (i = 0; i < _components.size(); i++)
         if (_components[i]->at(idx) == 1)
         {
            components.push(i);
            break;
         }

      return;
   }

   components.copy(getIncomingComponents(idx));
}

int BiconnectedDecomposer::getIncomingCount(int idx) const
{
   if (!isArticulationPoint(idx))
      return 0;

   return _component_ids[idx]->size();
}
