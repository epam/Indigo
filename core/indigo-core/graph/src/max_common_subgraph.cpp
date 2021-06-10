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

#include "graph/max_common_subgraph.h"
#include "base_cpp/array.h"
#include "base_cpp/cancellation_handler.h"
#include "time.h"
#include <algorithm>

using namespace indigo;

IMPL_ERROR(MaxCommonSubgraph, "MCS");

MaxCommonSubgraph::MaxCommonSubgraph(Graph& subgraph, Graph& supergraph)
    : conditionEdgeWeight(0), conditionVerticesColor(0), cbSolutionTerm(0), userdata(0), cbEmbedding(0), embeddingUserdata(0), _subgraph(&subgraph),
      _supergraph(&supergraph)
{

    parametersForExact.isStopped = false;
    parametersForExact.maxIteration = -1;
    parametersForExact.numberOfSolutions = 0;
    parametersForExact.throw_error_for_incorrect_map = false;

    parametersForApproximate.error = 0;
    parametersForApproximate.maxIteration = 1000;
    parametersForApproximate.numberOfSolutions = 0;
    parametersForApproximate.standardRandom = false;
    parametersForApproximate.randomize = false;
}

MaxCommonSubgraph::~MaxCommonSubgraph()
{
}

void MaxCommonSubgraph::setGraphs(Graph& subgraph, Graph& supergraph)
{
    _subgraph = &subgraph;
    _supergraph = &supergraph;
}

bool MaxCommonSubgraph::_findTrivialMcs()
{
    _clearSolutionMaps();
    parametersForExact.numberOfSolutions = 0;
    if (_subgraph->vertexCount() == 0 && _supergraph->vertexCount() == 0)
        return true;

    if (_subgraph->vertexCount() > 1 && _supergraph->vertexCount() > 1)
        return false;

    QS_DEF(Array<int>, v_map);
    QS_DEF(Array<int>, e_map);
    v_map.resize(_subgraph->vertexEnd());
    for (int i = 0; i < v_map.size(); ++i)
        v_map[i] = -1;
    e_map.clear();

    if (_subgraph->vertexCount() == 1)
    {
        Graph& q_graph = *_subgraph;
        Graph& t_graph = *_supergraph;
        int v_q = q_graph.vertexBegin();
        int v_t = t_graph.vertexBegin();
        if (conditionVerticesColor == 0)
        {
            ++parametersForExact.numberOfSolutions;
            v_map[v_q] = v_t;
            _addSolutionMap(v_map, e_map);
            return true;
        }
        for (; v_t != t_graph.vertexEnd(); v_t = t_graph.vertexNext(v_t))
        {
            if (conditionVerticesColor(q_graph, t_graph, 0, v_q, v_t, userdata))
            {
                ++parametersForExact.numberOfSolutions;
                v_map[v_q] = v_t;
                _addSolutionMap(v_map, e_map);
                return true;
            }
        }
    }
    else if (_supergraph->vertexCount() == 1)
    {
        Graph& q_graph = *_supergraph;
        Graph& t_graph = *_subgraph;
        int v_q = q_graph.vertexBegin();
        int v_t = t_graph.vertexBegin();
        if (conditionVerticesColor == 0)
        {
            ++parametersForExact.numberOfSolutions;
            v_map[v_t] = v_q;
            _addSolutionMap(v_map, e_map);
            return true;
        }
        for (; v_t != t_graph.vertexEnd(); v_t = t_graph.vertexNext(v_t))
        {
            if (conditionVerticesColor(q_graph, t_graph, 0, v_q, v_t, userdata))
            {
                ++parametersForExact.numberOfSolutions;
                v_map[v_t] = v_q;
                _addSolutionMap(v_map, e_map);
                return true;
            }
        }
    }
    return true;
}

void MaxCommonSubgraph::_clearSolutionMaps()
{
    _vertEdgeSolMap.clear();
}

void MaxCommonSubgraph::_addSolutionMap(Array<int>& v_map, Array<int>& e_map)
{
    int v_size = v_map.size();
    int e_size = e_map.size();
    _vertEdgeSolMap.push().resize(v_size + e_size + 2);

    Array<int>& ve_map = _vertEdgeSolMap.top();
    for (int i = 0; i < ve_map.size(); ++i)
        ve_map[i] = -1;

    ve_map[0] = v_size;
    ve_map[1] = e_size;
    for (int i = 0; i < v_size; ++i)
    {
        ve_map[2 + i] = v_map[i];
    }
    for (int i = 0; i < e_size; ++i)
    {
        ve_map[2 + i + v_size] = e_map[i];
    }
}

void MaxCommonSubgraph::findExactMCS()
{
    /*
     * Check for single input molecules
     */
    if (_findTrivialMcs())
        return;

    ReGraph regraph;
    regraph.setMaxIteration(parametersForExact.maxIteration);

    ReCreation rc(regraph, *this);
    rc.createRegraph();

    bool find_all_str = rc.setMapping();

    regraph.cbEmbedding = cbEmbedding;
    regraph.userdata = embeddingUserdata;

    regraph.parse(find_all_str);

    parametersForExact.isStopped = regraph.stopped();
    parametersForExact.numberOfSolutions = rc.createSolutionMaps();
}

void MaxCommonSubgraph::findApproximateMCS()
{
    int max_vsize = std::max(_subgraph->vertexEnd(), _supergraph->vertexEnd());
    int max_esize = std::max(_subgraph->edgeEnd(), _supergraph->edgeEnd());
    int max_asize = std::max(max_vsize, max_esize);

    AdjMatricesStore ams(*this, max_asize);
    ams.create(*_subgraph, *_supergraph);

    Greedy gr(ams);
    gr.greedyMethod();

    RandomDisDec rdd(ams);
    rdd.setIterationNumber(parametersForApproximate.maxIteration);

    rdd.refinementStage();

    parametersForApproximate.error = rdd.getError();

    parametersForApproximate.numberOfSolutions = ams.createSolutionMaps();
}

bool MaxCommonSubgraph::_getEdgeColorCondition(Graph& graph1, Graph& graph2, int i, int j) const
{
    bool v_color, e_weight;
    if (conditionVerticesColor != 0)
    {
        bool a1 = conditionVerticesColor(graph1, graph2, 0, graph1.getEdge(i).beg, graph2.getEdge(j).beg, userdata);
        bool b1 = conditionVerticesColor(graph1, graph2, 0, graph1.getEdge(i).end, graph2.getEdge(j).end, userdata);
        bool a2 = conditionVerticesColor(graph1, graph2, 0, graph1.getEdge(i).beg, graph2.getEdge(j).end, userdata);
        bool b2 = conditionVerticesColor(graph1, graph2, 0, graph1.getEdge(i).end, graph2.getEdge(j).beg, userdata);
        v_color = (a1 && b1) || (a2 && b2);
    }
    else
        v_color = true;
    if (conditionEdgeWeight != 0)
        e_weight = conditionEdgeWeight(graph1, graph2, i, j, userdata);
    else
        e_weight = true;
    return (e_weight && v_color);
}

void MaxCommonSubgraph::getSolutionMaps(ObjArray<Array<int>>* v_maps, ObjArray<Array<int>>* e_maps) const
{
    QS_DEF(ObjArray<Array<int>>, tmp_v);
    QS_DEF(ObjArray<Array<int>>, tmp_e);
    if (v_maps == 0)
        v_maps = &tmp_v;
    if (e_maps == 0)
        e_maps = &tmp_e;

    _getSolutionMaps(_vertEdgeSolMap.size(), *v_maps, *e_maps);
}
void MaxCommonSubgraph::getMaxSolutionMap(Array<int>* v_map, Array<int>* e_map) const
{
    QS_DEF(ObjArray<Array<int>>, tmp_v);
    QS_DEF(ObjArray<Array<int>>, tmp_e);

    if (v_map)
        v_map->clear();
    if (e_map)
        e_map->clear();

    _getSolutionMaps(1, tmp_v, tmp_e);
    if (tmp_v.size() > 0)
    {
        if (v_map)
            v_map->copy(tmp_v[0]);
        if (e_map)
            e_map->copy(tmp_e[0]);
    }
}

// Exact method: Hanser's algorithm
//------------------------------------------------------------------------------------------------------------------
MaxCommonSubgraph::ReCreation::ReCreation(ReGraph& rgr, MaxCommonSubgraph& context) : _regraph(rgr), _context(context)
{
}

void MaxCommonSubgraph::ReCreation::createRegraph()
{
    _regraph.clear();
    if (_regraph.cancellation_handler != nullptr)
    {
        if (_regraph.cancellation_handler->isCancelled())
            throw Error("mcs search was cancelled: %s", _regraph.cancellation_handler->cancelledRequestMessage());
    }
    _nodeConstructor();
    _edgesConstructor();
}

