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
#include "molecule/elements.h"

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
   for (int i : mol.vertices())
   {
      switch (mol.getAtomNumber(i))
      {
      case ELEM_N:
         if (_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 4)
         {
            mol.setAtomCharge(i, +1);
            if (_getNumberOfBonds(mol, i, BOND_SINGLE, true, ELEM_O) == 1)
            {
               const Vertex &v = mol.getVertex(i);
               for (int j : v.neighbors())
               {
                  if ((mol.getAtomNumber(v.neiVertex(j)) == ELEM_O) && (mol.getVertex(v.neiVertex(j)).degree() == 1))
                     mol.setAtomCharge(v.neiVertex(j), -1);
               }
            }
         }
         else if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 1) &&
                  (_getNumberOfBonds(mol, i, BOND_AROMATIC, false, 0) == 2))
         {
            mol.setAtomCharge(i, +1);
            if (_getNumberOfBonds(mol, i, BOND_SINGLE, true, ELEM_O) == 1)
            {
               const Vertex &v = mol.getVertex(i);
               for (int j : v.neighbors())
               {
                  if ((mol.getAtomNumber(v.neiVertex(j)) == ELEM_O) && (mol.getVertex(v.neiVertex(j)).degree() == 1))
                     mol.setAtomCharge(v.neiVertex(j), -1);
               }
            }
         }
         else if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 2) &&
                  (_getNumberOfBonds(mol, i, BOND_DOUBLE, false, 0) == 1) &&
                  (_getNumberOfBonds(mol, i, BOND_SINGLE, true, ELEM_O) == 1))
         {
            mol.setAtomCharge(i, +1);
            const Vertex &v = mol.getVertex(i);
            for (int j : v.neighbors())
            {
               if ((mol.getAtomNumber(v.neiVertex(j)) == ELEM_O) && (mol.getVertex(v.neiVertex(j)).degree() == 1))
                  mol.setAtomCharge(v.neiVertex(j), -1);
            }
         }
         else if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 2) &&
                  (_getNumberOfBonds(mol, i, BOND_DOUBLE, false, 0) == 1))
         {
            mol.setAtomCharge(i, +1);
         }
         break;
      case ELEM_O:
         if (mol.getVertex(i).degree() == 3)
         {
            mol.setAtomCharge(i, +1);
         }
         else if (mol.getVertex(i).degree() == 2)
         {
            if (_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 2)
               break;
            if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, ELEM_C) == 1) &&
                (_getNumberOfBonds(mol, i, BOND_DOUBLE, false, 0) == 1))
            {
               mol.setAtomCharge(i, +1);
            }
         }
         break;
      case ELEM_S:
         if (mol.getVertex(i).degree() == 3)
         {
            mol.setAtomCharge(i, +1);
         }
         else if (mol.getVertex(i).degree() == 2)
         {
            if (_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 2)
               break;
            if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, ELEM_C) == 1) &&
                (_getNumberOfBonds(mol, i, BOND_DOUBLE, false, 0) == 1))
            {
               mol.setAtomCharge(i, +1);
            }
         }
         break;
      case ELEM_F:
         if (mol.getVertex(i).degree() == 0)
         {
               mol.setAtomCharge(i, -1);
         }
         break;
      case ELEM_Cl:
         if (mol.getVertex(i).degree() == 0)
         {
               mol.setAtomCharge(i, -1);
         }
         break;
      case ELEM_Br:
         if (mol.getVertex(i).degree() == 0)
         {
               mol.setAtomCharge(i, -1);
         }
         break;
      case ELEM_I:
         if (mol.getVertex(i).degree() == 0)
         {
               mol.setAtomCharge(i, -1);
         }
         break;

      default:
         break;
      }
   }
}

void MoleculeStandardizer::_centerMolecule (Molecule &mol)
{
   throw Error("Not implemented yet");
}

void MoleculeStandardizer::_removeSingleAtomFragments (Molecule &mol)
{
   QS_DEF(Array<int>, single_atoms);
   single_atoms.clear();

   for (auto i : mol.vertices())
   {
      auto atom_number = mol.getAtomNumber(i);
      if ((atom_number > ELEM_H) && (atom_number < ELEM_MAX))
      {
         if (mol.getAtomConnectivity(i) <= 0)
            single_atoms.push(i);
      }
   }

   if (single_atoms.size() > 0)
      mol.removeAtoms(single_atoms);
   
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
   for (auto i : mol.vertices())
   {
      auto atom_number = mol.getAtomNumber(i);
      if ((atom_number > ELEM_H) && (atom_number < ELEM_MAX) && (atom_number != ELEM_C))
        mol.resetAtom(i, ELEM_C);
   }
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
   for (auto i : mol.edges())
   {
      if (mol.getBondOrder(i) != BOND_SINGLE)
         mol.setBondOrder(i, BOND_SINGLE, false);
   }
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
   for (auto i : mol.vertices())
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
   mol.unhighlightAll();
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
   for (auto i : mol.vertices())
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

int MoleculeStandardizer::_getNumberOfBonds(Molecule &mol, int idx, int bond_type, bool with_element_only, int element)
{
   auto num_bonds = 0;
   const Vertex &v = mol.getVertex(idx);

   for (int i : v.neighbors())
   {
      if (with_element_only)
      {
         if ((mol.getAtomNumber(v.neiVertex(i)) == element) && (mol.getBondOrder(v.neiEdge(i)) == bond_type))
            num_bonds++;        
      }
      else
      {
         if (mol.getBondOrder(v.neiEdge(i)) == bond_type)
            num_bonds++;        
      }
   }
   return num_bonds;
}
