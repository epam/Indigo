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

#include "molecule/icm_loader.h"

#include "base_cpp/scanner.h"
#include "molecule/cmf_loader.h"
#include "molecule/icm_common.h"
#include "molecule/icm_saver.h"
#include "molecule/molecule.h"

using namespace indigo;

IMPL_ERROR(IcmLoader, "ICM loader");

IcmLoader::IcmLoader(Scanner& scanner) : _scanner(scanner)
{
}

void IcmLoader::loadMolecule(Molecule& mol)
{
    char id[3];

    _scanner.readCharsFix(3, id);

    int version = -1;
    if (strncmp(id, IcmSaver::VERSION2, 3) == 0)
        version = 2;
    else if (strncmp(id, IcmSaver::VERSION1, 3) == 0)
        version = 1;
    else
        throw Error("expected '%s' or '%s', got %.*s. Resave your molecule with new format.", IcmSaver::VERSION1, IcmSaver::VERSION2, 3, id);

    char bits = _scanner.readChar();

    bool have_xyz = ((bits & ICM_XYZ) != 0);
    bool have_bond_dirs = ((bits & ICM_BOND_DIRS) != 0);

    CmfLoader loader(_scanner);

    loader.version = version;
    loader.loadMolecule(mol);

    if (have_xyz)
    {
        loader.loadXyz(_scanner);
        if (!have_bond_dirs)
        {
            mol.markBondsStereocenters();
            mol.markBondsAlleneStereo();
        }
    }
}
