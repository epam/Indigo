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

#ifndef __molecule_cis_trans__
#define __molecule_cis_trans__

#include "base_cpp/red_black.h"
#include "math/algebra.h"

namespace indigo {

class BaseMolecule;
class Filter;

class MoleculeCisTrans
{
public:
   enum
   {
      CIS = 1,
      TRANS = 2
   };

   DLLEXPORT void clear ();
   DLLEXPORT void clear (BaseMolecule &mol);
   DLLEXPORT void build (BaseMolecule &mol, int *exclude_bonds);
   DLLEXPORT void buildFromSmiles (int *dirs);

   DLLEXPORT bool exists () const;

   DLLEXPORT int count ();

   DLLEXPORT void setParity (int bond_idx, int parity);
   DLLEXPORT int  getParity (int bond_idx) const;

   DLLEXPORT void registerBond (int idx);

   DLLEXPORT void flipBond (BaseMolecule &mol, int atom_parent, int atom_from, int atom_to);

   DLLEXPORT const int * getSubstituents (int bond_idx) const;
   DLLEXPORT void getSubstituents_All (BaseMolecule &mol, int bond_idx, int subst[4]) const;

   DLLEXPORT void add (int bond_idx, int substituents[4], int parity);

   DLLEXPORT int applyMapping (int idx, const int *mapping) const;
   DLLEXPORT static int applyMapping (int parity, const int *substituents, const int *mapping);

   DLLEXPORT static int getMappingParitySign (BaseMolecule &query, BaseMolecule &target,
                                    int bond_idx, const int *mapping);

   DLLEXPORT static bool checkSub (BaseMolecule &query, BaseMolecule &target, const int *mapping);

   DLLEXPORT void buildOnSubmolecule (BaseMolecule &super, BaseMolecule &sub, int *mapping);

   DLLEXPORT void restoreSubstituents (BaseMolecule &mol, int bond_idx);

   DLLEXPORT static bool isAutomorphism (BaseMolecule &mol, const Array<int> &mapping, const Filter *edge_filter = NULL);

   DEF_ERROR("cis-trans");

   DLLEXPORT static bool isGeomStereoBond (BaseMolecule &mol, int bond_idx, int *substituents, bool have_xyz);
   DLLEXPORT static int  sameside (const Vec3f &beg, const Vec3f &end, const Vec3f &nei_beg, const Vec3f &nei_end);

protected:

   BaseMolecule & _getMolecule ();
   
   struct _Bond
   {
      int parity; // CIS ot TRANS
      int substituents[4];
   };

   Array<_Bond> _bonds;

   static bool _pureH (BaseMolecule &mol, int idx);
   static int _sameside (BaseMolecule &mol, int i_beg, int i_end, int i_nei_beg, int i_nei_end);
   bool _sortSubstituents (BaseMolecule &mol, int *substituents);
};

}

#endif
