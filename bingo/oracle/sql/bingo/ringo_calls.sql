-- Copyright (C) from 2009 to Present EPAM Systems.
-- 
-- This file is part of Indigo toolkit.
-- 
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
-- 
-- http://www.apache.org/licenses/LICENSE-2.0
-- 
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

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
create or replace procedure ringoCreateIndex(context_id in binary_integer, params in varchar2, 
                                             full_table_name in varchar2,
                                             column_name in varchar2, 
                                             column_data_type in varchar2) 
  AS language C name "oraRingoCreateIndex" library bingolib
  with context parameters(context, context_id, 
                          params,           params           indicator short, 
                          full_table_name,  full_table_name  indicator short, 
                          column_name,      column_name      indicator short,
                          column_data_type, column_data_type indicator short);
/
create or replace function RSmarts_clob (context_id in binary_integer,
  target in CLOB, query in VARCHAR2) return NUMBER
  AS language C name "oraRingoRSmarts" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          return indicator short, return OCINumber);
/
create or replace function RSmarts_blob (context_id in binary_integer,
  target in BLOB, query in VARCHAR2) return NUMBER
  AS language C name "oraRingoRSmarts" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          return indicator short, return OCINumber);
/
create or replace function RSmartsHi_clob (context_id in binary_integer,
  target in CLOB, query in VARCHAR2) return CLOB
  AS language C name "oraRingoRSmartsHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          return indicator short, return OCILobLocator);
/
create or replace function RSmartsHi_blob (context_id in binary_integer,
  target in BLOB, query in VARCHAR2) return CLOB
  AS language C name "oraRingoRSmartsHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
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
create or replace function RCML_clob (m in CLOB) return CLOB
  AS language C name "oraRingoCML" library bingolib  
  with context parameters (context, m, m indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function RCML_blob (m in BLOB) return CLOB
  AS language C name "oraRingoCML" library bingolib  
  with context parameters (context, m, m indicator short,
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
create or replace function RFingerprint_clob (r in CLOB, options in VARCHAR2) return BLOB
  AS language C name "oraRingoFingerprint" library bingolib  
  with context parameters (context, r, r indicator short, options, options indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function RFingerprint_blob (r in BLOB, options in VARCHAR2) return BLOB
  AS language C name "oraRingoFingerprint" library bingolib  
  with context parameters (context, r, r indicator short, options, options indicator short,
                           return indicator short, return OCILobLocator);
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
  IF m is NULL THEN
    return NULL;
  END IF;
  dbms_lob.createtemporary(lob, TRUE, dbms_lob.call);
  CompactReaction2(m, lob, save_xyz);
  return lob;
END CompactReaction;
/
grant execute on CompactReaction to public;
/

spool off;
