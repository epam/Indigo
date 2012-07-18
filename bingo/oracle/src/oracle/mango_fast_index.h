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

#ifndef __mango_fast_index__
#define __mango_fast_index__

#include "oracle/ora_wrap.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"
#include "oracle/bingo_fetch_engine.h"
#include "oracle/bingo_fingerprints.h"

using namespace indigo;

class MangoFetchContext;

class MangoFastIndex : public BingoFetchEngine
{
public:
   MangoFastIndex (MangoFetchContext &context);
   virtual ~MangoFastIndex ();

   void prepareSubstructure         (OracleEnv &env);
   void prepareSimilarity           (OracleEnv &env);
   void prepareTautomerSubstructure (OracleEnv &env);

   virtual void fetch (OracleEnv &env, int maxrows);
   virtual bool end ();
   virtual float calcSelectivity (OracleEnv &env, int total_count);
   virtual int getIOCost (OracleEnv &env, float selectivity);

   virtual bool getLastRowid (OraRowidText &id);

   DEF_ERROR("mango fast fetch");

protected:
   
   enum
   {
      _SUBSTRUCTURE = 1,
      _SIMILARITY = 2,
      _TAUTOMER_SUBSTRUCTURE = 3
   };

   MangoFetchContext &_context;

   int _fetch_type;

   int _cur_idx;
   int _matched;
   int _unmatched;

   int _last_id;

   BingoFingerprints::Screening _screening;

   bool _loadCoords (OracleEnv &env, const char *rowid, Array<char> &coords);
   void _match (OracleEnv &env, int idx);
   int  _countOnes (int idx);

   void _decompressRowid (const Array<char> &stored, OraRowidText &rid);

   void _fetchSubstructure (OracleEnv &env, int maxrows);
   void _fetchSimilarity (OracleEnv &env, int maxrows);

private:
   MangoFastIndex (const MangoFastIndex &); // noimplicitcopy
};

#endif
