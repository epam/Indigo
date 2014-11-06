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

#ifndef __molecule_standardize_options__
#define __molecule_standardize_options__


#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class StandardizeOptions
{
public:
   StandardizeOptions ();

   void reset ();

   // Sets or repairs the stereo on a molecule to a standard form using the coordinates
   // as the guide. Default is false.
   bool standardize_stereo;

   // Sets the charges on a molecule to a standard form. Default is false.
   bool standardize_charges;

   // Translates a molecule so its geometric center lies at the origin. Default is false.
   bool center_molecule;

   // Removes fragments that consist of only a single heavy atom. Default is false.
   bool remove_single_atom_fragments;

   // Keeps only the smallest fragment in the molecule. Default is false.
   bool keep_smallest_fragment;

   // Keeps only the largest fragment in the molecule. Default is false.
   bool keep_largest_fragment;

   // Removes the largest fragment in the molecule. Default is false.
   bool remove_largest_fragment;

   // Converts all non-Hydrogen atoms atoms in the molecule to carbon. Default is false.
   bool make_non_h_atoms_c_atoms;

   // Converts all non-Hydrogen atoms in the molecule to the A query atom type.
   // Default is false.
   bool make_non_h_atoms_a_atoms;

   // Converts all non-Carbon, non-Hydrogen atoms in the molecule to the Q query atom type.
   // Default is false.
   bool make_non_c_h_atoms_q_atoms;

   // Converts all bonds in the molecule to single bonds. Default is false.
   bool make_all_bonds_single;

   // Sets all x, y, z coordinates to zero. Default is false.
   bool clear_coordinates;

   // Sets the coordinate dimension (0D, 2D, 3D) based on the atomic coordinates.
   // Default is false.
   bool fix_coordinate_dimension;

   // Finds atoms with triple bonds and non-linear geometry and fixes them
   // so that the bond angles are 180 degrees. Default is false.
   bool straighten_triple_bonds;

   // Finds atoms with two double-bonds and non-linear geometry and fixes them
   // so that the bond angles are 180 degrees.Default is false.
   bool straighten_allenes;

   // Deletes all atoms and bonds in the molecule, keeping the molecule object
   // in the data record. Default is false.
   bool clear_molecule;

   // Deletes the molecule object from the data record. Default is false.
   bool remove_molecule;

   // Sets all atoms and bonds to NoStereo. Default is false.
   bool clear_stereo;

   // Removes all relative stereo groupings. Default is false.
   bool clear_enhanced_stereo;

   // Sets all atoms and bonds marked UnknownStereo to NoStereo. Default is false.
   bool clear_unknown_stereo;

   // Sets all atoms marked UnknownStereo to NoStereo. Default is false.
   bool clear_unknown_atom_stereo;

   // Sets all bonds marked UnknownStereo to NoStereo. Default is false.
   bool clear_unknown_cis_trans_bond_stereo;

   // Sets all bonds marked CisStereo or TransStereo to UnknownStereo. Default is false.
   bool clear_cis_trans_bond_stereo;

   // Uses 2D coordinates and up/down bond markings (or 3D coordinates) to assign
   // the stereochemistry of the atoms or bonds. Default is false.
   bool set_stereo_from_coordinates;

   // Repositions the stereo bond markings in an attempt to find the best bond
   // to mark as a wedge bond for each stereo atom. Default is false.
   bool reposition_stereo_bonds;

   // Repositions the stereo bond markings for axial stereo centers
   // (allenes and atropisomers) in an attempt to find the best bond
   // to mark as a wedge bond for each center. Default is false.
   bool reposition_axial_stereo_bonds;

   // Checks the wedge bonds in the molecule to ensure that the wedge
   // is drawn with the stereo atom at the narrow end of the wedge. Default is false.
   bool fix_direction_of_wedge_bonds;

   // Sets all formal charges to zero. Default is false.
   bool clear_charges;

   // Clears any pi bonds and Pi systems from the molecule. Default is false.
   bool clear_pi_bonds;

   // Clears any highlight colors from atoms and bonds. Default is false.
   bool clear_highlight_colors;

   // Deletes all query information from atoms and bonds. Default is false.
   bool clear_query_info;

   // Clears labels from atoms. Default is false.
   bool clear_atom_labels;

   // Clears labels from bonds. Default is false.
   bool clear_bond_labels;

   // Converts directly bonded zwitterions (positively charged atom bonded
   // to negatively charged atom, A+B-) to the neutral representation (A=B). Default is false.
   bool neutralize_bonded_zwitterions;

   // Clears any atom valence query features and resets all implicit hydrogen
   // counts to their standard values. Default is false.
   bool clear_unusual_valence;

   // Clears all isotope markings from atoms. Default is false.
   bool clear_isotopes;

   // Clears all explicit zero-order coordination bonds of dative type (V3000 type-9 bonds).
   // Default is false.
   bool clear_dative_bonds;

   // Clears all explicit zero-order hydrogen bonds (V3000 type-10 bonds). Default is false.
   bool clear_hydrogen_bonds;

   // R atoms bonded to the centers of rings are converted to R atoms at all open
   // positions on the ring. Default is false.
   bool localize_markush_r_atoms_on_rings;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_standardize_options__
