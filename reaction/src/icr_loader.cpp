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

#include "reaction/icr_loader.h"

#include "reaction/icr_saver.h"
#include "base_cpp/scanner.h"
#include "reaction/crf_loader.h"
#include "reaction/reaction.h"
#include "molecule/icm_common.h"

using namespace indigo;

IMPL_ERROR(IcrLoader, "ICR loader");

IcrLoader::IcrLoader (Scanner &scanner) : _scanner(scanner)
{
}

void IcrLoader::loadReaction (Reaction &reaction)
{
   char id[3];

   _scanner.readCharsFix(3, id);

   int version = -1;
   if (strncmp(id, IcrSaver::VERSION2, 3) == 0)
      version = 2;
   else if (strncmp(id, IcrSaver::VERSION1, 3) == 0)
      version = 1;
   else
      throw Error("expected '%s' or '%s', got %.*s. Resave your reaction with new format.", 
         IcrSaver::VERSION1, IcrSaver::VERSION2, 3, id);

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
