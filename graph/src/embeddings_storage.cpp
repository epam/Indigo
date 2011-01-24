/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "base_cpp/tlscont.h"
#include "graph/graph.h"
#include "graph/embeddings_storage.h"

using namespace indigo;

GraphEmbeddingsStorage::GraphEmbeddingsStorage ()
{
   unique_by_edges = false;
}
 
bool GraphEmbeddingsStorage::addEmbedding (const Array<int> &vertices, const Array<int> &edges)
{
   dword hash = _calcSetHash(vertices);
   if (unique_by_edges)
      hash ^= _calcSetHash(edges);

   QS_DEF(Array<int>, sorted_vertices);
   sorted_vertices.copy(vertices);
   sorted_vertices.qsort(_cmp_int, 0);

   QS_DEF(Array<int>, sorted_edges);
   if (unique_by_edges)
   {
      sorted_edges.copy(edges);
      sorted_edges.qsort(_cmp_int, 0);
   }
   else
      sorted_edges.clear();

   // Try to find element with the same hash
   int *id = _map_hash_to_id.at2(hash);
   int append_to = -1;
   if (id != 0)
   {
      // Compare elements in the list
      int cur = *id;
      while (true)
      {
         if (_compareEmbedding(cur, sorted_vertices, sorted_edges))
            return false; // Such embedding already exists
         if (_embedding_data[cur].next == -1)
            break;
         cur = _embedding_data[cur].next;
      }
      append_to = cur;
   }

   // Add new item to the storage
   _EmbeddingData &data = _embedding_data.push();
   data.vertex_begin = _all_vertices.size();
   data.vertex_count = vertices.size();
   _all_vertices.concat(sorted_vertices);

   data.edge_begin = _all_edges.size();
   data.edge_count = edges.size();
   _all_edges.concat(sorted_edges);
   
   data.next = -1;
   if (append_to != -1)
      // Append embedding to the list of embeddings with the same hashes
      _embedding_data[append_to].next = _embedding_data.size() - 1;
   else
      // Insert embedding into map
      _map_hash_to_id.insert(hash, _embedding_data.size() - 1);


   return true;
}

bool GraphEmbeddingsStorage::addEmbedding (const Graph &super, const Graph &sub, int *core_sub)
{
   QS_DEF(Array<int>, vertices);
   QS_DEF(Array<int>, edges);

   vertices.clear();
   for (int i = sub.vertexBegin(); i != sub.vertexEnd(); i = sub.vertexNext(i))
      if (core_sub[i] >= 0)
         vertices.push(core_sub[i]);

   edges.clear();
   if (unique_by_edges)
   {
      for (int i = sub.edgeBegin(); i != sub.edgeEnd(); i = sub.edgeNext(i))
      {
         const Edge &e = sub.getEdge(i);
         if (core_sub[e.beg] < 0 || core_sub[e.end] < 0)
            // Such edge isn't mapped because one vertex is ignored
            continue;

         int edge_index = Graph::findMappedEdge(sub, super, i, core_sub);
         if (edge_index == -1)
            throw Error("Edge should be mapped");

         edges.push(edge_index);
      }
   }

   return addEmbedding(vertices, edges);
}

bool GraphEmbeddingsStorage::isEmpty () const
{
   return _all_vertices.size() == 0;
}

void GraphEmbeddingsStorage::clear()
{
   _all_vertices.clear();
   _all_edges.clear();
   _embedding_data.clear();
   _map_hash_to_id.clear();
}

dword GraphEmbeddingsStorage::_calcSetHash (const Array<int> &set)
{
   dword hash = 0;
   for (int i = 0; i < set.size(); i++)
      hash ^= set[i] * 0x8088405 + 1;
   return hash;
}

bool GraphEmbeddingsStorage::_compareEmbedding (int id, 
         const Array<int> &vertices, const Array<int> &edges)
{
   _EmbeddingData &data = _embedding_data[id];

   // Compare vertices
   if (data.vertex_count != vertices.size())
      return false;
   for (int i = 0; i < data.vertex_count; i++)
      if (_all_vertices[i + data.vertex_begin] != vertices[i])
         return false;

   // Compare edges
   if (data.edge_count != edges.size())
      return false;
   for (int i = 0; i < data.edge_count; i++)
      if (_all_edges[i + data.edge_begin] != edges[i])
         return false;

   return true;
}

int GraphEmbeddingsStorage::_cmp_int (int v1, int v2, void *)
{
   return v1 - v2;
}

