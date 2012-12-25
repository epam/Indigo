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

#ifndef __reaction_fingerprint__

#include "base_cpp/tlscont.h"

namespace indigo {

class BaseReaction;
struct MoleculeFingerprintParameters;

class ReactionFingerprintBuilder
{
public:
   ReactionFingerprintBuilder (BaseReaction &reaction, const MoleculeFingerprintParameters &parameters);

   bool query;
   bool skip_ord;
   bool skip_sim;
   bool skip_ext;

   void process ();
   
   byte * get ();
   byte * getSim ();

   void parseFingerprintType(const char *type, bool query);

   DECL_ERROR;

protected:
         BaseReaction                  &_reaction;
   const MoleculeFingerprintParameters &_parameters;
   
   TL_CP_DECL(Array<byte>, _fingerprint);

private:
   ReactionFingerprintBuilder (const ReactionFingerprintBuilder &); // no implicit copy
};

}

#endif
