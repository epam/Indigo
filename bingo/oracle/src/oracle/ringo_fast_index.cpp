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

#include "oracle/ringo_fast_index.h"

#include "base_cpp/profiling.h"
#include "core/ringo_matchers.h"
#include "oracle/ringo_oracle.h"
#include "oracle/ringo_fetch_context.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/rowid_loader.h"

RingoFastIndex::RingoFastIndex (RingoFetchContext &context) :
_context(context)
{
   _fetch_type = 0;
   _last_id = -1;
}

RingoFastIndex::~RingoFastIndex ()
{
}

void RingoFastIndex::_decompressRowid (const Array<char> &stored, OraRowidText &rid)
{
   BufferScanner scanner(stored.ptr() + 2, stored[1]);

   RowIDLoader loader(_context.context().context().rid_dict, scanner);
   QS_DEF(Array<char>, rowid);

   loader.loadRowID(rowid);

   if (rowid.size() != 18)
      throw Error("rowid size=%d?", rowid.size());

   memcpy(rid.ptr(), rowid.ptr(), 18);
   rid.ptr()[18] = 0;
}

void RingoFastIndex::_match (OracleEnv &env, int idx)
{
   _last_id = idx;

   BingoStorage &storage = this->_context.context().context().storage;
   QS_DEF(Array<char>, stored);
   
   storage.get(idx, stored);

   if (stored[0] != 0)
      return; // reaction was removed from index

   BufferScanner scanner(stored);

   scanner.skip(1); // skip the deletion mark
   scanner.skip(scanner.readByte()); // skip the compessed rowid
   
   profTimerStart(tall, "match");
   bool res = _context.substructure.matchBinary(scanner);
   profTimerStop(tall);
   
   if (res)
   {
      OraRowidText & rid = matched.at(matched.add());

      _decompressRowid(stored, rid);
      profIncTimer("match.found", profTimerGetTime(tall));
      _matched++;
   }
   else
   {
      profIncTimer("match.not_found", profTimerGetTime(tall));
      _unmatched++;
   }
}


void RingoFastIndex::fetch (OracleEnv &env, int max_matches)
{
   env.dbgPrintf("requested %d hits\n", max_matches);
   matched.clear();
   
   BingoFingerprints &fingerprints = _context.context().fingerprints;
   
   if (_fetch_type == _SUBSTRUCTURE)
   {
      if (fingerprints.ableToScreen(_screening))
      {
         while (matched.size() < max_matches)
         {
            if (_screening.passed.size() > 0)
            {
               int idx = _screening.passed.begin();
               _match(env, _screening.passed.at(idx));
               _screening.passed.remove(idx);
               continue;
            }

            if (fingerprints.screenPart_Init(env, _screening))
            {
               while (fingerprints.screenPart_Next(env, _screening))
                  ;
               fingerprints.screenPart_End(env, _screening);
               _unmatched += _screening.block->used - _screening.passed.size();
            }
            else
            {
               env.dbgPrintfTS("screening ended\n");
               break;
            }

            _screening.items_passed += _screening.passed.size();
            
            env.dbgPrintfTS("%d reactions passed screening\n", _screening.passed.size());
         } 
      }
      else
      {
         while (matched.size() < max_matches && _cur_idx < _context.context().context().storage.count())
            _match(env, _cur_idx++);
         
         env.dbgPrintfTS("%d reactions matched\n", matched.size());
      }
   }
   else
      throw Error("unexpected fetch type: %d", _fetch_type);
}

void RingoFastIndex::prepareSubstructure (OracleEnv &env)
{
   env.dbgPrintf("preparing fastindex for reaction substructure search\n");
   
   _context.context().context().storage.validate(env);
   _context.context().fingerprints.validate(env);
   _context.context().fingerprints.screenInit(_context.substructure.getQueryFingerprint(), _screening);
   _fetch_type = _SUBSTRUCTURE;
   _cur_idx = 0;
   _matched = 0;
   _unmatched = 0;
}

float RingoFastIndex::calcSelectivity (OracleEnv &env, int total_count)
{
   if (_matched + _unmatched == 0)
      throw Error("calcSelectivity() called before fetch()");

   BingoFingerprints &fingerprints = _context.context().fingerprints;

   if (fingerprints.ableToScreen(_screening))
   {
      return (float)_matched * _screening.items_passed / (_screening.items_read * (_matched + _unmatched));
   }
   else
   {
      if (_matched == 0)
         return 0;
      return (float)_matched / (_matched + _unmatched);
   }
}

int RingoFastIndex::getIOCost (OracleEnv &env, float selectivity)
{
   BingoFingerprints &fingerprints = _context.context().fingerprints;

   int blocks = fingerprints.countOracleBlocks(env);
   float ratio = fingerprints.queryOnesRatio(_screening);

   return (int)(blocks * ratio);
}

bool RingoFastIndex::getLastRowid (OraRowidText &id)
{
   if (_last_id < 0)
      return false;

   BingoStorage &storage = this->_context.context().context().storage;
   QS_DEF(Array<char>, stored);
   
   storage.get(_last_id, stored);
   _decompressRowid(stored, id);
   return true;
}

int RingoFastIndex::getTotalCount (OracleEnv &env)
{
   return _context.context().fingerprints.getTotalCount(env);
}

bool RingoFastIndex::end ()
{
   return false;
}
