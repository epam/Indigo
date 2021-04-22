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

#include "graph/scaffold_detection.h"
#include "base_cpp/array.h"
#include "base_cpp/ptr_array.h"
#include "graph/max_common_subgraph.h"

using namespace indigo;

IMPL_ERROR(ScaffoldDetection, "Scaffold detection");

ScaffoldDetection::ScaffoldDetection(ObjArray<Graph>* graph_set)
    : cbEdgeWeight(0), cbVerticesColor(0), cbSortSolutions(0), userdata(0), cbEmbedding(0), embeddingUserdata(0), searchStructures(graph_set),
      basketStructures(0), maxIterations(0)
{
}

void ScaffoldDetection::_searchScaffold(Graph& scaffold, bool approximate)
{
    GraphBasket graph_basket;
    QS_DEF(ObjArray<Graph>, temp_set);
    if (basketStructures == 0)
    {
        basketStructures = &temp_set;
    }

    graph_basket.initBasket(searchStructures, basketStructures, GraphBasket::MAX_MOLECULES_NUMBER);

    if (approximate)
        _searchApproximateScaffold(graph_basket);
    else
        _searchExactScaffold(graph_basket);

    int max_index = graph_basket.getMaxGraphIndex();
    if (basketStructures->size() == 0)
        throw Error("no scaffolds found");
    scaffold.cloneGraph(graph_basket.getGraph(max_index), 0);
}

void ScaffoldDetection::_searchExactScaffold(GraphBasket& basket)
{
    ObjArray<Array<int>> v_lists;
    ObjArray<Array<int>> e_lists;

    int graphset_size = basket.getGraphSetSize();
    int first_graph_num = 0;

    SubstructureMcs sub_mcs;
    sub_mcs.cbMatchEdge = cbEdgeWeight;
    sub_mcs.cbMatchVertex = cbVerticesColor;
    sub_mcs.userdata = userdata;

    MaxCommonSubgraph mcs(basket.getGraph(0), basket.getGraph(0));
    mcs.conditionEdgeWeight = cbEdgeWeight;
    mcs.conditionVerticesColor = cbVerticesColor;
    mcs.userdata = userdata;
    if (maxIterations > 0)
        mcs.parametersForExact.maxIteration = maxIterations;

    basket.cbMatchEdges = cbEdgeWeight;
    basket.cbMatchVertices = cbVerticesColor;
    basket.userdata = userdata;

    for (int orgraph = first_graph_num + 1; orgraph < graphset_size; orgraph++)
    {

        Graph& graph_set = basket.getGraphFromSet(orgraph);

        for (int bgraph = basket.graphBegin(); bgraph >= 0; bgraph = basket.graphNext(bgraph))
        {
            Graph& graph_bask = basket.getGraph(bgraph);

            sub_mcs.setGraphs(graph_bask, graph_set);
            if (!sub_mcs.isInverted() && sub_mcs.searchSubstructure(0))
                continue;

            mcs.setGraphs(graph_bask, graph_set);

            MaxCommonSubgraph::ReGraph regraph(mcs);
            MaxCommonSubgraph::ReCreation build_graph(regraph, mcs);
            build_graph.createRegraph();
            regraph.parse(true);

            /*
             * Throw an exception if max limit was reached
             */
            if (regraph.stopped())
                throw Error("scaffold detection exact searching max iteration limit reached");

            build_graph.getSolutionListsSuper(v_lists, e_lists);

            for (int i = 0; i < e_lists.size(); i++)
            {
                basket.addToNextEmptySpot(graph_set, v_lists[i], e_lists[i]);
            }

            basket.removeGraph(bgraph);
        }
        basket.checkAddedGraphs();

        if (cbEmbedding != 0)
        {
            if (!cbEmbedding(&orgraph, &graphset_size, 0, embeddingUserdata))
                break;
        }
    }
}

