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

#ifndef __molecule_h__
#define __molecule_h__

#include "molecule/base_molecule.h"

class Molecule : public BaseMolecule
{      
public:
   DLLEXPORT Molecule ();
   DLLEXPORT virtual ~Molecule ();

   DLLEXPORT virtual Molecule & asMolecule ();

   DLLEXPORT virtual void clear ();

   DLLEXPORT virtual BaseMolecule * neu ();

   DLLEXPORT int addAtom (int label);
   
   DLLEXPORT void setPseudoAtom (int idx, const char *text);

   DLLEXPORT int addBond (int beg, int end, int order);

   DLLEXPORT void setAtomCharge (int idx, int charge);
   DLLEXPORT void setAtomIsotope (int idx, int isotope);
   DLLEXPORT void setAtomRadical (int idx, int radical);
   DLLEXPORT void setValence (int idx, int valence);
   DLLEXPORT void setExplicitValence (int idx, int valence);
   DLLEXPORT void setImplicitH       (int idx, int impl_h); // in fact, this is 'explicit implicit H'
   DLLEXPORT void resetImplicitH     (int idx);

   // Set bond order method.
   // If keep_connectivity is false then connectivity to bond ends 
   // will be recalculated. Connectivity should be kept only when bond order 
   // is changed to/from aromatic.
   DLLEXPORT void setBondOrder (int idx, int order, bool keep_connectivity = false);

   DLLEXPORT void setBondOrder_Silent (int idx, int order);
   
   DLLEXPORT virtual int getAtomNumber  (int idx);
   DLLEXPORT virtual int getAtomCharge  (int idx);
   DLLEXPORT virtual int getAtomIsotope (int idx);
   DLLEXPORT virtual int getAtomRadical (int idx);
   DLLEXPORT virtual int getBondOrder      (int idx);
   DLLEXPORT virtual int getBondTopology   (int idx);
   DLLEXPORT virtual int getAtomAromaticity (int idx);
   DLLEXPORT virtual int getExplicitValence (int idx);
   DLLEXPORT virtual int getAtomValence (int idx);
   DLLEXPORT virtual int getAtomSubstCount (int idx);
   DLLEXPORT virtual int getAtomRingBondsCount (int idx);

   DLLEXPORT virtual int getAtomMaxH   (int idx);
   DLLEXPORT virtual int getAtomMinH   (int idx);
   DLLEXPORT virtual int getAtomTotalH (int idx);

   DLLEXPORT virtual bool isPseudoAtom (int idx);
   DLLEXPORT virtual const char * getPseudoAtom (int idx);

   DLLEXPORT virtual bool isRSite (int atom_idx);
   DLLEXPORT virtual int  getRSiteBits (int atom_idx);
   DLLEXPORT virtual void allowRGroupOnRSite (int atom_idx, int rg_idx);
           void setRSiteBits (int atom_idx, int bits);

   DLLEXPORT virtual bool bondStereoCare (int idx);

   DLLEXPORT virtual void aromatize ();
   DLLEXPORT virtual void dearomatize ();

   DLLEXPORT int getImplicitH (int idx);

   DLLEXPORT int getAtomConnectivity (int idx);
   DLLEXPORT int getAtomConnectivity_noImplH (int idx);
   DLLEXPORT int calcAtomConnectivity_noImplH (int idx);
   DLLEXPORT bool isSaturatedAtom (int idx);

   DLLEXPORT int totalHydrogensCount ();

   DLLEXPORT virtual bool atomNumberBelongs (int idx, const int *numbers, int count);
   DLLEXPORT virtual bool possibleAtomNumber (int idx, int number);
   DLLEXPORT virtual bool possibleAtomNumberAndCharge (int idx, int number, int charge);
   DLLEXPORT virtual bool possibleAtomNumberAndIsotope (int idx, int number, int isotope);
   DLLEXPORT virtual bool possibleAtomIsotope (int idx, int isotope);
   DLLEXPORT virtual bool possibleAtomCharge (int idx, int charge);
   DLLEXPORT virtual void getAtomDescription (int idx, Array<char> &description);
   DLLEXPORT virtual void getBondDescription (int idx, Array<char> &description);
   DLLEXPORT virtual bool possibleBondOrder (int idx, int order);

   DLLEXPORT int getVacantPiOrbitals (int atom_idx, int *lonepairs_out);
   DLLEXPORT int getVacantPiOrbitals (int atom_idx, int conn, int *lonepairs_out);

   DLLEXPORT static int matchAtomsCmp (Graph &g1, Graph &g2, int idx1, int idx2,
                             void *userdata);

   DLLEXPORT void unfoldHydrogens (Array<int> *markers_out, int max_h_cnt = -1);

   DLLEXPORT static void saveBondOrders (Molecule &mol, Array<int> &orders);
   DLLEXPORT static void loadBondOrders (Molecule &mol, Array<int> &orders);

   DLLEXPORT bool convertableToImplicitHydrogen (int idx);

   DLLEXPORT void invalidateHCounters ();

protected:
   struct _Atom
   {
      int  number;
      bool explicit_valence;
      bool explicit_impl_h;
      int  isotope;
      int  charge;
      int  pseudoatom_value_idx; // if number == ELEM_PSEUDO, this is the corresponding
                                 // index from _pseudo_atom_values
      int  rgroup_bits;          // if number == ELEM_RSITE, these are 32 bits, each allowing
                                 // an r-group with corresponding number to go for this atom.
                                 // Simple 'R' atoms have this field equal to zero.
   };

   Array<_Atom> _atoms;
   Array<int>   _bond_orders;
   Array<int>   _connectivity;
   Array<int>   _aromaticity;
   Array<int>   _implicit_h;
   Array<int>   _total_h;
   Array<int>   _valence;
   Array<int>   _radicals;

   StringPool _pseudo_atom_values;
   

   virtual void _mergeWithSubmolecule (BaseMolecule &bmol, const Array<int> &vertices,
                                       const Array<int> *edges, const Array<int> &mapping, 
                                       int skip_flags);

   virtual void _flipBond (int atom_parent, int atom_from, int atom_to);
   virtual void _removeAtoms (const Array<int> &indices, const int *mapping);

   // If 'validate' is true then vertex connectivity and implicit hydrogens 
   // are calculates and stored. If 'validate' is false then connectivity 
   // information is cleared.
   void _validateVertexConnectivity   (int idx, bool validate);

private:
   Molecule (const Molecule &); // no implicit copy
};

#endif
