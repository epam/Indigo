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

#ifndef __mango_shadow_fetch__
#define __mango_shadow_fetch__

#include "base_cpp/auto_ptr.h"
#include "oracle/bingo_fetch_engine.h"
#include "oracle/mango_shadow_table.h"

class MangoFetchContext;

class MangoShadowFetch : public BingoFetchEngine
{
public:
   MangoShadowFetch (MangoFetchContext &context);
   virtual ~MangoShadowFetch ();

   virtual float calcSelectivity (OracleEnv &env, int total_count);
   virtual void  fetch (OracleEnv &env, int maxrows);
   virtual bool  end   ();
   virtual int getIOCost (OracleEnv &env, float selectivity);
   virtual int getTotalCount (OracleEnv &env);

   int countOracleBlocks (OracleEnv &env);

   void prepareNonSubstructure (OracleEnv &env);
   void prepareNonTautomerSubstructure (OracleEnv &env);
   void prepareTautomer (OracleEnv &env, int right_part);
   void prepareExact (OracleEnv &env, int right_part);
   void prepareGross (OracleEnv &env, int right_part);
   void prepareMass (OracleEnv &env);

   DEF_ERROR("mango shadow fetch");
protected:
   
   enum
   {
      _NON_SUBSTRUCTURE = 1,
      _NON_TAUTOMER_SUBSTRUCTURE = 2,
      _TAUTOMER = 3,
      _EXACT = 4,
      _GROSS = 5,
      _MASS = 6
   };

   void _prepareExactQueryStrings (Array<char> &table_copies, Array<char> &where_clause);

   MangoFetchContext       &_context;

   Array<char>              _table_name;
   Array<char>              _components_table_name;
   int                      _total_count;
   Array<char>              _counting_select;
   int                      _processed_rows;
   bool                     _end;
   AutoPtr<OracleEnv>       _env;
   AutoPtr<OracleStatement> _statement;
   AutoPtr<OracleLOB>       _lob_cmf;
   AutoPtr<OracleLOB>       _lob_xyz;
   bool                     _executed;
   int                      _fetch_type;
   char                     _gross[1024];
   OraRowidText             _rowid;
   bool                     _need_xyz;
   int                      _right_part;

};

#endif
