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

#include "graph/biconnected_decomposer.h"
#include "base_cpp/tlscont.h"
#include "graph/filter.h"

using namespace indigo;

IMPL_ERROR(BiconnectedDecomposer, "biconnected_decomposer");

CP_DEF(BiconnectedDecomposer);

BiconnectedDecomposer::BiconnectedDecomposer(const Graph& graph, bool split_fixed)
    : _graph(graph), _split_fixed(split_fixed), CP_INIT, TL_CP_GET(_components), TL_CP_GET(_dfs_order), TL_CP_GET(_lowest_order), TL_CP_GET(_component_lists),
      TL_CP_GET(_component_ids), TL_CP_GET(_edges_stack), TL_CP_GET(_forbidden_edges_processed), _cur_order(0)
{
    _components.clear();
    _component_lists.clear();
    _dfs_order.clear_resize(graph.vertexEnd());
    _dfs_order.zerofill();
    _lowest_order.clear_resize(graph.vertexEnd());
    _component_ids.clear_resize(graph.vertexEnd());
    _component_ids.zerofill();
    _forbidden_edges_processed.clear_resize(graph.edgeEnd());
    _forbidden_edges_processed.zerofill();
}

BiconnectedDecomposer::~BiconnectedDecomposer()
{
}

int BiconnectedDecomposer::decompose()
{
    Array<int> fixed_empty;
    return decomposeWithFixed(fixed_empty);
}

int BiconnectedDecomposer::decomposeWithFixed(const Array<int>& fixed_vertices)
{ // recursion? no, not heard...
    QS_DEF(Array<int>, dfs_stack);
    int i, v;

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
        if (_dfs_order[i] == 0)
        {
            dfs_stack.clear();
            dfs_stack.push(i);
            _cur_order++;
            _dfs_order[i] = _lowest_order[i] = _cur_order;

            // Start DFS
            while (dfs_stack.size() > 0)
            {
                v = dfs_stack.top();

                bool pushed = _pushToStack(dfs_stack, v, fixed_vertices);

                if (!pushed)
                {
                    dfs_stack.pop();

                    if (dfs_stack.size() == 0)
                        continue;

                    _processIfNotPushed(dfs_stack, v, fixed_vertices);
                }
            }
        }

    return componentsCount();
}

int BiconnectedDecomposer::componentsCount()
{
    return _components.size();
}

void BiconnectedDecomposer::getComponent(int idx, Filter& filter) const
{
    filter.init(_components[idx]->ptr(), Filter::EQ, 1);
}

bool BiconnectedDecomposer::isArticulationPoint(int idx) const
{
    return _component_ids[idx] != 0;
}

const Array<int>& BiconnectedDecomposer::getIncomingComponents(int idx) const
{
    if (!isArticulationPoint(idx))
        throw Error("vertex %d is not articulation point");

    return *_component_ids[idx];
}

void BiconnectedDecomposer::getVertexComponents(int idx, Array<int>& components) const
{
    if (!isArticulationPoint(idx))
    {
        int i;

        components.clear();

        for (i = 0; i < _components.size(); i++)
            if (_components[i]->at(idx) == 1)
            {
                components.push(i);
                break;
            }

        return;
    }

    components.copy(getIncomingComponents(idx));
}

int BiconnectedDecomposer::getIncomingCount(int idx) const
{
    if (!isArticulationPoint(idx))
        return 0;

    return _component_ids[idx]->size();
}

