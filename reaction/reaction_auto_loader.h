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

#ifndef __reaction_auto_loader__
#define __reaction_auto_loader__

#include "base_cpp/array.h"

namespace indigo {

class Scanner;
class BaseReaction;
class Reaction;
class QueryReaction;

class DLLEXPORT ReactionAutoLoader
{
public:
   ReactionAutoLoader (Scanner &scanner);
   ReactionAutoLoader (const Array<char> &arr);
   ReactionAutoLoader (const char *);

   ~ReactionAutoLoader ();

   void loadReaction (Reaction &reaction);
   void loadQueryReaction (QueryReaction &reaction);

   bool treat_x_as_pseudoatom;
   bool ignore_closing_bond_direction_mismatch;
   bool ignore_stereocenter_errors;
   bool ignore_noncritical_query_features;

   DECL_ERROR;

protected:
   Scanner *_scanner;
   bool     _own_scanner;

   void _init ();
   void _loadReaction (BaseReaction &reaction, bool query);
   bool _isSingleLine ();
   
private:
   ReactionAutoLoader (const ReactionAutoLoader &); // no implicit copy
};

}

#endif
