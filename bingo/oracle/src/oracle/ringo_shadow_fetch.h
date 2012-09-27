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

#ifndef __ringo_shadow_fetch__
#define __ringo_shadow_fetch__

#include "base_cpp/auto_ptr.h"
#include "oracle/bingo_fetch_engine.h"

using namespace indigo;

class RingoFetchContext;

class RingoShadowFetch : public BingoFetchEngine
{
public:
   RingoShadowFetch (RingoFetchContext &context);
   virtual ~RingoShadowFetch ();

   virtual float calcSelectivity (OracleEnv &env, int total_count);
   virtual void  fetch (OracleEnv &env, int maxrows);
   virtual bool  end   ();
   virtual int getIOCost (OracleEnv &env, float selectivity);
   virtual int getTotalCount (OracleEnv &env);

   virtual bool getLastRowid (OraRowidText &id);

   int countOracleBlocks (OracleEnv &env);

   void prepareNonSubstructure (OracleEnv &env);
   void prepareExact (OracleEnv &env, int right_part);

   DECL_ERROR;
protected:
   enum
   {
      _NON_SUBSTRUCTURE = 1,
      _EXACT = 2
   };

   RingoFetchContext       &_context;

   Array<char>              _table_name;
   int                      _total_count;
   Array<char>              _counting_select;
   int                      _processed_rows;
   bool                     _end;
   AutoPtr<OracleEnv>       _env;
   AutoPtr<OracleStatement> _statement;
   AutoPtr<OracleLOB>       _lob_crf;
   bool                     _executed;
   int                      _fetch_type;
   OraRowidText             _rowid;
   int                      _right_part;
};

#endif
