/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#ifndef __ringo_index__
#define __ringo_index__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"

#include "core/bingo_error.h"

using namespace indigo;

namespace indigo
{
   class Reaction;
}

class BingoContext;

class RingoIndex
{
public:

   RingoIndex (BingoContext &context);

   void prepare (Scanner &rxnfile, Output &fi_output);
   
   const byte * getFingerprint ();

   const Array<char> & getCrf ();

protected:
   BingoContext &_context;

   TL_CP_DECL(Array<byte>, _fp);
   TL_CP_DECL(Array<char>, _crf);
private:
   RingoIndex (const RingoIndex &); // no implicit copy
};
#endif
