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

#ifndef __ringo_index__
#define __ringo_index__

#include "base_cpp/array.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"

#include "core/bingo_index.h"

using namespace indigo;

class BingoContext;

class RingoIndex : public BingoIndex
{
public:
   virtual void prepare (Scanner &rxnfile, Output &fi_output, OsLock *lock_for_exclusive_access);
   
   const byte * getFingerprint ();
   const Array<char> & getCrf ();
   dword getHash ();
   const char * getHashStr ();

   void clear ();

private:
   Array<byte> _fp;
   Array<char> _crf;
   dword       _hash;
   Array<char> _hash_str;
};
#endif
