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

#ifndef __refinement_state_h__
#define __refinement_state_h__

#include "layout/molecule_layout_graph.h"

namespace indigo {

struct RefinementState  
{
   explicit RefinementState (MoleculeLayoutGraph &graph);

   void calcHeight ();
   void calcDistance (int v1, int v2);
   void calcEnergy ();

   void copy (const RefinementState &other);//existing states
   void copyFromGraph ();
   void applyToGraph ();

   void flipBranch    (const Filter &branch, const RefinementState &state, int v1_idx, int v2_idx);
   void rotateBranch  (const Filter &branch, const RefinementState &state, int v_idx, float angle);
   void stretchBranch (const Filter &branch, const RefinementState &state, int v1, int v2, int d);
   void rotateLayout  (const RefinementState &state, int v_idx, float angle);

   float dist;
   double energy;
   float height;
   TL_CP_DECL(Array<Vec2f>, layout);

   DECL_ERROR;
private:

   MoleculeLayoutGraph &_graph;
};

}

#endif
