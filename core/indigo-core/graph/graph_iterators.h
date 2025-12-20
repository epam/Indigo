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

#ifndef __graph_iterators_h__
#define __graph_iterators_h__

#include "base_cpp/auto_iter.h"

namespace indigo
{
    class Graph;
    class Vertex;
    struct Edge;

    class VertexIter : public AutoIterator
    {
    public:
        VertexIter(const Graph& owner, int idx);

        VertexIter& operator++();

    private:
        const Graph& _owner;
    };

    class VerticesAuto
    {
    public:
        VerticesAuto(const Graph& owner);

        VertexIter begin();
        VertexIter end();

    private:
        const Graph& _owner;
    };

    class EdgeIter : public AutoIterator
    {
    public:
        EdgeIter(Graph& owner, int idx);

        EdgeIter& operator++();

    private:
        Graph& _owner;
    };

    class EdgesAuto
    {
    public:
        EdgesAuto(Graph& owner);

        EdgeIter begin();
        EdgeIter end();

    private:
        Graph& _owner;
    };

    class NeighborIter : public AutoIterator
    {
    public:
        NeighborIter(const Vertex& owner, int idx);

        NeighborIter& operator++();

    private:
        const Vertex& _owner;
    };

    class NeighborsAuto
    {
    public:
        NeighborsAuto(const Vertex& owner);

        NeighborIter begin();
        NeighborIter end();

    private:
        const Vertex& _owner;
    };
}; // namespace indigo

#endif //__graph_iterators_h__