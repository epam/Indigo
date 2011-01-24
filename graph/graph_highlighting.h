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

#ifndef __graph_highlighting__
#define __graph_highlighting__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Graph;
class Filter;

class DLLEXPORT GraphHighlighting
{
public:
   GraphHighlighting ();

   void init (const Graph &graph);
   void nondestructiveUpdate ();
   void clear ();

   void copy (const GraphHighlighting &other, const Array<int> *mapping);

   void onVertex (int idx);
   void onEdge (int idx);

   bool hasVertex (int idx) const;
   bool hasEdge (int idx) const;

   const Array<int> & getVertices () const;
   const Array<int> & getEdges () const;

   void onVertices (const Filter & filter);
   void onEdges (const Filter &filter);

   void onSubgraph (const Graph &subgraph, const int *mapping);

   void removeVertex (int idx);
   void removeVertexOnly (int idx);
   void removeEdge   (int idx);

   int numVertices () const;
   int numEdges    () const;

   void invert ();

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

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
