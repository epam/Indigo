/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

class Graph;

class GraphEmbeddingsStorage
{
public:
   // Add embedding to the storage and returns true if such embedding 
   // hasn't already been in the storage.
   // Embeddings with the same set of vertices and edges are equivalent.
   bool addEmbedding (const Array<int> &vertices, const Array<int> &edges);

   bool addEmbedding (const Graph &super, const Graph &sub, int *core_sub);

   void clear();

   bool isEmpty () const;

   DEF_ERROR("embeddings storage");
private:
   Array<int> _all_vertices, _all_edges;
   struct _EmbeddingData
   {
      int vertex_begin, vertex_count;
      int edge_begin, edge_count;
      int next; // Index to the next
   };
   Array<_EmbeddingData> _embedding_data;
   RedBlackMap<dword, int> _map_hash_to_id;

   static dword _calcSetHash (const Array<int> &set);
   bool _compareEmbedding(int id, const Array<int> &vertices, const Array<int> &edges);

   static int _cmp_int (int v1, int v2, void *);
};

#endif // __embeddings_storage__
