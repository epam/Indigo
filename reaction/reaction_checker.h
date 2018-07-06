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

#ifndef __reaction_checker__
#define __reaction_checker__

#include "base_cpp/exception.h"
#include "base_cpp/array.h"

namespace indigo {

class Output;
class Reaction;
class BaseReaction;
class QueryReaction;
class StructureChecker;

class ReactionChecker
{
public:

   ReactionChecker(Output& output);
   ~ReactionChecker();

   void checkBaseReaction(BaseReaction& reaction);
   void checkReaction(Reaction& reaction);
   void checkQueryReaction(QueryReaction& reaction);

   void setCheckTypes (const char *params);

   DECL_ERROR;

protected:

   void _checkReaction();
   void _checkReactionComponent(StructureChecker &checker, int idx);
   void _addCheckResult(StructureChecker &checker, int idx);

   BaseReaction  *_brxn;
   QueryReaction *_qrxn;
   Reaction      *_rxn;

   Output &_output;

   Array<char> _check_types;
};

}

#endif
