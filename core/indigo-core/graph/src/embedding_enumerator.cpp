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

#include "graph/embedding_enumerator.h"

#include "base_c/defs.h"
#include "base_cpp/cancellation_handler.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"
#include "graph/graph_vertex_equivalence.h"

using namespace indigo;

IMPL_ERROR(EmbeddingEnumerator, "embedding enumerator");
IMPL_TIMEOUT_EXCEPTION(EmbeddingEnumerator, "embedding enumerator");

CP_DEF(EmbeddingEnumerator);

EmbeddingEnumerator::EmbeddingEnumerator(Graph& supergraph)
    : CP_INIT, TL_CP_GET(_core_1), TL_CP_GET(_core_2), TL_CP_GET(_term2), TL_CP_GET(_unterm2), TL_CP_GET(_s_pool), TL_CP_GET(_g1_fast), TL_CP_GET(_g2_fast),
      TL_CP_GET(_query_match_state), TL_CP_GET(_enumerators)
{
    _g2 = &supergraph;
    _core_2.clear();
    validate();

    cb_embedding = 0;
    cb_match_vertex = 0;
    cb_match_edge = 0;
    cb_vertex_remove = 0;
    cb_edge_add = 0;
    cb_vertex_add = 0;
    userdata = 0;

    _cancellation_handler = getCancellationHandler();
    _cancellation_check_number = 0;

    allow_many_to_one = false;

    _equivalence_handler = NULL;

    _enumerators.clear();
    _enumerators.push(*this);
}

EmbeddingEnumerator::~EmbeddingEnumerator()
{
}

void EmbeddingEnumerator::validate()
{
    // _core_2 must be preserved because there might be fixed vertices
    _core_2.expandFill(_g2->vertexEnd(), -1);
    _g2_fast.setGraph(*_g2);
}

void EmbeddingEnumerator::setSubgraph(Graph& subgraph)
{
    //   if (subgraph.vertexCount() < 1)
    //      throw Error("empty subgraph given");

    _g1 = &subgraph;

    _core_1.clear_resize(_g1->vertexEnd());
    _core_1.fffill(); // fill with UNMAPPED
    _t1_len_pre = 0;

    _terminatePreviousMatch();

    _g1_fast.setGraph(*_g1);
}

void EmbeddingEnumerator::ignoreSubgraphVertex(int idx)
{
    if (_g1 == 0)
        throw Error("no subgraph");

    _core_1[idx] = IGNORE;
}

void EmbeddingEnumerator::ignoreSupergraphVertex(int idx)
{
    _core_2[idx] = IGNORE;
}

void EmbeddingEnumerator::_terminatePreviousMatch()
{
    for (int i = _g2->vertexBegin(); i < _g2->vertexEnd(); i = _g2->vertexNext(i))
        if (_core_2[i] >= 0)
            _core_2[i] = IGNORE;
        else if (_core_2[i] == TERM_OUT)
            _core_2[i] = UNMAPPED;

    _term2.clear();
    _unterm2.clear();
    _enumerators[0].reset();
    _query_match_state.clear();
}

void EmbeddingEnumerator::setEquivalenceHandler(GraphVertexEquivalence* equivalence_handler)
{
    _equivalence_handler = equivalence_handler;
}

bool EmbeddingEnumerator::fix(int node1, int node2)
{
    return _enumerators[0].fix(node1, node2, true);
}

bool EmbeddingEnumerator::unsafeFix(int node1, int node2)
{
    return _enumerators[0].fix(node1, node2, false);
}

int EmbeddingEnumerator::process()
{
    processStart();

    if (processNext())
        return 0;

    return 1;
}

