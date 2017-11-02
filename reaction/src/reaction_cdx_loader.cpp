/****************************************************************************
 * Copyright (C) 2015 EPAM Systems
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

#include "reaction/reaction_cdx_loader.h"
#include "reaction/reaction.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule_cdx_loader.h"

using namespace indigo;

IMPL_ERROR(ReactionCdxLoader, "reaction CDX loader");

ReactionCdxLoader::ReactionCdxLoader (Scanner &scanner) : _scanner(scanner)
{
   ignore_bad_valence = false;
}

ReactionCdxLoader::~ReactionCdxLoader ()
{
}

void ReactionCdxLoader::loadReaction (Reaction &rxn)
{
   rxn.clear();

   QS_DEF(Array<char>, buf);
   _scanner.readAll(buf);
   buf.push(0);
}
