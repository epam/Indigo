/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __molecule_standardize__
#define __molecule_standardize__

#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Molecule;
class StandardizeOptions;

class MoleculeStandardizer
{
public:
	MoleculeStandardizer();
   // Interface function for stadardize molecule
   static bool standardize (Molecule &mol, const StandardizeOptions &options);

   DECL_ERROR;

protected:
   static void _standardizeStereo(Molecule &mol);
   static void _standardizeCharges(Molecule &mol);
   static void _centerMolecule(Molecule &mol);
   static void _removeSingleAtomFragments(Molecule &mol);
   static void _keepSmallestFragment(Molecule &mol);
   static void _keepLargestFragment(Molecule &mol);
   static void _removeLargestFragment(Molecule &mol);
   static void _makeNonHAtomsCAtoms(Molecule &mol);
   static void _makeNonHAtomsAAtoms(Molecule &mol);
   static void _makeNonCHAtomsQAtoms(Molecule &mol);
   static void _makeAllBondsSingle(Molecule &mol);
   static void _clearCoordinates(Molecule &mol);
   static void _fixCoordinateDimension(Molecule &mol);
   static void _straightenTripleBonds(Molecule &mol);
   static void _straightenAllenes(Molecule &mol);
   static void _clearMolecule(Molecule &mol);
   static void _removeMolecule(Molecule &mol);
   static void _clearStereo(Molecule &mol);
   static void _clearEnhancedStereo(Molecule &mol);
   static void _clearUnknownStereo(Molecule &mol);
   static void _clearUnknownAtomStereo(Molecule &mol);
   static void _clearUnknownCisTransBondStereo(Molecule &mol);
   static void _clearCisTransBondStereo(Molecule &mol);
   static void _setStereoFromCoordinates(Molecule &mol);
   static void _repositionStereoBonds(Molecule &mol);
   static void _repositionAxialStereoBonds(Molecule &mol);
   static void _fixDirectionOfWedgeBonds(Molecule &mol);
   static void _clearCharges(Molecule &mol);
   static void _clearPiBonds(Molecule &mol);
   static void _clearHighlightColors(Molecule &mol);
   static void _clearQueryInfo(Molecule &mol);
   static void _clearAtomLabels(Molecule &mol);
   static void _clearBondLabels(Molecule &mol);
   static void _neutralizeBondedZwitterions(Molecule &mol);
   static void _clearUnusualValence(Molecule &mol);
   static void _clearIsotopes(Molecule &mol);
   static void _clearDativeBonds(Molecule &mol);
   static void _clearHydrogenBonds(Molecule &mol);
   static void _localizeMarkushRAtomsOnRings(Molecule &mol);
   CP_DECL;

private:
   static int _getNumberOfBonds(Molecule &mol, int idx, int bond_type, bool with_element_only, int element);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
