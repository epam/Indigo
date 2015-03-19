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

#ifndef __reaction_cdx_saver__
#define __reaction_cdx_saver__

#include "base_cpp/exception.h"

namespace indigo {

class Output;
class Reaction;

class ReactionCdxSaver
{
public:
   explicit ReactionCdxSaver (Output &output);
   ~ReactionCdxSaver ();

   void saveReaction (Reaction &rxn);

   DECL_ERROR;

protected:
   Reaction *_rxn;
   Output   &_output;

private:
   ReactionCdxSaver (const ReactionCdxSaver &); // no implicit copy
};

}

#endif
