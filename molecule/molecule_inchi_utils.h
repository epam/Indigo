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

#ifndef __molecule_inchi_utils_h__
#define __molecule_inchi_utils_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo {

class Molecule;

// Utility class for InChI code creation
class MoleculeInChIUtils
{
public:
   // Helpful structure with mappings
   struct Mapping
   {
      Mapping (const Array<int> &_mapping, const Array<int> &_inv_mapping) 
           : mapping(_mapping), inv_mapping(_inv_mapping) {}

      const Array<int> &mapping, &inv_mapping;
   };

   // Returns indices for lexicographically-sorted atom labels
   // with exception that the first atom is Carbon
   static const Array<int>& getLexSortedAtomLables ();
   // Returns inverse permutation for getLexSortedLables
   static const Array<int>& getLexSortedAtomLablesRanks ();

   // Stable sort for small integer arrays with possibility to use array with ranks
   // Note: it is better to add stable sort method in Array and
   // modify qsort (and other stable sort) to accept any comparators.
   static void stableSmallSort (Array<int> &indices, const Array<int> *ranks);

   // Compare atoms with hydrogens: C < CH4 < CH3 < CH2 < CH
   static int compareHydrogens (int hyd1, int hyd2);

   // Get parity according to InChI standart
   static int getParityInChI (Molecule &mol, int bond);

   DECL_ERROR;
private:
   static void _ensureLabelsInitialized ();
   static void _initializeAtomLabels ();

   static int  _compareAtomLabels (int &label1, int &label2, void *context);

   static Array<int> _atom_lables_sorted;
   static Array<int> _atom_lables_ranks;
   
};

}

#endif // __molecule_inchi_utils_h__
