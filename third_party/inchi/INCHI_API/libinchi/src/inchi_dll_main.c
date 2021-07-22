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


/* inchi_dll_main.c : Defines the entry point for the DLL application. */

#include "../../../INCHI_BASE/src/mode.h"

#if defined(_WIN32) && defined(_USRDLL) && defined(_DEBUG) && !(defined(__STDC__) && __STDC__ == 1)
#include "inchi_dll_main.h"


/****************************************************************************/
int INCHI_DLLMAIN_TYPE DllMain( HANDLE hModule,
                                DWORD  ul_reason_for_call,
                                LPVOID lpReserved )
{
    return TRUE;
}
#else
int dummy_inchi_dll_main = 0;  /* avoid empty module to keep C compiler happy */
#endif
