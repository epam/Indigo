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

#ifndef __graph_decomposer__
#define __graph_decomposer__

#include "base_cpp/exception.h"
#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Graph;
class Filter;

class DLLEXPORT GraphDecomposer
{
public:
   GraphDecomposer (const Graph &graph);
   ~GraphDecomposer ();

   // returns the amount of connected components
   int decompose (const Filter *filter = NULL, const Filter *edge_filter = NULL);

   const Array<int> & getDecomposition () const;

   int getComponent       (int vertex) const;
   int getComponentsCount ()           const;

   int getComponentVerticesCount (int component) const;
   int getComponentEdgesCount    (int component) const;

   DECL_ERROR;
protected:
   const Graph &_graph;
   int n_comp;

   TL_CP_DECL(Array<int>, _component_ids);
   TL_CP_DECL(Array<int>, _component_vertices_count);
   TL_CP_DECL(Array<int>, _component_edges_count);
private:
   GraphDecomposer (const GraphDecomposer &);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
