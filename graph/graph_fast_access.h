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

#ifndef __graph_fast_access_h__
#define __graph_fast_access_h__

#include "base_cpp/array.h"

namespace indigo {

class Graph;

class GraphFastAccess
{
public:
   void setGraph (Graph &g);

   int* prepareVertices (int &count);
   int  getVertex (int idx); // unsafe
   int  vertexCount ();

   // Prepare both nei vertices and edges list. Runs faster then 
   // preparing them one by one.
   void prepareVertexNeiVerticesAndEdges (int v);
   // Returns nei vertices and nei edges for specified vertex
   // Numeration is coherent
   int* getVertexNeiVertices (int v, int &count);
   int* getVertexNeiEdges (int v, int &count);

   int findEdgeIndex (int v1, int v2);

private:
   Graph *_g;
   
   Array<int> _vertices;

   struct VertexNeiBlock
   {
      int v_begin, v_count;
      int e_begin, e_count;
   };
   Array<VertexNeiBlock> _vertices_nei;
   Array<int> _nei_vertices_data, _nei_edges_data;
};

}

#endif // __graph_fast_access_h__
