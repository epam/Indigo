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

#ifndef __bingo_core_c_h___
#define __bingo_core_c_h___

#include "base_c/defs.h"

CEXPORT int mangoIndexReadPreparedMolecule (int *id,
                 const char **cmf_buf, int *cmf_buf_len,
                 const char **xyz_buf, int *xyz_buf_len,
                 const char **gross_str, 
                 const char **counter_elements_str,
                 const char **fingerprint_buf, int *fingerprint_buf_len,
                 const char **fingerprint_sim_str, 
                 float *mass, int *sim_fp_bits_count);


#endif // __bingo_core_c_h___
