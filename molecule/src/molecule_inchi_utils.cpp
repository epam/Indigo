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

#include "molecule/molecule_inchi_utils.h"

#include "base_cpp/os_sync_wrapper.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cis_trans.h"

using namespace indigo;

//
// Sorted atoms lables
//
Array<int> MoleculeInChIUtils::_atom_lables_sorted;
Array<int> MoleculeInChIUtils::_atom_lables_ranks;

IMPL_ERROR(MoleculeInChIUtils, "InChI utility");

const Array<int>& MoleculeInChIUtils::getLexSortedAtomLables ()
{
   _ensureLabelsInitialized();
   return _atom_lables_sorted;
}

const Array<int>& MoleculeInChIUtils::getLexSortedAtomLablesRanks ()
{
   _ensureLabelsInitialized();
   return _atom_lables_ranks;
}

void MoleculeInChIUtils::_ensureLabelsInitialized ()
{
   // Double-checked locking
   if (_atom_lables_sorted.size() == 0)
   {
      static ThreadSafeStaticObj<OsLock> lock;
      OsLocker locker(lock.ref());

      if (_atom_lables_sorted.size() == 0)
         _initializeAtomLabels();
   }
}

void MoleculeInChIUtils::_initializeAtomLabels ()
{
   _atom_lables_sorted.reserve(ELEM_MAX);
   for (int i = ELEM_MIN; i < ELEM_MAX; i++)
      _atom_lables_sorted.push(i);
   _atom_lables_sorted.qsort(_compareAtomLabels, NULL);

   _atom_lables_ranks.resize(ELEM_MAX);
   _atom_lables_ranks.fffill();
   for (int i = 0; i < _atom_lables_sorted.size(); i++) 
   {
      int label = _atom_lables_sorted[i];
      _atom_lables_ranks[label] = i;
   }
}

int MoleculeInChIUtils::_compareAtomLabels (int &label1, int &label2, void *context)
{
   // Compare atom labels in alphabetic order with exception that 
   // atom C is the first atom and H as second atom
   if (label1 == ELEM_C && label2 != ELEM_C)
      return -1;
   if (label1 != ELEM_C && label2 == ELEM_C)
      return 1;

   return strcmp(Element::toString(label1), Element::toString(label2));
}

//
// Sorting
//

void MoleculeInChIUtils::stableSmallSort (Array<int> &indices, const Array<int> *ranks)
{
   // Stable insersion sort
   for (int i = 1; i < indices.size(); i++)
   {
      int i_value = indices[i];
      int i_value_rank = i_value;
      if (ranks != NULL)
         i_value_rank = ranks->at(i_value);

      int j = i - 1;
      while (j >= 0)
      {
         int j_value_rank = indices[j];
         if (ranks != NULL)
            j_value_rank = ranks->at(j_value_rank);

         if (i_value_rank >= j_value_rank)
            break;

         indices[j + 1] = indices[j];
         j--;
      }
      indices[j + 1] = i_value;
   }
}

//
// Other
//

int MoleculeInChIUtils::compareHydrogens (int hyd1, int hyd2)
{
   if (hyd1 == 0)
      hyd1 = 256;
   if (hyd2 == 0)
      hyd2 = 256;

   return hyd2 - hyd1;
}

int MoleculeInChIUtils::getParityInChI (Molecule &mol, int bond)
{
   if (mol.cis_trans.getParity(bond) == 0)
      throw Error("Specified bond ins't stereogenic");

   const Edge &edge = mol.getEdge(bond);
   
   const int *subst = mol.cis_trans.getSubstituents(bond);
   // Find substituents with maximal indices
   int max_first = __max(subst[0], subst[1]);
   int max_second = __max(subst[2], subst[3]);

   int value = MoleculeCisTrans::sameside(
      mol.getAtomXyz(edge.beg), mol.getAtomXyz(edge.end),
      mol.getAtomXyz(max_first), mol.getAtomXyz(max_second));
   if (value > 0)
      return -1;
   return 1;
}
