-- Copyright (C) 2009-2013 GGA Software Services LLC
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
spool mango_calls;

create or replace function Sub_clob (context_id in binary_integer,
  target in CLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraMangoSub" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function SubHi_clob (context_id in binary_integer,
  target in CLOB, query in CLOB, params in VARCHAR2) return CLOB
  AS language C name "oraMangoSubHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCILobLocator);
/
create or replace function Sub_blob (context_id in binary_integer,
  target in BLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraMangoSub" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function SubHi_blob (context_id in binary_integer,
  target in BLOB, query in CLOB, params in VARCHAR2) return CLOB
  AS language C name "oraMangoSubHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCILobLocator);
/
create or replace function Smarts_clob (context_id in binary_integer,
  target in CLOB, query in VARCHAR2) return NUMBER
  AS language C name "oraMangoSmarts" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          return indicator short, return OCINumber);
/
create or replace function Smarts_blob (context_id in binary_integer,
  target in BLOB, query in VARCHAR2) return NUMBER
  AS language C name "oraMangoSmarts" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          return indicator short, return OCINumber);
/
create or replace function SmartsHi_clob (context_id in binary_integer,
  target in CLOB, query in VARCHAR2) return CLOB
  AS language C name "oraMangoSmartsHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          return indicator short, return OCILobLocator);
/
create or replace function SmartsHi_blob (context_id in binary_integer,
  target in BLOB, query in VARCHAR2) return CLOB
  AS language C name "oraMangoSmartsHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          return indicator short, return OCILobLocator);
/

create or replace function Exact_clob (context_id in binary_integer,
  target in CLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraMangoExact" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function ExactHi_clob (context_id in binary_integer,
  target in CLOB, query in CLOB, params in VARCHAR2) return CLOB
  AS language C name "oraMangoExactHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCILobLocator);
/
create or replace function Exact_blob (context_id in binary_integer,
  target in BLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraMangoExact" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function ExactHi_blob (context_id in binary_integer,
  target in BLOB, query in CLOB, params in VARCHAR2) return CLOB
  AS language C name "oraMangoExactHi" library bingolib
  with context parameters(context, context_id, 
                          target, target indicator short,
                          query,  query  indicator short, 
                          params, params indicator short,
                          return indicator short, return OCILobLocator);
