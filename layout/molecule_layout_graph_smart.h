/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __molecule_layout_graph_h_smart__
#define __molecule_layout_graph_h_smart__

#include "graph/graph.h"
#include "graph/filter.h"
#include "math/algebra.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/obj_array.h"
#include "molecule/molecule.h"
#include "base_cpp/obj.h"
#include "base_cpp/cancellation_handler.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

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

struct LayoutVertexSmart
{
   LayoutVertexSmart () { memset(this, 0, sizeof(LayoutVertexSmart)); }

   int  orig_idx;
   int  ext_idx;
   long morgan_code;
   bool is_cyclic;
   int  type;

   Vec2f pos;
};

struct LayoutEdgeSmart
{
   LayoutEdgeSmart () { memset(this, 0, sizeof(LayoutEdgeSmart)); }

   int  orig_idx;
   int  ext_idx;
   bool is_cyclic;
   int  type;
};

struct local_pair_ii {
   int left;
   int right;

   local_pair_ii(int l, int r) { left = l; right = r; }
};

struct local_pair_id {
   int left;
   double right;

   local_pair_id(int l, double r) { left = l; right = r; }
};

class MoleculeLayoutGraphSmart;
class MoleculeLayoutMacrocycles;
class MoleculeLayoutMacrocyclesLattice;

class DLLEXPORT MoleculeLayoutSmoothingSegment {

private:
   float _length;
   Array<Vec2f> _pos;
   int _finish_number;
   int _start_number;
   Vec2f& _start;
   Vec2f& _finish;
   Vec2f _center;
   int _layout_component_number;
   double _square;
   double _radius;

   Vec2f _getPosition(Vec2f);
   double calc_radius(Vec2f);

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
   double get_square();
   void calculate_square();
   int get_start () const;
   int get_finish () const;
   double get_radius();
   bool can_touch_to(MoleculeLayoutSmoothingSegment&);

   bool is_start(int v) {return v == _start_number;}
   bool is_finish(int v) {return v == _finish_number;}

   float get_min_x();
   float get_min_y();
   float get_max_x();
   float get_max_y();
   

};

class DLLEXPORT MoleculeLayoutGraphSmart : public Graph
{
public:
   explicit MoleculeLayoutGraphSmart ();
   virtual ~MoleculeLayoutGraphSmart ();

   virtual void clear ();

   bool isSingleEdge () const;

   inline const Vec2f & getPos (int idx) const { return _layout_vertices[idx].pos; }
   inline       Vec2f & getPos (int idx)       { return _layout_vertices[idx].pos; }
   inline int getVertexExtIdx(int idx) const { return _layout_vertices[idx].ext_idx; }
   inline int getVertexOrigIdx(int idx) const { return _layout_vertices[idx].orig_idx; }
   inline int getVertexType(int idx) const { return _layout_vertices[idx].type; }
   inline int getEdgeExtIdx(int idx) const { return _layout_edges[idx].ext_idx; }
   inline int getEdgeOrigIdx(int idx) const { return _layout_edges[idx].orig_idx; }
   inline int getEdgeType(int idx) const { return _layout_edges[idx].type; }
   inline bool isEdgeDrawn(int idx) const { return _layout_edges[idx].type != ELEMENT_NOT_DRAWN; }
   inline bool isVertexDrawn(int idx) const { return _layout_vertices[idx].type != ELEMENT_NOT_DRAWN; }

   void registerLayoutVertex (int idx, const LayoutVertexSmart &vertex);
   void registerLayoutEdge   (int idx, const LayoutEdgeSmart &edge);
   int  addLayoutVertex      (int ext_idx, int type);
   int  addLayoutEdge        (int beg, int end, int ext_idx, int type);

   const LayoutVertexSmart &getLayoutVertex (int idx) const;
   const LayoutEdgeSmart   &getLayoutEdge   (int idx) const;

   void setVertexType (int idx, int type) { _layout_vertices[idx].type = type; }
   void setEdgeType   (int idx, int type) { _layout_edges[idx].type = type; }

   int findVertexByExtIdx (int ext_idx) const;

   float calculateAngle (int v, int &v1, int &v2) const;

