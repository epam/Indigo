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

#include "molecule/molecule_standardize.h"
#include "molecule/molecule_standardize_options.h"
#include "molecule/molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeStandardizer, "Molecule Standardizer");

CP_DEF(MoleculeStandardizer);
MoleculeStandardizer::MoleculeStandardizer():
CP_INIT{
}

bool MoleculeStandardizer::standardize (Molecule &mol, const StandardizeOptions &options)
{

   if (options.standardize_stereo)
   {
      _standardizeStereo(mol);
   }

   if (options.standardize_charges)
   {
      _standardizeCharges(mol);
   }

   if (options.center_molecule)
   {
      _centerMolecule(mol);
   }

   if (options.remove_single_atom_fragments)
   {
      _removeSingleAtomFragments(mol);
   }

   if (options.keep_smallest_fragment)
   {
      _keepSmallestFragment(mol);
   }

   if (options.keep_largest_fragment)
   {
      _keepLargestFragment(mol);
   }

   if (options.remove_largest_fragment)
   {
      _removeLargestFragment(mol);
   }

   if (options.make_non_h_atoms_c_atoms)
   {
      _makeNonHAtomsCAtoms(mol);
   }

   if (options.make_non_h_atoms_a_atoms)
   {
      _makeNonHAtomsAAtoms(mol);
   }

   if (options.make_non_c_h_atoms_q_atoms)
   {
      _makeNonCHAtomsQAtoms(mol);
   }

   if (options.make_all_bonds_single)
   {
      _makeAllBondsSingle(mol);
   }

   if (options.clear_coordinates)
   {
      _clearCoordinates(mol);
   }

   if (options.fix_coordinate_dimension)
   {
      _fixCoordinateDimension(mol);
   }

   if (options.straighten_triple_bonds)
   {
      _straightenTripleBonds(mol);
   }

   if (options.straighten_allenes)
   {
      _straightenAllenes(mol);
   }

   if (options.clear_molecule)
   {
      _clearMolecule(mol);
   }

   if (options.remove_molecule)
   {
      _removeMolecule(mol);
   }

   if (options.clear_stereo)
   {
      _clearStereo(mol);
   }

   if (options.clear_enhanced_stereo)
   {
      _clearEnhancedStereo(mol);
   }

   if (options.clear_unknown_stereo)
   {
      _clearUnknownStereo(mol);
   }

   if (options.clear_unknown_atom_stereo)
   {
      _clearUnknownAtomStereo(mol);
   }

   if (options.clear_unknown_cis_trans_bond_stereo)
   {
      _clearUnknownCisTransBondStereo(mol);
   }

   if (options.clear_cis_trans_bond_stereo)
   {
      _clearCisTransBondStereo(mol);
   }

   if (options.set_stereo_from_coordinates)
   {
      _setStereoFromCoordinates(mol);
   }

   if (options.reposition_stereo_bonds)
   {
      _repositionStereoBonds(mol);
   }

   if (options.reposition_axial_stereo_bonds)
   {
      _repositionAxialStereoBonds(mol);
   }

   if (options.fix_direction_of_wedge_bonds)
   {
      _fixDirectionOfWedgeBonds(mol);
   }

   if (options.clear_charges)
   {
      _clearCharges(mol);
   }

   if (options.clear_pi_bonds)
   {
      _clearPiBonds(mol);
   }

   if (options.clear_highlight_colors)
   {
      _clearHighlightColors(mol);
   }

   if (options.clear_query_info)
   {
      _clearQueryInfo(mol);
   }

   if (options.clear_atom_labels)
   {
      _clearAtomLabels(mol);
   }

   if (options.clear_bond_labels)
   {
      _clearBondLabels(mol);
   }

   if (options.neutralize_bonded_zwitterions)
   {
      _neutralizeBondedZwitterions(mol);
   }

   if (options.clear_unusual_valence)
   {
      _clearUnusualValence(mol);
   }

   if (options.clear_isotopes)
   {
      _clearIsotopes(mol);
   }

   if (options.clear_dative_bonds)
   {
      _clearDativeBonds(mol);
   }

   if (options.clear_hydrogen_bonds)
   {
      _clearHydrogenBonds(mol);
   }

   if (options.localize_markush_r_atoms_on_rings)
   {
      _localizeMarkushRAtomsOnRings(mol);
   }

   return true;
}

void MoleculeStandardizer::_standardizeStereo (Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_standardizeCharges (Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_centerMolecule (Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_removeSingleAtomFragments (Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_keepSmallestFragment (Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_keepLargestFragment(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_removeLargestFragment(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_makeNonHAtomsCAtoms(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_makeNonHAtomsAAtoms(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_makeNonCHAtomsQAtoms(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_makeAllBondsSingle(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearCoordinates(Molecule &mol)
{
	mol.clearXyz();
}

void MoleculeStandardizer::_fixCoordinateDimension(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_straightenTripleBonds(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_straightenAllenes(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearMolecule(Molecule &mol)
{
   mol.clear();
}

void MoleculeStandardizer::_removeMolecule(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearStereo(Molecule &mol)
{
	mol.stereocenters.clear();
	mol.cis_trans.clear();
	mol.allene_stereo.clear();
}

void MoleculeStandardizer::_clearEnhancedStereo(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearUnknownStereo(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearUnknownAtomStereo(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearUnknownCisTransBondStereo(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearCisTransBondStereo(Molecule &mol)
{
	mol.cis_trans.clear();
}

void MoleculeStandardizer::_setStereoFromCoordinates(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_repositionStereoBonds(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_repositionAxialStereoBonds(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_fixDirectionOfWedgeBonds(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearCharges(Molecule &mol)
{
	for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
	{
		mol.setAtomCharge(i, 0);
	}
}

void MoleculeStandardizer::_clearPiBonds(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearHighlightColors(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearQueryInfo(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearAtomLabels(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearBondLabels(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_neutralizeBondedZwitterions(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearUnusualValence(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearIsotopes(Molecule &mol)
{
	for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
	{
		mol.setAtomIsotope(i, 0);
	}
}

void MoleculeStandardizer::_clearDativeBonds(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearHydrogenBonds(Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_localizeMarkushRAtomsOnRings(Molecule &mol)
{
   throw Error("Not implemented yet");
}
