/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __molecule_decomposer__
#define __molecule_decomposer__

#include "graph/graph_decomposer.h"

class BaseMolecule;

class MoleculeDecomposer : public GraphDecomposer
{
public:
   explicit MoleculeDecomposer (BaseMolecule &mol);
   virtual ~MoleculeDecomposer ();

   void buildComponentMolecule (int ncomp, BaseMolecule &comp_mol,
      Array<int> *mapping_out = 0, Array<int> *inv_mapping = 0);

protected:
   BaseMolecule &_mol;
};

#endif