   void makeOnGraph (Graph &graph);
   void makeLayoutSubgraph (MoleculeLayoutGraphSmart &graph, Filter &vertex_filter);
   void makeLayoutSubgraph (MoleculeLayoutGraphSmart &graph, Filter &vertex_filter, Filter *edge_filter);
   void cloneLayoutGraph (MoleculeLayoutGraphSmart &other, Array<int> *mapping);
   void copyLayoutTo (MoleculeLayoutGraphSmart &other, const int *mapping) const;

   void layout (BaseMolecule &molecule, float bond_length, const Filter *filter, bool respect_existing);
   
   void calcMorganCode ();
   long getMorganCode ();

   void assignFirstVertex (int v);

   const BaseMolecule *getMolecule (const int **molecule_edge_mapping) { *molecule_edge_mapping = _molecule_edge_mapping; return _molecule; }
   const BaseMolecule *getMolecule () { return _molecule; }

   const int *getEdgeMapping () { return _molecule_edge_mapping; }

   int max_iterations;
   bool smart_layout;

   CancellationHandler* cancellation;
   
   double _get_square();


#ifdef M_LAYOUT_DEBUG
   void saveDebug ();
#endif

   DECL_ERROR;

protected:

   struct CycleSmart
   {
      explicit CycleSmart();
      explicit CycleSmart(const List<int> &edges, const MoleculeLayoutGraphSmart &graph);
      explicit CycleSmart(const Array<int> &vertices, const Array<int> &edges);

      void copy (const List<int> &edges, const MoleculeLayoutGraphSmart &graph);
      void copy (const Array<int> &vertices, const Array<int> &edges);

      int vertexCount () const { return _vertices.size(); }
      int getVertex  (int idx) const { return _vertices[idx]; }
      int getVertexC (int idx) const { return _vertices[(vertexCount() + idx) % vertexCount()]; }
      int getEdge    (int idx) const { return _edges[idx]; }
      int getEdgeC   (int idx) const { return _edges[(_edges.size() + idx) % _edges.size()]; }
      int getEdgeStart (int idx) const {return getVertexC(idx); }
      int getEdgeFinish (int idx) const {return getVertexC(idx + 1); }
      int findVertex (int idx) const { return _vertices.find(idx); }
      void setVertexWeight(int idx, int w) {_attached_weight[idx] = w;}
      void addVertexWeight(int idx, int w) {_attached_weight[idx] += w;}
      int getVertexWeight(int idx) const {return _attached_weight[idx];}
      long morganCode() const;
      void canonize ();
      bool contains (const CycleSmart &another) const;
      void calcMorganCode (const MoleculeLayoutGraphSmart &parent_graph);

      static int compare_cb (int &idx1, int &idx2, void *context);

   protected:

      CP_DECL;
      TL_CP_DECL(Array<int>, _vertices);
      TL_CP_DECL(Array<int>, _edges);
      TL_CP_DECL(Array<int>, _attached_weight);

//      Array<int> _vertices;
//      Array<int> _edges;
      int _max_idx;
      long _morgan_code;
      bool _morgan_code_calculated;

   private:
      CycleSmart(const CycleSmart &other); // No copy constructor

   };

   struct EnumContextSmart
   {
      const MoleculeLayoutGraphSmart *graph;
      RedBlackSet<int> *edges;
      int iterationNumber;
      int maxIterationNumber;
   };

   static bool _path_handle (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);

   // for whole graph
   void _assignAbsoluteCoordinates (float bond_length);
   void _findFirstVertexIdx (int n_comp, Array<int> & fixed_components, ObjArray<MoleculeLayoutGraphSmart> &bc_components, bool all_trivial);
   bool _prepareAssignedList (Array<int> &assigned_list, BiconnectedDecomposer &bc_decom, ObjArray<MoleculeLayoutGraphSmart> &bc_components, Array<int> &bc_tree);
   void _assignFinalCoordinates (float bond_length, const Array<Vec2f> &src_layout);
   void _copyLayout (MoleculeLayoutGraphSmart &component);
   void _getAnchor (int &v1, int &v2, int &v3) const;

   void _findFixedComponents (BiconnectedDecomposer &bc_decom, Array<int> &fixed_components, ObjArray<MoleculeLayoutGraphSmart> &bc_components);
   bool _assignComponentsRelativeCoordinates (ObjArray<MoleculeLayoutGraphSmart> &bc_components, Array<int> &fixed_components, BiconnectedDecomposer &bc_decom);

