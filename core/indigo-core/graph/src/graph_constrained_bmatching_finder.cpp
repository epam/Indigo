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

#include "graph/graph_constrained_bmatching_finder.h"
#include "graph/skew_symmetric_flow_finder.h"

using namespace indigo;

IMPL_ERROR(GraphConstrainedBMatchingFinder, "b-matching finder");

CP_DEF(GraphConstrainedBMatchingFinder);

GraphConstrainedBMatchingFinder::GraphConstrainedBMatchingFinder(const Graph& g, const ObjArray<Array<int>>& nodes_per_set, const Array<int>* per_set_set_id)
    : _g(g), CP_INIT, TL_CP_GET(_network), TL_CP_GET(_edges_graph_to_net), TL_CP_GET(_vertices_graph_to_net), TL_CP_GET(_vertices_capacity_arc_per_set),
      TL_CP_GET(_constraint_sets), TL_CP_GET(_edge_matching_multiplicity), TL_CP_GET(_node_incident_edges_count)
{
    _edges_graph_to_net.clear_resize(g.edgeEnd());
    _vertices_graph_to_net.clear_resize(g.vertexEnd());
    _edge_matching_multiplicity.clear_resize(g.edgeEnd());
    _node_incident_edges_count.clear_resize(g.vertexEnd());

    int n_sets = nodes_per_set.size();
    _constraint_sets.resize(n_sets);
    _constraint_sets.fffill();

    _vertices_capacity_arc_per_set.resize(n_sets);
    for (int i = 0; i < n_sets; i++)
    {
        _vertices_capacity_arc_per_set[i].resize(g.vertexEnd());
        _vertices_capacity_arc_per_set[i].fffill();
    }

    _network.clear();
    int source = _network.addVertex();
    _network.setSource(source);

    int after_source = _network.addVertex();
    _source_edge = _network.addArc(source, after_source, 0);

    _createVertices();
    _createSets(n_sets, after_source, per_set_set_id);
    _connectVerticesWithSets(nodes_per_set);
    _createEdges();
}

void GraphConstrainedBMatchingFinder::_createSet(int idx, int root, const Array<int>* per_set_set_id)
{
    if (_constraint_sets[idx].node != -1)
        return;

    int node = _network.addVertex();

    int parent_node = root;
    if (per_set_set_id != NULL)
    {
        int parent_set = per_set_set_id->at(idx);
        if (parent_set != -1)
        {
            _createSet(parent_set, root, per_set_set_id);
            parent_node = _constraint_sets[parent_set].node;
        }
    }
    _constraint_sets[idx].node = node;
    _constraint_sets[idx].in_arc = _network.addArc(parent_node, node, 0);
}

void GraphConstrainedBMatchingFinder::_createSets(int n, int root, const Array<int>* per_set_set_id)
{
    for (int s = 0; s < n; s++)
        _createSet(s, root, per_set_set_id);
}

void GraphConstrainedBMatchingFinder::_createVertices()
{
    for (int v = _g.vertexBegin(); v != _g.vertexEnd(); v = _g.vertexNext(v))
    {
        int net_vertex = _network.addVertex();
        _vertices_graph_to_net[v] = net_vertex;
    }
}

void GraphConstrainedBMatchingFinder::_connectVerticesWithSets(const ObjArray<Array<int>>& nodes_per_set)
{
    for (int s = 0; s < nodes_per_set.size(); s++)
    {
        int cs_root = _constraint_sets[s].node;

        const Array<int>& nodes = nodes_per_set[s];
        for (int i = 0; i < nodes.size(); i++)
        {
            int vertex = nodes[i];
            int node = _vertices_graph_to_net[vertex];
            int in_arc = _network.addArc(cs_root, node, 0);
            _vertices_capacity_arc_per_set[s][vertex] = in_arc;
        }
    }
}

void GraphConstrainedBMatchingFinder::_createEdges()
{
    for (int e = _g.edgeBegin(); e != _g.edgeEnd(); e = _g.edgeNext(e))
    {
        const Edge& edge = _g.getEdge(e);

        int v1 = _vertices_graph_to_net[edge.beg];
        int v2 = _vertices_graph_to_net[edge.end];
        int v2_sym = _network.getSymmetricVertex(v2);

        _edges_graph_to_net[e] = _network.addArc(v1, v2_sym, 1);
    }
}

void GraphConstrainedBMatchingFinder::setNodeCapacity(int node, int capacity, int set_id)
{
    int net_arc = _vertices_capacity_arc_per_set[set_id][node];
    if (net_arc == -1)
        throw Error("node has no arc to the specified set");
    _network.setArcCapacity(net_arc, capacity);
}

int GraphConstrainedBMatchingFinder::getNodeCapacity(int node, int set_id) const
{
    int net_arc = _vertices_capacity_arc_per_set[set_id][node];
    if (net_arc == -1)
        throw Error("node has no arc to the specified set");
    return _network.getArcCapacity(net_arc);
}

void GraphConstrainedBMatchingFinder::setNodeSetCapacity(int set_id, int capacity)
{
    ConstraintSet& cs = _constraint_sets[set_id];
    _network.setArcCapacity(cs.in_arc, capacity);
}

int GraphConstrainedBMatchingFinder::getNodeSetCapacity(int set_id) const
{
    const ConstraintSet& cs = _constraint_sets[set_id];
    return _network.getArcCapacity(cs.in_arc);
}

void GraphConstrainedBMatchingFinder::setMaxEdgeMultiplicity(int edge, int capacity)
{
    int net_arc = _edges_graph_to_net[edge];
    _network.setArcCapacity(net_arc, capacity);
}

int GraphConstrainedBMatchingFinder::getMaxEdgeMultiplicity(int edge) const
{
    int net_arc = _edges_graph_to_net[edge];
    return _network.getArcCapacity(net_arc);
}

bool GraphConstrainedBMatchingFinder::findMatching(int cardinality)
{
    _network.setArcCapacity(_source_edge, 2 * cardinality);

    SkewSymmetricFlowFinder flow_finder(_network);
    flow_finder.process();

    // Copy matching information
    _node_incident_edges_count.zerofill();
    for (int e = _g.edgeBegin(); e != _g.edgeEnd(); e = _g.edgeNext(e))
    {
        int net_edge = _edges_graph_to_net[e];
        int mult = flow_finder.getArcValue(net_edge);
        _edge_matching_multiplicity[e] = mult;

        const Edge& edge = _g.getEdge(e);
        _node_incident_edges_count[edge.beg] += mult;
        _node_incident_edges_count[edge.end] += mult;
    }

    int value = flow_finder.getArcValue(_source_edge);
    if (value % 2 != 0)
        throw Error("algorithmic error: flow should be even");
    return value / 2 == cardinality;
}

int GraphConstrainedBMatchingFinder::getEdgeMultiplicity(int edge) const
{
    return _edge_matching_multiplicity[edge];
}

int GraphConstrainedBMatchingFinder::getNodeIncidentEdgesCount(int node) const
{
    return _node_incident_edges_count[node];
}
