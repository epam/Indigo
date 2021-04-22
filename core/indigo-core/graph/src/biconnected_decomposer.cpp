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

BiconnectedDecomposer::BiconnectedDecomposer(const Graph& graph)
    : _graph(graph), CP_INIT, TL_CP_GET(_components), TL_CP_GET(_dfs_order), TL_CP_GET(_lowest_order), TL_CP_GET(_component_lists), TL_CP_GET(_component_ids),
      TL_CP_GET(_edges_stack), _cur_order(0)
{
    _components.clear();
    _component_lists.clear();
    _dfs_order.clear_resize(graph.vertexEnd());
    _dfs_order.zerofill();
    _lowest_order.clear_resize(graph.vertexEnd());
    _component_ids.clear_resize(graph.vertexEnd());
    _component_ids.zerofill();
}

BiconnectedDecomposer::~BiconnectedDecomposer()
{
}

int BiconnectedDecomposer::decompose()
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

                bool pushed = _pushToStack(dfs_stack, v);

                if (!pushed)
                {
                    dfs_stack.pop();

                    if (dfs_stack.size() == 0)
                        continue;

                    _processIfNotPushed(dfs_stack, v);
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

bool BiconnectedDecomposer::_pushToStack(Array<int>& dfs_stack, int v)
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
            // Push new edge
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
            new_edge.beg = v;
            new_edge.end = w;
            _edges_stack.push(new_edge);

            if (_lowest_order[v] > _dfs_order[w])
                _lowest_order[v] = _dfs_order[w];
        }
    }
    return false;
}

void BiconnectedDecomposer::_processIfNotPushed(Array<int>& dfs_stack, int w)
{
    int v = dfs_stack.top();

    if (_lowest_order[w] < _lowest_order[v])
        _lowest_order[v] = _lowest_order[w];

    if (_lowest_order[w] >= _dfs_order[v])
    {
        // v -articulation point in G;
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
            _components[cur_comp]->at(_edges_stack.top().beg) = 1;
            _components[cur_comp]->at(_edges_stack.top().end) = 1;
            _edges_stack.pop();
        }

        _components[cur_comp]->at(v) = 1;
        _components[cur_comp]->at(w) = 1;
        _edges_stack.pop();
    }
}
