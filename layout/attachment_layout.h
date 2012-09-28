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

#ifndef __attachment_layout_h__
#define __attachment_layout_h__

#include "layout/molecule_layout_graph.h"
#include "graph/biconnected_decomposer.h"

namespace indigo {

class AttachmentLayout
{
public:
   explicit AttachmentLayout (const BiconnectedDecomposer &bc_decom,
      const ObjArray<MoleculeLayoutGraph> &bc_components, 
      const Array<int> &bc_tree, MoleculeLayoutGraph &graph, int src_vertex);

   double calculateEnergy ();
   void  applyLayout ();
   void  markDrawnVertices ();

public:   
   int _src_vertex;
   TL_CP_DECL(Array<int>, _src_vertex_map); // _src_vertex_map[j] - index of the vertex _src_vertex in j component
   TL_CP_DECL(Array<int>, _attached_bc);   // BCnumbers[j] - index of j component attached;
                                           // BCnumbers[size-1] - drawn
   TL_CP_DECL(Array<float>, _bc_angles);   // BCangles[j] - internal angle of j component attached, 0 if single edge
   TL_CP_DECL(Array<int>, _vertices_l);    // _vertices_l[j] - index of the vertex in j component such the j component
                                           // lays on the left (CCW) from edge (v, _vertices_l[j]];
   float _alpha;                           // if positive then angle between components
   TL_CP_DECL(Array<int>, _new_vertices);  // indices in source graph of new verices
   TL_CP_DECL(Array<Vec2f>, _layout);      // layout of new vertices
   double _energy;                         // current energy between drawn part and new part

   const ObjArray<MoleculeLayoutGraph> &_bc_components;
   MoleculeLayoutGraph &_graph;
};

class LayoutChooser
{
public:
   LayoutChooser(AttachmentLayout &layout);

   void perform () { _perform(_layout._attached_bc.size() - 1); }

private:
   void _perform (int level);
   void _makeLayout ();

   int _n_components;
   double _cur_energy;
   TL_CP_DECL(Array<int>, _comp_permutation);
   TL_CP_DECL(Array<int>, _rest_numbers);
   AttachmentLayout &_layout;
};

}

#endif
