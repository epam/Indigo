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

#ifndef __query_molecule_h__
#define __query_molecule_h__

#include "molecule/base_molecule.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/ptr_array.h"
#include "molecule/molecule_rgroups.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_arom.h"

namespace indigo {

enum
{
   SKIP_3D_CONSTRAINTS = 0x0100,
   SKIP_FIXED_ATOMS = 0x0200,
   SKIP_RGROUP_FRAGMENTS = 0x0400,
   SKIP_RGROUPS = 0x0800,
   SKIP_AROMATICITY = 0x1000,
};

class Output;

class QueryMolecule : public BaseMolecule
{
public:

   enum OpType
   {
      OP_NONE, // used on totally unconstrained atoms
      OP_AND,
      OP_OR,
      OP_NOT,

      ATOM_NUMBER,
      ATOM_PSEUDO,
      ATOM_RSITE,
      ATOM_CHARGE,
      ATOM_ISOTOPE,
      ATOM_RADICAL,
      ATOM_VALENCE,
      //ATOM_DEGREE,
      ATOM_CONNECTIVITY,
      ATOM_TOTAL_BOND_ORDER,
      ATOM_TOTAL_H,
      //ATOM_IMPLICIT_H,
      ATOM_SUBSTITUENTS,
      ATOM_SSSR_RINGS,
      ATOM_SMALLEST_RING_SIZE,
      ATOM_RING_BONDS,
      ATOM_UNSATURATION,
      ATOM_FRAGMENT,
      ATOM_AROMATICITY,

      BOND_ORDER,
      BOND_TOPOLOGY
   };

   class Node
   {
   public:
      Node (int type_);
      virtual ~Node ();
      
      OpType type; // OP_*** or ATOM_*** or BOND_***

      // type is OP_NOT: one child
      // type is OP_AND or OP_OR: more that one child
      // otherwise: no children
      PtrArray<Node> children;

      // Check if node has any constraint of the specific type
      DLLEXPORT bool hasConstraint      (int what_type);

      // Check if there is no other constraint, except specified ones
      DLLEXPORT bool hasNoConstraintExcept (int what_type);
      DLLEXPORT bool hasNoConstraintExcept (int what_type1, int what_type2);

      // Remove all constraints of the given type
      DLLEXPORT void removeConstraints (int what_type);

      DLLEXPORT bool sureValue         (int what_type, int &value);
      DLLEXPORT bool sureValueInv      (int what_type, int &value);
      DLLEXPORT bool possibleValue     (int what_type, int what_value);
      DLLEXPORT bool possibleValueInv  (int what_type, int what_value);
      DLLEXPORT bool possibleValuePair (int what_type1, int what_value1,
                              int what_type2, int what_value2);
      DLLEXPORT bool possibleValuePairInv (int what_type1, int what_value1,
                                 int what_type2, int what_value2);

      DLLEXPORT bool sureValueBelongs    (int what_type, const int *arr, int count);
      DLLEXPORT bool sureValueBelongsInv (int what_type, const int *arr, int count);

   protected:
      // "neu" means "new" in German. This should have been a static
      // method, but static methods can not be virtual, and so it is not static.
      virtual Node * _neu () = 0;

      static Node * _und (Node *node1, Node *node2);
      static Node * _oder (Node *node1, Node *node2);
      static Node * _nicht (Node *node);

      virtual bool _possibleValue      (int what_type, int what_value) = 0;
      virtual bool _possibleValuePair  (int what_type1, int what_value1,
                                        int what_type2, int what_value2) = 0;
      
      virtual bool _sureValue        (int what_type, int &value_out) = 0;
      virtual bool _sureValueBelongs (int what_type, const int *arr, int count) = 0;
   };

   class Atom : public Node
   {
   public:
      DLLEXPORT Atom ();

      DLLEXPORT Atom (int type, int value);
      DLLEXPORT Atom (int type, int value_min, int value_max);
      DLLEXPORT Atom (int type, const char *value);
      DLLEXPORT Atom (int type, QueryMolecule *value);
      
      DLLEXPORT virtual ~Atom ();

      DLLEXPORT Atom * clone ();

      DLLEXPORT Atom * child (int idx);