   // refine
   void _refineCoordinates (const BiconnectedDecomposer &bc_decomposer, const ObjArray<MoleculeLayoutGraphSmart> &bc_components, const Array<int> &bc_tree);
   bool _allowRotateAroundVertex (int idx) const;
   void _makeBranches (Array<int> &branches, int edge, Filter &filter) const;
   void _findBranch (Array<int> &branches, int v, int edge) const;
   void _excludeDandlingIntersections ();

   // for components
   void _calcMorganCodes ();

   // assigning coordinates
   struct interval {
      int left;
      int right;

      void init(int _l, int _r) { left = _l; right = _r; }
      interval(int _l, int _r) { init(_l, _r); }
      
   };

   void _assignRelativeCoordinates (int &fixed_component, const MoleculeLayoutGraphSmart &supergraph);
   void _assignRelativeSingleEdge (int &fixed_component, const MoleculeLayoutGraphSmart &supergraph);
   void _get_toches_to_component(CycleSmart& cycle, int component_number, Array<interval>& interval_list);
   int _search_separated_component(CycleSmart& cycle, Array<interval>& interval_list);
   void _search_path(int start, int finish, Array<int>& path, int component_number);
   void _assignEveryCycle(const CycleSmart &cycle);
   void _assignFirstCycle(const CycleSmart &cycle);

   // smoothing
   void _segment_smoothing(const CycleSmart &cycle, const MoleculeLayoutMacrocyclesLattice &layout, Array<int> &rotation_vertex, Array<Vec2f> &rotation_point, ObjArray<MoleculeLayoutSmoothingSegment> &segment);
   void _update_touching_segments(Array<local_pair_ii >&, ObjArray<MoleculeLayoutSmoothingSegment> &);
   void _segment_smoothing_prepearing(const CycleSmart &cycle, Array<int> &rotation_vertex, Array<Vec2f> &rotation_point, ObjArray<MoleculeLayoutSmoothingSegment> &segment, MoleculeLayoutMacrocyclesLattice& layout);
   void _segment_calculate_target_angle(const MoleculeLayoutMacrocyclesLattice &layout, Array<int> &rotation_vertex, Array<float> &target_angle, ObjArray<MoleculeLayoutSmoothingSegment> &segment);
   void _segment_update_rotation_points(const CycleSmart &cycle, Array<int> &rotation_vertex, Array<Vec2f> &rotation_point, ObjArray<MoleculeLayoutSmoothingSegment> &segment);
   void _segment_smoothing_unstick(ObjArray<MoleculeLayoutSmoothingSegment> &segment);
   void _do_segment_smoothing(Array<Vec2f> &rotation_point, Array<float> &target_angle, ObjArray<MoleculeLayoutSmoothingSegment> &segment);
   void _segment_improoving(Array<Vec2f> &rotation_point, Array<float> &target_angle, ObjArray<MoleculeLayoutSmoothingSegment> &segment, int, float, Array<local_pair_ii>&);
   void _do_segment_smoothing_gradient(Array<Vec2f> &rotation_point, Array<float> &target_angle, ObjArray<MoleculeLayoutSmoothingSegment> &segment);
   bool _gradient_step(Array<Vec2f> &point, Array<float> &target_angle, ObjArray<MoleculeLayoutSmoothingSegment> &segment, float coef, Array<local_pair_ii>& touching_segments);
   Vec2f _get_len_derivative(Vec2f current_vector, float target_dist);
   Vec2f _get_len_derivative_simple(Vec2f current_vector, float target_dist);
   Vec2f _get_angle_derivative(Vec2f left_point, Vec2f right_point, float target_angle);

   void _attachCrossingEdges ();
   void _attachDandlingVertices (int vert_idx, Array<int> &adjacent_list);
   void _calculatePositionsOneNotDrawn (Array<Vec2f> &positions, int n_pos, int vert_idx, int not_drawn_idx);
   void _calculatePositionsSingleDrawn (int vert_idx, Array<int> &adjacent_list, int &n_pos, int drawn_idx, bool &two_ears, Array<Vec2f> &positions, int &parity);
   void _orderByEnergy (Array<Vec2f> &positions);

