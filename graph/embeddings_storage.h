/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __embeddings_storage__
#define __embeddings_storage__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/red_black.h"
#include "base_cpp/exception.h"
#include "graph/graph_fast_access.h"

namespace indigo {

class Graph;

class GraphEmbeddingsStorage
{
public:
   GraphEmbeddingsStorage ();

   // True if to take into account edges with checking for unique embedding
   bool unique_by_edges;

   bool check_uniquencess;
   bool save_edges, save_mapping;

   // Add embedding to the storage and returns true if such embedding 
   // hasn't already been in the storage.
   // Embeddings with the same set of vertices and edges are equivalent.
   bool addEmbedding (const Graph &super, const Graph &sub, int *core_sub);

   void clear();

   bool isEmpty () const;
   int count () const;

   const int* getVertices (int emb_idx, int &count) const;
   const int* getEdges (int emb_idx, int &count) const;
   const int* getMappingSub (int emb_idx, int &count) const;

   DECL_ERROR;
private:
   Array<int> _all_vertices, _all_edges, _all_mappings;
   struct _EmbeddingData
   {
      bool sorted;
      int vertex_begin, vertex_count;
      int edge_begin, edge_count;
      int sub_mapping_begin, sub_mapping_count;
      int next; // Index to the next
   };
   Array<_EmbeddingData> _embedding_data;
   RedBlackMap<dword, int> _map_hash_to_id;

   static dword _calcSetHash (const Array<int> &set, int offset, int size);
   bool _compareEmbedding(int id, int id2);

   void _prepareForCompare (int id);

   class IntCmpFunctor
   {
   public:
      int operator() (int v1, int v2)
      {
         return v1 - v2;
      }
   };
};

}

#endif // __embeddings_storage__
