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

#ifndef __molecule_pi_systems_matcher__
#define __molecule_pi_systems_matcher__

#include "base_cpp/array.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/exception.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_electrons_localizer.h"
#include "graph/graph_decomposer.h"

namespace indigo {
class Molecule;

class MoleculePiSystemsMatcher
{
public:
   MoleculePiSystemsMatcher (Molecule &target);

   bool isAtomInPiSystem (int atom);
   bool isBondInPiSystem (int bond);

   bool checkEmbedding (QueryMolecule &query, const int *mapping);

   void copyLocalization (Molecule &target);

   DECL_ERROR;
private:
   // Returns number of pi-systems
   int _initMarks (void);

   void _markAtomsFirst ();
   void _markUnstablePiSystems (Array<bool> &pi_system_used);

   void _markVerticesInPiSystemsWithCycles ();
   void _markVerticesInUnusedPiSystems (Array<bool> &pi_system_used);
   void _markVerticesInSingleAtomPiSystem (int n_pi_systems);

   void _calculatePiSystemsSizes (int n_pi_systems, Array<int> &sizes);

   void _copyPiSystemsIdFromDecomposer ();

   void _extractPiSystem (int pi_system_index);
   void _findPiSystemLocalization (int pool_id);

   bool _fixAtoms (QueryMolecule &query, const int *mapping);
   bool _fixBonds (QueryMolecule &query, const int *mapping);

   bool _findMatching ();
   bool _findMatchingForPiSystem (int pool_id);

   void _markMappedPiSystems (QueryMolecule &query, const int *mapping);

   bool _canAtomBeInPiSystem (int v);

   void _calcConnectivity (Molecule &mol, Array<int> &conn);

   enum { _NOT_IN_PI_SYSTEM = -3, _UNKNOWN = -2, _IN_AROMATIC  = -1 };

   Molecule &_target;
   Obj<GraphDecomposer> _decomposer;

   TL_CP_DECL(Array<int>, _atom_pi_system_idx);

   struct _Pi_System
   {
      Molecule pi_system;
      Array<int> inv_mapping, mapping;
      Obj<MoleculeElectronsLocalizer> localizer;

      struct Localizations
      {
         int double_bonds, primary_lp, seconary_lp;
      };
      Array<Localizations> localizations;

      bool pi_system_mapped;
      bool initialized;

      void clear ();
   };
   TL_CP_DECL(ReusableObjArray<_Pi_System>, _pi_systems);
   TL_CP_DECL(Array<int>, _connectivity);
};

}

#endif // __molecule_pi_systems_matcher__
