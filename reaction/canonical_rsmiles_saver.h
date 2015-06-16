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

#ifndef __canonical_rsmiles_saver__
#define __canonical_rsmiles_saver__

#include "reaction/rsmiles_saver.h"

#include "base_cpp/exception.h"

namespace indigo {

class Output;
class BaseReaction;
class CanonicalSmilesSaver;
class Reaction;

class DLLEXPORT CanonicalRSmilesSaver : public RSmilesSaver
{
public:

   explicit CanonicalRSmilesSaver (Output &output);
   ~CanonicalRSmilesSaver ();

   void saveReaction(Reaction &react);

   DECL_ERROR;

protected:
   void _saveReaction();
   void _writeMolecule(int i, CanonicalSmilesSaver &saver);
};

}

#endif
