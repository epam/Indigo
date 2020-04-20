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

#ifndef __cmf_symbol_codes__
#define __cmf_symbol_codes__

#include "molecule/elements.h"

namespace indigo
{

    /* Compressed molecule symbols constants */
    enum
    {
        /* Available values */
        CMF_NUM_OF_CYCLES = 16,
        CMF_MIN_CHARGE = -5,
        CMF_MAX_CHARGE = 8,
        CMF_NUM_OF_CHARGES = CMF_MAX_CHARGE - CMF_MIN_CHARGE + 1,
        CMF_MIN_MASS_DIFF = -30,
        CMF_MAX_MASS_DIFF = 20,
        CMF_MAX_STEREOGROUPS = 4,
        CMF_MAX_VALENCE = 6,
        CMF_MAX_IMPLICIT_H = 10,
        CMF_NUM_OF_BOND_FLAGS = 3,
        CMF_NUM_OF_ATOM_FLAGS = 2,

        /* Bit code used for compression */
        CMF_BIT_CODE_SIZE = 16,

        /* 105 - pseudo-atom prefix */
        CMF_PSEUDOATOM = ELEM_MAX,

        /* 106 - R-Site */
        CMF_RSITE = CMF_PSEUDOATOM + 1,

        /* 107 - where              *
         * bond codes begin         */
        CMF_BONDS = CMF_RSITE + 1,

        CMF_BOND_SINGLE_CHAIN = CMF_BONDS,
        CMF_BOND_SINGLE_RING = CMF_BONDS + 1,
        CMF_BOND_DOUBLE_CHAIN = CMF_BONDS + 2,
        CMF_BOND_DOUBLE_RING = CMF_BONDS + 3,
        CMF_BOND_DOUBLE_CHAIN_CIS = CMF_BONDS + 4,
        CMF_BOND_DOUBLE_CHAIN_TRANS = CMF_BONDS + 5,
        CMF_BOND_DOUBLE_RING_CIS = CMF_BONDS + 6,
        CMF_BOND_DOUBLE_RING_TRANS = CMF_BONDS + 7,
        CMF_BOND_TRIPLE_CHAIN = CMF_BONDS + 8,
        CMF_BOND_TRIPLE_RING = CMF_BONDS + 9,
        CMF_BOND_AROMATIC = CMF_BONDS + 10,

        /* 118 - '(' */
        CMF_OPEN_BRACKET = CMF_BONDS + 11,

        /* 119 - ')' */
        CMF_CLOSE_BRACKET = CMF_OPEN_BRACKET + 1,

        /* 120 - where cycle         *
         * codes begin: 0..15        */
        CMF_CYCLES = CMF_CLOSE_BRACKET + 1,

        /* 136 - adds 16 to the subsequent cycle number */
        CMF_CYCLES_PLUS = CMF_CYCLES + CMF_NUM_OF_CYCLES,

        /* 137 - where charge      *
         * codes begin -5..-1,1..8 */
        CMF_CHARGES = CMF_CYCLES_PLUS + 1,

        /* Separator like '.' in SMILES   *
         * (zero charge is not used) */
        CMF_SEPARATOR = CMF_CHARGES - CMF_MIN_CHARGE,

        /* 151..156 - isotope codes */
        CMF_ISOTOPE_ZERO = CMF_CHARGES + CMF_NUM_OF_CHARGES,
        CMF_ISOTOPE_PLUS1 = CMF_ISOTOPE_ZERO + 1,
        CMF_ISOTOPE_PLUS2 = CMF_ISOTOPE_PLUS1 + 1,
        CMF_ISOTOPE_MINUS1 = CMF_ISOTOPE_PLUS2 + 1,
        CMF_ISOTOPE_MINUS2 = CMF_ISOTOPE_MINUS1 + 1,
        CMF_ISOTOPE_OTHER = CMF_ISOTOPE_MINUS2 + 1, // followed by an extra byte

        /* 157 - where stereocenter codes begin */
        CMF_STEREO = CMF_ISOTOPE_OTHER + 1,

        /* 157 - 'any' stereocenter */
        CMF_STEREO_ANY = CMF_STEREO,

        /* 158 - 'and' stereo-group (1..4) */
        CMF_STEREO_AND_0 = CMF_STEREO + 1,

