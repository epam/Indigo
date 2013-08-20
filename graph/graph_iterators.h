#ifndef __graph_iterators_h__
#define __graph_iterators_h__

namespace indigo 
{
   class Graph;
   class Vertex;
   struct Edge;

   class VertexIter
   {
   public:
      VertexIter (Graph &owner, int idx);

      int operator* () const;

      bool operator!= ( const VertexIter &other ) const;

      VertexIter & operator++ ();

   private:
      Graph &_owner;
      int _idx;
   };

   class VerticesAuto
   {
   public:
      VerticesAuto (Graph &owner);

      VertexIter begin ();

      VertexIter end ();

   private:
      Graph &_owner;
   };

   class EdgeIter
   {
   public:
      EdgeIter (Graph &owner, int idx);

      int operator* () const;

      bool operator!= (const EdgeIter &other) const;

      EdgeIter & operator++ ();

   private:
      Graph &_owner;
      int _idx;
   };

   class EdgesAuto
   {
   public:
      EdgesAuto (Graph &owner);

      EdgeIter begin ();

      EdgeIter end ();

   private:
      Graph &_owner;
   };

   class NeighborIter
   {
   public:
      NeighborIter(const Vertex &owner, int idx);

      int operator* () const;

      bool operator!= (const NeighborIter &other) const;

      NeighborIter & operator++ ();

   private:
      const Vertex &_owner;
      int _idx;
   };

   class NeighborsAuto
   {
   public:
      NeighborsAuto ( const Vertex &owner);

      NeighborIter begin ();

      NeighborIter end ();

   private:
      const Vertex &_owner;
   };
};

#endif //__graph_iterators_h__