/****************************************************************************
 * Copyright (C) 2009-2014 GGA Software Services LLC
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

#ifndef __haworth_projection_finder__
#define __haworth_projection_finder__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/list.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;

class DLLEXPORT HaworthProjectionFinder
{
public:
   HaworthProjectionFinder (BaseMolecule &mol);

   void find ();
   void findAndAddStereocenters ();

   bool isBoldBond (int e_idx);

   const Array<bool>& getAtomsMask ();
   const Array<bool>& getBondsMask ();

private:
   void _find (bool add_stereo);
   bool _processRing (bool add_stereo, const Array<int> &vertices, const Array<int> &edges);

   void _markRingBonds (const Array<int> &vertices, const Array<int> &edges);
   void _addRingStereocenters (const Array<int> &vertices, const Array<int> &edges);

   bool _isCornerVertex (int v, int e1, int e2);
   bool _isHorizontalEdge (int e, float cos_threshold);
   bool _isVerticalEdge (int e, float cos_threshold);
   float _getAngleCos(int v, int e, float dx, float dy);
   float _getAngleCos(int v, int e1, int e2);
   float _getAngleSin(int v, int e1, int e2);

   BaseMolecule &_mol;
   CP_DECL;
   TL_CP_DECL(Array<bool>, _atoms_mask);
   TL_CP_DECL(Array<bool>, _bonds_mask);
   TL_CP_DECL(Array<bool>, _bold_bonds_mask);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __haworth_projection_finder__
