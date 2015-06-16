/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __refinement_state_smart_h__
#define __refinement_state_smart_h__

#include "layout/molecule_layout_graph_smart.h"

namespace indigo {

struct RefinementStateSmart  
{
   explicit RefinementStateSmart (MoleculeLayoutGraphSmart &graph);

   void calcHeight ();
   void calcDistance (int v1, int v2);
   void calcEnergy ();

   void copy (const RefinementStateSmart &other);//existing states
   void copyFromGraph ();
   void applyToGraph ();

   void flipBranch    (const Filter &branch, const RefinementStateSmart &state, int v1_idx, int v2_idx);
   void rotateBranch  (const Filter &branch, const RefinementStateSmart &state, int v_idx, float angle);
   void stretchBranch (const Filter &branch, const RefinementStateSmart &state, int v1, int v2, int d);
   void rotateLayout  (const RefinementStateSmart &state, int v_idx, float angle);

   float dist;
   double energy;
   float height;
   CP_DECL;
   TL_CP_DECL(Array<Vec2f>, layout);

   DECL_ERROR;
private:

   MoleculeLayoutGraphSmart &_graph;
};

}

#endif
