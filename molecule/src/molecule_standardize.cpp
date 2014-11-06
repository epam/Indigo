/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#include "molecule/molecule_standardize.h"
#include "molecule/molecule_standardize_options.h"

#include "molecule/molecule.h"


using namespace indigo;
CP_DEF(MoleculeStandardizer);
MoleculeStandardizer::MoleculeStandardizer():
CP_INIT{
}

bool MoleculeStandardizer::standardize (Molecule &mol, const StandardizeOptions &options)
{
   if (options.remove_single_atom_fragments)
   {
      _removeSingleAtomFragments(mol);
   }
   return true;
}

void MoleculeStandardizer::_stadardizeStereo (Molecule &mol)
{
}

void MoleculeStandardizer::_stadardizeCharges (Molecule &mol)
{
}

void MoleculeStandardizer::_centerMolecule (Molecule &mol)
{
}

void MoleculeStandardizer::_removeSingleAtomFragments (Molecule &mol)
{
}