void ScaffoldDetection::_searchApproximateScaffold(GraphBasket& basket)
{
    ObjArray<Array<int>> v_maps;
    ObjArray<Array<int>> e_maps;
    Array<int> v_list;
    Array<int> e_list;

    int graphset_size = basket.getGraphSetSize();

    int max_size = 0;
    for (int i = 0; i < graphset_size; i++)
    {
        if (basket.getGraphFromSet(i).edgeCount() > max_size)
            max_size = basket.getGraphFromSet(i).edgeCount();
        if (basket.getGraphFromSet(i).vertexCount() > max_size)
            max_size = basket.getGraphFromSet(i).vertexCount();
    }

    SubstructureMcs sub_mcs;
    sub_mcs.cbMatchEdge = cbEdgeWeight;
    sub_mcs.cbMatchVertex = cbVerticesColor;
    sub_mcs.userdata = userdata;

    MaxCommonSubgraph mcs(basket.getGraph(0), basket.getGraph(0));
    mcs.conditionEdgeWeight = cbEdgeWeight;
    mcs.conditionVerticesColor = cbVerticesColor;
    mcs.userdata = userdata;
    if (maxIterations > 0)
        mcs.parametersForApproximate.maxIteration = maxIterations;

    basket.cbMatchEdges = cbEdgeWeight;
    basket.cbMatchVertices = cbVerticesColor;
    basket.userdata = userdata;

    MaxCommonSubgraph::AdjMatricesStore adjm(mcs, 2 * max_size);
    MaxCommonSubgraph::Greedy greedy(adjm);
    MaxCommonSubgraph::RandomDisDec randdisdec(adjm);

    for (int orgraph = 1; orgraph < graphset_size; orgraph++)
    {

        Graph& graph_set = basket.getGraphFromSet(orgraph);

        for (int bgraph = basket.graphBegin(); bgraph >= 0; bgraph = basket.graphNext(bgraph))
        {
            Graph& graph_bask = basket.getGraph(bgraph);
            // search sub
            sub_mcs.setGraphs(graph_bask, graph_set);
            if (!sub_mcs.isInverted() && sub_mcs.searchSubstructure(0))
                continue;
            // search mcs

            mcs.setGraphs(graph_bask, graph_set);
            adjm.create(graph_bask, graph_set);
            greedy.greedyMethod();
            randdisdec.refinementStage();

            adjm.createSolutionMaps();
            mcs.getSolutionMaps(&v_maps, &e_maps);

            for (int i = 0; i < e_maps.size(); ++i)
            {
                v_list.clear();
                for (int j = 0; j < v_maps[i].size(); ++j)
                    if (v_maps[i].at(j) != SubstructureMcs::UNMAPPED)
                        v_list.push(v_maps[i].at(j));

                e_list.clear();
                for (int j = 0; j < e_maps[i].size(); ++j)
                    if (e_maps[i].at(j) != SubstructureMcs::UNMAPPED)
                        e_list.push(e_maps[i].at(j));

                if (v_list.size() > 1)
                    basket.addToNextEmptySpot(graph_set, v_list, e_list);
            }
            basket.removeGraph(bgraph);
        }
        basket.checkAddedGraphs();

        if (cbEmbedding != 0)
        {
            if (!cbEmbedding(&orgraph, &graphset_size, 0, embeddingUserdata))
                break;
        }
    }
}

void ScaffoldDetection::GraphBasket::_sortGraphsInSet()
{
    int set_size = _searchStructures->size();

    if (set_size == 0)
        throw Error("graph set size == 0");

    _orderArray.clear();
    for (int i = 0; i < set_size; i++)
    {
        if (_searchStructures->at(i).vertexCount() > 0)
        {
            _orderArray.push(i);
            ++_graphSetSize;
        }
    }
    // sort in order of edgeCount
    _orderArray.qsort(_compareEdgeCount, (void*)_searchStructures);
}

int ScaffoldDetection::GraphBasket::_compareEdgeCount(int i1, int i2, void* context)
{
    ObjArray<Graph>& graph_set = *(ObjArray<Graph>*)context;
    return graph_set.at(i1).edgeCount() - graph_set.at(i2).edgeCount();
}

int ScaffoldDetection::GraphBasket::_copmpareRingsCount(Graph& g1, Graph& g2, void*)
{
    // maximize number of the rings/ v-e+r=2 there v- number of vertices e - number of edges r - number of rings
    int result = (g2.edgeCount() - g2.vertexCount()) - (g1.edgeCount() - g1.vertexCount());
    if (result == 0 || g1.edgeCount() == 0 || g2.edgeCount() == 0)
        result = g2.edgeCount() - g1.edgeCount();
    return result;
}

IMPL_ERROR(ScaffoldDetection::GraphBasket, "Graph basket");

ScaffoldDetection::GraphBasket::GraphBasket()
    : cbSortSolutions(0), cbMatchEdges(0), cbMatchVertices(0), userdata(0), _searchStructures(0), _basketStructures(0), _graphSetSize(0)
{
}

