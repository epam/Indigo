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

#ifndef __edge_rotation_matcher__
#define __edge_rotation_matcher__

#include "base_cpp/exception.h"

namespace indigo {

class Graph;
struct Vec3f;

class EdgeRotationMatcher
{
public:
   // takes mapping from subgraph to supergraph
   EdgeRotationMatcher (Graph &subgraph, Graph &supergraph, const int *mapping);

   void (*cb_get_xyz)    (Graph &graph, int vertex_idx, Vec3f &pos);
   bool (*cb_can_rotate) (Graph &graph, int edge_idx);
   bool equalize_edges;

   bool match (float rsm_threshold, float eps);

   DECL_ERROR;

protected:
   struct _DirEdge
   {
      int idx, beg, end;
   };

   Graph &_subgraph;
   Graph &_supergraph;
   const int   *_mapping;

private:
   EdgeRotationMatcher (const EdgeRotationMatcher &); // no implicit copy
};

}

#endif
