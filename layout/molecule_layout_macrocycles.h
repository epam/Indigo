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

   void setVertexOutsideWeight (int v, int weight);
   void setVertexEdgeParallel (int v, bool parallel);
   void setEdgeStereo (int e, int stereo);
   void setVertexDrawn(int v, bool drawn);

   int getVertexStereo (int v);

   Vec2f &getPos (int v);

   void doLayout ();

// private:
public:
   static bool canApply (BaseMolecule &mol);

   double layout (BaseMolecule &mol);

   void smoothing(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, Vec2f *p, bool profi, int *able_to_move);
   double badness(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, Vec2f *p);
   double depictionMacrocycleMol(bool profi);
   double depictionCircle();

   DECL_ERROR;

private:
   int length;
   static const int max_size = 100;

   struct Data 
   {
      signed short minRotates[max_size][max_size][2][max_size][max_size];
   };

   TL_CP_DECL(Data, data);
   TL_CP_DECL(Array<int>, _vertex_weight);
   TL_CP_DECL(Array<int>, _vertex_stereo);
   TL_CP_DECL(Array<int>, _edge_stereo);
   TL_CP_DECL(Array<bool>, _vertex_drawn);
   TL_CP_DECL(Array<Vec2f>, _positions);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif 