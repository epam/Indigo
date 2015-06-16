/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __indigo_inchi__
#define __indigo_inchi__

#include "indigo.h"

CEXPORT const char* indigoInchiVersion ();

CEXPORT int indigoInchiResetOptions ();

CEXPORT int indigoInchiLoadMolecule (const char *inchi_string);

CEXPORT const char* indigoInchiGetInchi (int molecule);

CEXPORT const char* indigoInchiGetInchiKey (const char *inchi_string);

CEXPORT const char* indigoInchiGetWarning ();

CEXPORT const char* indigoInchiGetLog ();

CEXPORT const char* indigoInchiGetAuxInfo ();

#endif // __indigo_inchi__
