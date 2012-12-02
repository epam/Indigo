/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef __molecule_layout_graph_h__
#define __molecule_layout_graph_h__

#include "graph/graph.h"
#include "graph/filter.h"
#include "math/algebra.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/obj_array.h"
#include "molecule/molecule.h"
#include "layout/layout_pattern.h"
#include "base_cpp/obj.h"

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
   ELEMENT_IGNORE
};

struct LayoutVertex
{
   LayoutVertex () { memset(this, 0, sizeof(LayoutVertex)); }

   int  ext_idx;
   long morgan_code;
   bool is_cyclic;
   int  type;

   Vec2f pos;
};

struct LayoutEdge
{
   LayoutEdge () { memset(this, 0, sizeof(LayoutEdge)); }

   int  ext_idx;
   bool is_cyclic;
   int  type;
};

class DLLEXPORT MoleculeLayoutGraph : public Graph
{
public:
   explicit MoleculeLayoutGraph ();
   virtual ~MoleculeLayoutGraph ();

   virtual void clear ();

   bool isSingleEdge () const;

   inline const Vec2f & getPos (int idx) const { return _layout_vertices[idx].pos; }
   inline       Vec2f & getPos (int idx)       { return _layout_vertices[idx].pos; }
   inline int getVertexExtIdx (int idx) const { return _layout_vertices[idx].ext_idx; }
   inline int getVertexType (int idx) const { return _layout_vertices[idx].type; }
   inline int getEdgeExtIdx (int idx) const { return _layout_edges[idx].ext_idx; }
   inline int getEdgeType   (int idx) const { return _layout_edges[idx].type; }

   void registerLayoutVertex (int idx, const LayoutVertex &vertex);
   void registerLayoutEdge   (int idx, const LayoutEdge &edge);
   int  addLayoutVertex      (int ext_idx, int type);
   int  addLayoutEdge        (int beg, int end, int ext_idx, int type);

   const LayoutVertex &getLayoutVertex (int idx) const;
   const LayoutEdge   &getLayoutEdge   (int idx) const;

   void setVertexType (int idx, int type) { _layout_vertices[idx].type = type; }
   void setEdgeType   (int idx, int type) { _layout_edges[idx].type = type; }

   int findVertexByExtIdx (int ext_idx) const;

   float calculateAngle (int v, int &v1, int &v2) const;

   void makeOnGraph (Graph &graph);
   void makeLayoutSubgraph (MoleculeLayoutGraph &graph, Filter &filter);
   void cloneLayoutGraph (MoleculeLayoutGraph &other, Array<int> *mapping);
   void copyLayoutTo (MoleculeLayoutGraph &other, const Array<int> &mapping) const;

   void layout (BaseMolecule &molecule, float bond_length, const Filter *filter, bool respect_existing);
   
   const BaseMolecule *getMolecule (const int **molecule_edge_mapping) { *molecule_edge_mapping = _molecule_edge_mapping; return _molecule; }

   int max_iterations;
   
   void flipped () { _flipped = true; }
   bool isFlipped () const { return _flipped; }

#ifdef M_LAYOUT_DEBUG
   void saveDebug ();
#endif

   DECL_ERROR;

protected:

   struct Cycle
   {
      explicit Cycle();
      explicit Cycle(const List<int> &edges, const MoleculeLayoutGraph &graph);
      explicit Cycle(const Array<int> &vertices, const Array<int> &edges);

      void copy (const List<int> &edges, const MoleculeLayoutGraph &graph);
      void copy (const Array<int> &vertices, const Array<int> &edges);

      int vertexCount () const { return _vertices.size(); }
      int getVertex  (int idx) const { return _vertices[idx]; }
      int getVertexC (int idx) const { return _vertices[idx % vertexCount()]; }
      int getEdge    (int idx) const { return _edges[idx]; }
      int findVertex (int idx) const { return _vertices.find(idx); }
      long morganCode () const { return _morgan_code; }
      void canonize ();
      bool contains (const Cycle &another) const;
      void calcMorganCode (const MoleculeLayoutGraph &parent_graph);

      static int compare_cb (int &idx1, int &idx2, void *context);

   protected:

      TL_CP_DECL(Array<int>, _vertices);
      TL_CP_DECL(Array<int>, _edges);
      int _max_idx;
      long _morgan_code;
   };

   struct EnumContext
   {
      const MoleculeLayoutGraph *graph;
      RedBlackSet<int> *edges;
      int iterationNumber;
      int maxIterationNumber;
   };

   // patterns
   void _initPatterns ();
   static int _pattern_cmp  (PatternLayout &p1, PatternLayout &p2, void *context);
   static int _pattern_cmp2 (PatternLayout &p1, int n_v, int n_e, long code);
   static bool _match_pattern_bond (Graph &subgraph, Graph &supergraph, int self_idx, int other_idx, void *userdata);
   static int  _pattern_embedding (Graph &subgraph, Graph &supergraph, int *core_sub, int *core_super, void *userdata);

   static bool _path_handle (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);

