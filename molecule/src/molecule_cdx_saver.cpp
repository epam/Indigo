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

#include "molecule/molecule_cdx_saver.h"

using namespace indigo;

IMPL_ERROR(MoleculeCdxSaver, "molecule CDX saver");

MoleculeCdxSaver::MoleculeCdxSaver(Output& output) : _output(output)
{
}

void MoleculeCdxSaver::saveMolecule(Molecule& mol)
{
    throw Error("saveMolecule is not implemented for CDX saver");
    //   LocaleGuard locale_guard;
    //
    //   _mol = &mol;
    //
    //
    //   if (_mol->name.ptr() != 0)
    //   {
    //   }
    //
    //   bool have_hyz = _mol->have_xyz;
    //   bool have_z = BaseMolecule::hasZCoord(*_mol);
    //
    //   if (_mol->vertexCount() > 0)
    //   {
    //   }
    //
    //   if (_mol->edgeCount() > 0)
    //   {
    //   }
}
