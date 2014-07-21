/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#ifndef __molecule_layout_macrocycles_h__
#define __molecule_layout_macrocycles_h__

#include "molecule/molecule.h"
#include "layout/molecule_layout_graph.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

using namespace std;
namespace indigo {

class DLLEXPORT MoleculeLayoutMacrocycles
{
public:
   CP_DECL;
   MoleculeLayoutMacrocycles (int size);

   void addVertexOutsideWeight (int v, int weight);
   void setVertexEdgeParallel (int v, bool parallel);
   void set_vertex_added_square(int v, double s);
   void setEdgeStereo (int e, int stereo);
   void setVertexDrawn(int v, bool drawn);
   void set_component_finish(int v, int f);
   void set_target_angle(int v, double angle);
   void set_angle_importance(int, double);

   int getVertexStereo (int v);

   Vec2f &getPos (int v) const;

   void doLayout ();

// private:
public:
   static bool canApply (BaseMolecule &mol);

   double layout (BaseMolecule &mol);

   void smoothing(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, Vec2f *p, bool profi);
   void smoothing2(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, Vec2f *p);
   void improvement2(int i, int vertex_count, int cycle_size, int *rotate_angle, int *edge_lenght, int *vertex_number, Vec2f *p, int base_vertex, bool fix_angle, bool fix_next, double multiplyer);
   double badness(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, Vec2f *p, int diff);
   double depictionMacrocycleGreed(bool profi);
   double depictionCircle();

   DECL_ERROR;

private:
   int length;
   static const int max_size;
   static const int init_x;
   static const int init_y;
   static const int init_rot;
   static const double CHANGE_FACTOR;
   static const int WEIGHT_FACTOR = 12;


   int get_diff_grid(int x, int y, int rot, int value);
   int get_diff_circle(int x, int y, int rot, int value);

   struct Data
   {
      enum { max_size = 105 };
      unsigned short minRotates[max_size][max_size][2][max_size][max_size];
   };

   TL_CP_DECL(Data, data);
   TL_CP_DECL(Array<int>, _vertex_weight);
   TL_CP_DECL(Array<int>, _vertex_stereo);
   TL_CP_DECL(Array<double>, _vertex_added_square);
   TL_CP_DECL(Array<int>, _edge_stereo);
   TL_CP_DECL(Array<bool>, _vertex_drawn);
   TL_CP_DECL(Array<Vec2f>, _positions);
   TL_CP_DECL(Array<int>, _component_finish);
   TL_CP_DECL(Array<double>, _target_angle);
   TL_CP_DECL(Array<double>, _angle_importance);

};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif 