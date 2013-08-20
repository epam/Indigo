#include "graph/graph_iterators.h"
#include "graph/graph.h"

using namespace indigo;

AutoIterator::AutoIterator( int idx ) : _idx(idx)
{
}

int AutoIterator::operator*() const
{
   return _idx;
}

bool AutoIterator::operator!=( const AutoIterator &other ) const
{
   if (_idx != other._idx)
      return true;

   return false;
}

VertexIter::VertexIter( Graph &owner, int idx ) : _owner(owner), AutoIterator(idx)
{
}

VertexIter & VertexIter::operator++()
{
   _idx = _owner.vertexNext(_idx);

   return *this;
}

VerticesAuto::VerticesAuto (Graph &owner) : _owner(owner)
{
}

VertexIter VerticesAuto::begin ()
{
   return VertexIter(_owner, _owner.vertexBegin());
}

VertexIter VerticesAuto::end ()
{
   return VertexIter(_owner, _owner.vertexEnd());
}

EdgeIter::EdgeIter( Graph &owner, int idx ) : _owner(owner), AutoIterator(idx)
{
}

EdgeIter & EdgeIter::operator++()
{
   _idx = _owner.edgeNext(_idx);

   return *this;
}

EdgesAuto::EdgesAuto (Graph &owner) : _owner(owner)
{
}

EdgeIter EdgesAuto::begin ()
{
   return EdgeIter(_owner, _owner.vertexBegin());
}

EdgeIter EdgesAuto::end ()
{
   return EdgeIter(_owner, _owner.vertexEnd());
}

NeighborIter::NeighborIter(const Vertex &owner, int idx) : _owner(owner), AutoIterator(idx)
{
}

NeighborIter & NeighborIter::operator++ ()
{
   _idx = _owner.neiNext(_idx);

   return *this;
}

NeighborsAuto::NeighborsAuto (const Vertex &owner) : _owner(owner)
{
}

NeighborIter NeighborsAuto::begin ()
{
   return NeighborIter(_owner, _owner.neiBegin());
}

NeighborIter NeighborsAuto::end ()
{
   return NeighborIter(_owner, _owner.neiEnd());
}