void MaxCommonSubgraph::ReCreation::_nodeConstructor()
{
    Graph& sub_graph = *_context._subgraph;
    Graph& super_graph = *_context._supergraph;
    for (int i = sub_graph.edgeBegin(); i < sub_graph.edgeEnd(); i = sub_graph.edgeNext(i))
    {
        for (int j = super_graph.edgeBegin(); j < super_graph.edgeEnd(); j = super_graph.edgeNext(j))
        {
            if (_context._getEdgeColorCondition(sub_graph, super_graph, i, j))
                _regraph.addPoint(i, j);
        }
    }
}

void MaxCommonSubgraph::ReCreation::_edgesConstructor()
{
    Graph& sub_graph = *_context._subgraph;
    Graph& super_graph = *_context._supergraph;
    _regraph.setSizes(sub_graph.edgeEnd(), super_graph.edgeEnd());

    int regraph_size = _regraph.size();

    for (int i = 0; i < regraph_size; i++)
    {
        _regraph.getPoint(i)->setSizes(regraph_size, sub_graph.edgeEnd(), super_graph.edgeEnd());
        _regraph.getPoint(i)->forbidden.set(i);
    }

    for (int i = 0; i < regraph_size; i++)
    {
        for (int j = i + 1; j < regraph_size; j++)
        {
            int a1 = _regraph.getPoint(i)->getid1();
            int a2 = _regraph.getPoint(i)->getid2();
            int b1 = _regraph.getPoint(j)->getid1();
            int b2 = _regraph.getPoint(j)->getid2();
            if ((a1 == b1) || (a2 == b2) || !_hasCommonSymbol(a1, b1, a2, b2))
            {
                _regraph.getPoint(i)->forbidden.set(j);
                _regraph.getPoint(j)->forbidden.set(i);
            }
            else if (_hasCommonVertex(a1, b1, sub_graph))
            {
                _regraph.getPoint(i)->extension.set(j);
                _regraph.getPoint(j)->extension.set(i);
            }
            if ((a1 != b1) && (a2 != b2) && _hasCommonSymbol(a1, b1, a2, b2))
            {
                _regraph.getPoint(i)->allowed_g1.set(b1);
                _regraph.getPoint(i)->allowed_g2.set(b2);
                _regraph.getPoint(j)->allowed_g1.set(a1);
                _regraph.getPoint(j)->allowed_g2.set(a2);
            }
        }
    }
}

bool MaxCommonSubgraph::ReCreation::_hasCommonVertex(int e1, int e2, Graph& graph) const
{
    return (_getCommonVertex(e1, e2, graph) != -1);
}

int MaxCommonSubgraph::ReCreation::_getCommonVertex(int e1, int e2, Graph& graph) const
{
    int result = -1;
    if (graph.getEdge(e1).beg == graph.getEdge(e2).beg || graph.getEdge(e1).end == graph.getEdge(e2).beg)
    {
        result = graph.getEdge(e2).beg;
    }
    else if ((graph.getEdge(e1).beg == graph.getEdge(e2).end) || (graph.getEdge(e1).end == graph.getEdge(e2).end))
    {
        result = graph.getEdge(e2).end;
    }
    return result;
}

bool MaxCommonSubgraph::ReCreation::_hasCommonSymbol(int e11, int e12, int e21, int e22) const
{
    Graph& sub_graph = *_context._subgraph;
    Graph& super_graph = *_context._supergraph;
    int s1 = _getCommonVertex(e11, e12, sub_graph);
    int s2 = _getCommonVertex(e21, e22, super_graph);

    if (s1 == -1 && s2 == -1)
        return true;

    if (s1 != -1 && s2 != -1)
        return _context.conditionVerticesColor(sub_graph, super_graph, 0, s1, s2, _context.userdata);

    return false;
}

void MaxCommonSubgraph::ReCreation::_createList(const Dbitset& proj_bitset, Graph& graph, Array<int>& v_list, Array<int>& e_list) const
{
    int e_num, v_num1, v_num2;
    RedBlackSet<int> rb_set;

    v_list.clear();
    e_list.clear();
    rb_set.clear();

    for (int x = proj_bitset.nextSetBit(0); x >= 0; x = proj_bitset.nextSetBit(x + 1))
    {
        e_num = x;
        v_num1 = graph.getEdge(e_num).beg;
        v_num2 = graph.getEdge(e_num).end;
        rb_set.find_or_insert(v_num1);
        rb_set.find_or_insert(v_num2);
        e_list.push(e_num);
    }
    for (int i = rb_set.begin(); i < rb_set.end(); i = rb_set.next(i))
        v_list.push(rb_set.key(i));
}

void MaxCommonSubgraph::ReCreation::setCorrespondence(const Dbitset& bits, Array<int>& map) const
{
    Graph& sub_graph = *_context._subgraph;
    Graph& super_graph = *_context._supergraph;

    map.clear_resize(sub_graph.vertexEnd());
    for (int i = 0; i < map.size(); i++)
        map.at(i) = -1;
    int a, b, c, d, e, f, x, y;
    f = 0;
    for (x = bits.nextSetBit(0); x >= 0; x = bits.nextSetBit(x + 1))
        ++f;
    if (f == 1)
    {
        x = bits.nextSetBit(0);
        a = sub_graph.getEdge(_regraph.getPoint(x)->getid1()).beg;
        b = sub_graph.getEdge(_regraph.getPoint(x)->getid1()).end;
        c = super_graph.getEdge(_regraph.getPoint(x)->getid2()).beg;
        d = super_graph.getEdge(_regraph.getPoint(x)->getid2()).end;
        if (_context.conditionVerticesColor(sub_graph, super_graph, 0, a, c, _context.userdata))
        {
            map.at(a) = c;
            map.at(b) = d;
        }
        else
        {
            map.at(a) = d;
            map.at(b) = c;
        }
        return;
    }

    for (x = bits.nextSetBit(0); x >= 0; x = bits.nextSetBit(x + 1))
    {
        for (y = bits.nextSetBit(0); y >= 0; y = bits.nextSetBit(y + 1))
        {
            if (x != y)
            {
                a = _regraph.getPoint(x)->getid1();
                b = _regraph.getPoint(y)->getid1();
                e = _getCommonVertex(a, b, sub_graph);
                if (e != -1)
                {
                    c = _regraph.getPoint(x)->getid2();
                    d = _regraph.getPoint(y)->getid2();
                    f = _getCommonVertex(c, d, super_graph);

                    // first graph
                    if (sub_graph.getEdge(a).beg == e)
                    {
                        a = sub_graph.getEdge(a).end;
                    }
                    else
                    {
                        a = sub_graph.getEdge(a).beg;
                    }
                    if (sub_graph.getEdge(b).beg == e)
                    {
                        b = sub_graph.getEdge(b).end;
                    }
                    else
                    {
                        b = sub_graph.getEdge(b).beg;
                    }
                    // second graph
                    if (super_graph.getEdge(c).beg == f)
                    {
                        c = super_graph.getEdge(c).end;
                    }
                    else
                    {
                        c = super_graph.getEdge(c).beg;
                    }
                    if (super_graph.getEdge(d).beg == f)
                    {
                        d = super_graph.getEdge(d).end;
                    }
                    else
                    {
                        d = super_graph.getEdge(d).beg;
                    }
                    // set
                    map.at(e) = f;
                    map.at(a) = c;
                    map.at(b) = d;
                }
            }
        }
    }
}

bool MaxCommonSubgraph::ReCreation::insertSolution(const Array<int>& mapping)
{
    Graph& sub_graph = *_context._subgraph;
    Graph& super_graph = *_context._supergraph;
    Dbitset solution(_regraph.size());
    Dbitset solution_g1(sub_graph.edgeEnd());
    Dbitset solution_g2(super_graph.edgeEnd());

    int a, b, c;
    for (int i = sub_graph.vertexBegin(); i < sub_graph.vertexEnd(); i = sub_graph.vertexNext(i))
    {
        for (int j = sub_graph.vertexBegin(); j < sub_graph.vertexEnd(); j = sub_graph.vertexNext(j))
        {
            if (mapping.at(i) != -1 && mapping.at(j) != -1 && i != j)
            {
                super_graph.getVertex(mapping.at(i));
                super_graph.getVertex(mapping.at(j));
                a = sub_graph.findEdgeIndex(i, j);
                b = super_graph.findEdgeIndex(mapping.at(i), mapping.at(j));
                if (a != -1 && b != -1)
                {
                    c = _regraph.getPointIndex(a, b);
                    if (c == -1)
                    {
                        if (_context.parametersForExact.throw_error_for_incorrect_map)
                        {
                            throw Error("input mapping incorrect");
                        }
                    }
                    else
                    {
                        solution.set(c);
                        solution_g1.set(_regraph.getPoint(c)->getid1());
                        solution_g2.set(_regraph.getPoint(c)->getid2());
                    }
                }
            }
        }
    }
    if (!solution.isEmpty())
    {
        _regraph.insertSolution(0, true, solution, solution_g1, solution_g2, solution_g1.bitsNumber());
        return false;
    }
    return true;
}