void EmbeddingEnumerator::processStart()
{
    if (_g1 == 0)
        throw Error("subgraph not set");

    if (_equivalence_handler != NULL)
        _equivalence_handler->prepareForQueries();

    if (_equivalence_handler != NULL)
        _enumerators[0].setUseEquivalence(_equivalence_handler->useHeuristicFurther());
    else
        _enumerators[0].setUseEquivalence(false);

    // Restore enumerators stack
    while (_enumerators.size() > 1)
        _enumerators.pop();

    //
    // Save query indices ordered by preserving connectivity by walk
    // according to vertex numbers
    //
    QS_DEF(Array<int>, core1_pre);
    core1_pre.copy(_core_1);
    int t1_len_saved = _t1_len_pre;

    _query_match_state.clear();
    const int FIX_MARK = _g2->vertexEnd();

    int node1;
    while ((node1 = _getNextNode1()) != -1)
    {
        // Find node parent
        const Vertex& v = _g1->getVertex(node1);

        int parent = -1;
        for (int j = v.neiBegin(); j != v.neiEnd(); j = v.neiNext(j))
        {
            int nei_vertex = v.neiVertex(j);
            if (_core_1[nei_vertex] >= 0)
            {
                parent = nei_vertex;
                break;
            }
        }

        _query_match_state.push(_QuertMatchState(node1, parent, _t1_len_pre));

        _fixNode1(node1, FIX_MARK);
    }
    // Push last element to indicate the end of query atoms queue
    _query_match_state.push(_QuertMatchState(-1, -1, -1));

    // Restore core_1
    _core_1.copy(core1_pre);
    _t1_len_pre = t1_len_saved;
    _enumerators[0].initForFirstSearch(_t1_len_pre);
}

void EmbeddingEnumerator::_fixNode1(int node1, int node2)
{
    if (_core_1[node1] == TERM_OUT)
        _t1_len_pre--;

    _core_1[node1] = node2;

    const Vertex& v1 = _g1->getVertex(node1);
    for (int i = v1.neiBegin(); i != v1.neiEnd(); i = v1.neiNext(i))
    {
        int other1 = v1.neiVertex(i);

        if (_core_1[other1] == UNMAPPED)
        {
            _core_1[other1] = TERM_OUT;
            _t1_len_pre++;
        }
    }
}

int EmbeddingEnumerator::_getNextNode1()
{
    for (int i = _g1->vertexBegin(); i != _g1->vertexEnd(); i = _g1->vertexNext(i))
    {
        int val = _core_1[i];
        if (val == TERM_OUT)
            return i;
        if (_t1_len_pre == 0 && val == UNMAPPED)
            return i;
    }
    return -1;
}

bool EmbeddingEnumerator::processNext()
{
    if (_enumerators.size() > 1)
    {
        _enumerators.top().restore();
        _enumerators.pop();
    }

    while (1)
    {
        int command = _enumerators.top().nextPair();

        if (command == _NOWAY)
        {
            if (_enumerators.size() > 1)
            {
                _enumerators.top().restore();
                _enumerators.pop();
            }
            else
                break;
        }
        else if (command == _ADD_PAIR)
        {
            int node1 = _enumerators.top()._current_node1;
            int node2 = _enumerators.top()._current_node2;

            _enumerators.reserve(_enumerators.size() + 1);
            _enumerators.push(_enumerators.top());
            _enumerators.top().addPair(node1, node2);
        }
        else if (command == _RETURN0)
            return true;

        if (_cancellation_handler != nullptr)
        {
            // Check only each 100th time
            if ((_cancellation_check_number % 100) == 0)
                if (_cancellation_handler->isCancelled())
                    throw TimeoutException("%s", _cancellation_handler->cancelledRequestMessage());
            _cancellation_check_number++;
        }
    }

    while (_enumerators.size() > 1)
        _enumerators.pop();

    return false;
}

EmbeddingEnumerator::_Enumerator::_Enumerator(EmbeddingEnumerator& context) : _context(context), _mapped_orbit_ids(context._s_pool)
{
    _t1_len = 0;
    _t2_len = 0;
    _core_len = 0;

    _selected_node1 = -1;
    _selected_node2 = -1;

    _use_equivalence = false;

    _initState();
    _current_node1_idx = 0;
}

EmbeddingEnumerator::_Enumerator::_Enumerator(const EmbeddingEnumerator::_Enumerator& other)
    : _context(other._context), _mapped_orbit_ids(other._context._s_pool)
{
    _core_len = other._core_len;
    _t1_len = other._t1_len;
    _t2_len = other._t2_len;

    if (other._use_equivalence)
        _use_equivalence = _context._equivalence_handler->useHeuristicFurther();
    else
        _use_equivalence = false;

    _initState();
    _current_node1_idx = other._current_node1_idx;
}

