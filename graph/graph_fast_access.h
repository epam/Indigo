/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __graph_fast_access_h__
#define __graph_fast_access_h__

#include "base_cpp/array.h"
#include "graph/graph.h"

namespace indigo
{

    class Graph;

    class GraphFastAccess
    {
    public:
        void setGraph(Graph& g);

        int* prepareVertices(int& count);
        int getVertex(int idx); // unsafe
        int vertexCount();

        // Prepare both nei vertices and edges list. Runs faster then
        // preparing them one by one.
        void prepareVertexNeiVerticesAndEdges(int v);
        // Returns nei vertices and nei edges for specified vertex
        // Numeration is coherent
        // Note: pointer might become invalid after preparing calls for
        // different vertices. In this case use prepareVertexNeiVertices/getVertexNeiVertiex.
        int* getVertexNeiVertices(int v, int& count);
        int* getVertexNeiEdges(int v, int& count);

        // Returns vertex identifier that can be used in getVertexNeiVertiex
        int prepareVertexNeiVertices(int v, int& count);
        // Returns neighbor vertex for the specified
        // vertex id (returned by prepareVertexNeiVertices)
        int getVertexNeiVertiex(int v_id, int index);

        int findEdgeIndex(int v1, int v2);

        void prepareEdges();
        const Edge& getEdge(int e);
        const Edge* getEdges();

    private:
        Graph* _g;

        Array<int> _vertices;

        struct VertexNeiBlock
        {
            int v_begin, v_count;
            int e_begin, e_count;
        };
        Array<VertexNeiBlock> _vertices_nei;
        Array<int> _nei_vertices_data, _nei_edges_data;

        Array<Edge> _edges;
    };

} // namespace indigo

#endif // __graph_fast_access_h__
