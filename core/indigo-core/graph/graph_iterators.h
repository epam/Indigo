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
        VertexIter(Graph& owner, int idx);

        VertexIter& operator++();

    private:
        Graph& _owner;
    };

    class VerticesAuto
    {
    public:
        VerticesAuto(Graph& owner);

        VertexIter begin();
        VertexIter end();

    private:
        Graph& _owner;
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