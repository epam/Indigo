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

#ifndef __scaffold_detection_h_
#define __scaffold_detection_h_

#include "graph/max_common_subgraph.h"

namespace indigo
{

    class ScaffoldDetection
    {
    public:
        ScaffoldDetection(ObjArray<Graph>* graph_set);
        // two main methods for extracting scaffolds
        // extracting exact scaffold from graphs set
        void extractExactScaffold(Graph& scaffold)
        {
            _searchScaffold(scaffold, false);
        }
        // extracting approximate scaffold from graphs set
        void extractApproximateScaffold(Graph& scaffold)
        {
            _searchScaffold(scaffold, true);
        }

        // array for keeping graphs for searching scaffold
        // PtrArray<Graph> graphSet;

        // callbacks for edge and vertices comparsions for input graphs
        bool (*cbEdgeWeight)(Graph& graph1, Graph& graph2, int i, int j, void* userdata);
        bool (*cbVerticesColor)(Graph& graph1, Graph& graph2, const int* core_sub, int i, int j, void* userdata);
        int (*cbSortSolutions)(Graph& graph1, Graph& graph2, const void* userdata);
        void* userdata;

        int (*cbEmbedding)(const int* sub_vert_map, const int* sub_edge_map, const void* info, void* userdata);
        void* embeddingUserdata;

        ObjArray<Graph>* searchStructures;
        ObjArray<Graph>* basketStructures;

        int maxIterations;

        DECL_ERROR;

    public:
        // class for keeping graphs
        class GraphBasket
        {
        public:
            // max number of graphs to keep
            enum
            {
                MAX_MOLECULES_NUMBER = 100,
                NEXT_SOLUTION_SIZE_SUM = 100
            };
            GraphBasket();
            virtual ~GraphBasket();

            // initializes graphs basket
            void initBasket(ObjArray<Graph>* graph_set, ObjArray<Graph>* basket_set, int max_number);
            // returns index of first graph in basket
            int graphBegin();
            // returns next index of graph after input index
            int graphNext(int i);
            // checks if queue's graphs already exsist in basket. add this graphs in case false result
            void checkAddedGraphs();
            // remove graph form basket
            virtual void removeGraph(int index);
            // returns ptr of graph in basket with index
            virtual Graph& getGraph(int index) const;
            // adds new graph to queue and returns ptr of that
            virtual Graph& pickOutNextGraph();
            // returns index of graph in basket that has maximum number of edges
            virtual int getMaxGraphIndex();
            // this method adds graph from set (defines with edges and vertices lists) to basket queue
            virtual void addToNextEmptySpot(Graph& graph, Array<int>& v_list, Array<int>& e_list);

            virtual Graph& getGraphFromSet(int idx)
            {
                return _searchStructures->at(_orderArray[idx]);
            }
            virtual int getGraphSetSize() const
            {
                return _graphSetSize;
            }

            int (*cbSortSolutions)(Graph& graph1, Graph& graph2, void* userdata);
            bool (*cbMatchEdges)(Graph& graph1, Graph& graph2, int i, int j, void* userdata);
            bool (*cbMatchVertices)(Graph& graph1, Graph& graph2, const int* core_sub, int i, int j, void* userdata);
            void* userdata;

            DECL_ERROR;

        protected:
            // iterator for looking next empty graphs in set
            Dbitset _directIterator;
            // reverse iterator for looking next not empty graphs in set
            Dbitset _reverseIterator;

            ObjArray<Graph>* _searchStructures;
            ObjArray<Graph>* _basketStructures;

            virtual void _sortGraphsInSet();

            // array for keeping indexes of sorted graphs in set
            Array<int> _orderArray;

            int _graphSetSize;

            // function for sortMoleculesInSet() method
            static int _compareEdgeCount(int i1, int i2, void* context);

            static int _copmpareRingsCount(Graph& g1, Graph& g2, void* context);

        private:
            GraphBasket(const GraphBasket&); // no implicit copy
        };

    protected:
        // method for extracting exact scaffold from graph set
        void _searchExactScaffold(GraphBasket& basket);
        // method for extracting approximate scaffold from graph set
        void _searchApproximateScaffold(GraphBasket& basket);

    private:
        void _searchScaffold(Graph& scaffold, bool approximate);
    };

} // namespace indigo

#endif