bool MaxCommonSubgraph::ReCreation::setMapping()
{
    if (_context.incomingMap.size() == 0)
        return true;

    int vertex_number = 0;
    int vertex_idx = 0;
    for (int i = 0; i < _context.incomingMap.size(); ++i)
    {
        if (_context.incomingMap[i] >= 0)
        {
            ++vertex_number;
            vertex_idx = i;
        }
    }

    /*
     * One vertex map workaround
     * The algorithm works only with edges, therefore, we should create edges mapping (for every neighbor)
     */
    if (vertex_number == 0)
    {
        return true;
    }
    else if (vertex_number == 1)
    {
        QS_DEF(Array<int>, mapping);
        Graph& sub_graph = *_context._subgraph;
        Graph& super_graph = *_context._supergraph;
        const Vertex& vert_sub = sub_graph.getVertex(vertex_idx);
        const Vertex& vert_super = super_graph.getVertex(_context.incomingMap[vertex_idx]);
        bool result = true;
        /*
         * All possibilities for check
         */
        for (int i = vert_sub.neiBegin(); i != vert_sub.neiEnd(); i = vert_sub.neiNext(i))
        {
            int sub_nei_idx = vert_sub.neiVertex(i);
            for (int j = vert_super.neiBegin(); j != vert_super.neiEnd(); j = vert_super.neiNext(j))
            {
                int super_nei_idx = vert_super.neiVertex(j);
                /*
                 * Check vertex condition
                 */
                if (_context.conditionVerticesColor &&
                    !_context.conditionVerticesColor(sub_graph, super_graph, 0, sub_nei_idx, super_nei_idx, _context.userdata))
                {
                    continue;
                }
                /*
                 * Check edge condition
                 */
                int sub_nei_ed = vert_sub.neiEdge(i);
                int super_nei_ed = vert_super.neiEdge(j);
                if (_context.conditionEdgeWeight && !_context.conditionEdgeWeight(sub_graph, super_graph, sub_nei_ed, super_nei_ed, _context.userdata))
                {
                    continue;
                }
                mapping.copy(_context.incomingMap);
                mapping.at(sub_nei_idx) = super_nei_idx;
                result &= insertSolution(mapping);
            }
        }
        return result;
    }
    else
    {
        return insertSolution(_context.incomingMap);
    }
    return true;
}

int MaxCommonSubgraph::ReCreation::createSolutionMaps()
{
    QS_DEF(Array<int>, v_map);
    QS_DEF(Array<int>, e_map);

    Graph& sub_graph = *_context._subgraph;

    _context._vertEdgeSolMap.clear();
    int v_size = sub_graph.vertexEnd();
    int e_size = sub_graph.edgeEnd();

    for (int sol = _regraph.solBegin(); _regraph.solIsNotEnd(sol); sol = _regraph.solNext(sol))
    {
        const Dbitset& bits = _regraph.getSolBitset(sol);
        setCorrespondence(bits, v_map);
        v_map.resize(v_size);
        e_map.resize(e_size);
        for (int i = 0; i < e_size; ++i)
            e_map[i] = -1;

        for (int x = bits.nextSetBit(0); x >= 0; x = bits.nextSetBit(x + 1))
        {
            e_map[_regraph.getPoint(x)->getid1()] = _regraph.getPoint(x)->getid2();
        }
        _context._addSolutionMap(v_map, e_map);
    }

    if (_context.cbSolutionTerm == 0)
        _context._vertEdgeSolMap.qsort(_context.ringsSolutionTerm, 0);
    else
        _context._vertEdgeSolMap.qsort(_context.cbSolutionTerm, _context.userdata);

    return _context._vertEdgeSolMap.size();
}

void MaxCommonSubgraph::ReCreation::getSolutionListsSub(ObjArray<Array<int>>& v_lists, ObjArray<Array<int>>& e_lists) const
{
    v_lists.clear();
    e_lists.clear();

    Graph& sub_graph = *_context._subgraph;
    for (int x = _regraph.solBegin(); _regraph.solIsNotEnd(x); x = _regraph.solNext(x))
    {
        Array<int>& v_list = v_lists.push();
        Array<int>& e_list = e_lists.push();
        _createList(_regraph.getProj1Bitset(x), sub_graph, v_list, e_list);
    }
}

void MaxCommonSubgraph::ReCreation::getSolutionListsSuper(ObjArray<Array<int>>& v_lists, ObjArray<Array<int>>& e_lists) const
{
    v_lists.clear();
    e_lists.clear();

    Graph& super_graph = *_context._supergraph;
    for (int x = _regraph.solBegin(); _regraph.solIsNotEnd(x); x = _regraph.solNext(x))
    {
        Array<int>& v_list = v_lists.push();
        Array<int>& e_list = e_lists.push();
        _createList(_regraph.getProj2Bitset(x), super_graph, v_list, e_list);
    }
}

//-------------------------------------------------------------------------------------------------------------
MaxCommonSubgraph::ReGraph::ReGraph()
    : cbEmbedding(0), userdata(0), cancellation_handler(nullptr), _nbIteration(0), _maxIteration(-1), _firstGraphSize(0), _secondGraphSize(0),
      _findAllStructure(true), _stop(false), _solutionObjList(_pool)
{
    cancellation_handler = getCancellationHandler();
}

MaxCommonSubgraph::ReGraph::ReGraph(MaxCommonSubgraph& context)
    : cbEmbedding(0), userdata(0), cancellation_handler(nullptr), _nbIteration(0), _maxIteration(-1), _firstGraphSize(0), _secondGraphSize(0),
      _findAllStructure(true), _stop(false), _solutionObjList(_pool)
{
    setMaxIteration(context.parametersForExact.maxIteration);
    cancellation_handler = getCancellationHandler();
}

void MaxCommonSubgraph::ReGraph::setSizes(int n1, int n2)
{
    _firstGraphSize = n1;
    _secondGraphSize = n2;
}

void MaxCommonSubgraph::ReGraph::clear()
{
    _graph.clear();
    _solutionObjList.clear();
}

