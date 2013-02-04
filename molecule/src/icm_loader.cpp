/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include "molecule/icm_loader.h"
#include "molecule/icm_saver.h"
#include "base_cpp/scanner.h"
#include "molecule/cmf_loader.h"
#include "molecule/molecule.h"
#include "molecule/icm_common.h"

using namespace indigo;

IMPL_ERROR(IcmLoader, "ICM loader");

IcmLoader::IcmLoader (Scanner &scanner) : _scanner(scanner)
{
}

void IcmLoader::loadMolecule (Molecule &mol)
{
   char id[3];

   _scanner.readCharsFix(3, id);

   int version = -1;
   if (strncmp(id, IcmSaver::VERSION2, 3) == 0)
      version = 2;
   else if (strncmp(id, IcmSaver::VERSION1, 3) == 0)
      version = 1;
   else
      throw Error("expected '%s' or '%s', got %.*s. Resave your molecule with new format.", 
         IcmSaver::VERSION1, IcmSaver::VERSION2, 3, id);

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
         mol.stereocenters.markBonds();
         mol.allene_stereo.markBonds();
      }
   }
}
