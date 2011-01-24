/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#ifndef __cmf_symbol_codes_h__
#define __cmf_symbol_codes_h__

#include "molecule/elements.h"

namespace indigo {

/* Compressed molecule symbols constants */
enum {
   /* Available values */
   CMF_NUM_OF_CYCLES = 16,
   CMF_NUM_OF_AAM = 16,
   CMF_MIN_CHARGE = -5,
   CMF_MAX_CHARGE = 8,
   CMF_NUM_OF_CHARGES = CMF_MAX_CHARGE - CMF_MIN_CHARGE + 1,
   CMF_MIN_MASS_DIFF = -30,
   CMF_MAX_MASS_DIFF = 20,
   CMF_NUM_OF_ISOTOPES = CMF_MAX_MASS_DIFF - CMF_MIN_MASS_DIFF + 1,
   CMF_MAX_STEREOGROUPS = 4,
   CMF_MAX_VALENCE = 6,
   CMF_MAX_IMPLICIT_H = 10,
   CMF_NUM_OF_BOND_FLAGS = 3,
   CMF_NUM_OF_ATOM_FLAGS = 2,

   /* Bit code used for compression */
   CMF_BIT_CODE_SIZE = 16,

   /* 105 - pseudo-atom prefix */
   CMF_PSEUDOATOM = ELEM_MAX,
   CMF_RSITE = CMF_PSEUDOATOM + 1,

   /* 106 - where              *
    * bond codes begin         */
   CMF_BONDS = CMF_PSEUDOATOM + 1,
   
   CMF_BOND_SINGLE_CHAIN       = CMF_BONDS,
   CMF_BOND_SINGLE_RING        = CMF_BONDS + 1,
   CMF_BOND_DOUBLE_CHAIN       = CMF_BONDS + 2,
   CMF_BOND_DOUBLE_RING        = CMF_BONDS + 3,
   CMF_BOND_DOUBLE_CHAIN_CIS   = CMF_BONDS + 4,
   CMF_BOND_DOUBLE_CHAIN_TRANS = CMF_BONDS + 5,
   CMF_BOND_DOUBLE_RING_CIS    = CMF_BONDS + 6,
   CMF_BOND_DOUBLE_RING_TRANS  = CMF_BONDS + 7,
   CMF_BOND_TRIPLE_CHAIN       = CMF_BONDS + 8,
   CMF_BOND_TRIPLE_RING        = CMF_BONDS + 9,
   CMF_BOND_AROMATIC           = CMF_BONDS + 10,

   /* 117 - '(' */
   CMF_OPEN_BRACKET = CMF_BONDS + 11,

   /* 118 - ')' */
   CMF_CLOSE_BRACKET = CMF_OPEN_BRACKET + 1,

   /* 119 - where cycle         *
    * codes begin: 0..15        */
   CMF_CYCLES = CMF_CLOSE_BRACKET + 1,

   /* 135 - adds 16 to the subsequent cycle number */
   CMF_CYCLES_PLUS = CMF_CYCLES + CMF_NUM_OF_CYCLES,

   /* 136 - where charge      *
    * codes begin -5..-1,1..8 */
   CMF_CHARGES = CMF_CYCLES_PLUS + 1,

   /* Bond like '.' in SMILES   *
    * (zero charge is not used) */
   CMF_SEPARATOR = CMF_CHARGES - CMF_MIN_CHARGE,

   /* 150 - where isotopes        *
    * codes begin -30..-1,0,1..20 */
   CMF_ISOTOPES = CMF_CHARGES + CMF_NUM_OF_CHARGES,

   /* 201 - where stereocenter codes begin */
   CMF_STEREO = CMF_ISOTOPES + CMF_NUM_OF_ISOTOPES,
   
   /* 202 - 'any' stereocenter */
   CMF_STEREO_ANY = CMF_STEREO,
   
   /* 202 - 'and' stereo-group (1..4) */
   CMF_STEREO_AND_0 = CMF_STEREO + 1,
   
   /* 206 - 'and' stereo-group (1..4) */
   CMF_STEREO_OR_0  = CMF_STEREO_AND_0 + CMF_MAX_STEREOGROUPS,
   
   /* 210 - 'abs' stereocenter */
   CMF_STEREO_ABS_0 = CMF_STEREO_OR_0 + CMF_MAX_STEREOGROUPS,
   
   /* 211 - 'and' stereo-group (1..4) */
   CMF_STEREO_AND_1 = CMF_STEREO_ABS_0 + 1,
   
   /* 215 - 'and' stereo-group (1..4) */
   CMF_STEREO_OR_1  = CMF_STEREO_AND_1 + CMF_MAX_STEREOGROUPS,
   
   /* 219 - 'abs' stereocenter */
   CMF_STEREO_ABS_1 = CMF_STEREO_OR_1 + CMF_MAX_STEREOGROUPS,
   
   /* 220 - valence (1..5) */
   CMF_VALENCE = CMF_STEREO_ABS_1 + 1,
   
   /* 226 - implicit hydrogen count (1..5) */
   CMF_IMPLICIT_H = CMF_VALENCE + CMF_MAX_VALENCE,
   
   /* 236 - singlet radical */
   CMF_RADICAL_SINGLET = CMF_IMPLICIT_H + CMF_MAX_IMPLICIT_H,
   
   /* 237 - douplet radical */
   CMF_RADICAL_DOUPLET = CMF_RADICAL_SINGLET + 1,
   
   /* 238 - triplet radical  */
   CMF_RADICAL_TRIPLET = CMF_RADICAL_DOUPLET + 1,

   /* 239 - bond flags */
   CMF_BOND_FLAGS = CMF_RADICAL_TRIPLET + 1,

   /* 242 - atom flags */
   CMF_ATOM_FLAGS = CMF_BOND_FLAGS + CMF_NUM_OF_BOND_FLAGS,

   /* 244 - terminator */
   CMF_TERMINATOR = CMF_ATOM_FLAGS + CMF_NUM_OF_ATOM_FLAGS,

   /* Alphabet size = 245 */
   CMF_ALPHABET_SIZE = CMF_TERMINATOR + 1
};

/***********************************/

}

#endif /* __cmf_symbol_codes_h__ */

/* END OF 'CMF_SYMBOL_CODES.H' FILE */