      bool valueWithinRange (int value);

      bool hasConstraintWithValue (int what_type, int what_value);

      int value_min;
      int value_max;

      // available only when type is ATOM_PSEUDO
      Array<char> alias;

      // available only when type is ATOM_FRAGMENT
      AutoPtr<QueryMolecule> fragment;

      // when type is ATOM_RSITE, the value (value_min=valuemax)
      // are 32 bits, each allowing an r-group with corresponding number
      // to go for this atom. Simple 'R' atoms have this field equal to zero.

      // "und" means "and" in German. "and" is a C++ keyword.
      DLLEXPORT static Atom * und (Atom *atom1, Atom *atom2);

      // "oder" means "or" in German. "or" is a C++ keyword.
      DLLEXPORT static Atom * oder (Atom *atom1, Atom *atom2);

      // "nicht" means "not" in German. "not" is a C++ keyword.
      DLLEXPORT static Atom * nicht (Atom *atom);

   protected:

      virtual Node * _neu ();
      
      virtual bool _possibleValue      (int what_type, int what_value);
      virtual bool _possibleValuePair  (int what_type1, int what_value1,
                                        int what_type2, int what_value2);
      virtual bool _sureValue        (int what_type, int &value_out);
      virtual bool _sureValueBelongs (int what_type, const int *arr, int count);
      
      DEF_ERROR("query atom");
   };

   class Bond : public Node
   {
   public:
      Bond ();
      Bond (int type_, int value_);
      virtual ~Bond ();

      int value;

      Bond * clone ();

      Bond * child (int idx);

      // "und" means "and" in German. "and" is a C++ keyword.
      static Bond * und (Bond *node1, Bond *node2);

      // "oder" means "or" in German. "or" is a C++ keyword.
      static Bond * oder (Bond *node1, Bond *node2);

      // "nicht" means "not" in German. "not" is a C++ keyword.
      static Bond * nicht (Bond *node);

   protected:
      virtual Node * _neu ();

      virtual bool _possibleValue      (int what_type, int what_value);
      virtual bool _possibleValuePair  (int what_type1, int what_value1,
                                        int what_type2, int what_value2);
      virtual bool _sureValue        (int what_type, int &value_out);
      virtual bool _sureValueBelongs (int what_type, const int *arr, int count);
   };

   DLLEXPORT QueryMolecule ();
   DLLEXPORT virtual ~QueryMolecule ();

   DLLEXPORT virtual void clear ();

   DLLEXPORT virtual BaseMolecule * neu ();

   DLLEXPORT virtual QueryMolecule& asQueryMolecule ();
   DLLEXPORT virtual bool isQueryMolecule ();

   virtual int getAtomNumber  (int idx);
   virtual int getAtomCharge  (int idx); 
   virtual int getAtomIsotope (int idx);
   virtual int getAtomRadical (int idx);
   virtual int getExplicitValence (int idx);
   virtual int getAtomAromaticity (int idx);
   virtual int getAtomValence        (int idx);
   virtual int getAtomSubstCount     (int idx);
   virtual int getAtomRingBondsCount (int idx);

   virtual int getAtomMaxH   (int idx);
   virtual int getAtomMinH   (int idx);
   virtual int getAtomTotalH (int idx);

   virtual bool isPseudoAtom (int idx);
   virtual const char * getPseudoAtom (int idx);

   virtual bool isRSite (int atom_idx);
   virtual int  getRSiteBits (int atom_idx);
   virtual void allowRGroupOnRSite (int atom_idx, int rg_idx);

   virtual bool isSaturatedAtom (int idx);

   virtual int  getBondOrder      (int idx);
   virtual int  getBondTopology   (int idx);
   virtual bool atomNumberBelongs (int idx, const int *numbers, int count);
   virtual bool possibleAtomNumber (int idx, int number);
   virtual bool possibleAtomNumberAndCharge (int idx, int number, int charge);
   virtual bool possibleAtomNumberAndIsotope (int idx, int number, int isotope);
   virtual bool possibleAtomIsotope (int idx, int number);
   virtual bool possibleAtomCharge  (int idx, int charge);
   virtual bool possibleAtomRadical (int idx, int radical);
   virtual void getAtomDescription  (int idx, Array<char> &description);
   virtual void getBondDescription  (int idx, Array<char> &description);
   virtual bool possibleBondOrder   (int idx, int order);

