/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.04
 * September 9, 2011
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST. Modifications and additions by IUPAC 
 * and the InChI Trust.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the 
 * International Chemical Identifier (InChI) Software version 1.04
 * Copyright (C) IUPAC and InChI Trust Limited
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0, 
 * or any later version.
 * 
 * Please note that this library is distributed WITHOUT ANY WARRANTIES 
 * whatsoever, whether expressed or implied.  See the IUPAC/InChI Trust 
 * Licence for the International Chemical Identifier (InChI) Software 
 * version 1.04, October 2011 ("IUPAC/InChI-Trust InChI Licence No.1.0") 
 * for more details.
 * 
 * You should have received a copy of the IUPAC/InChI Trust InChI 
 * Licence No. 1.0 with this library; if not, please write to:
 * 
 * The InChI Trust
 * c/o FIZ CHEMIE Berlin
 *
 * Franklinstrasse 11
 * 10587 Berlin
 * GERMANY
 *
 * or email to: ulrich@inchi-trust.org.
 * 
 */



This directory contains code, VC++ projects and gcc makefiles of
InChI generation library with API (Win32 - libinchi.dll; Linux - libinchi.so) 
and demo applications which call libinchi.

The library produces both standard and non-standard InChI/InChIKey. 
				
The library does not provide any support for graphic user interface (GUI). 
It is not designed to work in a multithreaded environment and should be called 
from only one thread at a time.

Besides the InChI library itself, this directory contains examples of using 
previously available and newly added InChI software library functionality 
(inchi_main and make_inchi.py). Note that the demo programs are samples 
which are not supposed to be used for the production.


=========
  FILES 
=========

readme.txt        This file


inchi_dll         SUB-DIRECTORY
                  Contains InChI Library source code 


inchi_main        SUB-DIRECTORY
                  Contains ANSI-C demo application source to call InChI Library 
                  libinchi.dll under Microsoft Windows or libinchi.so 
                  under Linux or Unix. 


vc9               SUB-DIRECTORY
    
    inchi_dll         SUB-DIRECTORY
                      Contains MS Visual C++ 2008 project to build dynamically 
                      linked library libinchi.dll under Windows. 
    inchi_main        SUB-DIRECTORY
                      Contains MS Visual C++ 2008 project to build both 
                      dynamically linked library libinchi.dll and the testing 
                      application inchi_main.exe under MS Windows (both 
                      library and executable are placed into sub-directory
                      vc9/inchi_dll/Release).


gcc_so_makefile   SUB-DIRECTORY
                  Contains  a gcc makefile for INCHI_MAIN + INCHI_DLL code 
                  to create a InChI library as a shared object (Linux)  
                  or dll (Windows) dynamically linked to the main program

    result            SUB-DIRECTORY
                      Contains shared object libinchi.so.1.04.00.gz 
                      and demo application inchi_main.gz (for Linux)


python_sample         SUB-DIRECTORY
                      Contains Python demo application calling 
                      InChI Library functions.



Notes on inchi_main demo application
------------------------------------

Defining CREATE_INCHI_STEP_BY_STEP in e_mode.h makes program use the 
modularized interface to InChI generation process. Modularized interface 
is used by default; commenting out the line containing the #define makes 
the program use software version 1.01 ("classic") interface to InChI 
generation process. 

If the demo application is compiled with CREATE_INCHI_STEP_BY_STEP option, 
an additional defining of OUTPUT_NORMALIZATION_DATA in e_mode.h makes the 
program output the intermediate (normalization) data into the log file. 
The related data structures are described in header files inchi_api.h; 
their use is exemplified in e_ichimain_a.c file. 
Note that including the output of the intermediate (normalization) data may 
produce a very long log file.

Please notice that /D "BUILD_LINK_AS_DLL" Visual C ++ compiler option is 
necessary to create and link the dll and the testing executable with 
Microsoft Visual C++ under Win32.



=========
  LINKS
=========

IUPAC                   http://www.iupac.org/inchi
InChI Trust             http://www.inchi-trust.org                                      
InChI discussion group  https://lists.sourceforge.net/lists/listinfo/inchi-discuss
