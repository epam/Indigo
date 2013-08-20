#include "graph/graph_iterators.h"
#include "graph/graph.h"

using namespace indigo;

VertexIter::VertexIter( Graph &owner, int idx ) : _owner(owner), _idx(idx)
{
}

int VertexIter::operator*() const
{
   return _idx;
}

bool VertexIter::operator!=( const VertexIter &other ) const
{
   if (_idx != other._idx)
      return true;

   return false;
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

EdgeIter::EdgeIter( Graph &owner, int idx ) : _owner(owner), _idx(idx)
{
}

int EdgeIter::operator*() const
{
   return _idx;
}

bool EdgeIter::operator!=( const EdgeIter &other ) const
{
   if (_idx != other._idx)
      return true;

   return false;
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

NeighborIter::NeighborIter(const Vertex &owner, int idx) : _owner(owner), _idx(idx)
{
}

int NeighborIter::operator* () const
{
   return _idx;
}

bool NeighborIter::operator!= (const NeighborIter &other) const
{
   if (_idx != other._idx)
      return true;

   return false;
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