   // for whole graph
   void _assignAbsoluteCoordinates (float bond_length);
   void _findFirstVertexIdx (int n_comp, Array<int> & fixed_components, ObjArray<MoleculeLayoutGraph> &bc_components, bool all_trivial);
   bool _prepareAssignedList (Array<int> &assigned_list, BiconnectedDecomposer &bc_decom, ObjArray<MoleculeLayoutGraph> &bc_components, Array<int> &bc_tree);
   void _assignFinalCoordinates (float bond_length, const Array<Vec2f> &src_layout);
   void _copyLayout (MoleculeLayoutGraph &component);
   void _getAnchor (int &v1, int &v2, int &v3) const;

   void _findFixedComponents (BiconnectedDecomposer &bc_decom, Array<int> &fixed_components, ObjArray<MoleculeLayoutGraph> &bc_components);
   bool _assignComponentsRelativeCoordinates (ObjArray<MoleculeLayoutGraph> &bc_components, Array<int> &fixed_components, BiconnectedDecomposer &bc_decom);

   // refine
   void _refineCoordinates (const BiconnectedDecomposer &bc_decomposer, const ObjArray<MoleculeLayoutGraph> &bc_components, const Array<int> &bc_tree);
   bool _allowRotateAroundVertex (int idx) const;
   void _makeBranches (Array<int> &branches, int edge, Filter &filter) const;
   void _findBranch (Array<int> &branches, int v, int edge) const;
   void _excludeDandlingIntersections ();

   // for components
   void _calcMorganCodes ();

   // assigning coordinates
   void _assignRelativeCoordinates (int &fixed_component, const MoleculeLayoutGraph &supergraph);
   bool _tryToFindPattern (int &fixed_component);
   void _assignRelativeSingleEdge (int &fixed_component, const MoleculeLayoutGraph &supergraph);
   void _assignFirstCycle(const Cycle &cycle);
   void _attachCrossingEdges ();
   void _attachDandlingVertices (int vert_idx, Array<int> &adjacent_list);
   void _calculatePositionsOneNotDrawn (Array<Vec2f> &positions, int n_pos, int vert_idx, int not_drawn_idx);
   void _calculatePositionsSingleDrawn (int vert_idx, Array<int> &adjacent_list, int &n_pos, int drawn_idx, bool &two_ears, Array<Vec2f> &positions, int &parity);
   void _orderByEnergy (Array<Vec2f> &positions);

   void _attachEars (int vert_idx, int drawn_idx, int *ears, const Vec2f &rest_pos);
   void _buildOutline (void);

   // attaching cycles
   bool _attachCycleOutside (const Cycle &cycle, float length, int n_common);
   bool _drawEdgesWithoutIntersection (const Cycle &cycle, Array<int> & cycle_vertex_types);

   bool _checkBadTryBorderIntersection (Array<int> &chain_ext, MoleculeLayoutGraph &next_bc, Array<int> &mapping);
   bool _checkBadTryChainOutside (Array<int> &chain_ext, MoleculeLayoutGraph &next_bc, Array<int> & mapping);

   bool _attachCycleInside (const Cycle &cycle, float length);
   bool _attachCycleWithIntersections (const Cycle &cycle, float length);
   void _setChainType (const Array<int> &chain, const Array<int> &mapping, int type);
   bool _splitCycle (const Cycle &cycle, const Array<int> &cycle_vertex_types, bool check_boundary,
      Array<int> &chain_ext, Array<int> &chain_int, int &c_beg, int &c_end) const;
   void _splitCycle2 (const Cycle &cycle, const Array<int> &cycle_vertex_types, ObjArray < Array<int> > &chains_ext) const;

   // border functions
   void _getBorder (Cycle &border) const;
   void _splitBorder (int v1, int v2, Array<int> &part1v, Array<int> &part1e, Array<int> &part2v, Array<int> &part2e) const;
   bool _isPointOutside (const Vec2f &p) const;
   bool _isPointOutsideCycle   (const Cycle &cycle, const Vec2f &p) const;
   bool _isPointOutsideCycleEx (const Cycle &cycle, const Vec2f &p, const Array<int> &mapping) const;

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

   static bool _border_cb (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);
   static bool _edge_check (Graph &graph, int e_idx, void *context);

   // make tree of biconnected components (tree[i] - component incoming to vertex i or -1)
   static void _makeComponentsTree (BiconnectedDecomposer &decon,
      ObjArray<MoleculeLayoutGraph> &components, Array<int> &tree);

   void _layoutMultipleComponents (BaseMolecule & molecule, bool respect_existing, const Filter * filter, float bond_length);
   void _layoutSingleComponent (BaseMolecule &molecule, bool respect_existing, const Filter * filter, float bond_length);

   ObjArray<LayoutVertex> _layout_vertices;
   ObjArray<LayoutEdge>   _layout_edges;

   Array<int> _fixed_vertices;

   long _total_morgan_code;
   int  _first_vertex_idx;
   int _n_fixed;

   // Outline of the graph (from pattern)
   Obj< Array<Vec2f> > _outline;

   BaseMolecule *_molecule;
   const int *_molecule_edge_mapping;
   
   bool _flipped; // component was flipped after attaching

   TL_DECL(ObjArray<PatternLayout>, _patterns);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
