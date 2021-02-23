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

#include "graph/skew_symmetric_network.h"

using namespace indigo;

IMPL_ERROR(SkewSymmetricNetwork, "skew symmetric network");

const Graph& SkewSymmetricNetwork::g() const
{
    return _g;
}

void SkewSymmetricNetwork::clear()
{
    _g.clear();
    _symmetry.clear();
    _arcs.clear();
    _source = -1;
}

void SkewSymmetricNetwork::setSource(int source)
{
    _source = source;
}

int SkewSymmetricNetwork::getSource() const
{
    return _source;
}

int SkewSymmetricNetwork::addVertex(int* symmetry_vertex)
{
    int v = _g.addVertex();
    int v_sym = _g.addVertex();

    _symmetry.resize(_g.vertexEnd());
    _symmetry[v] = v_sym;
    _symmetry[v_sym] = v;
    if (symmetry_vertex != NULL)
        *symmetry_vertex = v_sym;
    return v;
}

void SkewSymmetricNetwork::removeVertex(int vertex)
{
    int v_sym = getSymmetricVertex(vertex);
    _g.removeVertex(vertex);
    _g.removeVertex(v_sym);
}

int SkewSymmetricNetwork::addArc(int from, int to, int capacity)
{
    int from_sym = getSymmetricVertex(from);
    int to_sym = getSymmetricVertex(to);

    if (_g.haveEdge(from, to))
        throw Error("both directions arcs are not supported");
    if (_g.haveEdge(from_sym, to_sym))
        throw Error("inconsistent skew-symmetric network state");

    int edge = _g.addEdge(from, to);
    int edge_sym = _g.addEdge(to_sym, from_sym);

    _arcs.resize(_g.edgeEnd());
    _arcs[edge].from = from;
    _arcs[edge].to = to;
    _arcs[edge].capacity = capacity;

    _arcs[edge_sym].from = to_sym;
    _arcs[edge_sym].to = from_sym;
    _arcs[edge_sym].capacity = capacity;

    return edge;
}

void SkewSymmetricNetwork::removeArc(int from, int to)
{
    int edge = _g.findEdgeIndex(from, to);
    int edge_sym = getSymmetricArc(edge);

    _g.removeEdge(edge);
    _g.removeEdge(edge_sym);
}

int SkewSymmetricNetwork::getSymmetricVertex(int vertex) const
{
    return _symmetry[vertex];
}

int SkewSymmetricNetwork::getArcType(int edge, int vertex) const
{
    const Arc& arc = _arcs[edge];
    if (arc.from == vertex)
        return ARC_OUT;
    else if (arc.to == vertex)
        return ARC_IN;
    else
        throw Error("invalid edge passed in getArcType method");
}

int SkewSymmetricNetwork::getArcCapacity(int edge) const
{
    return _arcs[edge].capacity;
}

int SkewSymmetricNetwork::getSymmetricArc(int edge) const
{
    const Arc& arc = _arcs[edge];
    int from_sym = getSymmetricVertex(arc.from);
    int to_sym = getSymmetricVertex(arc.to);
    int edge_sym = _g.findEdgeIndex(from_sym, to_sym);

    return edge_sym;
}

void SkewSymmetricNetwork::setArcCapacity(int edge, int capacity)
{
    if (capacity < 0)
        throw Error("capacity can't be negative");

    int edge_sym = getSymmetricArc(edge);

    _arcs[edge].capacity = capacity;
    _arcs[edge_sym].capacity = capacity;
}