void MaxCommonSubgraph::ReGraph::parse(bool findAllStructure)
{
    _size = _graph.size();
    _findAllStructure = findAllStructure;

    Dbitset pnode_g1(_firstGraphSize);
    Dbitset pnode_g2(_secondGraphSize);

    QS_DEF(ObjArray<Dbitset>, traversed);
    QS_DEF(ObjArray<Dbitset>, extension);
    QS_DEF(ObjArray<Dbitset>, forbidden);
    QS_DEF(ObjArray<Dbitset>, traversed_g1);
    QS_DEF(ObjArray<Dbitset>, traversed_g2);
    QS_DEF(ObjArray<Dbitset>, allowed_g1);
    QS_DEF(ObjArray<Dbitset>, allowed_g2);
    QS_DEF(Array<int>, xk);

    traversed.clear();
    extension.clear();
    forbidden.clear();
    traversed_g1.clear();
    traversed_g2.clear();
    allowed_g1.clear();
    allowed_g2.clear();

    int max_size = std::min(_firstGraphSize, _secondGraphSize) + 1;
    xk.clear_resize(max_size);

    for (int i = 0; i < max_size; i++)
    {
        traversed.push(_size);
        extension.push(_size);
        forbidden.push(_size);
        traversed_g1.push(_firstGraphSize);
        traversed_g2.push(_secondGraphSize);
        allowed_g1.push(_firstGraphSize);
        allowed_g2.push(_secondGraphSize);
        xk[i] = -1;
    }
    extension[0].set();
    allowed_g1[0].set();
    allowed_g2[0].set();

    int level = 0;
    int next_level = 1, xk_level;

    while (1)
    {
        for (xk[level] = extension[level].nextSetBit(xk[level] + 1); xk[level] >= 0 && !_stop; xk[level] = extension[level].nextSetBit(xk[level] + 1))
        {
            next_level = level + 1;
            xk_level = xk[level];

            forbidden[next_level].bsOrBs(forbidden[level], _graph.at(xk_level)->forbidden);
            allowed_g1[next_level].bsAndBs(allowed_g1[level], _graph.at(xk_level)->allowed_g1);
            allowed_g2[next_level].bsAndBs(allowed_g2[level], _graph.at(xk_level)->allowed_g2);

            if (traversed[level].isEmpty())
            {
                extension[next_level].bsAndNotBs(_graph.at(xk_level)->extension, forbidden[next_level]);
            }
            else
            {
                extension[next_level].bsOrBs(extension[level], _graph.at(xk_level)->extension);
                extension[next_level].andNotWith(forbidden[next_level]);
            }

            traversed[next_level].copy(traversed[level]);
            traversed[next_level].set(xk_level);

            traversed_g1[next_level].copy(traversed_g1[level]);
            traversed_g2[next_level].copy(traversed_g2[level]);
            traversed_g1[next_level].set(_graph.at(xk_level)->getid1());
            traversed_g2[next_level].set(_graph.at(xk_level)->getid2());

            forbidden[level].set(xk_level);

            ++level;

            if (extension[level].isEmpty())
            {
                _solution(traversed[level], traversed_g1[level], traversed_g2[level]);
                xk[level] = -1;
                --level;
                if (level <= -1)
                    break;
            }
            else
            {

                pnode_g1.bsOrBs(allowed_g1[level], traversed_g1[level]);
                pnode_g2.bsOrBs(allowed_g2[level], traversed_g2[level]);

                if (_mustContinue(pnode_g1, pnode_g2))
                {
                    ++_nbIteration;
                    if (_maxIteration > -1 && _nbIteration >= _maxIteration)
                        _stop = true;
                    if (_nbIteration % 10 == 0)
                    {
                        if (cancellation_handler != nullptr)
                        {
                            if (cancellation_handler->isCancelled())
                                throw Error("mcs search was cancelled: %s", cancellation_handler->cancelledRequestMessage());
                        }
                    }
                }
                else
                {
                    xk[level] = -1;
                    --level;
                    if (level <= -1)
                        break;
                }
            }
        }
        --level;

        if (level <= -1)
            break;
    }

    // printf("iter = %d\n", _nbIteration);
    // printf("size = %d\n", _solutionObjList.size());
}
void MaxCommonSubgraph::ReGraph::insertSolution(int ins_index, bool ins_after, const Dbitset& sol, const Dbitset& sol_g1, const Dbitset& sol_g2, int num_bits)
{

    if (_solutionObjList.size() == 0)
    {
        ins_index = _solutionObjList.add();
    }
    else
    {
        if (ins_after)
        {
            ins_index = _solutionObjList.insertAfter(ins_index);
        }
        else
        {
            ins_index = _solutionObjList.insertBefore(ins_index);
        }
    }
    _solutionObjList.at(ins_index).reSolution.copy(sol);
    _solutionObjList.at(ins_index).solutionProj1.copy(sol_g1);
    _solutionObjList.at(ins_index).solutionProj2.copy(sol_g2);
    _solutionObjList.at(ins_index).numBits = num_bits;

    if (cbEmbedding != 0)
    {
        QS_DEF(Array<int>, sub_edge_map);
        sub_edge_map.resize(_firstGraphSize);
        sub_edge_map.zerofill();

        for (int x = sol.nextSetBit(0); x >= 0; x = sol.nextSetBit(x + 1))
        {
            sub_edge_map[_graph.at(x)->getid1()] = _graph.at(x)->getid2();
        }
        if (!cbEmbedding(0, sub_edge_map.ptr(), 0, userdata))
            _stop = true;
    }
}

void MaxCommonSubgraph::ReGraph::_solution(const Dbitset& traversed, Dbitset& trav_g1, Dbitset& trav_g2)
{

    bool included = false;
    bool str_include = false;
    bool subset, ins_after = false, first_undel = false;

    int num_bits = trav_g1.bitsNumber();
    int insert_idx = _solutionObjList.begin();
    int idx_next;
    int suu = 0;

    for (int i = _solutionObjList.begin(); i < _solutionObjList.end() && !included;)
    {
        ++suu;

        Solution& solution = _solutionObjList.at(i);
        if (num_bits < solution.numBits)
        {
            if (trav_g1.isSubsetOf(solution.solutionProj1) || trav_g2.isSubsetOf(solution.solutionProj2))
                included = true;
            insert_idx = i;
            ins_after = true;
        }
        if (num_bits >= solution.numBits)
        {
            if (_findAllStructure)
                subset = (solution.solutionProj1.isSubsetOf(trav_g1) || solution.solutionProj2.isSubsetOf(trav_g2));
            else
                subset =
                    (solution.solutionProj1.isSubsetOf(trav_g1) && solution.solutionProj2.isSubsetOf(trav_g2) && solution.reSolution.isSubsetOf(traversed));

            if (subset)
            {
                idx_next = _solutionObjList.next(i);
                _solutionObjList.remove(i);
                i = idx_next;
                str_include = true;
                continue;
            }
            if (!ins_after && !first_undel)
            {
                insert_idx = i;
                first_undel = true;
            }
        }
        i = _solutionObjList.next(i);
    }

    if (!included)
    {
        if (_findAllStructure)
        {
            insertSolution(insert_idx, ins_after, traversed, trav_g1, trav_g2, num_bits);
        }
        else if (str_include)
        {
            insertSolution(insert_idx, ins_after, traversed, trav_g1, trav_g2, num_bits);
        }
    }
}

bool MaxCommonSubgraph::ReGraph::_mustContinue(const Dbitset& pnode_g1, const Dbitset& pnode_g2) const
{
    bool result = true;
    int num_bits = std::min(pnode_g1.bitsNumber(), pnode_g2.bitsNumber());

    for (int i = _solutionObjList.begin(); i != _solutionObjList.end(); i = _solutionObjList.next(i))
    {
        Solution& solution = _solutionObjList.at(i);
        if (solution.numBits >= num_bits)
        {
            if (pnode_g1.isSubsetOf(solution.solutionProj1) || pnode_g2.isSubsetOf(solution.solutionProj2))
            {
                result = false;
                break;
            }
        }
        else
        {
            break;
        }
    }
    return result;
}

int MaxCommonSubgraph::ReGraph::getPointIndex(int i, int j) const
{
    for (int x = 0; x < _graph.size(); x++)
    {
        if ((_graph.at(x)->getid1() == i && _graph.at(x)->getid2() == j))
        {
            return x;
        }
    }
    return -1;
}
//-------------------------------------------------------------------------------------------------------------------
MaxCommonSubgraph::RePoint::RePoint(int n1, int n2) : _id1(n1), _id2(n2)
{
}

void MaxCommonSubgraph::RePoint::setSizes(int size, int size_g1, int size_g2)
{
    extension.resize(size);
    forbidden.resize(size);
    allowed_g1.resize(size_g1);
    allowed_g2.resize(size_g2);
}

// Approximate algorithm: two stage optimization method (2DOM)
//-------------------------------------------------------------------------------------------------------------------

// Adjacency matrix
MaxCommonSubgraph::AdjMatricesStore::AdjMatricesStore(MaxCommonSubgraph& context, int maxsize) : _context(context), _maxsize(maxsize), _swap(false)
{

    for (int i = 0; i < maxsize; i++)
    {
        _ajEdge1.add(new Array<int>());
        _ajEdge1[i]->resize(maxsize);
        _ajEdge2.add(new Array<int>());
        _ajEdge2[i]->resize(maxsize);
        _aj2.add(new Array<bool>());
        _aj2[i]->resize(maxsize);
        _errorEdgesMatrix.add(new Array<int>());
        _errorEdgesMatrix[i]->resize(maxsize);
        _daj1.add(new Dbitset(maxsize));
        _daj2.add(new Dbitset(maxsize));
    }
    _map.resize(maxsize);
    _invmap.resize(maxsize);
    _degreeVec1.resize(maxsize);
    _degreeVec2.resize(maxsize);
    _x.resize(maxsize);
    _y.resize(maxsize);
    _cr1.resize(maxsize);
    _cr2.resize(maxsize);
}

void MaxCommonSubgraph::AdjMatricesStore::create(Graph& g1, Graph& g2)
{
    _swap = _checkSize(g1, g2);
    if (_swap)
    {
        _graph1 = &g2;
        _graph2 = &g1;
    }
    else
    {
        _graph1 = &g1;
        _graph2 = &g2;
    }
    _createMaps();
    _createCorrespondence();
    _createAdjacencyMatrices();
    _createLabelMatrices();
    _createErrorEdgesMatrix();
}

