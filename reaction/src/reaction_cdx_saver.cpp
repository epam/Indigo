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

#include "reaction/reaction_cdx_saver.h"
#include "base_cpp/output.h"
#include "reaction/reaction.h"
#include "molecule/molecule_cdx_saver.h"

using namespace indigo;

IMPL_ERROR(ReactionCdxSaver, "reaction CDX saver");

ReactionCdxSaver::ReactionCdxSaver (Output &output) : _output(output)
{
}

ReactionCdxSaver::~ReactionCdxSaver ()
{
}

void ReactionCdxSaver::saveReaction (Reaction &rxn)
{
   MoleculeCdxSaver molsaver(_output);

   if (rxn.reactantsCount() > 0)
   {
   }
   if (rxn.productsCount() > 0)
   {
   }
   if (rxn.catalystCount() > 0)
   {
   }
}
