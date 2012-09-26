/****************************************************************************
 * Copyright (C) 2011 GGA Software Services LLC
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

#ifndef __molecule_allene_stereo__
#define __molecule_allene_stereo__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

#include "base_cpp/exception.h"
#include "base_cpp/red_black.h"

namespace indigo {

class BaseMolecule;

class DLLEXPORT MoleculeAlleneStereo
{
public:
   MoleculeAlleneStereo ();

   void clear ();

   void buildFromBonds (bool ignore_errors, int *sensible_bonds_out);
   void markBonds ();
   static int  sameside (const Vec3f &dir1, const Vec3f &dir2, const Vec3f &sep);
   void buildOnSubmolecule (MoleculeAlleneStereo &super, int *mapping);
   static bool checkSub (BaseMolecule &query, BaseMolecule &target, const int *mapping);

   static bool possibleCenter (BaseMolecule &mol, int idx, int &left, int &right, int subst[4], bool pure_h[4]);

   bool isCenter (int atom_idx);
   int  size ();
   int begin () const;
   int end () const;
   int next (int i) const;
   void get (int i, int &atom_idx, int &left, int &right, int subst[4], int &parity);
   void getByAtomIdx (int atom_idx, int &left, int &right, int subst[4], int &parity);
   void invert (int atom_idx);
   void reset (int atom_idx);

   void add (int atom_idx, int left, int right, int subst[4], int parity);

   void removeAtoms (const Array<int> &indices);
   void removeBonds (const Array<int> &indices);
   void registerUnfoldedHydrogen (int atom_idx, int added_hydrogen);

   DECL_ERROR;


protected:
   struct _Atom
   {
      int left;     // number of the "left" neighbor atom
      int right;    // number of the "right" neighbor atom

      // substituens: [0] and [1] are connected to the "left" neighbor,
      //              [2] and [3] are connected to the "right" neighbor.
      //              [1] and [3] may be -1 (implicit H)
      //              [0] and [2] are never -1
      int subst[4];

      // parity = 1  if [2]-nd substituent is rotated CCW w.r.t. [0]-th
      //             substituent when we look at it from "left" to "right"
      // parity = 2  if it is rotated CW
      int parity;
   };


   BaseMolecule & _getMolecule();
   bool _isAlleneCenter (BaseMolecule &mol, int idx, _Atom &atom, int *sensible_bonds_out);

   RedBlackMap<int, _Atom> _centers;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
