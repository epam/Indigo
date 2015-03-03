/****************************************************************************
 * Copyright (C) 2015 GGA Software Services LLC
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

#ifndef __molecule_layered_molecules_h__
#define __molecule_layered_molecules_h__

#include "common/base_cpp/d_bitset.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class DLLEXPORT LayeredMolecules : public BaseMolecule
{      
public:
   LayeredMolecules(BaseMolecule& molecule);
   virtual ~LayeredMolecules();

   Dbitset &getBondMaskIND(int idx, int order);
   bool isMobilePosition(int idx);
   void setMobilePosition(int idx, bool value);
   Dbitset &getMobilePositionOccupiedMask(int idx);
   void setMobilePositionOccupiedMask(int idx, Dbitset &mask, bool value);
   void addLayers(Dbitset &mask, Array<int> &path, int beg, int end, bool forward);

   void constructMolecule(Molecule &molecule, int layer);

   virtual void clear ();

   virtual BaseMolecule * neu ();

   virtual int getAtomNumber  (int idx);
   virtual int getAtomCharge  (int idx);
   virtual int getAtomIsotope (int idx);
   virtual int getAtomRadical (int idx);
   virtual int getAtomAromaticity (int idx);
   virtual int getExplicitValence (int idx);
   virtual int getAtomValence (int idx);
   virtual int getAtomSubstCount (int idx);
   virtual int getAtomRingBondsCount (int idx);

   virtual int getAtomMaxH   (int idx);
   virtual int getAtomMinH   (int idx);
   virtual int getAtomTotalH (int idx);

   virtual bool isPseudoAtom (int idx);
   virtual const char * getPseudoAtom (int idx);

   virtual bool  isRSite (int atom_idx);
   virtual dword getRSiteBits (int atom_idx);
   virtual void  allowRGroupOnRSite (int atom_idx, int rg_idx);

   virtual int getBondOrder      (int idx);
   virtual int getBondTopology   (int idx);

   virtual bool atomNumberBelongs (int idx, const int *numbers, int count);
   virtual bool possibleAtomNumber (int idx, int number);
   virtual bool possibleAtomNumberAndCharge (int idx, int number, int charge);
   virtual bool possibleAtomNumberAndIsotope (int idx, int number, int isotope);
   virtual bool possibleAtomIsotope (int idx, int isotope);
   virtual bool possibleAtomCharge (int idx, int charge);
   virtual void getAtomDescription (int idx, Array<char> &description);
   virtual void getBondDescription (int idx, Array<char> &description);
   virtual bool possibleBondOrder (int idx, int order);

   virtual bool isSaturatedAtom (int idx);

   virtual bool bondStereoCare (int idx);

   virtual bool aromatize (const AromaticityOptions &options);
   virtual bool dearomatize (const AromaticityOptions &options);

   int layers;

protected:
   Molecule _proto;
   ObjArray<Dbitset> _bond_masks[4];
   Array<bool>   _mobilePositions;
   ObjArray<Dbitset> _mobilePositionsOccupied;

   virtual void _mergeWithSubmolecule (BaseMolecule &bmol, const Array<int> &vertices,
                                       const Array<int> *edges, const Array<int> &mapping, 
                                       int skip_flags);

private:
   LayeredMolecules(const LayeredMolecules &); // no implicit copy
   unsigned _wordsNeeded;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
