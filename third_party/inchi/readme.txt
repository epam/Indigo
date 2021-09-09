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


This directory contains InChI Software source codes.
It also contains examples of InChI API usage, for C 
('inchi_main', 'mol2inchi', 'test_ixa'); see projects 
for MS Visual Studio 2015 in 'vc14' and for gcc/Linux 
in 'gcc' subdirs) and Python 3 ('python_sample'). 

Also supplied are InChI API Library source codes and 
related projects/makefiles.

For more details, please refer to respective sub-directories.

The general layout is as follows.


INCHI-1-SRC/INCHI_BASE
    src                     C source files of common codebase
                            used by both InChI Library and
                            inchi-1 executable

INCHI-1-SRC/INCHI_EXE
    inchi-1/src             C source files specific for 
                            inchi-1 executable

INCHI-1-SRC/INCHI_API               
    libinchi/src            C source files specific for 
                            InChI Software Library (API)

    demos/inchi_main/src    C source files specific for 
                            inchi_main demo

    demos/mol2inchi/src     C source files specific for 
                            mol2inchi  demo

    demos/test_ixa/src     C source files specific for 
                            test_ixa demo

    demos/python_sample    Python 3 source files specific for 
                            Python demo


The portion of this distribution, the files sha2.c and sha2.h
are Copyright (C) Brainspark B.V., and are distributed under 
the terms of the same  IUPAC/InChI-Trust Licence for the 
International Chemical Identifier (InChI) Software version 1.0.


The text of IUPAC/InChI-Trust Licence for the 
International Chemical Identifier (InChI) Software version 1.0
is included (the file LICENCE) in this distribution.
