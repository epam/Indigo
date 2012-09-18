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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class DLLEXPORT MoleculeLayoutMacrocycles
{
public:
   static bool canApply (BaseMolecule &mol);

   double layout (BaseMolecule &mol);

   void smoothing(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, double *x, double *y, bool profi);
   double badness(int ind, int molSize, int *rotateAngle, int *edgeLenght, int *vertexNumber, double *x, double *y);
   double depictionMacrocycleMol(BaseMolecule &mol, bool profi);

   DEF_ERROR("macrocycles");
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif 