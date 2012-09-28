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

#ifndef __molecule_cis_trans__
#define __molecule_cis_trans__

#include "base_cpp/red_black.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;
class Filter;

class DLLEXPORT MoleculeCisTrans
{
public:
   enum
   {
      CIS = 1,
      TRANS = 2
   };

   void clear ();
   void build (int *exclude_bonds);
   void buildFromSmiles (int *dirs);

   bool exists () const;

   int count ();

   void setParity (int bond_idx, int parity);
   int  getParity (int bond_idx) const;
   bool isIgnored (int bond_idx) const;
   void ignore (int bond_idx);

   void registerBond (int idx);

   void flipBond (int atom_parent, int atom_from, int atom_to);

   const int * getSubstituents (int bond_idx) const;
   void getSubstituents_All (int bond_idx, int subst[4]);

   void add (int bond_idx, int substituents[4], int parity);
   bool registerBondAndSubstituents (int idx);

   int applyMapping (int idx, const int *mapping, bool sort) const;
   static int applyMapping (int parity, const int *substituents, const int *mapping, bool sort);

   // Returns -2 if mapping is not valid
   static int getMappingParitySign (BaseMolecule &query, BaseMolecule &target,
                                    int bond_idx, const int *mapping);

   static bool checkSub (BaseMolecule &query, BaseMolecule &target, const int *mapping);

   void buildOnSubmolecule (BaseMolecule &super, int *mapping);

   static bool sortSubstituents (BaseMolecule &mol, int *substituents, bool *parity_changed);

   void restoreSubstituents (int bond_idx);
   void registerUnfoldedHydrogen (int atom_idx, int added_hydrogen);

   static bool isAutomorphism (BaseMolecule &mol, const Array<int> &mapping, const Filter *edge_filter = NULL);

   bool isRingTransBond (int bond_idx);

   bool convertableToImplicitHydrogen (int idx);

   DECL_ERROR;

   static bool isGeomStereoBond (BaseMolecule &mol, int bond_idx, int *substituents, bool have_xyz);
   static int  sameside (const Vec3f &beg, const Vec3f &end, const Vec3f &nei_beg, const Vec3f &nei_end);
   static bool sameline (const Vec3f &beg, const Vec3f &end, const Vec3f &nei_beg);

protected:

   BaseMolecule & _getMolecule ();
   
   struct _Bond
   {
      void clear ()
      {
         parity = 0;
         ignored = 0;
      }

      int parity; // CIS ot TRANS
      int ignored; // explicitly ignored cis-trans configuration on this bond
      int substituents[4];
   };

   Array<_Bond> _bonds;

   static bool _pureH (BaseMolecule &mol, int idx);
   static int _sameside (BaseMolecule &mol, int i_beg, int i_end, int i_nei_beg, int i_nei_end);
   static bool _sameline (BaseMolecule &molecule, int i_beg, int i_end, int i_nei_beg);

   static int _getPairParity (int v1, int v2, const int *mapping, bool sort);
   static bool _commonHasLonePair (BaseMolecule &mol, int v1, int v2);

   static void _fillExplicitHydrogens (BaseMolecule &mol, int bond_idx, int subst[4]);
   static void _fillAtomExplicitHydrogens (BaseMolecule &mol, int atom_idx, int subst[2]);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