void EmbeddingEnumerator::_Enumerator::_initState()
{
    _current_node2 = -1;
    _current_node2_idx = -1;
    _current_node2_parent = -1;
    _current_node2_nei_index = -1;

    _term2_begin = _context._term2.size();
    _unterm2_begin = _context._unterm2.size();
}

void EmbeddingEnumerator::_Enumerator::_fixPair(int node1, int node2)
{
    _context._fixNode1(node1, node2);
    _t1_len = _context._t1_len_pre;
    _addPairNode2(node1, node2);
}

void EmbeddingEnumerator::_Enumerator::addPair(int node1, int node2)
{
    // Check if such node is added as expected
    if (_context._query_match_state[_current_node1_idx].atom_index != node1)
        throw Error("internal error: query atom %d is unexpected in addPair", node1);
    _current_node1_idx++;
    const _QuertMatchState& s = _context._query_match_state[_current_node1_idx];
    _current_node1 = s.atom_index;
    _t1_len = s.t1_len;

    _addPairNode2(node1, node2);
}

void EmbeddingEnumerator::_Enumerator::_addPairNode2(int node1, int node2)
{
    if (_context._core_2[node2] == TERM_OUT)
        _t2_len--;

    _selected_node1 = node1;
    _selected_node2 = node2;

    _node1_prev_value = _context._core_1[node1];
    _node2_prev_value = _context._core_2[node2];

    _context._core_1[node1] = node2;
    _context._core_2[node2] = node1;

    _core_len++;

    int i;

    if (_t1_len > 0)
    {
        int node2_nei_count;
        int* node2_nei_v = _context._g2_fast.getVertexNeiVertices(node2, node2_nei_count);
        for (i = 0; i < node2_nei_count; i++)
        {
            int other2 = node2_nei_v[i];

            if (_context._core_2[other2] == UNMAPPED)
            {
                _context._core_2[other2] = TERM_OUT;
                _t2_len++;
                _context._term2.push(other2);
            }
        }
    }
    else
    {
        // A connected component of subgraph has been mapped.
        // Need to reset TERM_OUT flags on supergraph.
        // Vertices only from _term2 array can have TERM_OUT marks
        int* t2_ptr = _context._term2.ptr();
        int t2_size = _context._term2.size();
        for (i = 0; i < t2_size; i++)
        {
            int v = t2_ptr[i];
            if (_context._core_2[v] == TERM_OUT)
            {
                _context._core_2[v] = UNMAPPED;
                _context._unterm2.push(v);
            }
        }

        _t2_len = 0;
    }

    if (_use_equivalence)
        _context._equivalence_handler->fixVertex(node2);
}

bool EmbeddingEnumerator::_Enumerator::_checkNode2(int node2, int for_node1)
{
    int val = _context._core_2[node2];

    if (val == TERM_OUT)
        return true;

    if (_t2_len == 0 && val == UNMAPPED)
        return true;

    if (_context.allow_many_to_one)
    {
        if (val == IGNORE)
            return false;
        if (_context.cb_allow_many_to_one != 0)
        {
            if (!_context.cb_allow_many_to_one(*_context._g1, for_node1, _context.userdata))
                return false;
            // Check node that has already been mapped
            if (val >= 0 && !_context.cb_allow_many_to_one(*_context._g1, val, _context.userdata))
                return false;
        }

        return true;
    }

    return false;
}

