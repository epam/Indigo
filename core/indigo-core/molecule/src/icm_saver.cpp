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

#include "molecule/icm_saver.h"

#include "base_cpp/output.h"
#include "molecule/cmf_saver.h"
#include "molecule/icm_common.h"

using namespace indigo;

const char* IcmSaver::VERSION2 = "IM2";
const char* IcmSaver::VERSION1 = "ICM";

IMPL_ERROR(IcmSaver, "ICM saver");

bool IcmSaver::checkVersion(const char* prefix)
{
    return strncmp(prefix, VERSION1, 3) == 0 || strncmp(prefix, VERSION2, 3) == 0;
}

IcmSaver::IcmSaver(Output& output) : _output(output)
{
    save_xyz = false;
    save_bond_dirs = false;
    save_highlighting = false;
    save_ordering = false;
}

void IcmSaver::saveMolecule(Molecule& mol)
{
    _output.writeString(VERSION2);

    int features = 0;

    if (save_xyz)
        features |= ICM_XYZ;

    if (save_bond_dirs)
        features |= ICM_BOND_DIRS;

    _output.writeChar(features);

    CmfSaver saver(_output);

    saver.save_bond_dirs = save_bond_dirs;
    saver.save_highlighting = save_highlighting;
    saver.save_mapping = save_ordering;

    saver.saveMolecule(mol);
    if (save_xyz)
        saver.saveXyz(_output);
}
