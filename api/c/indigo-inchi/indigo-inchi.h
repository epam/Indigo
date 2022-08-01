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

#ifndef __indigo_inchi__
#define __indigo_inchi__

#include "indigo.h"

CEXPORT const char* indigoInchiVersion();

CEXPORT int indigoInchiInit(qword id);
CEXPORT int indigoInchiDispose(qword id);

CEXPORT int indigoInchiResetOptions();

CEXPORT int indigoInchiLoadMolecule(const char* inchi_string);

CEXPORT const char* indigoInchiGetInchi(int molecule);

CEXPORT const char* indigoInchiGetInchiKey(const char* inchi_string);

CEXPORT const char* indigoInchiGetWarning();

CEXPORT const char* indigoInchiGetLog();

CEXPORT const char* indigoInchiGetAuxInfo();

#endif // __indigo_inchi__