ScaffoldDetection::GraphBasket::~GraphBasket()
{
}

void ScaffoldDetection::GraphBasket::initBasket(ObjArray<Graph>* graph_set, ObjArray<Graph>* basket_set, int max_number)
{

    if (graph_set == 0)
        throw Error("graph set is null");
    if (basket_set == 0)
        throw Error("basket set is null");

    _searchStructures = graph_set;
    _basketStructures = basket_set;

    _sortGraphsInSet();

    _basketStructures->clear();

    for (int i = 0; i < max_number; i++)
        _basketStructures->push();

    _directIterator.resize(max_number);
    _reverseIterator.resize(max_number);
    _reverseIterator.set();

    _basketStructures->at(0).cloneGraph(_searchStructures->at(_orderArray[0]), 0);
    _reverseIterator.set(0, false);
    _directIterator.set(0);
}

int ScaffoldDetection::GraphBasket::graphBegin()
{
    return _directIterator.nextSetBit(0);
}

int ScaffoldDetection::GraphBasket::graphNext(int i)
{
    return _directIterator.nextSetBit(i + 1);
}

void ScaffoldDetection::GraphBasket::checkAddedGraphs()
{
    bool add_to_basket;

    Dbitset added_iter(_reverseIterator.size());
    added_iter.copy(_reverseIterator);
    added_iter.orWith(_directIterator);
    added_iter.flip();

    SubstructureMcs sub_mcs;
    sub_mcs.cbMatchEdge = cbMatchEdges;
    sub_mcs.cbMatchVertex = cbMatchVertices;
    sub_mcs.userdata = userdata;

    for (int x = added_iter.nextSetBit(0); x >= 0; x = added_iter.nextSetBit(x + 1))
    {
        add_to_basket = true;
        for (int y = graphBegin(); y >= 0; y = graphNext(y))
        {
            sub_mcs.setGraphs(getGraph(x), getGraph(y));
            if (sub_mcs.searchSubstructure(0))
            {
                add_to_basket = false;
                if (sub_mcs.isInverted())
                {
                    removeGraph(y);
                    add_to_basket = true;
                }
                break;
            }
        }
        if (add_to_basket)
            _directIterator.set(x);
        else
            _reverseIterator.set(x);
    }
}

void ScaffoldDetection::GraphBasket::removeGraph(int index)
{
    _directIterator.set(index, false);
    _reverseIterator.set(index);
}

Graph& ScaffoldDetection::GraphBasket::getGraph(int index) const
{
    if (index >= _basketStructures->size())
        throw Error("basket size < index");
    return _basketStructures->at(index);
}

Graph& ScaffoldDetection::GraphBasket::pickOutNextGraph()
{

    int empty_index = _reverseIterator.nextSetBit(0);

    if (empty_index == -1)
    {
        _directIterator.resize(_directIterator.size() + NEXT_SOLUTION_SIZE_SUM);
        _reverseIterator.resize(_directIterator.size());
        empty_index = _basketStructures->size();
        for (int i = _directIterator.size() - NEXT_SOLUTION_SIZE_SUM; i < _directIterator.size(); i++)
            _reverseIterator.set(i);
        for (int i = 0; i < NEXT_SOLUTION_SIZE_SUM; i++)
            _basketStructures->push();
    }

    _reverseIterator.set(empty_index, false);
    return _basketStructures->at(empty_index);
}

int ScaffoldDetection::GraphBasket::getMaxGraphIndex()
{

    for (int x = _reverseIterator.nextSetBit(0); x >= 0; x = _reverseIterator.nextSetBit(x + 1))
    {
        Graph& graph_basket = _basketStructures->at(x);

        if (graph_basket.vertexCount() > 0)
            graph_basket.clear();
    }

    if (cbSortSolutions == 0)
        _basketStructures->qsort(&_copmpareRingsCount, 0);
    else
        _basketStructures->qsort(cbSortSolutions, userdata);

    while (_basketStructures->size() && _basketStructures->top().vertexCount() == 0)
        _basketStructures->pop();

    return 0;
}
void ScaffoldDetection::GraphBasket::addToNextEmptySpot(Graph& graph, Array<int>& v_list, Array<int>& e_list)
{
    pickOutNextGraph().makeEdgeSubgraph(graph, v_list, e_list, 0, 0);
}