bool BiconnectedDecomposer::_pushToStack(Array<int>& dfs_stack, int v, const Array<int>& fixed_vertices)
{
    Edge new_edge;

    const Vertex& v_vert = _graph.getVertex(v);

    int u;
    if (dfs_stack.size() > 1)
        u = dfs_stack[dfs_stack.size() - 2];
    else
        u = -1;

    for (int j = v_vert.neiBegin(); j < v_vert.neiEnd(); j = v_vert.neiNext(j))
    {
        int w = v_vert.neiVertex(j);

        if (_dfs_order[w] == 0)
        {
            // Tree edge (v, w)
            if (_split_fixed && !isPermitted(v, w, fixed_vertices))
            {
                // Forbidden tree edge - create separate single-edge component
                // Do NOT mark w as visited - outer loop will visit it later
                // Use canonical edge representation: (min, max) to check if already processed
                int v_min = (v < w) ? v : w;
                int v_max = (v < w) ? w : v;
                int edge_idx = _graph.findEdgeIndex(v_min, v_max);
                if (edge_idx == -1)
                    edge_idx = _graph.findEdgeIndex(v_max, v_min);

                if (edge_idx == -1 || _forbidden_edges_processed[edge_idx] == 0)
                {
                    Array<int>& one = _components.add(new Array<int>());
                    one.clear_resize(_graph.vertexEnd());
                    one.zerofill();
                    one[v] = 1;
                    one[w] = 1;
                    if (_component_ids[v] == 0)
                        _component_ids[v] = &_component_lists.add(new Array<int>());
                    _component_ids[v]->push(_components.size() - 1);

                    if (edge_idx != -1)
                        _forbidden_edges_processed[edge_idx] = 1;
                }
                continue; // Skip this edge, don't go deeper
            }

            // Permitted tree edge - add to stack and continue DFS
            new_edge.beg = v;
            new_edge.end = w;

            _edges_stack.push(new_edge);
            dfs_stack.push(w);

            _cur_order++;
            _dfs_order[w] = _lowest_order[w] = _cur_order;
            return true;
        }
        else if (_dfs_order[w] < _dfs_order[v] && w != u)
        {
            // Back edge (v, w)
            if (!_split_fixed || isPermitted(v, w, fixed_vertices))
            {
                // Permitted edge - add to stack
                new_edge.beg = v;
                new_edge.end = w;
                _edges_stack.push(new_edge);
                if (_lowest_order[v] > _dfs_order[w])
                    _lowest_order[v] = _dfs_order[w];
            }
            else
            {
                // Forbidden edge - create separate single-edge component
                // Use canonical edge representation: (min, max) to check if already processed
                int v_min = (v < w) ? v : w;
                int v_max = (v < w) ? w : v;
                int edge_idx = _graph.findEdgeIndex(v_min, v_max);
                if (edge_idx == -1)
                    edge_idx = _graph.findEdgeIndex(v_max, v_min);

                if (edge_idx != -1 && _forbidden_edges_processed[edge_idx] != 0)
                {
                    // Already created a component for this edge, skip
                    continue;
                }

                Array<int>& one = _components.add(new Array<int>());
                one.clear_resize(_graph.vertexEnd());
                one.zerofill();
                one[v] = 1;
                one[w] = 1;
                if (_component_ids[v] == 0)
                    _component_ids[v] = &_component_lists.add(new Array<int>());
                _component_ids[v]->push(_components.size() - 1);

                // Mark this edge as processed
                if (edge_idx != -1)
                    _forbidden_edges_processed[edge_idx] = 1;
            }
        }
    }
    return false;
}

void BiconnectedDecomposer::_processIfNotPushed(Array<int>& dfs_stack, int w, const Array<int>& fixed_vertices)
{
    int v = dfs_stack.top();

    // Tree edge (v, w) is always permitted here (already checked in _pushToStack)
    if (_lowest_order[w] < _lowest_order[v])
        _lowest_order[v] = _lowest_order[w];

    if (_lowest_order[w] >= _dfs_order[v])
    {
        // v - articulation point in G;
        // start new BCcomp;
        Array<int>& new_comp = _components.add(new Array<int>());
        new_comp.clear_resize(_graph.vertexEnd());
        new_comp.zerofill();

        int cur_comp = _components.size() - 1;

        if (_component_ids[v] == 0)
            _component_ids[v] = &_component_lists.add(new Array<int>());

        _component_ids[v]->push(cur_comp);

        while (_dfs_order[_edges_stack.top().beg] >= _dfs_order[w])
        {
            Edge e = _edges_stack.top();
            _edges_stack.pop();
            // All edges in stack are permitted (forbidden edges don't go into stack)
            _components[cur_comp]->at(e.beg) = 1;
            _components[cur_comp]->at(e.end) = 1;
        }
        // add the final tree edge (v,w)
        _components[cur_comp]->at(v) = 1;
        _components[cur_comp]->at(w) = 1;
        _edges_stack.pop();
    }
}
