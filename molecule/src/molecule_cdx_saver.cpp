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

#include "base_cpp/output.h"
#include "molecule/molecule_cdx_saver.h"
#include "molecule/molecule.h"
#include "molecule/elements.h"
#include "base_cpp/locale_guard.h"

using namespace indigo;

IMPL_ERROR(MoleculeCdxSaver, "molecule CDX saver");

MoleculeCdxSaver::MoleculeCdxSaver (Output &output) : _output(output)
{
}

void MoleculeCdxSaver::saveMolecule (Molecule &mol)
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
