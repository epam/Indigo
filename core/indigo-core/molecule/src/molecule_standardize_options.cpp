/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "molecule/molecule_standardize_options.h"

#include "base_cpp/scanner.h"

using namespace indigo;

//
// StandardizeOptions
//
StandardizeOptions::StandardizeOptions()
{
    reset();
}

void StandardizeOptions::reset()
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
    remove_extra_stereo_bonds = false;
}

void StandardizeOptions::parseFromString(const char* options)
{
    BufferScanner scanner(options);
    QS_DEF(Array<char>, word);

    scanner.skipSpace();
    while (!scanner.isEOF())
    {
        scanner.skipSpace();
        scanner.readWord(word, 0);

        if (strcasecmp(word.ptr(), "standardize-stereo") == 0)
            standardize_stereo = true;
        else if (strcasecmp(word.ptr(), "standardize-charges") == 0)
            standardize_charges = true;
        else if (strcasecmp(word.ptr(), "center-molecule") == 0)
            center_molecule = true;
        else if (strcasecmp(word.ptr(), "remove-single-atom-fragments") == 0)
            remove_single_atom_fragments = true;
        else if (strcasecmp(word.ptr(), "keep-smallest-fragment") == 0)
            keep_smallest_fragment = true;
        else if (strcasecmp(word.ptr(), "keep-largest-fragment") == 0)
            keep_largest_fragment = true;
        else if (strcasecmp(word.ptr(), "remove-largest-fragment") == 0)
            remove_largest_fragment = true;
        else if (strcasecmp(word.ptr(), "make-non-h-atoms-c-atoms") == 0)
            make_non_h_atoms_c_atoms = true;
        else if (strcasecmp(word.ptr(), "make-non-h-atoms-a-atoms") == 0)
            make_non_h_atoms_a_atoms = true;
        else if (strcasecmp(word.ptr(), "make-non-c-h-atoms-q-atoms") == 0)
            make_non_c_h_atoms_q_atoms = true;
        else if (strcasecmp(word.ptr(), "make-all-bonds-single") == 0)
            make_all_bonds_single = true;
        else if (strcasecmp(word.ptr(), "clear-coordinates") == 0)
            clear_coordinates = true;
        else if (strcasecmp(word.ptr(), "fix-coordinate-dimension") == 0)
            fix_coordinate_dimension = true;
        else if (strcasecmp(word.ptr(), "straighten-triple-bonds") == 0)
            straighten_triple_bonds = true;
        else if (strcasecmp(word.ptr(), "straighten-allenes") == 0)
            straighten_allenes = true;
        else if (strcasecmp(word.ptr(), "clear-molecule") == 0)
            clear_molecule = true;
        else if (strcasecmp(word.ptr(), "remove-molecule") == 0)
            remove_molecule = true;
        else if (strcasecmp(word.ptr(), "clear-stereo") == 0)
            clear_stereo = true;
        else if (strcasecmp(word.ptr(), "clear-enhanced-stereo") == 0)
            clear_enhanced_stereo = true;
        else if (strcasecmp(word.ptr(), "clear-unknown-stereo") == 0)
            clear_unknown_stereo = true;
        else if (strcasecmp(word.ptr(), "clear-unknown-atom-stereo") == 0)
            clear_unknown_atom_stereo = true;
        else if (strcasecmp(word.ptr(), "clear-unknown-cis-trans-bond-stereo") == 0)
            clear_unknown_cis_trans_bond_stereo = true;
        else if (strcasecmp(word.ptr(), "clear-cis-trans-bond-stereo") == 0)
            clear_cis_trans_bond_stereo = true;
        else if (strcasecmp(word.ptr(), "set-stereo-from-coordinates") == 0)
            set_stereo_from_coordinates = true;
        else if (strcasecmp(word.ptr(), "reposition-stereo-bonds") == 0)
            reposition_stereo_bonds = true;
        else if (strcasecmp(word.ptr(), "reposition-axial-stereo-bonds") == 0)
            reposition_axial_stereo_bonds = true;
        else if (strcasecmp(word.ptr(), "fix-direction-of-wedge-bonds") == 0)
            fix_direction_of_wedge_bonds = true;
        else if (strcasecmp(word.ptr(), "clear-charges") == 0)
            clear_charges = true;
        else if (strcasecmp(word.ptr(), "clear-pi-bonds") == 0)
            clear_pi_bonds = true;
        else if (strcasecmp(word.ptr(), "clear-highlight-colors") == 0)
            clear_highlight_colors = true;
        else if (strcasecmp(word.ptr(), "clear-query-info") == 0)
            clear_query_info = true;
        else if (strcasecmp(word.ptr(), "clear-atom-labels") == 0)
            clear_atom_labels = true;
        else if (strcasecmp(word.ptr(), "clear-bond-labels") == 0)
            clear_bond_labels = true;
        else if (strcasecmp(word.ptr(), "neutralize-bonded-zwitterions") == 0)
            neutralize_bonded_zwitterions = true;
        else if (strcasecmp(word.ptr(), "clear-unusual_valence") == 0)
            clear_unusual_valence = true;
        else if (strcasecmp(word.ptr(), "clear-isotopes") == 0)
            clear_isotopes = true;
        else if (strcasecmp(word.ptr(), "clear-dative-bonds") == 0)
            clear_dative_bonds = true;
        else if (strcasecmp(word.ptr(), "clear-hydrogen-bonds") == 0)
            clear_hydrogen_bonds = true;
        else if (strcasecmp(word.ptr(), "localize-markush-r-atoms-on-rings") == 0)
            localize_markush_r_atoms_on_rings = true;
        else if (strcasecmp(word.ptr(), "create-coordination-bonds") == 0)
            create_coordination_bonds = true;
        else if (strcasecmp(word.ptr(), "create-hydrogen-bonds") == 0)
            create_hydrogen_bonds = true;
        else if (strcasecmp(word.ptr(), "remove-extra-stereo-bonds") == 0)
            remove_extra_stereo_bonds = true;

        scanner.skipSpace();
    }
}