   enum QUERY_ATOM {QUERY_ATOM_A, QUERY_ATOM_X, QUERY_ATOM_Q, QUERY_ATOM_LIST, QUERY_ATOM_NOTLIST};
   enum QUERY_BOND {QUERY_BOND_DOUBLE_OR_AROMATIC = 0, QUERY_BOND_SINGLE_OR_AROMATIC, QUERY_BOND_SINGLE_OR_DOUBLE, QUERY_BOND_ANY};
   DLLEXPORT static bool isKnownAttr (QueryMolecule::Atom& qa);
   DLLEXPORT static bool isNotAtom (QueryMolecule::Atom& qa, int elem);
   DLLEXPORT static QueryMolecule::Atom* stripKnownAttrs (QueryMolecule::Atom& qa);
   DLLEXPORT static bool collectAtomList (Atom& qa, Array<int>& list, bool& notList);
   DLLEXPORT static int parseQueryAtom (QueryMolecule& qm, int aid, Array<int>& list);
   DLLEXPORT static Bond* getBondOrderTerm (Bond& qb, bool& complex);
   DLLEXPORT static bool isOrBond (Bond& qb, int type1, int type2);
   DLLEXPORT static bool isSingleOrDouble (Bond& qb);
   DLLEXPORT static int getQueryBondType (Bond& qb);

   DLLEXPORT virtual bool bondStereoCare (int idx);
   DLLEXPORT void setBondStereoCare (int idx, bool stereo_care);

   DLLEXPORT virtual void aromatize ();
   DLLEXPORT virtual void dearomatize ();

   DLLEXPORT int addAtom (Atom *atom);
   DLLEXPORT Atom & getAtom (int idx);
   DLLEXPORT Atom * releaseAtom (int idx);
   DLLEXPORT void   resetAtom (int idx, Atom *atom);

   DLLEXPORT Bond & getBond (int idx);
   DLLEXPORT Bond * releaseBond (int idx);
   DLLEXPORT void   resetBond (int idx, Bond *bond);
   DLLEXPORT int addBond (int beg, int end, Bond *bond);

   MoleculeRGroups rgroups;

   Molecule3dConstraints spatial_constraints;
   Array<int> fixed_atoms;
   
   DLLEXPORT bool isRGroupFragment ();
   DLLEXPORT void createRGroupFragment ();
   DLLEXPORT MoleculeRGroupFragment & getRGroupFragment ();

   QueryMoleculeAromaticity aromaticity;

   Array<char> fragment_smarts;

   // for component-level grouping of SMARTS
   // components[i] = 0 means nothing;
   // components[i] = components[j] > 0 means that i-th and j-th vertices
   // must belong to the same connected component of the target molecule;
   // components[i] != components[j] > 0 means that i-th and j-th vertices
   // must belong to different connected components of the target molecule
   Array<int> components;

protected:

   int _calcAtomConnectivity (int idx);
   void _getAtomDescription (Atom *atom, Output &out);
   void _getBondDescription (Bond *bond, Output &out);
   int _getAtomMinH (Atom *atom);
   
   virtual void _flipBond (int atom_parent, int atom_from, int atom_to);
   virtual void _mergeWithSubmolecule (BaseMolecule &bmol, const Array<int> &vertices,
                                       const Array<int> *edges, const Array<int> &mapping,
                                       int skip_flags);
   virtual void _postMergeWithSubmolecule (BaseMolecule &bmol, const Array<int> &vertices,
                                       const Array<int> *edges, const Array<int> &mapping,
                                       int skip_flags);
   virtual void _removeAtoms (const Array<int> &indices, const int *mapping);
   virtual void _removeBonds (const Array<int> &indices);

   Array<int> _min_h;

   Array<bool> _bond_stereo_care;

   PtrArray<Atom> _atoms;
   PtrArray<Bond> _bonds;

   AutoPtr<MoleculeRGroupFragment> _rgroup_fragment;
};

}

#endif