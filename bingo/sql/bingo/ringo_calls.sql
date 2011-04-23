-- Copyright (C) 2009-2011 GGA Software Services LLC
-- 
-- This file is part of Indigo toolkit.
-- 
-- This file may be distributed and/or modified under the terms of the
-- GNU General Public License version 3 as published by the Free Software
-- Foundation and appearing in the file LICENSE.GPL included in the
-- packaging of this file.
-- 
-- This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
-- WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

set verify off
spool ringo_calls;

create or replace function AutoAAM_clob (target in CLOB, params in VARCHAR2) return CLOB
  AS language C name "oraRingoAAM" library bingolib
  with context parameters(context, target, target indicator short,
                          params, params indicator short,
                          return indicator short, return OCILobLocator);
/
create or replace function AutoAAM_blob (target in BLOB, params in VARCHAR2) return CLOB
  AS language C name "oraRingoAAM" library bingolib
  with context parameters(context, target, target indicator short,
                          params, params indicator short,
                          return indicator short, return OCILobLocator);
/
create or replace function RSub_clob (context_id in binary_integer,
                                   target in CLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraRingoSub" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short,
                          params, params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function RSub_blob (context_id in binary_integer,
                                   target in BLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraRingoSub" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short,
                          params, params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function RSubHi_clob (context_id in binary_integer,
                                     target in CLOB, query in CLOB, params in VARCHAR2) return CLOB
  AS language C name "oraRingoSubHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short,
                          params, params indicator short,
                          return indicator short, return OCILobLocator);
/
create or replace function RSubHi_blob (context_id in binary_integer,
                                     target in BLOB, query in CLOB, params in VARCHAR2) return CLOB
  AS language C name "oraRingoSubHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short,
                          params, params indicator short,
                          return indicator short, return OCILobLocator);
/
create or replace function RExact_clob (context_id in binary_integer,
  target in CLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraRingoExact" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function RExact_blob (context_id in binary_integer,
  target in BLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraRingoExact" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCINumber);
/
create or replace procedure ringoCreateIndex (context_id in binary_integer, params in varchar2)
  AS language C name "oraRingoCreateIndex" library bingolib
  with context parameters(context, context_id, params, params indicator short);
/
create or replace procedure ringoDropIndex (context_id in binary_integer)
  AS language C name "oraRingoDropIndex" library bingolib
  with context parameters(context, context_id);
/
create or replace procedure ringoTruncateIndex (context_id in binary_integer)
  AS language C name "oraRingoTruncateIndex" library bingolib
  with context parameters(context, context_id);
/
create or replace procedure ringoIndexInsert_clob (context_id in binary_integer, rid in VARCHAR2, item in CLOB)
  AS language C name "oraRingoIndexInsert" library bingolib
  with context parameters(context, context_id,
                          rid, rid indicator short,
                          item, item indicator short);
/
create or replace procedure ringoIndexInsert_blob (context_id in binary_integer, rid in VARCHAR2, item in BLOB)
  AS language C name "oraRingoIndexInsert" library bingolib
  with context parameters(context, context_id,
                          rid, rid indicator short,
                          item, item indicator short);
/
create or replace procedure ringoIndexDelete (context_id in binary_integer, rid in VARCHAR2)
  AS language C name "oraRingoIndexDelete" library bingolib
  with context parameters(context, context_id,
                          rid, rid indicator short);
/
create or replace function ringoIndexStart (context_id in binary_integer, oper in VARCHAR2,
                                             query in CLOB, strt in NUMBER, stop in NUMBER,
                                             params in VARCHAR2) return binary_integer
  AS language C name "oraRingoIndexStart" library bingolib
  with context parameters(context, context_id,
            oper, oper indicator short,
            query, query indicator short,
            strt, strt indicator short,
            stop, stop indicator short,
            params, params indicator short);
/
create or replace function ringoIndexFetch (fetch_id in binary_integer,
                                            maxnrows in BINARY_INTEGER, arr in out sys.ODCIRidList)
  return binary_integer
  AS language C name "oraRingoIndexFetch" library bingolib
  with context parameters(context, fetch_id, maxnrows, arr, arr indicator short);
/
create or replace function ringoIndexSelectivity (context_id in binary_integer, oper in VARCHAR2,
           query in CLOB, strt in NUMBER, stop in NUMBER, params in VARCHAR2) return NUMBER
  AS language C name "oraRingoIndexSelectivity" library bingolib
  with context parameters(context, context_id,
            oper, oper indicator short,
            query, query indicator short,
            strt, strt indicator short,
            stop, stop indicator short,
            params, params indicator short,
            return indicator short, return OCINumber);
/
create or replace procedure ringoIndexCost (context_id in binary_integer, sel in NUMBER, oper in VARCHAR2,
           query in CLOB, strt in NUMBER, stop in NUMBER, params in VARCHAR2,
           iocost out binary_integer, cpucost out binary_integer)
  AS language C name "oraRingoIndexCost" library bingolib
  with context parameters(context, context_id,
            sel, sel indicator short,
            oper, oper indicator short,
            query, query indicator short,
            strt, strt indicator short,
            stop, stop indicator short,
            params, params indicator short,
            iocost, cpucost);
/
create or replace procedure ringoIndexClose (fetch_id in binary_integer)
  AS language C name "oraRingoIndexClose" library bingolib
  with context parameters(context, fetch_id);
/
create or replace procedure ringoCollectStatistics (context_id in binary_integer)
  AS language C name "oraRingoCollectStatistics" library bingolib
  with context parameters(context, context_id);
/
create or replace function Rxnfile_clob (r in CLOB) return CLOB
  AS language C name "oraRingoRxnfile" library bingolib  
  with context parameters (context, r, r indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function Rxnfile_blob (r in BLOB) return CLOB
  AS language C name "oraRingoRxnfile" library bingolib  
  with context parameters (context, r, r indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function RSMILES_clob (r in CLOB) return VARCHAR2
  AS language C name "oraRingoRSMILES" library bingolib  
  with context parameters (context, r, r indicator short,
                             return indicator short, return OCIString);
/
create or replace function RSMILES_blob (r in BLOB) return VARCHAR2
  AS language C name "oraRingoRSMILES" library bingolib  
  with context parameters (context, r, r indicator short,
                             return indicator short, return OCIString);
/
create or replace function CheckReaction (r in CLOB) return VARCHAR2
  AS language C name "oraRingoCheckReaction" library bingolib  
  with context parameters (context, r, r indicator short,
                             return indicator short, return OCIString);
/
grant execute on CheckReaction to public;
/
create or replace procedure CompactReaction2 (m in CLOB, res in BLOB, save_xyz in binary_integer)
  AS language C name "oraRingoICR2" library bingolib  
  with context parameters (context, m, m indicator short,
                           res, res indicator short, save_xyz);
/
grant execute on CompactReaction2 to public;
/
create or replace function CompactReaction (m in CLOB, save_xyz in binary_integer)
                  return BLOB IS
  lob BLOB;
BEGIN
  dbms_lob.createtemporary(lob, TRUE, dbms_lob.call);
  CompactReaction2(m, lob, save_xyz);
  return lob;
END CompactReaction;
/
grant execute on CompactReaction to public;
/

spool off;
