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

#ifndef __bingo_index__
#define __bingo_index__

#include "base_cpp/array.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"

#include "core/bingo_error.h"

using namespace indigo;

class BingoContext;

namespace indigo
{
   class OsLock;
}

class BingoIndex
{
public:
   BingoIndex ()  { _context = 0; skip_calculate_fp = false; }
   virtual ~BingoIndex () {}
   void init (BingoContext &context)    { _context = &context; };

   virtual void prepare (Scanner &scanner, Output &output, OsLock *lock_for_exclusive_access) = 0;

   bool skip_calculate_fp;

protected:
   BingoContext *_context;

private:
   BingoIndex (const BingoIndex &); // no implicit copy
};
#endif
