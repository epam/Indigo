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

#ifndef __embeddings_storage__
#define __embeddings_storage__

#include <map>

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/red_black.h"
#include "graph/graph_fast_access.h"

namespace indigo
{

    class Graph;

    class GraphEmbeddingsStorage
    {
    public:
        GraphEmbeddingsStorage();

        // True if to take into account edges with checking for unique embedding
        bool unique_by_edges;

        bool check_uniquencess;
        bool save_edges, save_mapping;

        // Add embedding to the storage and returns true if such embedding
        // hasn't already been in the storage.
        // Embeddings with the same set of vertices and edges are equivalent.
        bool addEmbedding(const Graph& super, const Graph& sub, int* core_sub);

        void clear();

        bool isEmpty() const;
        int count() const;

        const int* getVertices(int emb_idx, int& count) const;
        const int* getEdges(int emb_idx, int& count) const;
        const int* getMappingSub(int emb_idx, int& count) const;

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
        std::map<dword, int> _map_hash_to_id;

        static dword _calcSetHash(const Array<int>& set, int offset, int size);
        bool _compareEmbedding(int id, int id2);

        void _prepareForCompare(int id);

        class IntCmpFunctor
        {
        public:
            int operator()(int v1, int v2)
            {
                return v1 - v2;
            }
        };
    };

} // namespace indigo

#endif // __embeddings_storage__
