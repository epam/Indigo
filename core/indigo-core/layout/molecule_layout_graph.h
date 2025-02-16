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

#ifndef __molecule_layout_graph_h__
#define __molecule_layout_graph_h__

#include "base_cpp/cancellation_handler.h"
#include "base_cpp/obj.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/tlscont.h"
#include "graph/filter.h"
#include "graph/graph.h"
#include "layout/layout_pattern_holder.h"
#include "math/algebra.h"
#include "molecule/molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    const int SOME_MAGIC_INT_FOR_RANDOM_1 = 931170243;
    const int SOME_MAGIC_INT_FOR_RANDOM_2 = 931170240;
    const int SOME_MAGIC_INT_FOR_RANDOM_3 = 931170242;
    const int SOME_MAGIC_INT_FOR_RANDOM_4 = 931170241;

    class BiconnectedDecomposer;

#ifdef _DEBUG
#define M_LAYOUT_DEBUG
#endif

    enum
    {
        ELEMENT_NOT_DRAWN = 0,
        ELEMENT_INTERNAL,
        ELEMENT_BOUNDARY,
        ELEMENT_NOT_PLANAR,
        ELEMENT_IGNORE,
        ELEMENT_DRAWN
    };

    struct LayoutVertex
    {
        LayoutVertex()
        {
            memset(this, 0, sizeof(LayoutVertex));
        }

        int ext_idx;
        int orig_idx;
        long morgan_code;
        bool is_cyclic;
        int type;

        Vec2f pos;
    };

    struct LayoutEdge
    {
        LayoutEdge()
        {
            memset(this, 0, sizeof(LayoutEdge));
        }

        int ext_idx;
        int orig_idx;
        bool is_cyclic;
        int type;
    };

    class DLLEXPORT MoleculeLayoutGraph : public Graph
    {
    public:
        explicit MoleculeLayoutGraph();
        ~MoleculeLayoutGraph() override;

        virtual MoleculeLayoutGraph* getInstance() = 0;

        inline const Vec2f& getPos(int idx) const
        {
            return _layout_vertices[idx].pos;
        }
        inline Vec2f& getPos(int idx)
        {
            return _layout_vertices[idx].pos;
        }
        inline int getVertexExtIdx(int idx) const
        {
            return _layout_vertices[idx].ext_idx;
        }
        inline int getVertexType(int idx) const
        {
            return _layout_vertices[idx].type;
        }
        inline int getEdgeExtIdx(int idx) const
        {
            return _layout_edges[idx].ext_idx;
        }
        inline int getEdgeType(int idx) const
        {
            return _layout_edges[idx].type;
        }

        void setVertexType(int idx, int type)
        {
            _layout_vertices[idx].type = type;
        }
        void setEdgeType(int idx, int type)
        {
            _layout_edges[idx].type = type;
        }

        void clear() override;

        bool isSingleEdge() const;

        void registerLayoutVertex(int idx, const LayoutVertex& vertex);
        void registerLayoutEdge(int idx, const LayoutEdge& edge);
        int addLayoutVertex(int ext_idx, int type);
        int addLayoutEdge(int beg, int end, int ext_idx, int type);

        const LayoutVertex& getLayoutVertex(int idx) const;
        const LayoutEdge& getLayoutEdge(int idx) const;

        int findVertexByExtIdx(int ext_idx) const;

        virtual float calculateAngle(int v, int& v1, int& v2) const = 0;

        void makeOnGraph(Graph& graph);
        virtual void makeLayoutSubgraph(MoleculeLayoutGraph& graph, Filter& filter) = 0;
        void cloneLayoutGraph(MoleculeLayoutGraph& other, Array<int>* mapping);
        void copyLayoutTo(MoleculeLayoutGraph& other, const Array<int>& mapping) const;

        virtual void layout(BaseMolecule& molecule, float bond_length, const Filter* filter, bool respect_existing,
                            std::optional<Vec2f> multiple_distance = std::nullopt);

        const BaseMolecule* getMolecule(const int** molecule_edge_mapping) const
        {
            *molecule_edge_mapping = _molecule_edge_mapping;
            return _molecule;
        }

        virtual void flipped() = 0;
        bool isFlipped() const
        {
            return _flipped;
        }

        bool _flipped; // component was flipped after attaching

        int max_iterations;
        LAYOUT_ORIENTATION layout_orientation;

        bool preserve_existing_layout;

        CancellationHandler* cancellation;

        DECL_ERROR;

        ObjArray<LayoutVertex> _layout_vertices;
        ObjArray<LayoutEdge> _layout_edges;

        Array<int> _fixed_vertices;

        long _total_morgan_code;
        int _first_vertex_idx;
        int _n_fixed;

        // Outline of the graph (from pattern)
        Obj<Array<Vec2f>> _outline;

        BaseMolecule* _molecule;
        const int* _molecule_edge_mapping;

        struct Cycle
        {
            explicit Cycle();
            explicit Cycle(const List<int>& edges, const MoleculeLayoutGraph& graph);
            explicit Cycle(const Array<int>& vertices, const Array<int>& edges);

            void copy(const List<int>& edges, const MoleculeLayoutGraph& graph);
            void copy(const Array<int>& vertices, const Array<int>& edges);

            int vertexCount() const
            {
                return _vertices.size();
            }
            int getVertex(int idx) const
            {
                return _vertices[idx];
            }
            int getVertexC(int idx) const
            {
                return _vertices[(vertexCount() + idx) % vertexCount()];
            }
            int getEdge(int idx) const
            {
                return _edges[idx];
            }
            int getEdgeC(int idx) const
            {
                return _edges[(_edges.size() + idx) % _edges.size()];
            }
            int getEdgeStart(int idx) const
            {
                return getVertexC(idx);
            }
            int getEdgeFinish(int idx) const
            {
                return getVertexC(idx + 1);
            }
            int findVertex(int idx) const
            {
                return _vertices.find(idx);
            }
            long morganCode() const
            {
                if (!_morgan_code_calculated)
                {
                    throw Error("Morgan code does not calculated yet.");
                }
                return _morgan_code;
            }
            void setVertexWeight(int idx, int w)
            {
                _attached_weight[idx] = w;
            }
            void addVertexWeight(int idx, int w)
            {
                _attached_weight[idx] += w;
            }
            int getVertexWeight(int idx) const
            {
                return _attached_weight[idx];
            }
            void canonize();
            bool contains(const Cycle& another) const;
            void calcMorganCode(const MoleculeLayoutGraph& parent_graph);

            static int compare_cb(int& idx1, int& idx2, void* context);

        protected:
            CP_DECL;
            TL_CP_DECL(Array<int>, _vertices);
            TL_CP_DECL(Array<int>, _edges);
            TL_CP_DECL(Array<int>, _attached_weight);

            int _max_idx;
            long _morgan_code;
            bool _morgan_code_calculated;

        private:
            Cycle(const Cycle& other); // No copy constructor
        };
        struct EnumContext
        {
            const MoleculeLayoutGraph* graph;
            RedBlackSet<int>* edges;
            int iterationNumber;
            int maxIterationNumber;
        };

        // geometry functions
        int _calcIntersection(int edge1, int edge2) const;
        bool _isVertexOnEdge(int vert_idx, int edge_beg, int edge_end) const;
        bool _isVertexOnSomeEdge(int vert_idx) const;
        void _shiftEdge(int edge_idx, float delta);
        bool _drawRegularCurve(const Array<int>& chain, int begin, int end, float length, bool ccw, int type);
        bool _drawRegularCurveEx(const Array<int>& chain, int begin, int end, float length, bool ccw, int type, const Array<int>& mapping);
        static void _findAngles(int k, float s, float& x, float& y);
        static float _dichotomy1(float a0, float b0, int L, float s);
        static float _dichotomy2(float a0, float b0, int L, float s);
        static void _calculatePos(float phi, const Vec2f& v1, const Vec2f& v2, Vec2f& v);

        // border functions
        virtual void _getBorder(Cycle& border) const = 0;
        virtual bool _isPointOutside(const Vec2f& p) const = 0;
        virtual bool _isPointOutsideCycle(const Cycle& cycle, const Vec2f& p) const = 0;
        bool _isPointOutsideCycleEx(const Cycle& cycle, const Vec2f& p, const Array<int>& mapping) const;
        static bool _border_cb(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context);

        void getBoundingBox(Rect2f& bbox) const;
        void getBoundingBox(Vec2f& left_bottom, Vec2f& right_top) const;
        void copyCoordsFromComponent(MoleculeLayoutGraph& component, Vec2f shift = {0, 0});

        // for components
        virtual void _calcMorganCodes();

        // for whole graph
        void _assignAbsoluteCoordinates(float bond_length);

        bool _checkBadTryBorderIntersection(Array<int>& chain_ext, MoleculeLayoutGraph& next_bc, Array<int>& mapping);
        bool _checkBadTryChainOutside(Array<int>& chain_ext, MoleculeLayoutGraph& next_bc, Array<int>& mapping);

        virtual void _assignRelativeCoordinates(int& fixed_component, const MoleculeLayoutGraph& supergraph) = 0;

        // refine
        static bool _edge_check(Graph& graph, int e_idx, void* context);
        void _refineCoordinates(const BiconnectedDecomposer& bc_decomposer, const PtrArray<MoleculeLayoutGraph>& bc_components, const Array<int>& bc_tree);
        bool _allowRotateAroundVertex(int idx) const;
        void _makeBranches(Array<int>& branches, int edge, Filter& filter) const;
        void _excludeDandlingIntersections();
        static bool _path_handle(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context);

        // attaching
        void _attachEars(int vert_idx, int drawn_idx, int* ears, const Vec2f& rest_pos);

        // assigning coordinates
        void _attachDandlingVertices(int vert_idx, Array<int>& adjacent_list);
        void _calculatePositionsOneNotDrawn(Array<Vec2f>& positions, int n_pos, int vert_idx, int not_drawn_idx);
        void _calculatePositionsSingleDrawn(int vert_idx, Array<int>& adjacent_list, int& n_pos, int drawn_idx, bool& two_ears, Array<Vec2f>& positions,
                                            int& parity);
        void _calculatePositionsManyNotDrawn(int vert_idx, Array<int>& adjacent_list, Array<Vec2f>& positions);
        void _orderByEnergy(Array<Vec2f>& positions);
        void _assignRelativeSingleEdge(int& fixed_component, const MoleculeLayoutGraph& supergraph);
        void _findFirstVertexIdx(int n_comp, Array<int>& fixed_components, PtrArray<MoleculeLayoutGraph>& bc_components, bool all_trivial);
        bool _prepareAssignedList(Array<int>& assigned_list, BiconnectedDecomposer& bc_decom, PtrArray<MoleculeLayoutGraph>& bc_components,
                                  Array<int>& bc_tree);
        void _assignFinalCoordinates(float bond_length, const Array<Vec2f>& src_layout);
        void _copyLayout(MoleculeLayoutGraph& component);
        void _getAnchor(int& v1, int& v2, int& v3) const;

        void _findFixedComponents(BiconnectedDecomposer& bc_decom, Array<int>& fixed_components, PtrArray<MoleculeLayoutGraph>& bc_components);
        bool _assignComponentsRelativeCoordinates(PtrArray<MoleculeLayoutGraph>& bc_components, Array<int>& fixed_components, BiconnectedDecomposer& bc_decom);
        void _attachCrossingEdges();

        void _buildOutline(void);

        // make tree of biconnected components (tree[i] - component incoming to vertex i or -1)
        static void _makeComponentsTree(BiconnectedDecomposer& decon, PtrArray<MoleculeLayoutGraph>& components, Array<int>& tree);

        void _layoutMultipleComponents(BaseMolecule& molecule, bool respect_existing, const Filter* filter, float bond_length,
                                       std::optional<Vec2f> multiple_distance = std::nullopt);
        void _layoutSingleComponent(BaseMolecule& molecule, bool respect_existing, const Filter* filter, float bond_length);

    protected:
        virtual void _layout_component(BiconnectedDecomposer& bc_decom, PtrArray<MoleculeLayoutGraph>& bc_components, Array<int>& bc_tree,
                                       Array<int>& fixed_components, int src_vertex) = 0;
    };

    class DLLEXPORT MoleculeLayoutGraphSimple : public MoleculeLayoutGraph
    {
    public:
        explicit MoleculeLayoutGraphSimple();
        ~MoleculeLayoutGraphSimple() override;

        MoleculeLayoutGraph* getInstance() override;

        void clear() override;

        float calculateAngle(int v, int& v1, int& v2) const override;

        void makeLayoutSubgraph(MoleculeLayoutGraph& graph, Filter& filter) override;

        void flipped() override
        {
            _flipped = true;
        };

        ObjArray<PatternLayout>& getPatterns();

#ifdef M_LAYOUT_DEBUG
        void saveDebug();
#endif

        DECL_ERROR;

    protected:
        // patterns
        static int _pattern_cmp2(const PatternLayout& p1, int n_v, int n_e, long code);
        static bool _match_pattern_bond(Graph& subgraph, Graph& supergraph, int self_idx, int other_idx, void* userdata);
        static int _pattern_embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata);

        // THERE

        // assigning coordinates
        void _assignRelativeCoordinates(int& fixed_component, const MoleculeLayoutGraph& supergraph) override;
        bool _tryToFindPattern(int& fixed_component);
        void _assignFirstCycle(const Cycle& cycle);

        // attaching cycles
        bool _attachCycleOutside(const Cycle& cycle, float length, int n_common);
        bool _drawEdgesWithoutIntersection(const Cycle& cycle, Array<int>& cycle_vertex_types);

        bool _attachCycleInside(const Cycle& cycle, float length);
        bool _attachCycleWithIntersections(const Cycle& cycle, float length);
        void _setChainType(const Array<int>& chain, const Array<int>& mapping, int type);
        bool _splitCycle(const Cycle& cycle, const Array<int>& cycle_vertex_types, bool check_boundary, Array<int>& chain_ext, Array<int>& chain_int,
                         int& c_beg, int& c_end) const;
        void _splitCycle2(const Cycle& cycle, const Array<int>& cycle_vertex_types, ObjArray<Array<int>>& chains_ext) const;

        // border functions
        void _getBorder(Cycle& border) const override;
        void _splitBorder(int v1, int v2, Array<int>& part1v, Array<int>& part1e, Array<int>& part2v, Array<int>& part2e) const;
        bool _isPointOutside(const Vec2f& p) const override;
        bool _isPointOutsideCycle(const Cycle& cycle, const Vec2f& p) const override;

        static bool _edge_check(Graph& graph, int e_idx, void* context);

        virtual void _layout_component(BiconnectedDecomposer& bc_decom, PtrArray<MoleculeLayoutGraph>& bc_components, Array<int>& bc_tree,
                                       Array<int>& fixed_components, int src_vertex) override;
    };

    struct local_pair_ii
    {
        int left;
        int right;

        local_pair_ii(int l, int r)
        {
            left = l;
            right = r;
        }
    };

    struct local_pair_id
    {
        int left;
        float right;

        local_pair_id(int l, float r)
        {
            left = l;
            right = r;
        }
    };

    class MoleculeLayoutGraphSmart;
    class MoleculeLayoutMacrocycles;
    class MoleculeLayoutMacrocyclesLattice;

    class DLLEXPORT MoleculeLayoutSmoothingSegment
    {

    private:
        float _length;
        Array<Vec2f> _pos;
        int _finish_number;
        int _start_number;
        Vec2f& _start;
        Vec2f& _finish;
        Vec2f _center;
        int _layout_component_number;
        float _square;
        float _radius;

        Vec2f _getPosition(Vec2f);
        float calc_radius(Vec2f);

    public:
        MoleculeLayoutGraphSmart& _graph;

        MoleculeLayoutSmoothingSegment(MoleculeLayoutGraphSmart& mol, Vec2f& start, Vec2f& finish);
        Vec2f getPosition(int);
        Vec2f getIntPosition(int) const;
        void shiftStartBy(Vec2f shift);
        void shiftFinishBy(Vec2f shift);
        float getLength() const;
        float getLengthCoef() const;
        Vec2f getCenter();
        Vec2f getIntCenter();
        void updateStartFinish();
        bool isVertexUp(int v);
        int get_layout_component_number();
        void set_layout_component_number(int number);
        void inverse();
        void set_start_finish_number(int, int);
        float get_square();
        void calculate_square();
        int get_start() const;
        int get_finish() const;
        float get_radius();
        bool can_touch_to(MoleculeLayoutSmoothingSegment&);

        bool is_start(int v) const
        {
            return v == _start_number;
        }
        bool is_finish(int v) const
        {
            return v == _finish_number;
        }

        float get_min_x();
        float get_min_y();
        float get_max_x();
        float get_max_y();
    };

    class DLLEXPORT SmoothingCycle
    {
    public:
        CP_DECL;
        SmoothingCycle(Array<Vec2f>&, Array<float>&);
        SmoothingCycle(Array<Vec2f>&, Array<float>&, Array<int>&, int);
        SmoothingCycle(Array<Vec2f>&, Array<float>&, ObjArray<MoleculeLayoutSmoothingSegment>&);

        int cycle_length;
        Array<Vec2f>& point;
        Array<float>& target_angle;
        MoleculeLayoutSmoothingSegment* segment;
        TL_CP_DECL(Array<float>, edge_length);

        bool is_simple_component(int i) const
        {
            return segment == 0 || segment[i].get_layout_component_number() < 0;
        }
        float get_radius(int i)
        {
            return segment == 0 ? (point[(i + 1) % cycle_length] - point[i]).length() / 2 : segment[i].get_radius();
        }
        Vec2f get_center(int i)
        {
            return segment == 0 ? (point[(i + 1) % cycle_length] + point[i]) / 2 : segment[i].getCenter();
        }
        float get_length(int i)
        {
            return edge_length[i];
        }

        DECL_ERROR;

        void _do_smoothing(int iter_count);

        void _gradient_step(float coef, Array<local_pair_ii>& touching_segments, bool);

        static Vec2f _get_len_derivative(Vec2f current_vector, float target_dist, bool);
        static Vec2f _get_len_derivative_simple(Vec2f current_vector, float target_dist);
        static Vec2f _get_angle_derivative(Vec2f left_point, Vec2f right_point, float target_angle, bool flag = false);
    };

    class DLLEXPORT MoleculeLayoutGraphSmart : public MoleculeLayoutGraph
    {
    public:
        explicit MoleculeLayoutGraphSmart();
        ~MoleculeLayoutGraphSmart() override;

        MoleculeLayoutGraph* getInstance() override;

        void clear() override;

        inline int getVertexOrigIdx(int idx) const
        {
            return _layout_vertices[idx].orig_idx;
        }
        inline int getEdgeOrigIdx(int idx) const
        {
            return _layout_edges[idx].orig_idx;
        }
        inline bool isEdgeDrawn(int idx) const
        {
            return _layout_edges[idx].type != ELEMENT_NOT_DRAWN;
        }
        inline bool isVertexDrawn(int idx) const
        {
            return _layout_vertices[idx].type != ELEMENT_NOT_DRAWN;
        }

        float calculateAngle(int v, int& v1, int& v2) const override;

        void makeLayoutSubgraph(MoleculeLayoutGraph& graph, Filter& vertex_filter) override;
        void makeLayoutSubgraph(MoleculeLayoutGraph& graph, Filter& vertex_filter, Filter* edge_filter);

        void calcMorganCode();
        long getMorganCode();

        void assignFirstVertex(int v);

        const BaseMolecule* getMolecule(const int** molecule_edge_mapping)
        {
            *molecule_edge_mapping = _molecule_edge_mapping;
            return _molecule;
        }
        const BaseMolecule* getMolecule()
        {
            return _molecule;
        }

        const int* getEdgeMapping()
        {
            return _molecule_edge_mapping;
        }

        float _get_square();
        void flipped() override;

#ifdef M_LAYOUT_DEBUG
        void saveDebug();
#endif

        DECL_ERROR;

    protected:
        // THERE

        // assigning coordinates
        struct interval
        {
            int left;
            int right;

            void init(int _l, int _r)
            {
                left = _l;
                right = _r;
            }
            interval(int _l, int _r)
            {
                init(_l, _r);
            }
        };

        void _assignRelativeCoordinates(int& fixed_component, const MoleculeLayoutGraph& supergraph) override;
        void _get_toches_to_component(Cycle& cycle, int component_number, Array<interval>& interval_list);
        int _search_separated_component(Cycle& cycle, Array<interval>& interval_list);
        void _search_path(int start, int finish, Array<int>& path, int component_number);
        void _assignEveryCycle(const Cycle& cycle);

        // smoothing
        void _segment_smoothing(const Cycle& cycle, const MoleculeLayoutMacrocyclesLattice& layout, Array<int>& rotation_vertex, Array<Vec2f>& rotation_point,
                                ObjArray<MoleculeLayoutSmoothingSegment>& segment);
        void _update_touching_segments(Array<local_pair_ii>&, ObjArray<MoleculeLayoutSmoothingSegment>&);
        void _segment_smoothing_prepearing(const Cycle& cycle, Array<int>& rotation_vertex, Array<Vec2f>& rotation_point,
                                           ObjArray<MoleculeLayoutSmoothingSegment>& segment, MoleculeLayoutMacrocyclesLattice& layout);
        void _segment_calculate_target_angle(const MoleculeLayoutMacrocyclesLattice& layout, Array<int>& rotation_vertex, Array<float>& target_angle,
                                             ObjArray<MoleculeLayoutSmoothingSegment>& segment);
        void _segment_update_rotation_points(const Cycle& cycle, Array<int>& rotation_vertex, Array<Vec2f>& rotation_point,
                                             ObjArray<MoleculeLayoutSmoothingSegment>& segment);
        void _segment_smoothing_unstick(ObjArray<MoleculeLayoutSmoothingSegment>& segment);
        void _do_segment_smoothing(Array<Vec2f>& rotation_point, Array<float>& target_angle, ObjArray<MoleculeLayoutSmoothingSegment>& segment);
        void _segment_improoving(Array<Vec2f>& rotation_point, Array<float>& target_angle, ObjArray<MoleculeLayoutSmoothingSegment>& segment, int, float,
                                 Array<local_pair_ii>&);
        void _do_segment_smoothing_gradient(Array<Vec2f>& rotation_point, Array<float>& target_angle, ObjArray<MoleculeLayoutSmoothingSegment>& segment);
        void _gradient_step(Array<Vec2f>& point, Array<float>& target_angle, ObjArray<MoleculeLayoutSmoothingSegment>& segment, float coef,
                            Array<local_pair_ii>& touching_segments);

        // attaching cycles
        bool _attachCycleOutside(const Cycle& cycle, float length, int n_common);
        bool _drawEdgesWithoutIntersection(const Cycle& cycle, Array<int>& cycle_vertex_types);

        bool _attachCycleInside(const Cycle& cycle, float length);
        bool _attachCycleWithIntersections(const Cycle& cycle, float length);
        void _setChainType(const Array<int>& chain, const Array<int>& mapping, int type);
        bool _splitCycle(const Cycle& cycle, const Array<int>& cycle_vertex_types, bool check_boundary, Array<int>& chain_ext, Array<int>& chain_int,
                         int& c_beg, int& c_end) const;
        void _splitCycle2(const Cycle& cycle, const Array<int>& cycle_vertex_types, ObjArray<Array<int>>& chains_ext) const;

        // border functions
        void _getBorder(Cycle& border) const override;
        void _getSurroundCycle(Cycle& cycle, Vec2f p) const;
        void _splitBorder(int v1, int v2, Array<int>& part1v, Array<int>& part1e, Array<int>& part2v, Array<int>& part2e) const;
        bool _isPointOutside(const Vec2f& p) const override;
        bool _isPointOutsideCycle(const Cycle& cycle, const Vec2f& p) const override;

        // geometry functions
        const float _energyOfPoint(Vec2f p) const;
        int _isCisConfiguratuin(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4);

        virtual void _layout_component(BiconnectedDecomposer& bc_decom, PtrArray<MoleculeLayoutGraph>& bc_components, Array<int>& bc_tree,
                                       Array<int>& fixed_components, int src_vertex);

        Array<int> _layout_component_number; // number of layout component of certain edge
        int _layout_component_count;

        MoleculeLayoutGraph* _graph;

    private:
        MoleculeLayoutGraphSmart(const MoleculeLayoutGraphSmart&);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