   void _attachEars (int vert_idx, int drawn_idx, int *ears, const Vec2f &rest_pos);
   void _buildOutline (void);

   // attaching cycles
   bool _attachCycleOutside (const CycleSmart &cycle, float length, int n_common);
   bool _drawEdgesWithoutIntersection (const CycleSmart &cycle, Array<int> & cycle_vertex_types);

   bool _checkBadTryBorderIntersection (Array<int> &chain_ext, MoleculeLayoutGraphSmart &next_bc, Array<int> &mapping);
   bool _checkBadTryChainOutside (Array<int> &chain_ext, MoleculeLayoutGraphSmart &next_bc, Array<int> & mapping);

   bool _attachCycleInside (const CycleSmart &cycle, float length);
   bool _attachCycleWithIntersections (const CycleSmart &cycle, float length);
   void _setChainType (const Array<int> &chain, const Array<int> &mapping, int type);
   bool _splitCycle (const CycleSmart &cycle, const Array<int> &cycle_vertex_types, bool check_boundary,
      Array<int> &chain_ext, Array<int> &chain_int, int &c_beg, int &c_end) const;
   void _splitCycle2 (const CycleSmart &cycle, const Array<int> &cycle_vertex_types, ObjArray < Array<int> > &chains_ext) const;

   // border functions
   void _getBorder (CycleSmart &border) const;
   void _getSurroundCycle (CycleSmart &cycle, Vec2f p) const;
   void _splitBorder (int v1, int v2, Array<int> &part1v, Array<int> &part1e, Array<int> &part2v, Array<int> &part2e) const;
   bool _isPointOutside (const Vec2f &p) const;
   bool _isPointOutsideCycle   (const CycleSmart &cycle, const Vec2f &p) const;
   bool _isPointOutsideCycleEx (const CycleSmart &cycle, const Vec2f &p, const Array<int> &mapping) const;

   // geometry functions
   int _calcIntersection     (int edge1, int edge2) const;
   bool _isVertexOnEdge      (int vert_idx, int edge_beg, int edge_end) const;
   bool _isVertexOnSomeEdge  (int vert_idx) const;
   void _shiftEdge           (int edge_idx, float delta);
   bool _drawRegularCurve    (const Array<int> &chain, int begin, int end, float length, bool ccw, int type);
   bool _drawRegularCurveEx  (const Array<int> &chain, int begin, int end, float length, bool ccw, int type, const Array<int> &mapping);
   static void  _findAngles  (int k, float s, float &x, float &y);
   static float _dichotomy1  (float a0, float b0, int L, float s);
   static float _dichotomy2  (float a0, float b0, int L, float s);
   static void _calculatePos (float phi, const Vec2f &v1, const Vec2f &v2, Vec2f &v);
   const float _energyOfPoint (Vec2f p) const;
   int _isCisConfiguratuin (Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4);

   static bool _border_cb (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);
   static bool _edge_check(Graph &graph, int e_idx, void *context);
   static bool _edge_check_norm(Graph &graph, int e_idx, void *context);

   // make tree of biconnected components (tree[i] - -1 or component incoming to vertex i)
   static void _makeComponentsTree (BiconnectedDecomposer &decon,
      ObjArray<MoleculeLayoutGraphSmart> &components, Array<int> &tree);

   void _layoutMultipleComponents (BaseMolecule & molecule, bool respect_existing, const Filter * filter, float bond_length);
   void _layoutSingleComponent (BaseMolecule &molecule, bool respect_existing, const Filter * filter, float bond_length);

   ObjArray<LayoutVertexSmart> _layout_vertices;
   ObjArray<LayoutEdgeSmart>   _layout_edges;

   Array<int> _fixed_vertices;
   Array<int> _layout_component_number; // number of layout component of certain edge

   int _layout_component_count;
   long _total_morgan_code;
   int  _first_vertex_idx;
   int _n_fixed;

   // Outline of the graph (from pattern)
   Obj< Array<Vec2f> > _outline;

   BaseMolecule *_molecule;
   MoleculeLayoutGraphSmart *_graph;
   const int *_molecule_edge_mapping;
   
private:
   MoleculeLayoutGraphSmart (const MoleculeLayoutGraphSmart&);
};
   


}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif