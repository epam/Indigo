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

#ifndef _SIMPLE_CYCLE_BASIS_H
#define	_SIMPLE_CYCLE_BASIS_H

#include "base_cpp/obj_array.h"
#include "base_cpp/red_black.h"
#include "base_cpp/array.h"
#include "graph/graph.h"

namespace indigo {

class SimpleCycleBasis {
public:
    SimpleCycleBasis(const Graph& graph);

    void create();

    int getCyclesCount() const { return _cycles.size(); }
    const Array<int>& getCycle(int num) const {return _cycles[num]; }
    
    ObjArray< Array<int> > _cycles;
private:
   
    static void  constructKernelVector(Array<bool>& u, ObjArray< Array<bool> >& a, int i);
    SimpleCycleBasis(SimpleCycleBasis&);// no implicit copy
    bool _getParentVertex(const Graph& graph, int vertex, int& parent_vertex);
    void _minimize(int startIndex);
    void _getCycleEdgeIncidenceMatrix(ObjArray< Array<bool> >& a);
    void _createEdgeIndexMap();
    int _getEdgeIndex(int edge) const;

    void _prepareSubgraph (Graph &subgraph);

    RedBlackMap<int, int> vertices_spanning_tree;
    
    RedBlackMap<int, int> spanning_tree_vertices;
    RedBlackMap<int, int> _edgeIndexMap;

    const Graph& _graph;

    Array<int> _edgeList;

    bool _isMinimized;
};

class AuxiliaryGraph: public Graph {


    // graph to aux. graph
    RedBlackMap<int, int> _vertexMap0;
    RedBlackMap<int, int> _vertexMap1;

    RedBlackMap<int, int> _auxVertexMap;

    // aux. edge to edge
    RedBlackMap<int, int> _auxEdgeMap;

    const Graph& _graph;
    Array<bool>& _u;
    RedBlackMap<int, int>& _edgeIndexMap;


public:
    
    AuxiliaryGraph(const Graph& graph, Array<bool>& u, RedBlackMap<int, int>& edgeIndexMap) :
        _graph(graph),
        _u(u),
        _edgeIndexMap(edgeIndexMap)
        {}

    int auxVertex0(int vertex);
    int auxVertex1(int vertex);

    const Vertex& getVertexAndBuild(int vertex) ;

    int edge(int auxEdge);
};

}

#endif	/* _SIMPLE_CYCLE_BASIS_H */

