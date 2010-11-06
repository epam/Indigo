/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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
#include "base_cpp/scanner.h"
#include "reaction/crf_loader.h"
#include "reaction/reaction.h"

using namespace indigo;

IcrLoader::IcrLoader (Scanner &scanner) : _scanner(scanner)
{
}

void IcrLoader::loadReaction (Reaction &reaction)
{
   char id[3];

   _scanner.readCharsFix(3, id);
   if (strncmp(id, "ICR", 3) != 0)
      throw Error("expected 'ICR', got %.*s", 3, id);

   bool have_xyz = (_scanner.readChar() == 1);

   CrfLoader loader(_scanner);

   if (have_xyz)
      loader.xyz_scanner = &_scanner;

   loader.loadReaction(reaction);

   if (have_xyz)
      reaction.markStereocenterBonds();
}
