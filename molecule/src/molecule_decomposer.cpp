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

#include "molecule/molecule_decomposer.h"
#include "molecule/base_molecule.h"
#include "graph/filter.h"

MoleculeDecomposer::MoleculeDecomposer (BaseMolecule &mol) :
GraphDecomposer(mol),
_mol(mol)
{
}

MoleculeDecomposer::~MoleculeDecomposer ()
{
}

void MoleculeDecomposer::buildComponentMolecule (int ncomp, BaseMolecule &comp_mol,
      Array<int> *mapping_out, Array<int> *inv_mapping)
{
   Filter filt(_component_ids.ptr(), Filter::EQ, ncomp);

   comp_mol.makeSubmolecule(_mol, filt, mapping_out, inv_mapping);
}