bool EmbeddingEnumerator::_Enumerator::_checkPair(int node1, int node2)
{
    if (_context.cb_match_vertex != 0)
        if (!_context.cb_match_vertex(*_context._g1, *_context._g2, _context._core_1.ptr(), node1, node2, _context.userdata))
            return false;

    int j;
    bool needRemove = false;

    int node1_nei_count;
    int* node1_nei_v = _context._g1_fast.getVertexNeiVertices(node1, node1_nei_count);
    int* node1_nei_e = _context._g1_fast.getVertexNeiEdges(node1, node1_nei_count);

    for (j = 0; j < node1_nei_count; j++)
    {
        int other1 = node1_nei_v[j];
        int other2 = _context._core_1[other1];

        if (other2 >= 0)
        {
            int edge1 = node1_nei_e[j];
            int edge2 = _context._g2_fast.findEdgeIndex(node2, other2);

            if (edge2 == -1)
                break;

            if (_context.cb_match_edge != 0)
                if (!_context.cb_match_edge(*_context._g1, *_context._g2, edge1, edge2, _context.userdata))
                    break;

            if (_context.cb_edge_add != 0)
                _context.cb_edge_add(*_context._g1, *_context._g2, edge1, edge2, _context.userdata);
            needRemove = true;
        }
    }

    if (j != node1_nei_count)
    {
        if (needRemove && _context.cb_vertex_remove != 0)
            _context.cb_vertex_remove(*_context._g1, node1, _context.userdata);
        return false;
    }

    // This is vertex equivalence heuristics.
    if (_use_equivalence)
    {
        int eq_class = _context._equivalence_handler->getVertexEquivalenceClassId(node2);

        // Check if class isn't trivial
        if (eq_class != -1)
        {
            int pair_id = (node1 << 16) + eq_class;

            if (_mapped_orbit_ids.find_or_insert(pair_id))
            {
                if (needRemove && _context.cb_vertex_remove != 0)
                    _context.cb_vertex_remove(*_context._g1, node1, _context.userdata);
                return false;
            }
        }
    }

    if (_context.cb_vertex_add != 0)
        _context.cb_vertex_add(*_context._g1, *_context._g2, node1, node2, _context.userdata);

    return true;
}

void EmbeddingEnumerator::_Enumerator::restore()
{
    int i, size;
    int* data;

    size = _context._term2.size();
    data = _context._term2.ptr();
    for (i = _term2_begin; i < size; i++)
        _context._core_2[data[i]] = UNMAPPED;
    _context._term2.resize(_term2_begin);

    size = _context._unterm2.size();
    data = _context._unterm2.ptr();
    for (i = _unterm2_begin; i < size; i++)
        _context._core_2[data[i]] = TERM_OUT;
    _context._unterm2.resize(_unterm2_begin);

    if (_selected_node1 >= 0)
    {
        _context._core_1[_selected_node1] = _node1_prev_value;
        _context._core_2[_selected_node2] = _node2_prev_value;

        if (_context.cb_vertex_remove != 0)
            _context.cb_vertex_remove(*_context._g1, _selected_node1, _context.userdata);

        if (_use_equivalence)
            _context._equivalence_handler->unfixVertex(_selected_node2);
    }
}

void EmbeddingEnumerator::_Enumerator::initForFirstSearch(int t1_len)
{
    _t1_len = t1_len;
    _current_node1_idx = 0;
    _current_node1 = _context._query_match_state[_current_node1_idx].atom_index;
}