int MaxCommonSubgraph::AdjMatricesStore::createSolutionMaps()
{
    QS_DEF(ObjArray<Array<int>>, v_maps);
    int sub_beg, sub_end, super_beg, super_end, super_edge;

    getSolutions(v_maps);

    _context._vertEdgeSolMap.clear();
    int v_size = _context._subgraph->vertexEnd();
    int e_size = _context._subgraph->edgeEnd();

    for (int sol = 0; sol < v_maps.size(); ++sol)
    {
        Array<int>& new_map = _context._vertEdgeSolMap.push();
        new_map.resize(2 + v_size + e_size);
        new_map[0] = v_size;
        new_map[1] = e_size;
        for (int i = 0; i < v_size; ++i)
            new_map[2 + i] = v_maps[sol].at(i);
        for (int i = 0; i < e_size; ++i)
            new_map[2 + i + v_size] = SubstructureMcs::UNMAPPED;

        for (int sub_edge = _context._subgraph->edgeBegin(); sub_edge < _context._subgraph->edgeEnd(); sub_edge = _context._subgraph->edgeNext(sub_edge))
        {
            sub_beg = _context._subgraph->getEdge(sub_edge).beg;
            sub_end = _context._subgraph->getEdge(sub_edge).end;
            super_beg = v_maps[sol].at(sub_beg);
            super_end = v_maps[sol].at(sub_end);

            if (super_beg >= 0 && super_end >= 0)
            {
                const Vertex& sup_vert = _context._supergraph->getVertex(super_beg);
                int k = sup_vert.findNeiVertex(super_end);
                if (k != -1)
                {
                    super_edge = sup_vert.neiEdge(k);
                    if (_context._getEdgeColorCondition(*_context._subgraph, *_context._supergraph, sub_edge, super_edge))
                        new_map[2 + v_size + sub_edge] = super_edge;
                }
            }
        }
    }
    if (_context.cbSolutionTerm == 0)
        _context._vertEdgeSolMap.qsort(_context.ringsSolutionTerm, 0);
    else
        _context._vertEdgeSolMap.qsort(_context.cbSolutionTerm, _context.userdata);

    return _context._vertEdgeSolMap.size();
}

void MaxCommonSubgraph::AdjMatricesStore::_setFirstElement(int i, int j, int value)
{
    _ajEdge1[i]->at(j) = value;
    if (value >= 0)
    {
        _daj1[i]->set(j, true);
    }
    else
    {
        _daj1[i]->set(j, false);
    }
}

void MaxCommonSubgraph::AdjMatricesStore::_setSecondElement(int i, int j, int value)
{
    _ajEdge2[i]->at(j) = value;
    if (value >= 0)
    {
        _aj2[i]->at(j) = true;
        _daj2[i]->set(j, true);
    }
    else
    {
        _daj2[i]->set(j, false);
        _aj2[i]->at(j) = false;
    }
}

void MaxCommonSubgraph::AdjMatricesStore::_createAdjacencyMatrices()
{
    int i, j, s;

    for (i = 0; i < _size1; i++)
    {
        _daj1[i]->zeroFill();
        for (j = 0; j < _size1; j++)
        {
            _setFirstElement(i, j, -1);
        }
    }

    for (i = _graph1->edgeBegin(); i < _graph1->edgeEnd(); i = _graph1->edgeNext(i))
    {
        int j1 = _getFirstC(_graph1->getEdge(i).beg);
        int j2 = _getFirstC(_graph1->getEdge(i).end);
        if (j1 >= 0 && j2 >= 0)
        {
            _setFirstElement(j1, j2, i);
            _setFirstElement(j2, j1, i);
        }
    }
    for (i = 0; i < _size1; i++)
    {
        s = 0;
        for (j = getFirstRow(i)->nextSetBit(0); j != -1; j = getFirstRow(i)->nextSetBit(j + 1))
            ++s;
        _degreeVec1[i] = s;
    }

    for (i = 0; i < _size2; i++)
    {
        _daj2[i]->zeroFill();
        for (j = 0; j < _size2; j++)
        {
            _setSecondElement(i, j, -1);
        }
    }

    for (i = _graph2->edgeBegin(); i < _graph2->edgeEnd(); i = _graph2->edgeNext(i))
    {
        int j1 = _getSecondC(_graph2->getEdge(i).beg);
        int j2 = _getSecondC(_graph2->getEdge(i).end);
        if (j1 >= 0 && j2 >= 0)
        {
            _setSecondElement(j1, j2, i);
            _setSecondElement(j2, j1, i);
        }
    }
    for (i = 0; i < _size2; i++)
    {
        s = 0;
        for (j = getSecondRow(i)->nextSetBit(0); j != -1; j = getSecondRow(i)->nextSetBit(j + 1))
            ++s;
        _degreeVec2[i] = s;
    }
}

void MaxCommonSubgraph::AdjMatricesStore::_createLabelMatrices()
{
    _mLabel1.clear();
    for (int i = 0; i < _size1; i++)
    {
        _mLabel1.add(new Array<int>());
        ;
    }

    for (int i = 0; i < _size1; i++)
    {
        for (int j = 0; j < _size2; j++)
        {
            if (getVerticesColorCondition(i, j))
            {
                _mLabel1[i]->push(j);
            }
        }
    }
}

void MaxCommonSubgraph::AdjMatricesStore::_createErrorEdgesMatrix()
{

    for (int i = 0; i < _maxsize; i++)
    {
        _errorEdgesMatrix[i]->zerofill();
    }
    for (int i = _graph1->edgeBegin(); i < _graph1->edgeEnd(); i = _graph1->edgeNext(i))
    {
        for (int j = _graph2->edgeBegin(); j < _graph2->edgeEnd(); j = _graph2->edgeNext(j))
        {
            if (!_context._getEdgeColorCondition(*_graph1, *_graph2, i, j))
                _errorEdgesMatrix[i]->at(j) = 1;
        }
    }
}
bool MaxCommonSubgraph::AdjMatricesStore::_checkSize(Graph& g1, Graph& g2)
{
    int size1 = g1.vertexCount();
    int size2 = g2.vertexCount();
    QS_DEF(Array<int>, inv_map);
    int i, j;
    _x.zerofill();
    _y.zerofill();
    if (_context.incomingMap.size() > 0)
    {
        inv_map.resize(g2.vertexEnd());
        for (int i = 0; i < inv_map.size(); i++)
            inv_map[i] = -1;
        _makeInvertMap(_context.incomingMap, inv_map);
    }

    for (i = g1.vertexBegin(); i < g1.vertexEnd(); i = g1.vertexNext(i))
    {
        if (_context.incomingMap.size() > 0 && _context.incomingMap.at(i) != -1)
            continue;

        for (j = g2.vertexBegin(); j < g2.vertexEnd(); j = g2.vertexNext(j))
        {
            if (_context.incomingMap.size() > 0 && inv_map[j] != -1)
                continue;
            if (_context.conditionVerticesColor(g1, g2, 0, i, j, _context.userdata))
            {
                ++_x[i];
                ++_y[j];
            }
        }
    }
    for (i = g1.vertexBegin(); i < g1.vertexEnd(); i = g1.vertexNext(i))
    {
        if (_x[i] == 0)
            size1--;
    }
    for (j = g2.vertexBegin(); j < g2.vertexEnd(); j = g2.vertexNext(j))
    {
        if (_y[j] == 0)
            size2--;
    }
    if (size1 > size2)
        return true;

    return false;
}

void MaxCommonSubgraph::AdjMatricesStore::_createMaps()
{
    for (int i = 0; i < _maxsize; i++)
    {
        _map[i] = -1;
        _invmap[i] = -1;
    }
    if (_context.incomingMap.size() > 0)
    {
        if (_swap)
        {
            for (int i = _graph2->vertexBegin(); i < _graph2->vertexEnd(); i = _graph2->vertexNext(i))
            {
                _invmap[i] = _context.incomingMap[i];
            }
            _makeInvertMap(_invmap, _map);
        }
        else
        {
            for (int i = _graph1->vertexBegin(); i < _graph1->vertexEnd(); i = _graph1->vertexNext(i))
            {
                _map[i] = _context.incomingMap[i];
            }
            _makeInvertMap(_map, _invmap);
        }
    }
}

