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

#include "molecule/molecule_standardize_options.h"

using namespace indigo;

//
// StandardizeOptions
//
StandardizeOptions::StandardizeOptions ()
{
   reset();
}

void StandardizeOptions::reset ()
{
   standardize_stereo = false;
   standardize_charges = false;
   center_molecule = false;
   remove_single_atom_fragments = false;
   keep_smallest_fragment = false;
   keep_largest_fragment = false;
   remove_largest_fragment = false;
   make_non_h_atoms_c_atoms = false;
   make_non_h_atoms_a_atoms = false;
   make_non_c_h_atoms_q_atoms = false;
   make_all_bonds_single = false;
   clear_coordinates = false;
   fix_coordinate_dimension = false;
   straighten_triple_bonds = false;
   straighten_allenes = false;
   clear_molecule = false;
   remove_molecule = false;
   clear_stereo = false;
   clear_enhanced_stereo = false;
   clear_unknown_stereo = false;
   clear_unknown_atom_stereo = false;
   clear_unknown_cis_trans_bond_stereo = false;
   clear_cis_trans_bond_stereo = false;
   set_stereo_from_coordinates = false;
   reposition_stereo_bonds = false;
   reposition_axial_stereo_bonds = false;
   fix_direction_of_wedge_bonds = false;
   clear_charges = false;
   clear_pi_bonds = false;
   clear_highlight_colors = false;
   clear_query_info = false;
   clear_atom_labels = false;
   clear_bond_labels = false;
   neutralize_bonded_zwitterions = false;
   clear_unusual_valence = false;
   clear_isotopes = false;
   clear_dative_bonds = false;
   clear_hydrogen_bonds = false;
   localize_markush_r_atoms_on_rings = false;
   create_coordination_bonds = false;
   create_hydrogen_bonds = false;
}

