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

#ifndef __molecule_stereocenters__
#define __molecule_stereocenters__

#include "base_cpp/red_black.h"
#include "math/algebra.h"

namespace indigo {

class BaseMolecule;
class Filter;

class MoleculeStereocenters
{
public:
   
   enum
   {
      ATOM_ANY = 1,
      ATOM_AND = 2,
      ATOM_OR  = 3,
      ATOM_ABS = 4
   };

   enum
   {
      BOND_UP = 1,
      BOND_EITHER = 2,
      BOND_DOWN = 3
   };

   explicit DLLEXPORT MoleculeStereocenters ();

   DLLEXPORT void clear ();

   DLLEXPORT void buildFromBonds (const int *atom_types, const int *atom_groups, const int *bond_types, bool ignore_errors);

   DLLEXPORT void buildFrom3dCoordinates ( void );

   DLLEXPORT void markBonds ();

   // takes mapping from supermolecule to submolecule
   DLLEXPORT void buildOnSubmolecule (const MoleculeStereocenters &super, int *mapping);

   DLLEXPORT void removeAtoms (const Array<int> &indices);
   DLLEXPORT void removeBonds (const Array<int> &indices);

   DLLEXPORT int size () const;

   DLLEXPORT void add (int atom_idx, int type, int group, bool inverse_pyramid);
   DLLEXPORT void add (int atom_idx, int type, int group, const int pyramid[4]);
   DLLEXPORT void get (int i, int &atom_idx, int &type, int &group, int *pyramid) const;
   DLLEXPORT void remove (int idx);

   DLLEXPORT bool exists (int atom_idx) const;
   DLLEXPORT void get (int atom_idx, int &type, int &group, int *pyramid) const;

   DLLEXPORT int getType  (int idx) const;
   DLLEXPORT int getGroup (int idx) const;
   DLLEXPORT const int * getPyramid (int idx) const;
   DLLEXPORT int * getPyramid (int idx);
   DLLEXPORT void setType (int idx, int type, int group);
   DLLEXPORT void inversePyramid (int idx);

   DLLEXPORT bool sameGroup (int idx1, int idx2);

   DLLEXPORT void getAbsAtoms  (Array<int> &indices);
   DLLEXPORT void getOrGroups  (Array<int> &numbers);
   DLLEXPORT void getAndGroups (Array<int> &numbers);
   DLLEXPORT void getOrGroup   (int number, Array<int> &indices);
   DLLEXPORT void getAndGroup  (int number, Array<int> &indices);

   DLLEXPORT bool haveAllAbs ();
   DLLEXPORT bool haveAllAbsAny ();
   DLLEXPORT bool haveAllAndAny ();
   DLLEXPORT int  getBondDirection (int idx) const;

   DLLEXPORT void registerUnfoldedHydrogen (int atom_idx, int added_hydrogen);

   DLLEXPORT void flipBond (int atom_parent, int atom_from, int atom_to);

   DLLEXPORT int begin () const;
   DLLEXPORT int end () const;
   DLLEXPORT int next (int i) const;

   DLLEXPORT int getAtomIndex (int i) const;

   DLLEXPORT static bool checkSub (const MoleculeStereocenters &query, const MoleculeStereocenters &target, 
      const int *mapping, bool reset_h_isotopes, Filter *stereocenters_vertex_filter = 0);

   DLLEXPORT static bool isPyramidMappingRigid (const int *pyramid, int size, const int *mapping);
   DLLEXPORT static bool isPyramidMappingRigid (const int mapping[4]);
   DLLEXPORT static bool isPyramidMappingRigid_Sort (int *pyramid, const int *mapping);

   DLLEXPORT static void moveImplicitHydrogenToEnd (int pyramid[4]);
   DLLEXPORT static void moveMinimalToEnd (int pyramid[4]);

   DLLEXPORT static bool isAutomorphism (BaseMolecule &mol, const Array<int> &mapping, const Filter *filter = NULL);

   DEF_ERROR("stereocenters");

   DLLEXPORT static void getPyramidMapping (const MoleculeStereocenters &query,
                                  const MoleculeStereocenters &target,
                                  int query_atom, const int *mapping, int *mapping_out, bool reset_h_isotopes);

protected:

   struct _Atom
   {
      int type;       // ANY, AND, OR, ABS
      int group;      // stereogroup index
      // [X, Y, Z, W] -- atom indices or -1 for implicit hydrogen
      // (X, Y, Z) go counterclockwise when looking from W.
      // if there are pure (implicit) hydrogen, it is W
      int pyramid[4];
   };

   struct _EdgeIndVec
   {
      int edge_idx;
      int nei_idx;
      int rank;
      Vec3f vec;
   };

   struct _Configuration
   {
      int elem;
      int charge;
      int degree;
      int n_double_bonds;
      int implicit_degree;
   };

   RedBlackMap<int, _Atom> _stereocenters;
   Array<int>              _bond_directions;

   int _getBondStereo (int center_idx, int nei_idx) const;

   static int _sign  (const Vec3f &v1, const Vec3f &v2, const Vec3f &v3);
   static int _xyzzy (const Vec3f &v1, const Vec3f &v2, const Vec3f &u);
   static int _onPlane (const Vec3f &v1, const Vec3f &v2, const Vec3f &v3, const Vec3f &v4);

   bool _isPossibleStereocenter (int atom_idx, bool *possible_implicit_h = 0, bool *possible_lone_pair = 0);
   void _buildOneCenter (int atom_idx, int group, int type, const int *bond_orientations);

   void _getGroups (int type, Array<int> &numbers);
   void _getGroup  (int type, int number, Array<int> &indices);
   void _restorePyramid (int idx, int pyramid[4], int invert_pyramid);

   static void _rotatePyramid (int *pyramid);

   static void _convertAtomToImplicitHydrogen (int pyramid[4], int atom_to_remove);

   void _markBonds_One  (int atom_idx);
   void _removeBondDir (int atom_from, int atom_to);

   BaseMolecule & _getMolecule() const;

private:
   MoleculeStereocenters (const MoleculeStereocenters &); // no implicit copy
};

}

#endif
