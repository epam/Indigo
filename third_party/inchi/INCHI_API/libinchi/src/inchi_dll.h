/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
 * December 15, 2020
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the
 * International Chemical Identifier (InChI)
 * Copyright (C) IUPAC and InChI Trust
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0,
 * or any later version.
 *
 * Please note that this library is distributed WITHOUT ANY WARRANTIES
 * whatsoever, whether expressed or implied.
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 *
 * You should have received a copy of the IUPAC/InChI Trust InChI
 * Licence No. 1.0 with this library; if not, please e-mail:
 *
 * info@inchi-trust.org
 *
 */


#ifndef __INCHI_DLL_H__
#define __INCHI_DLL_H__


#define INCHI_MAX_NUM_ARG 32

int parse_options_string( char *cmd,
                          const char *argv[],
                          int maxargs );
void produce_generation_output( inchi_Output *out,
                                STRUCT_DATA *sd,
                                INPUT_PARMS *ip,
                                INCHI_IOSTREAM *log_file,
                                INCHI_IOSTREAM *out_file );
void copy_corrected_log_tail( inchi_Output *out,
                              INCHI_IOSTREAM *log_file );

int input_erroneously_contains_pseudoatoms( inchi_Input *inp,
                                            inchi_Output *out );

#endif /* __INCHI_DLL_H__ */
