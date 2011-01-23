/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "graph/graph_highlighting.h"
#include "graph/graph.h"
#include "graph/filter.h"

using namespace indigo;

GraphHighlighting::GraphHighlighting ()
{
   _graph = 0;
   _n_vertices = 0;
   _n_edges = 0;
}

void GraphHighlighting::init (const Graph &graph)
{
   _graph = &graph;

   _v_flags.clear_resize(graph.vertexEnd());
   _e_flags.clear_resize(graph.edgeEnd());

   _v_flags.zerofill();
   _e_flags.zerofill();
   _n_vertices = 0;
   _n_edges = 0;
}

void GraphHighlighting::nondestructiveUpdate ()
{
   if (_graph == 0)
      throw Error("no graph");
   
   _v_flags.expandFill(_graph->vertexEnd(), 0);
   _e_flags.expandFill(_graph->edgeEnd(), 0);
}

void GraphHighlighting::clear ()
{
   _v_flags.clear();
   _e_flags.clear();

   _n_vertices = 0;
   _n_edges = 0;
}

void GraphHighlighting::copy (const GraphHighlighting &other, const Array<int> *mapping)
{
   if (other._graph == 0 || _graph == 0)
      throw Error("no graph");

   int i;

   for (i = other._graph->vertexBegin(); i != other._graph->vertexEnd(); i = other._graph->vertexNext(i))
      if (other.hasVertex(i))
      {
         if (mapping == 0)
            onVertex(i);
         else
         {
            int mapped = mapping->at(i);
            if (mapped >= 0)
               onVertex(mapped);
         }
      }

   for (i = other._graph->edgeBegin(); i != other._graph->edgeEnd(); i = other._graph->edgeNext(i))
      if (other.hasEdge(i))
      {
         const Edge &edge = other._graph->getEdge(i);

         if (mapping == 0)
            onEdge(_graph->findEdgeIndex(edge.beg, edge.end));
         else
         {
            int mapped1 = mapping->at(edge.beg);
            int mapped2 = mapping->at(edge.end);
            if (mapped1 >= 0 && mapped2 >= 0)
               onEdge(_graph->findEdgeIndex(mapped1, mapped2));
         }
      }
}

void GraphHighlighting::onVertex (int idx)
{
   if (_v_flags[idx] != 0)
      return;
   _v_flags[idx] = 1;
   _n_vertices++;
}

void GraphHighlighting::removeEdge (int idx)
{
   if (_e_flags[idx] == 0)
      return;
   _e_flags[idx] = 0;
   _n_edges--;
}

void GraphHighlighting::removeVertex (int idx)
{
   if (_v_flags[idx] == 0)
      return;

   const Vertex &vertex = _graph->getVertex(idx);
   int j;

   for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
      removeEdge(vertex.neiEdge(j));

   _v_flags[idx] = 0;
   _n_vertices--;
}

void GraphHighlighting::removeVertexOnly (int idx)
{
   if (_v_flags[idx] == 0)
      return;

   _v_flags[idx] = 0;
   _n_vertices--;
}

void GraphHighlighting::onEdge (int idx)
{
   if (_e_flags[idx] != 0)
      return;
   _e_flags[idx] = 1;
   _n_edges++;
}

bool GraphHighlighting::hasVertex (int idx) const
{
   if (_v_flags.size() < idx + 1)
      return false;
   return _v_flags[idx] != 0;
}

bool GraphHighlighting::hasEdge (int idx) const
{
   if (_e_flags.size() < idx + 1)
      return false;
   return _e_flags[idx] != 0;
}

const Array<int> & GraphHighlighting::getVertices () const
{
   return _v_flags;
}

const Array<int> & GraphHighlighting::getEdges () const
{
   return _e_flags;
}

void GraphHighlighting::onVertices (const Filter &filter)
{
   int i;

   if (_graph == 0)
      throw Error("no graph");

   for (i = _graph->vertexBegin(); i != _graph->vertexEnd(); i = _graph->vertexNext(i))
      if (filter.valid(i))
         onVertex(i);
}

void GraphHighlighting::onEdges (const Filter &filter)
{
   int i;

   if (_graph == 0)
      throw Error("no graph");

   for (i = _graph->edgeBegin(); i < _graph->edgeEnd(); i = _graph->edgeNext(i))
   {
      if (filter.valid(i))
         onEdge(i);
   }
}

void GraphHighlighting::onSubgraph (const Graph &subgraph, const int *mapping)
{
   int i;

   if (_graph == 0)
      throw Error("no graph");

   for (i = subgraph.vertexBegin(); i != subgraph.vertexEnd(); i = subgraph.vertexNext(i))
      if (mapping[i] >= 0)
         onVertex(mapping[i]);

   for (i = subgraph.edgeBegin(); i != subgraph.edgeEnd(); i = subgraph.edgeNext(i))
   {
      const Edge &edge = subgraph.getEdge(i);

      int beg = mapping[edge.beg];
      int end = mapping[edge.end];

      if (beg >= 0 && end >= 0)
         onEdge(_graph->findEdgeIndex(beg, end));
   }
}


int GraphHighlighting::numVertices () const
{
   return _n_vertices;
}

int GraphHighlighting::numEdges () const
{
   return _n_edges;
}

void GraphHighlighting::invert ()
{
   int i;

   for (i = _graph->vertexBegin(); i != _graph->vertexEnd(); i = _graph->vertexNext(i))
   {
      if (hasVertex(i))
         removeVertexOnly(i);
      else
         onVertex(i);
   }

   for (i = _graph->edgeBegin(); i != _graph->edgeEnd(); i = _graph->edgeNext(i))
   {
      if (hasEdge(i))
         removeEdge(i);
      else
         onEdge(i);
   }

}
