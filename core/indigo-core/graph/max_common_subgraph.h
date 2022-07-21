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

#ifndef _max_common_subgraph
#define _max_common_subgraph

#include "base_cpp/cancellation_handler.h"
#include "base_cpp/d_bitset.h"
#include "base_cpp/obj_list.h"
#include "base_cpp/output.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"
#include "graph/embedding_enumerator.h"
#include "graph/graph.h"
#include "math.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT MaxCommonSubgraph
    {
    public:
        DECL_ERROR;

        MaxCommonSubgraph(Graph& subgraph, Graph& supergraph);
        ~MaxCommonSubgraph();

        void setGraphs(Graph& subgraph, Graph& supergraph);
        // two main methods for maximum common subgraph search
        // exact searching mcs method. Hanser's algorithm used
        void findExactMCS();
        // approximate method for searching mcs. 2DOM algorithm used
        void findApproximateMCS();

        // parameters for exact method
        struct ParametersForExact
        {
            // boolean true if method reached max iteration number
            bool isStopped;
            // max iteration number
            int maxIteration;
            // number of solutions that are found by exact algorithm
            int numberOfSolutions;
            // throw error if input map is incorrect
            bool throw_error_for_incorrect_map;
        };

        // parameters for approximate algorithm
        struct ParametersForApproximate
        {
            // error is the number of unmatched edges in solution
            int error;
            // max iteration number.
            int maxIteration;
            // number of solutions (connected graphs in solution given by algorithm 2DOM)
            int numberOfSolutions;
            // if this parameter set to false then only max solution would be writen. true - then all solution and at first place max solution
            bool randomize;

            bool standardRandom;
        };

        // two callbacks for edge and vertices matching
        bool (*conditionEdgeWeight)(Graph& graph1, Graph& graph2, int i, int j, void* userdata);
        bool (*conditionVerticesColor)(Graph& graph1, Graph& graph2, const int* core_sub, int i, int j, void* userdata);

        // parameters for mcs methods
        ParametersForExact parametersForExact;
        ParametersForApproximate parametersForApproximate;
        // array for accept input mapping and working with it
        Array<int> incomingMap;
        // this method sorts solutions and maximizes number of the rings in graph
        static int ringsSolutionTerm(Array<int>&, Array<int>&, void*);
        // returns all maps-solutions-mcs
        void getSolutionMaps(ObjArray<Array<int>>* v_maps, ObjArray<Array<int>>* e_maps) const;
        // returns first element in sorted solution array
        void getMaxSolutionMap(Array<int>* v_map, Array<int>* e_map) const;
        // callback for sorting solutions (see _vertEdgeSolMap)
        int (*cbSolutionTerm)(Array<int>& array1, Array<int>& array2, void* userdata);
        // context for all callbacks (edge and vertices matching and sort solutions
        void* userdata;

        // callback for managing/ if return 0 then algorithm will end its work
        int (*cbEmbedding)(const int* sub_vert_map, const int* sub_edge_map, const void* info, void* userdata);
        void* embeddingUserdata;

        // Exact method: Hanser's algorithm
        //-------------------------------------------------------------------------------------------------------------------
        // class represent node of the resolution graph (ReGraph) An RePoint represents an association
        // betwwen two edges of the source graphs G1 and G2 that are compared
        class RePoint
        {
        public:
            // creates RePoint for input edge ids
            RePoint(int n1, int n2);

            // gets edge id in first graph
            int getid1() const
            {
                return _id1;
            };
            // gets edge id in second graph
            int getid2() const
            {
                return _id2;
            };
            // set of neighbour nodes in the ReGraph
            Dbitset extension;
            // set of incompatible nodes in the ReGraph
            Dbitset forbidden;

            Dbitset allowed_g1;
            Dbitset allowed_g2;
            // sets sizes for all containers
            void setSizes(int size, int size_g1, int size_g2);

        private:
            // number of the edge in the graph 1
            int _id1;
            // number of the edge in the graph 2
            int _id2;

            RePoint(const RePoint&); // no implicit copy
        };

        // solution structure for Resolution graph
        class Solution
        {
        public:
            Solution(){};

            int numBits;
            Dbitset reSolution;
            Dbitset solutionProj1;
            Dbitset solutionProj2;

        private:
            Solution(const Solution&); // no implicit copy
        };

        // This class implements the Resolution Graph (ReGraph).
        // The ReGraph is a graph based representation of the search problem.
        // An ReGraph is constructred from the two compared graphs (G1 and G2).
        // Each vertex (node) in the ReGraph represents a possible association
        // from an edge in G1 with an edge in G2. Thus two compatible edges
        // in two graphs are represented by a vertex in the ReGraph.
        // Each edge in the ReGraph corresponds to a common adjacency relationship
        // between the 2 couple of compatible edges associated to the 2 ReGraph nodes
        // forming this edge.

        // Resolution Graph
        class ReGraph
        {
        public:
            ReGraph();
            ReGraph(MaxCommonSubgraph& context);

            // clears resolution graph
            void clear();
            // sets maximum iterations number
            void setMaxIteration(int m)
            {
                _maxIteration = m;
            };
            // set sizes for util variables
            void setSizes(int n1, int n2);
            // adds new RePoint to nodes set
            void addPoint(int id1, int id2)
            {
                _graph.add(new RePoint(id1, id2));
            };

            // main method to perform a query
            //  Parsing of the ReGraph. This is the recursive method
            //  to perform a query. The method will recursively
            //  parse the RGraph thru connected nodes and visiting the
            //  RGraph using allowed adjacency relationship.

            void parse(bool findAllStructure);
            // retruns index of RePoint which corespondes to input edges ids
            int getPointIndex(int i, int j) const;
            // returns number of nodes (RePoints) in resolution graph
            int size() const
            {
                return _graph.size();
            };
            // returns true if algorithm has reached maximum iteration
            bool stopped()
            {
                return _stop;
            };
            // gets RePoint with index i
            RePoint* getPoint(int i)
            {
                return _graph[i];
            };

            // solution getters
            // begin solution index
            int solBegin() const
            {
                return _solutionObjList.begin();
            };
            // next solution index
            int solNext(int index) const
            {
                return _solutionObjList.next(index);
            };
            // return false then it is no more solutions
            bool solIsNotEnd(int index) const
            {
                return (index != _solutionObjList.end());
            };
            // solutions store capacity
            int solutionSize() const
            {
                return _solutionObjList.size();
            };
            // returns solution list
            const Dbitset& getSolBitset(int index) const
            {
                return _solutionObjList[index].reSolution;
            };
            // retruns project of solution list to graph 1
            const Dbitset& getProj1Bitset(int index) const
            {
                return _solutionObjList[index].solutionProj1;
            };
            // retruns project of solution list to graph 2
            const Dbitset& getProj2Bitset(int index) const
            {
                return _solutionObjList[index].solutionProj2;
            };

            void insertSolution(int ins_index, bool ins_after, const Dbitset& sol, const Dbitset& sol_g1, const Dbitset& sol_g2, int num_bits);

            // callback for managing/ if return 0 then algorithm will end its work
            int (*cbEmbedding)(const int* sub_vert_map, const int* sub_edge_map, const void* info, void* userdata);
            void* userdata;

            std::shared_ptr<CancellationHandler> cancellation_handler;

        protected:
            // list of ReGraph nodes each node keeping track of its  neighbours
            PtrArray<RePoint> _graph;
            // size of ReGRaph
            int _size;
            // current number of iterations
            int _nbIteration;
            // maximal number of iterations before search break
            int _maxIteration;
            // dimensions of the compared graphs
            int _firstGraphSize;
            int _secondGraphSize;
            // flag to define if we want to get all possible 'structures'
            bool _findAllStructure;
            // flag to define if search was breaking
            bool _stop;

            // Checks if a potantial solution is a real one
            // (not included in a previous solution)
            //  and add this solution to the solution list
            // in case of success.
            void _solution(const Dbitset& traversed, Dbitset& trav_g1, Dbitset& trav_g2);
            // Determine if there are potential soltution remaining.
            bool _mustContinue(const Dbitset& pnode_g1, const Dbitset& pnode_g2) const;

            // solution bitset store's parameters
            Pool<ObjList<Solution>::Elem> _pool;
            ObjList<Solution> _solutionObjList;

        private:
            ReGraph(const ReGraph&); // no implicit copy
        };

        // Create Resolution Graph for MCSS
        class ReCreation
        {
        public:
            ReCreation(ReGraph& rgr, MaxCommonSubgraph& context);
            // creates resolution graph
            void createRegraph();
            // method change input array to map which corresponds to list of ReGraph nodes (they are in bitset)
            void setCorrespondence(const Dbitset& b, Array<int>& map) const;
            /*
             * Inserts solution from the given mapping
             */
            bool insertSolution(const Array<int>& mapping);
            // sets input mapping to algorithm using
            bool setMapping();
            // creates all solutions
            int createSolutionMaps();

            // retruns all solutions edge and vertices lists
            void getSolutionListsSub(ObjArray<Array<int>>& v_lists, ObjArray<Array<int>>& e_lists) const;
            void getSolutionListsSuper(ObjArray<Array<int>>& v_lists, ObjArray<Array<int>>& e_lists) const;

        protected:
            // resolution graph to work with
            ReGraph& _regraph;
            // max common subgraph as context
            MaxCommonSubgraph& _context;

            // creates nodes of resolution graph
            void _nodeConstructor();
            // creates edges of resolution graph
            void _edgesConstructor();
            // returns common vertex id of two edges in graph. returns -1 if edges hasn't it
            int _getCommonVertex(int e1, int e2, Graph& graph) const;
            // returns true if two edges in graph has common vertex
            bool _hasCommonVertex(int e1, int e2, Graph& graph) const;
            // returns true if common vertices are matched
            bool _hasCommonSymbol(int e11, int e12, int e21, int e22) const;

            // returns edge and vertices list
            void _createList(const Dbitset& proj_bitset, Graph& graph, Array<int>& v_list, Array<int>& e_list) const;

        private:
            ReCreation(const ReCreation&); // no implicit copy
        };

        // Approximate algorithm: two stage optimization method (2DOM)
        //-------------------------------------------------------------------------------------------------------------------
        // this class is main util for keeping adjacancy matrix and other parameters for fast work of 2DOM algorithm
        // Adjacency matrix
        class AdjMatricesStore
        {
        public:
            AdjMatricesStore(MaxCommonSubgraph& context, int maxsize);
            // creates utilite store
            void create(Graph& g1, Graph& g2);

            // creates all solutions
            int createSolutionMaps();

            // retruns size of first graph to compared
            int getFirstSize()
            {
                return _size1;
            }
            // returns size of second graph to campare
            int getSecondSize()
            {
                return _size2;
            }
            // returns element with input indexes of adjacency matrix of second graph
            bool getSecondElement(int i, int j)
            {
                return _aj2[i]->at(j);
            }
            // retruns elements of utilites matrices which are stored matched edges
            int getFirstIdxEdge(int i, int j)
            {
                return _ajEdge1[i]->at(j);
            }
            int getSecondIdxEdge(int i, int j)
            {
                return _ajEdge2[i]->at(j);
            }
            // retruns degree of vertex in first graph adj matrix
            int getFirstVDegree(int i)
            {
                return _degreeVec1[i];
            }
            // retruns degree of vertex in first graph adj matrix
            int getSecondVDegree(int i)
            {
                return _degreeVec2[i];
            }
            // returns number of unmatched edges
            int countErrorAtEdges(int i, int j);

            // returns color and weight conditions if it is presented in max common subgraph context
            // methods takes account to corresponding between input graph ids and its equals
            // this for two graphs
            bool getEdgeWeightCondition(int i, int j);
            // this for two graphs
            bool getVerticesColorCondition(int i, int j);
            // this for one graph (first)
            bool getVColorOneCondition(int i, int j);

            // returns dbitset represent row in adjacency matrix of first graph
            Dbitset* getFirstRow(int i)
            {
                return _daj1[i];
            }
            // returns dbitset represent row in adjacency matrix of first graph
            Dbitset* getSecondRow(int i)
            {
                return _daj2[i];
            }

            // retruns solution correspondings between two graphs
            int* getX()
            {
                return _x.ptr();
            }
            int* getY()
            {
                return _y.ptr();
            }

            void getSolutions(ObjArray<Array<int>>& v_maps);

            // returns correspondence parameters between each vertex and vertex in other graph with the same label
            int getFLSize(int i)
            {
                return _mLabel1[i]->size();
            }
            int getFLV(int i, int j)
            {
                return _mLabel1[i]->at(j);
            }

            // context includes input parameters and output solution
            MaxCommonSubgraph& _context;

        protected:
            // sizes of graphs to compared
            int _size1;
            int _size2;

            // adjacency matrix
            // PtrArray< Array<bool> > _aj1;
            PtrArray<Array<bool>> _aj2;
            // indexes of edges
            PtrArray<Array<int>> _ajEdge1;
            PtrArray<Array<int>> _ajEdge2;
            // correspondence between each vertex and vertex in other graph with the same label
            PtrArray<Array<int>> _mLabel1;
            // adjacency matrix in bitset view
            PtrArray<Dbitset> _daj1;
            PtrArray<Dbitset> _daj2;
            // correspondence between two graphs
            Array<int> _x;
            Array<int> _y;
            // correspondence between real graph and matrix
            Array<int> _cr1;
            Array<int> _cr2;
            // degree vectors
            Array<int> _degreeVec1;
            Array<int> _degreeVec2;
            // matrix with not corresponding edges
            PtrArray<Array<int>> _errorEdgesMatrix;
            // maps
            Array<int> _map;
            Array<int> _invmap;

            // max size to reserve space in arrays
            int _maxsize;
            // true if two input graphs was swapped then initializes
            bool _swap;

            // sets adjacency matrix elements for two graphs
            void _setFirstElement(int i, int j, int value);
            void _setSecondElement(int i, int j, int value);

            // returns reverse correspondence between input graphs and its adj matrices
            int _getFirstC(int x);
            int _getSecondC(int x);

            // creation of matrices
            void _createCorrespondence();
            void _createLabelMatrices();
            void _createAdjacencyMatrices();
            void _createErrorEdgesMatrix();
            void _createMaps();
            // creation of graph for solution
            void _createConnectedGraph(Graph& graph, Array<int>& map_gr);

            // two graphs to compare
            Graph* _graph1;
            Graph* _graph2;
            // retruns false if there is no need to swap graphs
            bool _checkSize(Graph& g1, Graph& g2);
            // makes invert map
            void _makeInvertMap(Array<int>& map, Array<int>& invmap);

        private:
            AdjMatricesStore(const AdjMatricesStore&); // no implicit copy
        };

        // Construction stage: greedy method
        class Greedy
        {
        public:
            Greedy(AdjMatricesStore& aj);
            // main method in greedy stage of 2DOM algorithm
            void greedyMethod();

        private:
            // creates lists if vertices
            void _createLgLh();
            // returns number of matched edges
            int _matchedEdges();
            // keeping util class for 2DOM method
            AdjMatricesStore& _adjMstore;
            // list of unsigned vertices in first graph
            Array<int> _unsignVert1;
            // list of unsigned vertices in second graph
            PtrArray<Array<int>> _unsignVert2;
            // adjancy status whether vertex in 2 is adjaent to assigned
            Array<int> _adjStatus;
            // assign vertex from 1 graph to 2
            int* _x;
            // assign vertex from 2 graph to 1
            int* _y;
            // size of first graph
            int _n;
            // size of second graph
            int _m;

        protected:
            // callbacks for sorting list of vertices
            static int _compareFirstDegree(int& i1, int& i2, void* context);
            static int _compareSecondDegree(int& i1, int& i2, void* context);

        private:
            Greedy(const Greedy&); // no implicit copy
        };

        // Refinement stage: random discrete descent method
        class RandomDisDec
        {
        public:
            enum
            {
                MAX_ITERATION = 1000
            };

            RandomDisDec(AdjMatricesStore& aj);
            // main method for refinement stage
            void refinementStage();
            // returns number of unmatched edges
            int getError()
            {
                return _errorNumber;
            }
            // sets maximum iteration number limit
            void setIterationNumber(int max);

            std::shared_ptr<CancellationHandler> cancellation_handler;

        private:
            // returns number of unmatched edges
            int _goalFunction();
            // returns true if there is advantage (minimum error) to do move
            bool _acceptanceMove(int x);
            // returns true if there is advantage to do swap operation
            bool _acceptanceSwap(int x, int y);
            // creates list of error vertices
            void _makeLe();

            // keeping util for 2DOM
            AdjMatricesStore& _adjMstore;
            // assign vertex from 1 graph to 2
            int* _x;
            // assign vertex from 2 graph to 1
            int* _y;
            // error list
            CP_DECL;
            TL_CP_DECL(Array<int>, _errorList);
            // list of error vertrces
            TL_CP_DECL(Array<int>, _listErrVertices);
            // size of first graph
            int _n;
            // size of second graph
            int _m;
            // sum of all errors
            int _errorNumber;
            // new state`s sum of all errors
            int _newErrorNumber;
            // flag for keeping breaks
            bool _stop;
            // max iteration number. Algortihm breaks its work then reached it
            int _maxIteration;

            // for stucking override
            Array<int> _stateArray;
            // error number in previous stuck state
            int _errorNumberStuck;
            // number of iterations before algorithm consider current state as stuck state
            int _stuckCount;
            // save current state in case of stuck
            void _saveState();
            // load state
            void _loadState();

        private:
            RandomDisDec(const RandomDisDec&); // no implicit copy
        };

        // Randomizator for approximate algorithm
        class DLLEXPORT RandomHandler
        {
        public:
            enum
            {
                DEFSEED = 54217137,
                BIG_PRIME = 899999963
            };

            RandomHandler(int ijkl) : strand(false)
            {
                ranmarin(ijkl % BIG_PRIME);
            }
            RandomHandler(long ijkl) : strand(false)
            {
                ranmarin(ijkl % BIG_PRIME);
            }
            RandomHandler() : strand(false)
            {
                ranmarin(DEFSEED);
            };

            void ranmarin(int ijkl)
            {
                int ij, kl;
                int i, ii, j, jj, k, l, m;
                double s, t;

                u.resize(97);
                uvec.resize(97);

                ij = ijkl / 30082;
                kl = ijkl - 30082 * ij;

                i = ((ij / 177) % 177) + 2;
                j = (ij % 177) + 2;
                k = ((kl / 169) % 178) + 1;
                l = kl % 169;
                for (ii = 0; ii < 97; ++ii)
                {
                    s = 0.0;
                    t = 0.5;
                    for (jj = 0; jj < 24; ++jj)
                    {
                        m = (((i * j) % 179) * k) % 179;
                        i = j;
                        j = k;
                        k = m;
                        l = (53 * l + 1) % 169;
                        if (((l * m) % 64) >= 32)
                            s += t;
                        t *= 0.5;
                    }
                    u[ii] = s;
                }
                c = 362436.0 / 16777216.0;
                cd = 7654321.0 / 16777216.0;
                cm = 16777213.0 / 16777216.0;
                i97 = 96;
                j97 = 32;
            }

            inline double next()
            {
                double uni;
                uni = u[i97] - u[j97];
                if (uni < 0.0)
                    uni += 1.0;
                u[i97] = uni;
                if (--i97 < 0)
                    i97 = 96;
                if (--j97 < 0)
                    j97 = 96;
                c -= cd;
                if (c < 0.0)
                    c += cm;
                uni -= c;
                if (uni < 0.0)
                    uni += 1.0;
                return uni;
            }

            inline int next(int max)
            {
                if (strand)
                    return (rand() % max);
                else
                    return (int)(max * next());
            }

            void next(Array<double>& d)
            {
                double uni;
                int n = d.size();
                for (int i = 0; i < n; ++i)
                {
                    uni = u[i97] - u[j97];
                    if (uni < 0.0)
                        uni += 1.0;
                    u[i97] = uni;
                    if (--i97 < 0)
                        i97 = 96;
                    if (--j97 < 0)
                        j97 = 96;
                    c -= cd;
                    if (c < 0.0)
                        c += cm;
                    uni -= c;
                    if (uni < 0.0)
                        uni += 1.0;
                    d[i] = uni;
                }
            }

            double c, cd, cm;
            Array<double> u;
            Array<double> uvec;
            int i97, j97;
            bool strand;
        };

    protected:
        // keeping graphs
        Graph* _subgraph;
        Graph* _supergraph;

        bool _findTrivialMcs();
        void _clearSolutionMaps();
        void _addSolutionMap(Array<int>& v_map, Array<int>& e_map);

        // method returns true if edges with input number completely matched
        bool _getEdgeColorCondition(Graph& graph1, Graph& graph2, int i, int j) const;

        // returns all solutions
        void _getSolutionMaps(int count, ObjArray<Array<int>>& v_maps, ObjArray<Array<int>>& e_maps) const;

        // array for keeping all solutions. In each subarray element[0] = vertex size, [1] = edge size, and
        // next '[0]' elements for vertex map, next '[1]' for edge map (in sum 2+vertexEnd()+edgeEnd() elements)
        ObjArray<Array<int>> _vertEdgeSolMap;

        RandomHandler _random;

    private:
        MaxCommonSubgraph(const MaxCommonSubgraph&); // no implicit copy
    };

    // for searching substructure with map
    class SubstructureMcs
    {
    public:
        enum
        {
            UNMAPPED = -1
        };

        // constuctors
        SubstructureMcs();
        SubstructureMcs(Graph& sub, Graph& super);
        virtual ~SubstructureMcs()
        {
        }

        // sets graphs for substructure search considering their size
        void setGraphs(Graph& sub, Graph& super);
        // searches substructure for graphs and maps vertices
        virtual bool searchSubstructure(Array<int>* map);

        // condition for edge match
        bool (*cbMatchEdge)(Graph& graph1, Graph& graph2, int i, int j, void* userdata);
        // condition for vertex match
        bool (*cbMatchVertex)(Graph& graph1, Graph& graph2, const int* core_sub, int i, int j, void* userdata);
        void* userdata;
        // returns true if graphs was swapped then initializing
        bool isInverted()
        {
            return _invert;
        };
        // function for substructure search always return 0
        static int _embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata);

    protected:
        // ptrs for input graphs
        Graph* _sub;
        Graph* _super;

        // variable for considering swap input graphs
        bool _invert;

    private:
        SubstructureMcs(const SubstructureMcs&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
