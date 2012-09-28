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

#ifndef __graph_affine_matcher__
#define __graph_affine_matcher__

#include "base_cpp/array.h"

namespace indigo {

class Graph;
struct Vec3f;

class GraphAffineMatcher
{
public:
   // takes mapping from subgraph to supergraph
   GraphAffineMatcher (Graph &subgraph, Graph &supergraph, const int *mapping);

   bool match (float rms_threshold);

   void (*cb_get_xyz) (Graph &graph, int vertex_idx, Vec3f &pos);

   const Array<int> *fixed_vertices;

   DECL_ERROR;

protected:

   Graph &_subgraph;
   Graph &_supergraph;
   const int   *_mapping;

private:
   GraphAffineMatcher (const GraphAffineMatcher &); // guess what? tip: look at any other class
};

}

#endif
