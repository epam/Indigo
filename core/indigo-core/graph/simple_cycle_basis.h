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

#ifndef _SIMPLE_CYCLE_BASIS_H
#define _SIMPLE_CYCLE_BASIS_H

#include <map>

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/red_black.h"
#include "graph/graph.h"

namespace indigo
{

    class SimpleCycleBasis
    {
    public:
        SimpleCycleBasis(const Graph& graph);

        void create();

        int getCyclesCount() const
        {
            return _cycles.size();
        }
        const Array<int>& getCycle(int num) const
        {
            return _cycles[num];
        }

        ObjArray<Array<int>> _cycles;

    private:
        static void constructKernelVector(Array<bool>& u, ObjArray<Array<bool>>& a, int i);
        SimpleCycleBasis(SimpleCycleBasis&); // no implicit copy
        bool _getParentVertex(const Graph& graph, int vertex, int& parent_vertex);
        void _minimize(int startIndex);
        void _getCycleEdgeIncidenceMatrix(ObjArray<Array<bool>>& a);
        void _createEdgeIndexMap();
        int _getEdgeIndex(int edge) const;

        void _prepareSubgraph(Graph& subgraph);

        std::map<int, int> vertices_spanning_tree;

        std::map<int, int> spanning_tree_vertices;
        std::map<int, int> _edgeIndexMap;

        const Graph& _graph;

        Array<int> _edgeList;

        bool _isMinimized;
    };

    class AuxiliaryGraph : public Graph
    {

        // graph to aux. graph
        std::map<int, int> _vertexMap0;
        std::map<int, int> _vertexMap1;

        std::map<int, int> _auxVertexMap;

        // aux. edge to edge
        std::map<int, int> _auxEdgeMap;

        const Graph& _graph;
        Array<bool>& _u;
        std::map<int, int>& _edgeIndexMap;

        int _findOrCreateVertex(std::map<int, int>& vertexMap, int vertex);

    public:
        AuxiliaryGraph(const Graph& graph, Array<bool>& u, std::map<int, int>& edgeIndexMap) : _graph(graph), _u(u), _edgeIndexMap(edgeIndexMap)
        {
        }

        int auxVertex0(int vertex);
        int auxVertex1(int vertex);

        const Vertex& getVertexAndBuild(int vertex);

        int edge(int auxEdge);
    };

} // namespace indigo

#endif /* _SIMPLE_CYCLE_BASIS_H */