        /* 162 - 'and' stereo-group (1..4) */
        CMF_STEREO_OR_0 = CMF_STEREO_AND_0 + CMF_MAX_STEREOGROUPS,

        /* 166 - 'abs' stereocenter */
        CMF_STEREO_ABS_0 = CMF_STEREO_OR_0 + CMF_MAX_STEREOGROUPS,

        /* 167 - 'and' stereo-group (1..4) */
        CMF_STEREO_AND_1 = CMF_STEREO_ABS_0 + 1,

        /* 171 - 'and' stereo-group (1..4) */
        CMF_STEREO_OR_1 = CMF_STEREO_AND_1 + CMF_MAX_STEREOGROUPS,

        /* 175 - 'abs' stereocenter */
        CMF_STEREO_ABS_1 = CMF_STEREO_OR_1 + CMF_MAX_STEREOGROUPS,

        /* 176-177 - allene centers */
        CMF_STEREO_ALLENE_0 = CMF_STEREO_ABS_1 + 1,
        CMF_STEREO_ALLENE_1 = CMF_STEREO_ALLENE_0 + 1,

        /* 178 - valence (1..5) */
        CMF_VALENCE = CMF_STEREO_ALLENE_1 + 1,

        /* 185 - implicit hydrogen count (1..5) */
        CMF_IMPLICIT_H = CMF_VALENCE + CMF_MAX_VALENCE + 1,

        /* 196 - singlet radical */
        CMF_RADICAL_SINGLET = CMF_IMPLICIT_H + CMF_MAX_IMPLICIT_H + 1,

        /* 197 - douplet radical */
        CMF_RADICAL_DOUBLET = CMF_RADICAL_SINGLET + 1,

        /* 198 - triplet radical  */
        CMF_RADICAL_TRIPLET = CMF_RADICAL_DOUBLET + 1,

        /* 199 - bond flags */
        CMF_BOND_FLAGS = CMF_RADICAL_TRIPLET + 1,

        /* 202 - atom flags */
        CMF_ATOM_FLAGS = CMF_BOND_FLAGS + CMF_NUM_OF_BOND_FLAGS,

        /* 204-206- bond directions (up/down/either) */
        CMF_BOND_UP = CMF_ATOM_FLAGS + CMF_NUM_OF_ATOM_FLAGS,
        CMF_BOND_DOWN = CMF_BOND_UP + 1,
        CMF_BOND_EITHER = CMF_BOND_DOWN + 1,

        /* 207 - "swap bond ends" flag */
        CMF_BOND_SWAP_ENDS = CMF_BOND_EITHER + 1,

        /* 208 - highlighting */
        CMF_HIGHLIGHTED = CMF_BOND_SWAP_ENDS + 1,

        /* 209 - attachment point */
        CMF_ATTACHPT = CMF_HIGHLIGHTED + 1,

        /* 210 - terminator */
        CMF_TERMINATOR = CMF_ATTACHPT + 1,

        /* 211 - Extended part: sgroups, ... */
        CMF_EXT = CMF_TERMINATOR + 1,

        /* SGroups */
        /* 212 - Data SGroup */
        CMF_DATASGROUP,
        /* 213 - Superatom */
        CMF_SUPERATOM,
        /* 214 - Repeating unit */
        CMF_REPEATINGUNIT,
        /* 215 - Multiple Group */
        CMF_MULTIPLEGROUP,
        /* 216 - Generic SGroup */
        CMF_GENERICSGROUP,

        /* 217 - R-Site attachments */
        CMF_RSITE_ATTACHMENTS,

        /* 218, 219 */
        CMF_BOND_DOUBLE_IGNORED_CIS_TRANS_RING,
        CMF_BOND_DOUBLE_IGNORED_CIS_TRANS_CHAIN,

        /* 220 - R-Site */
        CMF_RSITE_EXT,

        /* 221 - General charge */
        CMF_CHARGE_EXT,

        /* 222 - General valence */
        CMF_VALENCE_EXT,

        /* 223 - Atom mapping to restore */
        CMF_MAPPING,

        /* Alphabet size = 256. Any number can be used because of writing integer indices */
        CMF_ALPHABET_SIZE = 256
    };

} // namespace indigo

#endif
