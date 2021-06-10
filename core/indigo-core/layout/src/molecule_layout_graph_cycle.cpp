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

#include "layout/molecule_layout_graph.h"

using namespace indigo;

CP_DEF(MoleculeLayoutGraph::Cycle);

MoleculeLayoutGraph::Cycle::Cycle() : CP_INIT, TL_CP_GET(_vertices), TL_CP_GET(_edges), TL_CP_GET(_attached_weight)
{
    _vertices.clear();
    _edges.clear();
    _attached_weight.clear();
    _max_idx = 0;
    _morgan_code_calculated = false;
}

MoleculeLayoutGraph::Cycle::Cycle(const List<int>& edges, const MoleculeLayoutGraph& graph)
    : CP_INIT, TL_CP_GET(_vertices), TL_CP_GET(_edges), TL_CP_GET(_attached_weight)
{
    copy(edges, graph);
    _attached_weight.resize(graph.vertexCount());
    _attached_weight.zerofill();
    _morgan_code_calculated = false;
}

MoleculeLayoutGraph::Cycle::Cycle(const Array<int>& vertices, const Array<int>& edges)
    : CP_INIT, TL_CP_GET(_vertices), TL_CP_GET(_edges), TL_CP_GET(_attached_weight)
{
    copy(vertices, edges);
    _attached_weight.resize(vertices.size());
    _attached_weight.zerofill();
    _morgan_code_calculated = false;
}

void MoleculeLayoutGraph::Cycle::copy(const List<int>& edges, const MoleculeLayoutGraph& graph)
{
    int i = edges.begin();
    const Edge& edge1 = graph.getEdge(edges[i]);
    const Edge& edge2 = graph.getEdge(edges[edges.next(i)]);

    _vertices.clear();
    _edges.clear();

    if (edge1.beg == edge2.beg || edge1.beg == edge2.end)
        _vertices.push(edge1.end);
    else
        _vertices.push(edge1.beg);

    for (; i < edges.end(); i = edges.next(i))
    {
        const Edge& edge = graph.getEdge(edges[i]);

        if (_vertices.top() == edge.beg)
            _vertices.push(edge.end);
        else
            _vertices.push(edge.beg);

        _edges.push(edges[i]);
    }

    _vertices.pop();

    _max_idx = 0;

    for (int i = 0; i < _vertices.size(); i++)
        if (_vertices[i] > _max_idx)
            _max_idx = _vertices[i];
}

void MoleculeLayoutGraph::Cycle::copy(const Array<int>& vertices, const Array<int>& edges)
{
    _vertices.copy(vertices);
    _edges.copy(edges);
    _max_idx = 0;

    for (int i = 0; i < _vertices.size(); i++)
        if (_vertices[i] > _max_idx)
            _max_idx = _vertices[i];
}

void MoleculeLayoutGraph::Cycle::calcMorganCode(const MoleculeLayoutGraph& parent_graph)
{
    _morgan_code = 0;

    for (int i = 0; i < vertexCount(); i++)
        _morgan_code += parent_graph.getLayoutVertex(_vertices[i]).morgan_code;

    _morgan_code_calculated = true;
}

void MoleculeLayoutGraph::Cycle::canonize()
{
    // 1. v(0)<v(i), i=1,...,l-1 ;
    // 2. v(1)< v(l-2) => unique representation of cycle

    if (vertexCount() == 0)
        return;
    int min_idx = 0, i;
    bool vert_invert = false;

    Cycle src_cycle(_vertices, _edges);

    for (i = 1; i < vertexCount(); i++)
        if (_vertices[i] < _vertices[min_idx])
            min_idx = i;

    int prev_idx = std::max(0, min_idx - 1);
    int next_idx = std::min(vertexCount() - 1, min_idx + 1);

    // rotate direction
    if (_vertices[prev_idx] < _vertices[next_idx])
        vert_invert = true;

    // rotate
    if (vert_invert)
    {
        for (i = 0; i < min_idx + 1; i++)
        {
            _vertices[i] = src_cycle._vertices[min_idx - i];
            _edges[i] = src_cycle._edges[i == min_idx ? vertexCount() - 1 : min_idx - i - 1];
        }
        for (; i < vertexCount(); i++)
        {
            _vertices[i] = src_cycle._vertices[min_idx - i + vertexCount()];
            _edges[i] = src_cycle._edges[min_idx - i + vertexCount() - 1];
        }
    }
    else
    {
        for (i = 0; i < vertexCount() - min_idx; i++)
        {
            _vertices[i] = src_cycle._vertices[min_idx + i];
            _edges[i] = src_cycle._edges[min_idx + i];
        }
        for (; i < vertexCount(); i++)
        {
            _vertices[i] = src_cycle._vertices[min_idx + i - vertexCount()];
            _edges[i] = src_cycle._edges[min_idx + i - vertexCount()];
        }
    }
}

bool MoleculeLayoutGraph::Cycle::contains(const Cycle& another) const
{
    if (vertexCount() < another.vertexCount())
        return false;

    QS_DEF(Array<int>, vertex_found);

    vertex_found.clear_resize(_max_idx + 1);
    vertex_found.zerofill();

    for (int i = 0; i < vertexCount(); i++)
        vertex_found[_vertices[i]] = 1;

    for (int i = 0; i < another.vertexCount(); i++)
        if (another._vertices[i] >= vertex_found.size() || vertex_found[another._vertices[i]] == 0)
            return false;

    return true;
}

// Cycle sorting callback
// Order by size: 6, 5, 7, 8, 4, 3, 9, 10, 11, ..
// If cycles has the same size then Morgan code in descending order (higher first)
int MoleculeLayoutGraph::Cycle::compare_cb(int& idx1, int& idx2, void* context)
{
    const ObjPool<Cycle>& cycles = *(const ObjPool<Cycle>*)context;

    int size_freq[] = {6, 5, 7, 8, 4, 3};
    int freq_idx1, freq_idx2;

    for (freq_idx1 = 0; freq_idx1 < NELEM(size_freq); freq_idx1++)
        if (cycles[idx1].vertexCount() == size_freq[freq_idx1])
            break;

    for (freq_idx2 = 0; freq_idx2 < NELEM(size_freq); freq_idx2++)
        if (cycles[idx2].vertexCount() == size_freq[freq_idx2])
            break;

    if (freq_idx1 != freq_idx2)
        return freq_idx1 - freq_idx2;

    if (freq_idx1 == NELEM(size_freq) && cycles[idx1].vertexCount() != cycles[idx2].vertexCount())
        return cycles[idx1].vertexCount() - cycles[idx2].vertexCount();

    return cycles[idx2].morganCode() - cycles[idx1].morganCode();
}

void MoleculeLayoutGraphSmart::calcMorganCode()
{
    _calcMorganCodes();
}

long MoleculeLayoutGraphSmart::getMorganCode()
{
    return _total_morgan_code;
}

void MoleculeLayoutGraphSmart::assignFirstVertex(int v)
{
    _first_vertex_idx = v;
}
