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

#ifndef __graph_highlighting__
#define __graph_highlighting__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"

namespace indigo {

class Graph;
class Filter;

class GraphHighlighting
{
public:
   DLLEXPORT GraphHighlighting ();

   DLLEXPORT void init (const Graph &graph);
   DLLEXPORT void clear ();

   DLLEXPORT void copy (const GraphHighlighting &other, const Array<int> *mapping);

   DLLEXPORT void onVertex (int idx);
   DLLEXPORT void onEdge (int idx);

   DLLEXPORT bool hasVertex (int idx) const;
   DLLEXPORT bool hasEdge (int idx) const;

   DLLEXPORT const Array<int> & getVertices () const;
   DLLEXPORT const Array<int> & getEdges () const;

   DLLEXPORT void onVertices (const Filter & filter);
   DLLEXPORT void onEdges (const Filter &filter);

   DLLEXPORT void onSubgraph (const Graph &subgraph, const int *mapping);

   DLLEXPORT void removeVertex (int idx);
   DLLEXPORT void removeVertexOnly (int idx);
   DLLEXPORT void removeEdge   (int idx);

   DLLEXPORT int numVertices () const;
   DLLEXPORT int numEdges    () const;

   DLLEXPORT void invert ();

   DEF_ERROR("graph highlighting");

protected:
   const Graph *_graph;
   Array<int> _v_flags;
   Array<int> _e_flags;
   int _n_vertices;
   int _n_edges;
   
private:
   GraphHighlighting (const GraphHighlighting &); // no implicit copy
};

}

#endif