void MaxCommonSubgraph::AdjMatricesStore::_createCorrespondence()
{
    int i, j, tmp;
    _x.zerofill();
    _y.zerofill();
    tmp = 0;
    for (i = _graph1->vertexBegin(); i < _graph1->vertexEnd(); i = _graph1->vertexNext(i))
    {
        if (_map[i] == -1)
        {
            _cr1[tmp] = i;
            ++tmp;
        }
    }
    _size1 = tmp;

    tmp = 0;
    for (i = _graph2->vertexBegin(); i < _graph2->vertexEnd(); i = _graph2->vertexNext(i))
    {
        if (_invmap[i] == -1)
        {
            _cr2[tmp] = i;
            ++tmp;
        }
    }
    _size2 = tmp;

    for (i = 0; i < _size1; i++)
    {
        for (j = 0; j < _size2; j++)
        {
            if (getVerticesColorCondition(i, j))
            {
                _x[i]++;
                _y[j]++;
            }
        }
    }

    for (i = 0; i < _size1 - 1; i++)
    {
        for (j = i + 1; j < _size1; j++)
        {
            if (_x[i] < _x[j])
            {
                std::swap(_cr1[i], _cr1[j]);
                std::swap(_x[i], _x[j]);
            }
        }
    }
    for (i = 0; i < _size2 - 1; i++)
    {
        for (j = i + 1; j < _size2; j++)
        {
            if (_y[i] < _y[j])
            {
                std::swap(_cr2[i], _cr2[j]);
                std::swap(_y[i], _y[j]);
            }
        }
    }

    // delete null vertices
    int size_dec = 0;
    for (i = 0; i < _size1; i++)
    {
        if (_x[i] == 0)
            ++size_dec;
    }
    _size1 = _size1 - size_dec;
    size_dec = 0;
    for (i = 0; i < _size2; i++)
    {
        if (_y[i] == 0)
            ++size_dec;
    }
    _size2 = _size2 - size_dec;

    // shuffle cr[i]

    if (_context.parametersForApproximate.randomize)
    {
        time_t t1;
        time(&t1);
        srand((int)t1);
    }
    else
    {
        srand(0);
    }

    RandomHandler& random_handler = _context._random;
    random_handler.strand = _context.parametersForApproximate.standardRandom;
    int r;
    for (i = 0; i < _size1; i++)
    {
        r = random_handler.next(_size1);
        std::swap(_cr1[i], _cr1[r]);
    }
    for (i = 0; i < _size2; i++)
    {
        r = random_handler.next(_size2);
        std::swap(_cr2[i], _cr2[r]);
    }
}

void MaxCommonSubgraph::AdjMatricesStore::_makeInvertMap(Array<int>& map, Array<int>& invmap)
{
    for (int i = 0; i < map.size(); i++)
    {
        if (map[i] != -1)
        {
            invmap[map[i]] = i;
        }
    }
}

bool MaxCommonSubgraph::AdjMatricesStore::getVerticesColorCondition(int i, int j)
{
    if (_context.conditionVerticesColor == 0)
        return true;
    return _context.conditionVerticesColor(*_graph1, *_graph2, 0, _cr1[i], _cr2[j], _context.userdata);
}

bool MaxCommonSubgraph::AdjMatricesStore::getVColorOneCondition(int i, int j)
{
    if (_context.conditionVerticesColor == 0)
        return true;
    return _context.conditionVerticesColor(*_graph1, *_graph1, 0, _cr1[i], _cr1[j], _context.userdata);
}

int MaxCommonSubgraph::AdjMatricesStore::countErrorAtEdges(int i, int j)
{
    if (!getSecondElement(_x[i], _x[j]))
        return 1;
    else
        return _errorEdgesMatrix[getFirstIdxEdge(i, j)]->at(getSecondIdxEdge(_x[i], _x[j]));
}

bool MaxCommonSubgraph::AdjMatricesStore::getEdgeWeightCondition(int i, int j)
{
    bool r1 = true;
    if (_context.conditionEdgeWeight != 0)
        r1 = _context.conditionEdgeWeight(*_graph1, *_graph2, getFirstIdxEdge(i, j), getSecondIdxEdge(_x[i], _x[j]), _context.userdata);
    bool r2 = getVerticesColorCondition(i, _x[i]) && getVerticesColorCondition(j, _x[j]);
    return r1 && r2;
}

int MaxCommonSubgraph::AdjMatricesStore::_getFirstC(int x)
{
    for (int j = 0; j < _size1; j++)
    {
        if (_cr1[j] == x)
            return j;
    }
    return -1;
}

int MaxCommonSubgraph::AdjMatricesStore::_getSecondC(int x)
{
    for (int j = 0; j < _size2; j++)
    {
        if (_cr2[j] == x)
            return j;
    }
    return -1;
}

void MaxCommonSubgraph::AdjMatricesStore::_createConnectedGraph(Graph& graph, Array<int>& map_gr)
{
    QS_DEF(Array<int>, filter);
    int i, j, size_g1;
    bool c1, c2, c3;

    size_g1 = _graph1->vertexEnd();
    graph.clear();
    filter.resize(size_g1);
    filter.zerofill();
    map_gr.clear();

    for (i = _graph1->vertexBegin(); i < _graph1->vertexEnd(); i = _graph1->vertexNext(i))
    {
        for (j = _graph1->vertexBegin(); j < _graph1->vertexEnd(); j = _graph1->vertexNext(j))
        {
            if (i != j && _map[i] >= 0 && _map[j] >= 0)
            {
                int e_idx1 = _graph1->findEdgeIndex(i, j);
                int e_idx2 = _graph2->findEdgeIndex(_map[i], _map[j]);
                c1 = (e_idx1 >= 0) && (e_idx2 >= 0) && _context.conditionEdgeWeight(*_graph1, *_graph2, e_idx1, e_idx2, _context.userdata);
                c2 = _context.conditionVerticesColor(*_graph1, *_graph2, 0, i, _map[i], _context.userdata);
                c3 = _context.conditionVerticesColor(*_graph1, *_graph2, 0, j, _map[j], _context.userdata);
                if (c1 && c2 && c3)
                {
                    filter[i] = 1;
                }
            }
        }
    }
    for (i = 0; i < size_g1; i++)
    {
        if (filter[i] == 1)
        {
            filter[i] = graph.addVertex();
            map_gr.push(i);
        }
    }

    for (i = _graph1->vertexBegin(); i < _graph1->vertexEnd(); i = _graph1->vertexNext(i))
    {
        for (j = _graph1->vertexBegin(); j < _graph1->vertexEnd(); j = _graph1->vertexNext(j))
        {
            if (i != j && _map[i] >= 0 && _map[j] >= 0)
            {
                int e_idx1 = _graph1->findEdgeIndex(i, j);
                int e_idx2 = _graph2->findEdgeIndex(_map[i], _map[j]);
                c1 = (e_idx1 >= 0) && (e_idx2 >= 0) && _context.conditionEdgeWeight(*_graph1, *_graph2, e_idx1, e_idx2, _context.userdata);
                bool c2 = _context.conditionVerticesColor(*_graph1, *_graph2, 0, i, _map[i], _context.userdata);
                bool c3 = _context.conditionVerticesColor(*_graph1, *_graph2, 0, j, _map[j], _context.userdata);
                if (c1 && c2 && c3 && !graph.haveEdge(filter[i], filter[j]))
                {
                    graph.addEdge(filter[i], filter[j]);
                }
            }
        }
    }
}

void MaxCommonSubgraph::AdjMatricesStore::getSolutions(ObjArray<Array<int>>& v_maps)
{

    int i, j, size_g1;

    QS_DEF(Graph, graph);
    QS_DEF(Array<int>, map_gr);
    QS_DEF(Array<int>, tmp_map);

    size_g1 = _graph1->vertexEnd();

    for (i = 0; i < _size1; i++)
    {
        if (_x[i] >= 0 && _map[_cr1[i]] == -1)
            _map[_cr1[i]] = _cr2[_x[i]];
    }

    _createConnectedGraph(graph, map_gr);

    int ncomp = graph.countComponents();
    const Array<int>& decomposition = graph.getDecomposition();

    // check for maximum
    v_maps.clear();
    if (ncomp == 0)
        return;

    for (i = 0; i < ncomp; i++)
        v_maps.push();

    if (_swap)
    {
        tmp_map.resize(size_g1);
        for (int cur_comp = 0; cur_comp < ncomp; ++cur_comp)
        {
            for (i = 0; i < size_g1; i++)
                tmp_map[i] = SubstructureMcs::UNMAPPED;

            for (i = 0; i < decomposition.size(); i++)
            {
                if (decomposition[i] == cur_comp)
                    tmp_map[map_gr[i]] = _map[map_gr[i]];
            }
            v_maps[cur_comp].resize(_graph2->vertexEnd());
            for (j = 0; j < _graph2->vertexEnd(); j++)
                v_maps[cur_comp].at(j) = SubstructureMcs::UNMAPPED;

            _makeInvertMap(tmp_map, v_maps[cur_comp]);
        }
    }
    else
    {
        for (int cur_comp = 0; cur_comp < ncomp; ++cur_comp)
        {
            v_maps[cur_comp].resize(size_g1);
            for (j = 0; j < size_g1; j++)
                v_maps[cur_comp].at(j) = SubstructureMcs::UNMAPPED;
            for (i = 0; i < decomposition.size(); i++)
            {
                if (decomposition[i] == cur_comp)
                    v_maps[cur_comp].at(map_gr[i]) = _map[map_gr[i]];
            }
        }
    }
}

// Construction stage: greedy method
//-------------------------------------------------------------------------------------------------------------------

MaxCommonSubgraph::Greedy::Greedy(AdjMatricesStore& aj) : _adjMstore(aj)
{
}

