/****************************************************************************
 * Copyright (C) 2010-2015 GGA Software Services LLC
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

#include "indigo_iupac_inchi.h"

CEXPORT const char* indigoInchiVersion_plugin()
{
   return indigoInchiVersion();
}

CEXPORT int indigoInchiResetOptions_plugin(void)
{
   return indigoInchiResetOptions();
}

CEXPORT int indigoInchiLoadMolecule_plugin(const char *inchi_string)
{
   return indigoInchiLoadMolecule(inchi_string);
}

CEXPORT const char* indigoInchiGetInchi_plugin(int molecule)
{
   return indigoInchiGetInchi(molecule);
}

CEXPORT const char* indigoInchiGetInchiKey_plugin(const char *inchi_string)
{
   return indigoInchiGetInchiKey(inchi_string);
}

CEXPORT const char* indigoInchiGetWarning_plugin()
{
   return indigoInchiGetWarning();
}

CEXPORT const char* indigoInchiGetLog_plugin()
{
   return indigoInchiGetLog();
}

CEXPORT const char* indigoInchiGetAuxInfo_plugin()
{
   return indigoInchiGetAuxInfo();
}
