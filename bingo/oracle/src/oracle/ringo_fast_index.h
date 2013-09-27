/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __ringo_fast_index__
#define __ringo_fast_index__

#include "bingo_fetch_engine.h"
#include "bingo_fingerprints.h"

using namespace indigo;

class RingoFetchContext;

class RingoFastIndex : public BingoFetchEngine
{
public:
   explicit RingoFastIndex (RingoFetchContext &context);
   virtual ~RingoFastIndex ();

   void prepareSubstructure         (OracleEnv &env);

   virtual void fetch (OracleEnv &env, int maxrows);
   virtual bool end ();
   virtual float calcSelectivity (OracleEnv &env, int total_count);
   virtual int getIOCost (OracleEnv &env, float selectivity);
   
   virtual bool getLastRowid (OraRowidText &id);

   int getTotalCount (OracleEnv &env);

   DECL_ERROR;

protected:
   enum
   {
      _SUBSTRUCTURE = 1
   };

   RingoFetchContext &_context;

   int _fetch_type;
   int _cur_idx;
   int _matched;
   int _unmatched;

   int _last_id;

   BingoFingerprints::Screening _screening;

   void _match (OracleEnv &env, int idx);
   void _decompressRowid (const Array<char> &stored, OraRowidText &rid);
private:
   RingoFastIndex (const RingoFastIndex &); // noimplicitcopy
};


#endif

