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

#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction_checker.h"
#include "base_cpp/output.h"
#include "molecule/structure_checker.h"

using namespace indigo;

IMPL_ERROR(ReactionChecker, "Reaction checker");

ReactionChecker::ReactionChecker(Output &output) : 
   _output(output)
{
}

ReactionChecker::~ReactionChecker()
{
}

void ReactionChecker::checkBaseReaction (BaseReaction& reaction)
{
   if (reaction.isQueryReaction())
      checkQueryReaction(reaction.asQueryReaction());
   else
      checkReaction(reaction.asReaction());
}

void ReactionChecker::checkReaction (Reaction &reaction)
{
   _rxn = &reaction;
   _brxn = &reaction;
   _qrxn = 0;
   _checkReaction();
}

void ReactionChecker::checkQueryReaction (QueryReaction &reaction)
{
   _qrxn = &reaction;
   _brxn = &reaction;
   _rxn = 0;
   _checkReaction();
}

void ReactionChecker::_checkReaction()
{
   int i;

   StructureChecker ch(_output);
   ch.parseCheckTypes(_check_types.ptr());

   _output.writeString("[");
  
   for (i = _brxn->reactantBegin(); i < _brxn->reactantEnd(); i = _brxn->reactantNext(i) )
   {
      _checkReactionComponent(ch, i);
   }


   for (i = _brxn->productBegin(); i < _brxn->productEnd(); i = _brxn->productNext(i))
   {
      _checkReactionComponent(ch, i);
   }


   for (i = _brxn->catalystBegin(); i < _brxn->catalystEnd(); i = _brxn->catalystNext(i))
   {
      _checkReactionComponent(ch, i);
   }

   _output.writeString("]");
}

void ReactionChecker::setCheckTypes (const char *params)
{
   _check_types.clear();
   if (params != 0)
      _check_types.readString(params, true);
}

void ReactionChecker::_checkReactionComponent (StructureChecker &ch, int idx)
{
   ch.clearCheckResult();
   if (_qrxn != 0)
   {
      ch.checkMolecule(_brxn->getBaseMolecule(idx), true);
   }
   else
   {
      ch.checkMolecule(_brxn->getBaseMolecule(idx), false);
   }

   if (ch.check_result != 0)
      _addCheckResult (ch, idx);

}

void ReactionChecker::_addCheckResult (StructureChecker &ch, int idx)
{
   if (_output.tell() > 2)
      _output.printf(",");

   _output.printf("{\"id\":%d, \"result\":", idx + 1);

   ch.buildCheckResult();

   _output.printf("}");
}

