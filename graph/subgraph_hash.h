/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __subgraph_hash__
#define __subgraph_hash__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "graph/graph_fast_access.h"

namespace indigo {

class Graph;

class SubgraphHash
{
public:
   SubgraphHash (Graph &g);

   int max_iterations;
   bool calc_different_codes_count;

   dword getHash ();
   dword getHash (const Array<int> &vertices, const Array<int> &edges);

   int getDifferentCodesCount ();

   const Array<int> *vertex_codes, *edge_codes;

private:
   int _getVertexCode (int vertex);
   int _getEdgeCode (int edge);

   static int _cmp_int (int a, int b, void *context);

   Graph &_g;
   int _different_codes_count;

   CP_DECL;
   TL_CP_DECL(Array<dword>, _codes);
   TL_CP_DECL(Array<dword>, _oldcodes);
   TL_CP_DECL(GraphFastAccess, _gf);
};

}

#endif // __subgraph_hash__

