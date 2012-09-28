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

#ifndef __molecule_stereocenters__
#define __molecule_stereocenters__

#include "base_cpp/red_black.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;
class Filter;

class DLLEXPORT MoleculeStereocenters
{
public:
   
   enum
   {
      ATOM_ANY = 1,
      ATOM_AND = 2,
      ATOM_OR  = 3,
      ATOM_ABS = 4
   };

   explicit MoleculeStereocenters ();

   void clear ();

   void buildFromBonds (bool ignore_errors, int *sensible_bonds_out);

   void buildFrom3dCoordinates ( void );

   void markBonds ();
   void markBond  (int atom_idx);

   // takes mapping from supermolecule to submolecule
   void buildOnSubmolecule (const MoleculeStereocenters &super, int *mapping);

   void removeAtoms (const Array<int> &indices);
   void removeBonds (const Array<int> &indices);

   int size () const;

   void add (int atom_idx, int type, int group, bool inverse_pyramid);
   void add (int atom_idx, int type, int group, const int pyramid[4]);
   void get (int i, int &atom_idx, int &type, int &group, int *pyramid) const;
   void remove (int idx);

   bool exists (int atom_idx) const;
   void get (int atom_idx, int &type, int &group, int *pyramid) const;

   int getType  (int idx) const;
   int getGroup (int idx) const;
   const int * getPyramid (int idx) const;
   int * getPyramid (int idx);
   void setType (int idx, int type, int group);
   void invertPyramid (int idx);

   bool sameGroup (int idx1, int idx2);

   void getAbsAtoms  (Array<int> &indices);
   void getOrGroups  (Array<int> &numbers);
   void getAndGroups (Array<int> &numbers);
   void getOrGroup   (int number, Array<int> &indices);
   void getAndGroup  (int number, Array<int> &indices);

   bool haveAllAbs ();
   bool haveAllAbsAny ();
   bool haveAllAndAny ();

   void registerUnfoldedHydrogen (int atom_idx, int added_hydrogen);

   void flipBond (int atom_parent, int atom_from, int atom_to);

   int begin () const;
   int end () const;
   int next (int i) const;

   int getAtomIndex (int i) const;

   static bool checkSub (const MoleculeStereocenters &query, const MoleculeStereocenters &target, 
      const int *mapping, bool reset_h_isotopes, Filter *stereocenters_vertex_filter = 0);

   static bool isPyramidMappingRigid (const int *pyramid, int size, const int *mapping);
   static bool isPyramidMappingRigid (const int mapping[4]);
   static bool isPyramidMappingRigid_Sort (int *pyramid, const int *mapping);

   static void moveImplicitHydrogenToEnd (int pyramid[4]);
   static void moveMinimalToEnd (int pyramid[4]);
   static void moveElementToEnd (int pyramid[4], int element);

   static bool isAutomorphism (BaseMolecule &mol, const Array<int> &mapping, const Filter *filter = NULL);

   DECL_ERROR;

   static void getPyramidMapping (const MoleculeStereocenters &query,
                                  const MoleculeStereocenters &target,
                                  int query_atom, const int *mapping, int *mapping_out, bool reset_h_isotopes);

   bool isPossibleStereocenter (int atom_idx, bool *possible_implicit_h = 0, bool *possible_lone_pair = 0);

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

   static int _sign  (const Vec3f &v1, const Vec3f &v2, const Vec3f &v3);
   static int _xyzzy (const Vec3f &v1, const Vec3f &v2, const Vec3f &u);
   static int _onPlane (const Vec3f &v1, const Vec3f &v2, const Vec3f &v3, const Vec3f &v4);

   void _buildOneCenter (int atom_idx, int *sensible_bonds_out);

   void _getGroups (int type, Array<int> &numbers);
   void _getGroup  (int type, int number, Array<int> &indices);
   void _restorePyramid (int idx, int pyramid[4], int invert_pyramid);

   static void _rotatePyramid (int *pyramid);

   static void _convertAtomToImplicitHydrogen (int pyramid[4], int atom_to_remove);

   void _removeBondDir (int atom_from, int atom_to);

   BaseMolecule & _getMolecule() const;

private:
   MoleculeStereocenters (const MoleculeStereocenters &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
