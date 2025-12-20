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

#include "graph/graph_iterators.h"
#include "graph/graph.h"

using namespace indigo;

AutoIterator::AutoIterator(int idx) : _idx(idx)
{
}

int AutoIterator::operator*() const
{
    return _idx;
}

bool AutoIterator::operator!=(const AutoIterator& other) const
{
    if (_idx != other._idx)
        return true;

    return false;
}

VertexIter::VertexIter(const Graph& owner, int idx) : AutoIterator(idx), _owner(owner)
{
}

VertexIter& VertexIter::operator++()
{
    _idx = _owner.vertexNext(_idx);

    return *this;
}

VerticesAuto::VerticesAuto(const Graph& owner) : _owner(owner)
{
}

VertexIter VerticesAuto::begin()
{
    return VertexIter(_owner, _owner.vertexBegin());
}

VertexIter VerticesAuto::end()
{
    return VertexIter(_owner, _owner.vertexEnd());
}

EdgeIter::EdgeIter(Graph& owner, int idx) : AutoIterator(idx), _owner(owner)
{
}

EdgeIter& EdgeIter::operator++()
{
    _idx = _owner.edgeNext(_idx);

    return *this;
}

EdgesAuto::EdgesAuto(Graph& owner) : _owner(owner)
{
}

EdgeIter EdgesAuto::begin()
{
    return EdgeIter(_owner, _owner.edgeBegin());
}

EdgeIter EdgesAuto::end()
{
    return EdgeIter(_owner, _owner.edgeEnd());
}

NeighborIter::NeighborIter(const Vertex& owner, int idx) : AutoIterator(idx), _owner(owner)
{
}

NeighborIter& NeighborIter::operator++()
{
    _idx = _owner.neiNext(_idx);

    return *this;
}

NeighborsAuto::NeighborsAuto(const Vertex& owner) : _owner(owner)
{
}

NeighborIter NeighborsAuto::begin()
{
    return NeighborIter(_owner, _owner.neiBegin());
}

NeighborIter NeighborsAuto::end()
{
    return NeighborIter(_owner, _owner.neiEnd());
}