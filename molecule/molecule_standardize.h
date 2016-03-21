/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

class BaseMolecule;
class Molecule;
class QueryMolecule;
class StandardizeOptions;

class MoleculeStandardizer
{
public:
   MoleculeStandardizer();
   // Interface function for stadardize molecule
   static bool standardize (Molecule &mol, const StandardizeOptions &options);
   static bool standardize (QueryMolecule &query, const StandardizeOptions &options);

   static bool isFragmentLinear(BaseMolecule &mol, int idx);

   DECL_ERROR;

protected:
   static void _standardizeStereo(Molecule &mol);
   static void _standardizeStereo(QueryMolecule &mol);
   static void _standardizeCharges(Molecule &mol);
   static void _standardizeCharges(QueryMolecule &mol);
   static void _centerMolecule(BaseMolecule &mol);
   static void _removeSingleAtomFragments(BaseMolecule &mol);
   static void _keepSmallestFragment(BaseMolecule &mol);
   static void _keepLargestFragment(BaseMolecule &mol);
   static void _removeLargestFragment(BaseMolecule &mol);
   static void _makeNonHAtomsCAtoms(Molecule &mol);
   static void _makeNonHAtomsCAtoms(QueryMolecule &mol);
   static void _makeNonHAtomsAAtoms(Molecule &mol);
   static void _makeNonHAtomsAAtoms(QueryMolecule &mol);
   static void _makeNonCHAtomsQAtoms(Molecule &mol);
   static void _makeNonCHAtomsQAtoms(QueryMolecule &mol);
   static void _makeAllBondsSingle(Molecule &mol);
   static void _makeAllBondsSingle(QueryMolecule &mol);
   static void _clearCoordinates(BaseMolecule &mol);
   static void _fixCoordinateDimension(BaseMolecule &mol);
   static void _straightenTripleBonds(BaseMolecule &mol);
   static void _straightenAllenes(BaseMolecule &mol);
   static void _clearMolecule(BaseMolecule &mol);
   static void _removeMolecule(BaseMolecule &mol);
   static void _clearStereo(BaseMolecule &mol);
   static void _clearEnhancedStereo(BaseMolecule &mol);
   static void _clearUnknownStereo(BaseMolecule &mol);
   static void _clearUnknownAtomStereo(BaseMolecule &mol);
   static void _clearUnknownCisTransBondStereo(BaseMolecule &mol);
   static void _clearCisTransBondStereo(BaseMolecule &mol);
   static void _setStereoFromCoordinates(BaseMolecule &mol);
   static void _repositionStereoBonds(BaseMolecule &mol);
   static void _repositionAxialStereoBonds(BaseMolecule &mol);
   static void _fixDirectionOfWedgeBonds(BaseMolecule &mol);
   static void _clearCharges(Molecule &mol);
   static void _clearCharges(QueryMolecule &mol);
   static void _clearPiBonds(BaseMolecule &mol);
   static void _clearHighlightColors(BaseMolecule &mol);
   static void _clearQueryInfo(BaseMolecule &mol);
   static void _clearAtomLabels(Molecule &mol);
   static void _clearAtomLabels(QueryMolecule &mol);
   static void _clearBondLabels(Molecule &mol);
   static void _clearBondLabels(QueryMolecule &mol);
   static void _neutralizeBondedZwitterions(Molecule &mol);
   static void _neutralizeBondedZwitterions(QueryMolecule &mol);
   static void _clearUnusualValence(Molecule &mol);
   static void _clearUnusualValence(QueryMolecule &mol);
   static void _clearIsotopes(Molecule &mol);
   static void _clearIsotopes(QueryMolecule &mol);
   static void _clearDativeBonds(BaseMolecule &mol);
   static void _clearHydrogenBonds(BaseMolecule &mol);
   static void _localizeMarkushRAtomsOnRings(Molecule &mol);
   static void _localizeMarkushRAtomsOnRings(QueryMolecule &mol);
   static void _createCoordinationBonds(BaseMolecule &mol);
   static void _createHydrogenBonds(BaseMolecule &mol);
   static void _removeExtraStereoBonds(BaseMolecule &mol);
   CP_DECL;

private:
   static int _getNumberOfBonds(BaseMolecule &mol, int idx, int bond_type, bool with_element_only, int element);
   static void _linearizeFragment(BaseMolecule &mol, int idx);
   static bool _isNonMetalAtom(int atom_number);
   static bool _isMetalAtom(int atom_number);
   static int _asc_cmp_cb (int &v1, int &v2, void *context);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
