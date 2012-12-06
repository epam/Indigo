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

#include "molecule/molecule_savers.h"

#include "molecule/elements.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule.h"

using namespace indigo;

int MoleculeSavers::getHCount (BaseMolecule &mol, int index, int atom_number, int atom_charge)
{
   int hydrogens_count = -1;
   if (!mol.isRSite(index) && !mol.isPseudoAtom(index))
   {
      if (!mol.isQueryMolecule())
      {
         if (mol.getAtomAromaticity(index) == ATOM_AROMATIC &&
            ((atom_number != ELEM_C && atom_number != ELEM_O) || atom_charge != 0))
            hydrogens_count = mol.asMolecule().getImplicitH_NoThrow(index, -1);
      }
      else
      {
         QueryMolecule::Atom &atom = mol.asQueryMolecule().getAtom(index);

         if (!atom.sureValue(QueryMolecule::ATOM_TOTAL_H, hydrogens_count))
         {
            // Try to check if there are only one constraint
            QueryMolecule::Atom *constraint = atom.sureConstraint(QueryMolecule::ATOM_TOTAL_H);
            if (constraint != NULL)
               hydrogens_count = constraint->value_min;
            else
               hydrogens_count = -1;
         }
      }
   }
   return hydrogens_count;
}
