/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __bingo_fetch_engine__
#define __bingo_fetch_engine__

#include "oracle/ora_wrap.h"
#include "base_cpp/list.h"
#include "base_cpp/tlscont.h"

using namespace indigo;

class BingoFetchEngine
{
public:
   explicit BingoFetchEngine () : TL_CP_GET(matched)
   {
      matched.clear();
   }
   
   virtual ~BingoFetchEngine () {}

   virtual float calcSelectivity (OracleEnv &env, int total_count) = 0;
   virtual void  fetch (OracleEnv &env, int maxrows) = 0;
   virtual bool  end () = 0;
   virtual int getIOCost (OracleEnv &env, float selectivity) = 0;

   TL_CP_DECL(List<OraRowidText>, matched);
};

#endif