int EmbeddingEnumerator::_Enumerator::nextPair()
{
    if (_current_node1 == -1)
    {
        // _RETURN0 should be returned only once.
        _current_node1 = -2;
        // all nodes of subgraph are mapped
        if (_context.cb_embedding == 0 ||
            _context.cb_embedding(*_context._g1, *_context._g2, _context._core_1.ptr(), _context._core_2.ptr(), _context.userdata) == 0)
            return _RETURN0;
        else
            return _NOWAY;
    }
    if (_current_node1 == -2)
        return _NOWAY;

    // check for dead state
    if (_t1_len > _t2_len && !_context.allow_many_to_one)
        return _NOWAY;

    if (_t2_len == 0)
    {
        int v2_count;
        int* g2_vertices = _context._g2_fast.prepareVertices(v2_count);

        // If _current_node2_idx == -1 then _current_node2_idx will be 0
        _current_node2_idx++;

        for (; _current_node2_idx < v2_count; _current_node2_idx++)
        {
            _current_node2 = g2_vertices[_current_node2_idx];
            if (!_checkNode2(_current_node2, _current_node1))
                continue;

            if (!_checkPair(_current_node1, _current_node2))
                continue;

            break;
        }

        if (_current_node2_idx == v2_count)
            return _NOWAY;
    }
    else
    {
        // Find parent vertex for query _current_node1 vertex
        // and take coresponding vertex in target
        if (_current_node2_parent == -1)
        {
            int node1_parent = _context._query_match_state[_current_node1_idx].parent_index;
            if (node1_parent == -1)
                throw Error("internal error: node1_parent == -1");

            _current_node2_parent = _context._core_1[node1_parent];
            if (_current_node2_parent < 0)
                throw Error("_current_node2_parent < 0");
        }

        int nei_count;
        int node2_parent_nei_id = _context._g2_fast.prepareVertexNeiVertices(_current_node2_parent, nei_count);

        _current_node2_nei_index++;
        for (; _current_node2_nei_index != nei_count; _current_node2_nei_index++)
        {
            _current_node2 = _context._g2_fast.getVertexNeiVertiex(node2_parent_nei_id, _current_node2_nei_index);

            if (!_checkNode2(_current_node2, _current_node1))
                continue;

            if (!_checkPair(_current_node1, _current_node2))
                continue;

            break;
        }
        if (_current_node2_nei_index == nei_count)
            return _NOWAY;
    }

    return _ADD_PAIR;
}

bool EmbeddingEnumerator::_Enumerator::fix(int node1, int node2, bool safe)
{
    if (_context._core_1[node1] != UNMAPPED && _context._core_1[node1] != TERM_OUT)
        return false;

    if (_context._core_2[node2] != UNMAPPED && _context._core_2[node2] != TERM_OUT)
        return false;

    if (safe && !_checkPair(node1, node2))
        return false;

    _fixPair(node1, node2);

    return true;
}

void EmbeddingEnumerator::_Enumerator::setUseEquivalence(bool use)
{
    _use_equivalence = use;
}

void EmbeddingEnumerator::_Enumerator::reset()
{
    _mapped_orbit_ids.clear();

    _current_node1_idx = 0;
    _initState();
}

const int* EmbeddingEnumerator::getSubgraphMapping()
{
    return _core_1.ptr();
}

const int* EmbeddingEnumerator::getSupergraphMapping()
{
    return _core_2.ptr();
}

int EmbeddingEnumerator::countUnmappedSubgraphVertices()
{
    if (_g1 == 0)
        throw Error("subgraph not set");

    int i, res = 0;

    for (i = _g1->vertexBegin(); i != _g1->vertexEnd(); i = _g1->vertexNext(i))
        if (_core_1[i] == TERM_OUT || _core_1[i] == UNMAPPED)
            res++;

    return res;
}

int EmbeddingEnumerator::countUnmappedSupergraphVertices()
{
    int i, res = 0;

    for (i = _g2->vertexBegin(); i != _g2->vertexEnd(); i = _g2->vertexNext(i))
        if (_core_2[i] == TERM_OUT || _core_2[i] == UNMAPPED)
            res++;

    return res;
}

int EmbeddingEnumerator::countUnmappedSubgraphEdges()
{
    int i, res = 0;

    for (i = _g1->edgeBegin(); i != _g1->edgeEnd(); i = _g1->edgeNext(i))
    {
        const Edge& edge = _g1->getEdge(i);

        if (_core_1[edge.beg] != TERM_OUT && _core_1[edge.beg] != UNMAPPED)
            continue;
        if (_core_1[edge.end] != TERM_OUT && _core_1[edge.end] != UNMAPPED)
            continue;

        res++;
    }

    return res;
}

int EmbeddingEnumerator::countUnmappedSupergraphEdges()
{
    int i, res = 0;

    for (i = _g2->edgeBegin(); i != _g2->edgeEnd(); i = _g2->edgeNext(i))
    {
        const Edge& edge = _g2->getEdge(i);

        if (_core_2[edge.beg] != TERM_OUT && _core_2[edge.beg] != UNMAPPED)
            continue;
        if (_core_2[edge.end] != TERM_OUT && _core_2[edge.end] != UNMAPPED)
            continue;

        res++;
    }

    return res;
}
