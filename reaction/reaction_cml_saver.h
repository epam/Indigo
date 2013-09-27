/****************************************************************************
 * Copyright (C) 2011 GGA Software Services LLC
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

#ifndef __reaction_cml_saver__
#define __reaction_cml_saver__

#include "base_cpp/exception.h"

namespace indigo {

class Output;
class Reaction;

class ReactionCmlSaver
{
public:
   explicit ReactionCmlSaver (Output &output);
   ~ReactionCmlSaver ();

   void saveReaction (Reaction &rxn);
   bool skip_cml_tag; // skips <?xml> and <cml> tags

   DECL_ERROR;

protected:
   Reaction *_rxn;
   Output   &_output;

private:
   ReactionCmlSaver (const ReactionCmlSaver &); // no implicit copy
};

}

#endif