void MaxCommonSubgraph::Greedy::greedyMethod()
{

    int ss = 0;
    int ssmax = 0;
    _n = _adjMstore.getFirstSize();
    _m = _adjMstore.getSecondSize();
    _x = _adjMstore.getX();
    _y = _adjMstore.getY();
    _adjStatus.resize(_m);

    _createLgLh();

    if (_unsignVert1.size() == 0)
        return;

    for (int i = 0; i < _n; i++)
    {
        _x[i] = -1;
    }
    for (int i = 0; i < _m; i++)
    {
        _adjStatus[i] = -1;
        _y[i] = -1;
    }

    int i1 = _unsignVert1.size() - 1;
    int p = _unsignVert1[i1];

    int i20 = _unsignVert2[0]->at(p);
    int i2 = _unsignVert2[i20]->size() - 1;

    int q = _unsignVert2[i20]->at(i2);
    while (true)
    {
        _x[p] = q;
        _y[q] = p;

        _unsignVert1.remove(i1);
        _unsignVert2[i20]->remove(i2);

        if (_unsignVert1.size() == 0)
            break;

        i1 = _unsignVert1.size() - 1;
        for (int i = _adjMstore.getSecondRow(q)->nextSetBit(0); i != -1; i = _adjMstore.getSecondRow(q)->nextSetBit(i + 1))
        {
            if (_y[i] == -1)
                _adjStatus[i] = 1;
        }
        ssmax = 0;
        for (int iter = 0; iter < _unsignVert1.size(); iter++)
        {
            ss = 0;

            for (int i = _adjMstore.getFirstRow(_unsignVert1[iter])->nextSetBit(0); i != -1; i = _adjMstore.getFirstRow(_unsignVert1[iter])->nextSetBit(i + 1))
            {
                if (_x[i] >= 0)
                    ++ss;
            }

            if (ss >= ssmax)
            {
                i1 = iter;
                ssmax = ss;
            }
        }
        p = _unsignVert1[i1];
        ssmax = 0;
        i20 = _unsignVert2[0]->at(p);
        i2 = _unsignVert2[i20]->size() - 1;
        if (i2 == -1)
        {
            ssmax = 1 << 16;
            for (int i = 1; i < _unsignVert2.size(); i++)
            {
                if (_unsignVert2[i]->size() > 0)
                {
                    i20 = i;
                    break;
                }
            }
            for (int iter = 0; iter < _unsignVert2[i20]->size(); iter++)
            {
                _x[p] = _unsignVert2[i20]->at(iter);
                ss = _matchedEdges();
                if (ss < ssmax)
                {
                    i2 = iter;
                    ssmax = ss;
                }
                _x[p] = -1;
            }
        }
        else
        {
            for (int iter = 0; iter < _unsignVert2[i20]->size(); iter++)
            {
                _x[p] = _unsignVert2[i20]->at(iter);
                ss = _matchedEdges();
                if (ss > ssmax || (ss >= ssmax && _adjStatus[_unsignVert2[i20]->at(iter)] == 1))
                {
                    i2 = iter;
                    ssmax = ss;
                }
                _x[p] = -1;
            }
        }
        q = _unsignVert2[i20]->at(i2);
    }
}

void MaxCommonSubgraph::Greedy::_createLgLh()
{
    int i = 0;
    int j = 0;
    _unsignVert1.clear();
    _unsignVert2.clear();
    for (i = 0; i < _n; i++)
    {
        _unsignVert1.push(i);
    }
    _unsignVert2.add(new Array<int>());
    bool nfind;
    for (i = 0; i < _n; i++)
    {
        nfind = true;
        for (j = 0; j < _unsignVert2[0]->size(); j++)
        {
            if (_adjMstore.getVColorOneCondition(i, j))
            {
                nfind = false;
                _unsignVert2[0]->push(_unsignVert2[0]->at(j));
                break;
            }
        }
        if (nfind)
        {
            _unsignVert2.add(new Array<int>());
            int last = _unsignVert2.size() - 1;
            _unsignVert2[last]->clear_resize(_adjMstore.getFLSize(i));
            for (j = 0; j < _adjMstore.getFLSize(i); j++)
            {
                _unsignVert2[last]->at(j) = _adjMstore.getFLV(i, j);
            }
            _unsignVert2[0]->push(last);
        }
    }

    _unsignVert1.qsort(_compareFirstDegree, &_adjMstore);
    for (i = 1; i < _unsignVert2.size(); i++)
    {
        _unsignVert2[i]->qsort(_compareSecondDegree, &_adjMstore);
    }
}

int MaxCommonSubgraph::Greedy::_compareFirstDegree(int& i1, int& i2, void* context)
{
    AdjMatricesStore& aj = *(AdjMatricesStore*)context;
    return aj.getFirstVDegree(i2) - aj.getFirstVDegree(i1);
}
int MaxCommonSubgraph::Greedy::_compareSecondDegree(int& i1, int& i2, void* context)
{
    AdjMatricesStore& aj = *(AdjMatricesStore*)context;
    return aj.getSecondVDegree(i2) - aj.getSecondVDegree(i1);
}

int MaxCommonSubgraph::Greedy::_matchedEdges()
{
    int matchedges = 0;
    for (int i = 0; i < _n; i++)
    {
        if (_x[i] >= 0)
        {
            for (int j = _adjMstore.getFirstRow(i)->nextSetBit(0); j != -1; j = _adjMstore.getFirstRow(i)->nextSetBit(j + 1))
            {
                if (_x[j] >= 0 && _adjMstore.getSecondElement(_x[i], _x[j]) && _adjMstore.getEdgeWeightCondition(i, j))
                {
                    ++matchedges;
                }
            }
        }
    }
    return matchedges >> 1;
}

// Refinement stage: random discrete descent method
//-------------------------------------------------------------------------------------------------------------------
CP_DEF(MaxCommonSubgraph::RandomDisDec);

MaxCommonSubgraph::RandomDisDec::RandomDisDec(AdjMatricesStore& aj)
    : _adjMstore(aj), CP_INIT, TL_CP_GET(_errorList), TL_CP_GET(_listErrVertices), _maxIteration(MAX_ITERATION)
{
    setIterationNumber(aj._context.parametersForApproximate.maxIteration);
    cancellation_handler = getCancellationHandler();
}

void MaxCommonSubgraph::RandomDisDec::refinementStage()
{
    _n = _adjMstore.getFirstSize();
    _m = _adjMstore.getSecondSize();

    _x = _adjMstore.getX();
    _y = _adjMstore.getY();

    _stop = false;

    _errorList.resize(_n);
    _listErrVertices.resize(_n + 1);

    int t = 0;
    _stuckCount = 0;

    _errorNumber = _goalFunction();
    _errorNumberStuck = _errorNumber;
    _saveState();

    _makeLe();
    int r, p, q, a, b;

    if (_adjMstore._context.parametersForApproximate.randomize)
    {
        time_t clock;
        time(&clock);
        srand((int)clock);
    }
    else
    {
        srand(0);
    }
    RandomHandler& random_handler = _adjMstore._context._random;
    random_handler.strand = _adjMstore._context.parametersForApproximate.standardRandom;

    while (true)
    {
        ++t;
        // if iteration reach max limit - when end cycle
        if (t > (_maxIteration * _n) || _errorNumber == 0)
        {
            _stop = true;
        }
        if (t % 100 == 0)
        {
            if (cancellation_handler != 0)
            {
                if (cancellation_handler->isCancelled())
                    throw Error("mcs search was cancelled: %s", cancellation_handler->cancelledRequestMessage());
            }
        }

        if (_stop)
            break;
        // if algorithm get stuck in local minimum - when give him potencial to override it
        ++_stuckCount;
        if (_stuckCount > (log((double)_n) * _n))
        {
            if (_errorNumber <= _stateArray[0])
                _saveState();
            _stuckCount = 0;
            for (int i = 0; i < _errorList.size(); i++)
                ++_errorList[i];
        }
        // select random vertex in list of error vertices to swap or move
        while (true)
        {
            r = random_handler.next(_listErrVertices[0]) + 1;
            p = _listErrVertices[r];
            if (!((_adjMstore.getFLSize(p) == 1 && _x[p] == _adjMstore.getFLV(p, 0)) || (_adjMstore.getFLSize(p) == 0)))
                break;
        }
        // select random vertex in other graph
        a = _x[p];
        r = a;
        while (r == a)
            r = _adjMstore.getFLV(p, random_handler.next(_adjMstore.getFLSize(p)));
        q = r;

        if (_y[q] >= 0)
        {
            // swap assignments p and y(q)
            b = _y[q];
            _x[b] = a;
            _x[p] = q;
            if (_acceptanceSwap(b, p))
            {
                _errorNumber = _newErrorNumber;
                _y[a] = b;
                _y[q] = p;
                _makeLe();
            }
            else
            {
                // change back swap
                _x[b] = q;
                _x[p] = a;
            }
        }
        else
        {
            // move
            _x[p] = q;
            if (_acceptanceMove(p))
            {
                _errorNumber = _newErrorNumber;
                _y[a] = -1;
                _y[q] = p;
                _makeLe();
            }
            else
            {
                // change back move
                _x[p] = a;
            }
        }
    }
    // if error in the end of work more when saved error then load state
    if (_errorNumber > _stateArray[0])
    {
        _loadState();
    }
}

