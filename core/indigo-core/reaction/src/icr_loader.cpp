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

#include "reaction/icr_loader.h"

#include "base_cpp/scanner.h"
#include "molecule/icm_common.h"
#include "reaction/crf_loader.h"
#include "reaction/icr_saver.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(IcrLoader, "ICR loader");

IcrLoader::IcrLoader(Scanner& scanner) : _scanner(scanner)
{
}

void IcrLoader::loadReaction(Reaction& reaction)
{
    char id[3];

    _scanner.readCharsFix(3, id);

    int version = -1;
    if (strncmp(id, IcrSaver::VERSION2, 3) == 0)
        version = 2;
    else if (strncmp(id, IcrSaver::VERSION1, 3) == 0)
        version = 1;
    else
        throw Error("expected '%s' or '%s', got %.*s. Resave your reaction with new format.", IcrSaver::VERSION1, IcrSaver::VERSION2, 3, id);

    char bits = _scanner.readChar();

    bool have_xyz = ((bits & ICM_XYZ) != 0);
    bool have_bond_dirs = ((bits & ICM_BOND_DIRS) != 0);

    CrfLoader loader(_scanner);

    if (have_xyz)
        loader.xyz_scanner = &_scanner;

    loader.version = version;
    loader.loadReaction(reaction);

    if (have_xyz)
        if (!have_bond_dirs)
            reaction.markStereocenterBonds();
}