/
create or replace function Sim_clob (context_id in binary_integer,
  target in CLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraMangoSim" library bingolib
  with context parameters(context, context_id,
                          target,  target indicator short,
                          query,   query  indicator short,
                          params,  params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function Sim_blob (context_id in binary_integer,
  target in BLOB, query in CLOB, params in VARCHAR2) return NUMBER
  AS language C name "oraMangoSim" library bingolib
  with context parameters(context, context_id,
                          target,  target indicator short,
                          query,   query  indicator short,
                          params,  params indicator short,
                          return indicator short, return OCINumber);
/
create or replace function GrossCalc_clob (target in CLOB) return VARCHAR2
  AS language C name "oraMangoGrossCalc" library bingolib
  with context parameters(context, target, target indicator short,
                          return indicator short, return OCIString);
/
create or replace function GrossCalc_blob (target in BLOB) return VARCHAR2
  AS language C name "oraMangoGrossCalc" library bingolib
  with context parameters(context, target, target indicator short,
                          return indicator short, return OCIString);
/
create or replace function Gross_clob (context_id in binary_integer,
  target in CLOB, query in VARCHAR2) return NUMBER
  AS language C name "oraMangoGross" library bingolib
  with context parameters(context, context_id,
                          target, target indicator short,
                          query, query indicator short,
                          return indicator short, return OCINumber);
/
create or replace function Gross_blob (context_id in binary_integer,
  target in BLOB, query in VARCHAR2) return NUMBER
  AS language C name "oraMangoGross" library bingolib
  with context parameters(context, context_id,
                          target, target indicator short,
                          query, query indicator short,
                          return indicator short, return OCINumber);
/
create or replace function Mass_clob (context_id in binary_integer,
                               target in CLOB, typee in VARCHAR2) return NUMBER
  AS language C name "oraMangoMolecularMass" library bingolib
  with context parameters(context, context_id,
                          target, target indicator short,
                          typee, typee indicator short,
                          return indicator short, return OCINumber);
/
create or replace function Mass_blob (context_id in binary_integer,
                               target in BLOB, typee in VARCHAR2) return NUMBER
  AS language C name "oraMangoMolecularMass" library bingolib
  with context parameters(context, context_id,
                          target, target indicator short,
                          typee, typee indicator short,
                          return indicator short, return OCINumber);
/

create or replace procedure mangoCreateIndex(context_id in binary_integer, params in varchar2, 
                                             full_table_name in varchar2, 
                                             column_name in varchar2, 
                                             column_data_type in varchar2) 
  AS language C name "oraMangoCreateIndex" library bingolib
  with context parameters(context, context_id, 
                          params, params indicator short, 
                          full_table_name, full_table_name indicator short, 
                          column_name, column_name indicator short,
                          column_data_type, column_data_type indicator short);
/
create or replace procedure mangoDropIndex (context_id in binary_integer)
  AS language C name "oraMangoDropIndex" library bingolib
  with context parameters(context, context_id);
/
create or replace procedure mangoTruncateIndex (context_id in binary_integer)
  AS language C name "oraMangoTruncateIndex" library bingolib
  with context parameters(context, context_id);
/
create or replace procedure mangoIndexInsert_clob (context_id in binary_integer, rid in VARCHAR2, item in CLOB)
  AS language C name "oraMangoIndexInsert" library bingolib
  with context parameters(context, context_id,
                          rid, rid indicator short,
                          item, item indicator short);
/
create or replace procedure mangoIndexInsert_blob (context_id in binary_integer, rid in VARCHAR2, item in BLOB)
  AS language C name "oraMangoIndexInsert" library bingolib
  with context parameters(context, context_id,
                          rid, rid indicator short,
                          item, item indicator short);
/
create or replace procedure mangoIndexDelete (context_id in binary_integer, rid in VARCHAR2)
  AS language C name "oraMangoIndexDelete" library bingolib
  with context parameters(context, context_id,
                          rid, rid indicator short);
/
create or replace function mangoIndexStart (context_id in binary_integer,
      oper in VARCHAR2, query in CLOB, strt in NUMBER, stop in NUMBER,
      flags in binary_integer, params in VARCHAR2) return binary_integer
  AS language C name "oraMangoIndexStart" library bingolib
  with context parameters(context, context_id,
            oper, oper indicator short,
            query, query indicator short,
            strt, strt indicator short,
            stop, stop indicator short,
            flags,
            params, params indicator short);
/
create or replace function mangoIndexFetch (fetch_id in binary_integer,
                                            maxnrows in BINARY_INTEGER, arr in out sys.ODCIRidList)
  return binary_integer
  AS language C name "oraMangoIndexFetch" library bingolib
  with context parameters(context, fetch_id, maxnrows, arr, arr indicator short);
/
create or replace function mangoIndexSelectivity (context_id in binary_integer,
           oper in VARCHAR2, query in CLOB, strt in NUMBER, stop in NUMBER,
           flags in binary_integer, params in VARCHAR2) return NUMBER
  AS language C name "oraMangoIndexSelectivity" library bingolib
  with context parameters(context, context_id,
            oper, oper indicator short,
            query, query indicator short,
            strt, strt indicator short,
            stop, stop indicator short,
            flags,
            params, params indicator short,
            return indicator short, return OCINumber);
/
create or replace procedure mangoIndexCost (context_id in binary_integer, sel in NUMBER, oper in VARCHAR2,
           query in CLOB, strt in NUMBER, stop in NUMBER, flags in binary_integer,
           params in VARCHAR2, iocost out binary_integer, cpucost out binary_integer)
  AS language C name "oraMangoIndexCost" library bingolib
  with context parameters(context, context_id,
            sel, sel indicator short,
            oper, oper indicator short,
            query, query indicator short,
            strt, strt indicator short,
            stop, stop indicator short,
            flags,
            params, params indicator short,
            iocost, cpucost);
/
create or replace procedure mangoIndexClose (fetch_id in binary_integer)
  AS language C name "oraMangoIndexClose" library bingolib
  with context parameters(context, fetch_id);
/
create or replace procedure mangoCollectStatistics (context_id in binary_integer)
  AS language C name "oraMangoCollectStatistics" library bingolib
  with context parameters(context, context_id);
/

create or replace procedure mangoAnalyzeMolecules(context_id in binary_integer)
    as language C name "oraMangoAnalyzeMolecules" library bingolib  
    with context parameters (context, context_id); 
/

create or replace function Molfile_clob (m in CLOB) return CLOB
  AS language C name "oraMangoMolfile" library bingolib  
  with context parameters (context, m, m indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function Molfile_blob (m in BLOB) return CLOB
  AS language C name "oraMangoMolfile" library bingolib  
  with context parameters (context, m, m indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function CML_clob (m in CLOB) return CLOB  AS language C name "oraMangoCML" library bingolib    
  with context parameters (context, m, m indicator short, 
                           return indicator short, return OCILobLocator);
/
create or replace function CML_blob (m in BLOB) return CLOB
  AS language C name "oraMangoCML" library bingolib
  with context parameters (context, m, m indicator short,
                           return indicator short,
                           return OCILobLocator);
/
create or replace function SMILES_clob (m in CLOB) return VARCHAR2
  AS language C name "oraMangoSMILES" library bingolib  
  with context parameters (context, m, m indicator short,
                           return indicator short, return OCIString);
/
create or replace function SMILES_blob (m in BLOB) return VARCHAR2
  AS language C name "oraMangoSMILES" library bingolib  
  with context parameters (context, m, m indicator short,
                           return indicator short, return OCIString);
/
create or replace function InChI_clob (m in CLOB, options in VARCHAR2) return CLOB
  AS language C name "oraMangoInchi" library bingolib  
  with context parameters (context, m, m indicator short, options, options indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function InChI_blob (m in BLOB, options in VARCHAR2) return CLOB
  AS language C name "oraMangoInchi" library bingolib  
  with context parameters (context, m, m indicator short, options, options indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function InChIKey_clob (inchi in CLOB) return VARCHAR2
  AS language C name "oraMangoInchiKey" library bingolib
  with context parameters (context, inchi, inchi indicator short,
                           return indicator short, return OCIString);
/
create or replace function Fingerprint_clob (m in CLOB, options in VARCHAR2) return BLOB
  AS language C name "oraMangoFingerprint" library bingolib  
  with context parameters (context, m, m indicator short, options, options indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function Fingerprint_blob (m in BLOB, options in VARCHAR2) return BLOB
  AS language C name "oraMangoFingerprint" library bingolib  
  with context parameters (context, m, m indicator short, options, options indicator short,
                           return indicator short, return OCILobLocator);
/
create or replace function CANSMILES_clob (m in CLOB) return VARCHAR2
  AS language C name "oraMangoCanonicalSMILES" library bingolib  
  with context parameters (context, m, m indicator short,
                           return indicator short, return OCIString);
/
create or replace function CANSMILES_blob (m in BLOB) return VARCHAR2
  AS language C name "oraMangoCanonicalSMILES" library bingolib  
  with context parameters (context, m, m indicator short,
                           return indicator short, return OCIString);
/
create or replace function CheckMolecule (m in CLOB) return VARCHAR2
  AS language C name "oraMangoCheckMolecule" library bingolib  
  with context parameters (context, m, m indicator short,
                             return indicator short, return OCIString);
/
grant execute on CheckMolecule to public;
/
create or replace procedure CompactMolecule2 (m in CLOB, res in BLOB, save_xyz in binary_integer) 
  AS language C name "oraMangoICM2" library bingolib  
  with context parameters (context, m, m indicator short, 
                           res, res indicator short, save_xyz);
/
grant execute on CompactMolecule2 to public;
/
create or replace function CompactMolecule (m in CLOB, save_xyz in binary_integer)
                  return BLOB IS
  lob BLOB;
BEGIN
  IF m is null THEN
    return NULL;
  END IF;
  dbms_lob.createtemporary(lob, TRUE, dbms_lob.call);
  CompactMolecule2(m, lob, save_xyz);
  return lob;
END CompactMolecule;
/
grant execute on CompactMolecule to public;
/

spool off;