void MaxCommonSubgraph::RandomDisDec::setIterationNumber(int max)
{
    _maxIteration = max;
}

int MaxCommonSubgraph::RandomDisDec::_goalFunction()
{

    // function that collect all errors
    int err = 0;
    for (int i = 0; i < _n; i++)
    {
        _errorList[i] = 0;
        for (int j = _adjMstore.getFirstRow(i)->nextSetBit(0); j != -1; j = _adjMstore.getFirstRow(i)->nextSetBit(j + 1))
        {
            _errorList[i] += _adjMstore.countErrorAtEdges(i, j);
        }
        err += _errorList[i];
    }
    return err >> 1;
}
void MaxCommonSubgraph::RandomDisDec::_makeLe()
{
    // creation of error list of the vertices
    _listErrVertices[0] = 0;
    int sum = 0;
    for (int i = 0; i < _n; i++)
    {
        if (_errorList[i] > 0)
        {
            ++_listErrVertices[0];
            _listErrVertices[_listErrVertices[0]] = i;
            if (_adjMstore.getFLSize(i) == 1 && _x[i] == _adjMstore.getFLV(i, 0))
                ++sum;
        }
    }
    if (_listErrVertices[0] == sum)
        _stop = true;
    if (_errorNumber < _errorNumberStuck)
    {
        _stuckCount = 0;
        _errorNumberStuck = _errorNumber;
    }
}

bool MaxCommonSubgraph::RandomDisDec::_acceptanceMove(int x)
{

    int ex = 0;
    bool result;
    for (int j = _adjMstore.getFirstRow(x)->nextSetBit(0); j != -1; j = _adjMstore.getFirstRow(x)->nextSetBit(j + 1))
    {
        ex += _adjMstore.countErrorAtEdges(x, j);
    }
    if (ex <= _errorList[x])
    {
        _newErrorNumber = _goalFunction();
        result = true;
    }
    else
        result = false;

    return result;
}

bool MaxCommonSubgraph::RandomDisDec::_acceptanceSwap(int x, int y)
{
    int ex = 0;
    int ey = 0;
    bool result;

    for (int j = _adjMstore.getFirstRow(x)->nextSetBit(0); j != -1; j = _adjMstore.getFirstRow(x)->nextSetBit(j + 1))
    {
        ex += _adjMstore.countErrorAtEdges(x, j);
    }
    for (int j = _adjMstore.getFirstRow(y)->nextSetBit(0); j != -1; j = _adjMstore.getFirstRow(y)->nextSetBit(j + 1))
    {
        ey += _adjMstore.countErrorAtEdges(y, j);
    }
    if ((ex + ey) <= (_errorList[x] + _errorList[y]))
    {
        _newErrorNumber = _goalFunction();
        result = true;
    }
    else
        result = false;

    return result;
}

void MaxCommonSubgraph::RandomDisDec::_saveState()
{
    if ((_n + _m + 1) > _stateArray.size())
        _stateArray.resize(_n + _m + 1);

    _stateArray[0] = _errorNumber;
    for (int i = 0; i < _n; i++)
        _stateArray[i + 1] = _x[i];

    for (int i = 0; i < _m; i++)
        _stateArray[i + 1 + _n] = _y[i];

    if (_adjMstore._context.cbEmbedding != 0)
    {
        if (!_adjMstore._context.cbEmbedding(0, 0, 0, _adjMstore._context.embeddingUserdata))
            _stop = true;
    }
}

void MaxCommonSubgraph::RandomDisDec::_loadState()
{
    _errorNumber = _stateArray[0];
    for (int i = 0; i < _n; i++)
        _x[i] = _stateArray[i + 1];

    for (int i = 0; i < _m; i++)
        _y[i] = _stateArray[i + 1 + _n];
}

// maximize number of the rings/ v-e+r=2 there v- number of vertices e - number of edges r - number of rings
int MaxCommonSubgraph::ringsSolutionTerm(Array<int>& a1, Array<int>& a2, void* dummy)
{
    int a1_vlen = 0, a2_vlen = 0, a1_elen = 0, a2_elen = 0;
    for (int i = 0; i < a1[0]; ++i)
        if (a1[i + 2] >= 0)
            ++a1_vlen;
    for (int i = 0; i < a1[1]; ++i)
        if (a1[i + 2 + a1[0]] >= 0)
            ++a1_elen;
    for (int i = 0; i < a2[0]; ++i)
        if (a2[i + 2] >= 0)
            ++a2_vlen;
    for (int i = 0; i < a2[1]; ++i)
        if (a2[i + 2 + a2[0]] >= 0)
            ++a2_elen;
    // maximize number of the rings/ v-e+r=2 there v- number of vertices e - number of edges r - number of rings
    int result = (a2_elen - a2_vlen) - (a1_elen - a1_vlen);
    if (result == 0)
        result = a2_elen - a1_elen;
    return result;
}

void MaxCommonSubgraph::_getSolutionMaps(int count, ObjArray<Array<int>>& v_maps, ObjArray<Array<int>>& e_maps) const
{
    v_maps.clear();
    e_maps.clear();

    for (int i = 0; (i < count) && (i < _vertEdgeSolMap.size()); ++i)
    {
        int v_size = _vertEdgeSolMap[i].at(0);
        int e_size = _vertEdgeSolMap[i].at(1);
        Array<int>& v_arr = v_maps.push();
        Array<int>& e_arr = e_maps.push();
        v_arr.resize(v_size);
        e_arr.resize(e_size);

        for (int j = 0; j < v_size; ++j)
            v_arr[j] = _vertEdgeSolMap[i].at(2 + j);
        for (int j = 0; j < e_size; ++j)
            e_arr[j] = _vertEdgeSolMap[i].at(2 + v_size + j);
    }
}

//----------------------------------------------------------------------------------------------------------------
// substructure search
SubstructureMcs::SubstructureMcs() : cbMatchEdge(0), cbMatchVertex(0), userdata(0), _sub(0), _super(0), _invert(false)
{
}

SubstructureMcs::SubstructureMcs(Graph& sub, Graph& super) : cbMatchEdge(0), cbMatchVertex(0), userdata(0), _sub(0), _super(0), _invert(false)
{
    setGraphs(sub, super);
}

void SubstructureMcs::setGraphs(Graph& sub, Graph& super)
{
    if (sub.vertexCount() < super.vertexCount())
        _invert = false;
    else if (sub.vertexCount() > super.vertexCount())
        _invert = true;
    else if ((sub.vertexCount() == super.vertexCount()) && (sub.edgeCount() < super.edgeCount()))
        _invert = false;
    else
        _invert = true;

    if (!_invert)
    {
        _sub = &sub;
        _super = &super;
    }
    else
    {
        _sub = &super;
        _super = &sub;
    }
}
// searches substructure for graphs and maps vertices
bool SubstructureMcs::searchSubstructure(Array<int>* map)
{

    if (_sub == 0 || _super == 0)
        throw MaxCommonSubgraph::Error("internal AAM error: not initialized sub-mcs graphs");

    EmbeddingEnumerator emb_enum(*_super);
    emb_enum.setSubgraph(*_sub);
    emb_enum.cb_match_edge = cbMatchEdge;
    emb_enum.cb_match_vertex = cbMatchVertex;

    emb_enum.cb_embedding = _embedding;
    emb_enum.userdata = userdata;
    int result = emb_enum.process();

    if (result == 1)
        return false;

    if (map != 0)
    {
        if (!_invert)
        {
            map->clear_resize(_sub->vertexEnd());
            for (int i = 0; i < map->size(); i++)
                map->at(i) = UNMAPPED;
            for (int i = _sub->vertexBegin(); i < _sub->vertexEnd(); i = _sub->vertexNext(i))
                map->at(i) = emb_enum.getSubgraphMapping()[i];
        }
        else
        {
            map->clear_resize(_super->vertexEnd());
            for (int i = 0; i < map->size(); i++)
                map->at(i) = UNMAPPED;
            for (int i = _super->vertexBegin(); i < _super->vertexEnd(); i = _super->vertexNext(i))
                map->at(i) = emb_enum.getSupergraphMapping()[i];
        }
    }
    return true;
}

int SubstructureMcs::_embedding(Graph&, Graph&, int*, int*, void*)
{
    return 0;
}
