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

#include "graph/embeddings_storage.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"

using namespace indigo;

IMPL_ERROR(GraphEmbeddingsStorage, "embeddings storage");

GraphEmbeddingsStorage::GraphEmbeddingsStorage()
{
    unique_by_edges = false;
    check_uniquencess = true;
    save_edges = false;
    save_mapping = false;
}

void GraphEmbeddingsStorage::_prepareForCompare(int id)
{
    _EmbeddingData& data = _embedding_data[id];
    if (data.sorted)
        return;

    _all_edges.qsort(data.edge_begin, data.edge_begin + data.edge_count - 1, IntCmpFunctor());
    _all_vertices.qsort(data.vertex_begin, data.vertex_begin + data.vertex_count - 1, IntCmpFunctor());
    data.sorted = true;
}

bool GraphEmbeddingsStorage::addEmbedding(const Graph& super, const Graph& sub, int* core_sub)
{
    // Add new item to the storage
    // If it isn't unque then remove it
    _EmbeddingData& data = _embedding_data.push();
    int added_index = _embedding_data.size() - 1;
    data.vertex_begin = _all_vertices.size();
    data.edge_begin = _all_edges.size();
    data.sub_mapping_begin = _all_mappings.size();

    if (save_mapping)
        _all_mappings.concat(core_sub, sub.vertexEnd());

    for (int i = sub.vertexBegin(); i != sub.vertexEnd(); i = sub.vertexNext(i))
        if (core_sub[i] >= 0)
            _all_vertices.push(core_sub[i]);

    if (unique_by_edges || save_edges)
    {
        for (int i = sub.edgeBegin(); i != sub.edgeEnd(); i = sub.edgeNext(i))
        {
            const Edge& e = sub.getEdge(i);
            if (core_sub[e.beg] < 0 || core_sub[e.end] < 0)
                // Such edge isn't mapped because one vertex is ignored
                continue;

            int edge_index = Graph::findMappedEdge(sub, super, i, core_sub);
            if (edge_index == -1)
                throw Error("Edge should be mapped");

            _all_edges.push(edge_index);
        }
    }

    data.vertex_count = _all_vertices.size() - data.vertex_begin;
    data.edge_count = _all_edges.size() - data.edge_begin;
    data.sub_mapping_count = _all_mappings.size() - data.sub_mapping_begin;

    dword hash = _calcSetHash(_all_vertices, data.vertex_begin, data.vertex_count);

    if (unique_by_edges)
        hash ^= _calcSetHash(_all_edges, data.edge_begin, data.edge_count);

    data.sorted = false;

    // Try to find element with the same hash
    const auto it = _map_hash_to_id.find(hash);
    int append_to = -1;
    if (it != _map_hash_to_id.end())
    {
        // Compare elements in the list
        int cur = it->second;
        while (true)
        {
            if (check_uniquencess && _compareEmbedding(cur, added_index))
            {
                // Such embedding already exists
                // Remove added element
                _all_vertices.resize(data.vertex_begin);
                _all_edges.resize(data.edge_begin);
                _all_mappings.resize(data.sub_mapping_begin);
                _embedding_data.pop();
                return false;
            }
            if (_embedding_data[cur].next == -1)
                break;
            cur = _embedding_data[cur].next;
        }
        append_to = cur;
    }

    data.next = -1;
    if (append_to != -1)
        // Append embedding to the list of embeddings with the same hashes
        _embedding_data[append_to].next = added_index;
    else
        // Insert embedding into map
        _map_hash_to_id.insert(std::make_pair(hash, added_index));

    return true;
}

bool GraphEmbeddingsStorage::isEmpty() const
{
    return _all_vertices.size() == 0;
}

int GraphEmbeddingsStorage::count() const
{
    return _embedding_data.size();
}

const int* GraphEmbeddingsStorage::getVertices(int emb_idx, int& count) const
{
    const _EmbeddingData& data = _embedding_data[emb_idx];
    count = data.vertex_count;
    return _all_vertices.ptr() + data.vertex_begin;
}

const int* GraphEmbeddingsStorage::getEdges(int emb_idx, int& count) const
{
    const _EmbeddingData& data = _embedding_data[emb_idx];
    count = data.edge_count;
    return _all_edges.ptr() + data.edge_begin;
}

const int* GraphEmbeddingsStorage::getMappingSub(int emb_idx, int& count) const
{
    const _EmbeddingData& data = _embedding_data[emb_idx];
    count = data.sub_mapping_count;
    return _all_mappings.ptr() + data.sub_mapping_begin;
}

void GraphEmbeddingsStorage::clear()
{
    _all_vertices.clear();
    _all_edges.clear();
    _embedding_data.clear();
    _map_hash_to_id.clear();
}

dword GraphEmbeddingsStorage::_calcSetHash(const Array<int>& set, int offset, int size)
{
    dword hash = 0;
    const int* data = set.ptr() + offset;
    for (int i = 0; i < size; i++)
        hash ^= data[i] * 0x8088405 + 1;
    return hash;
}

bool GraphEmbeddingsStorage::_compareEmbedding(int id, int id2)
{
    _prepareForCompare(id);
    _prepareForCompare(id2);

    _EmbeddingData& data = _embedding_data[id];
    _EmbeddingData& data2 = _embedding_data[id2];

    // Compare vertices
    if (data.vertex_count != data2.vertex_count)
        return false;
    for (int i = 0; i < data.vertex_count; i++)
        if (_all_vertices[i + data.vertex_begin] != _all_vertices[i + data2.vertex_begin])
            return false;

    // Compare edges
    if (unique_by_edges)
    {
        if (data.edge_count != data2.edge_count)
            return false;
        for (int i = 0; i < data.edge_count; i++)
            if (_all_edges[i + data.edge_begin] != _all_edges[i + data2.edge_begin])
                return false;
    }

    return true;
}
