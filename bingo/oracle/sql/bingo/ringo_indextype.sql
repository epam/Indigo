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
spool ringo_indextype;

create or replace operator RSub binding
  (VARCHAR2,  CLOB) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (VARCHAR2,  CLOB, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (VARCHAR2,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (VARCHAR2,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (CLOB,  CLOB) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (CLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (CLOB,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (CLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (BLOB,  CLOB) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (BLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (BLOB,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub,
  (BLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSub;

create or replace operator RSubHi binding (NUMBER) return CLOB ANCILLARY TO
   RSub (VARCHAR2, CLOB),
   RSub (VARCHAR2, CLOB, VARCHAR2),
   RSub (VARCHAR2, VARCHAR2),
   RSub (VARCHAR2, VARCHAR2, VARCHAR2),
   RSub (CLOB, CLOB),
   RSub (CLOB, CLOB, VARCHAR2),
   RSub (CLOB, VARCHAR2),
   RSub (CLOB, VARCHAR2, VARCHAR2),
   RSub (BLOB, CLOB),
   RSub (BLOB, CLOB, VARCHAR2),
   RSub (BLOB, VARCHAR2),
   RSub (BLOB, VARCHAR2, VARCHAR2) using RingoPackage.RSubHi;

create or replace operator RSmarts binding
  (VARCHAR2,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSmarts,
  (CLOB,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSmarts,
  (BLOB,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                         compute ancillary data using RingoPackage.RSmarts;

create or replace operator RSmartsHi binding (NUMBER) return CLOB ANCILLARY TO 
   RSmarts (VARCHAR2, VARCHAR2),
   RSmarts (CLOB, VARCHAR2),
   RSmarts (BLOB, VARCHAR2) using RingoPackage.RSmartsHi;


create or replace operator RExact binding
  (VARCHAR2,  CLOB) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (VARCHAR2,  CLOB, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (VARCHAR2,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (VARCHAR2,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (CLOB,  CLOB) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (CLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (CLOB,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (CLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (BLOB,  CLOB) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (BLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (BLOB,  VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact,
  (BLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context RingoIndex
                                          using RingoPackage.RExact;


create or replace operator AAM binding  
  (VARCHAR2, VARCHAR2) return CLOB using RingoPackage.AAM,
  (CLOB, VARCHAR2) return CLOB using RingoPackage.AAM,
  (BLOB, VARCHAR2) return CLOB using RingoPackage.AAM;

create or replace operator Rxnfile binding
   (VARCHAR2) return CLOB using RingoPackage.Rxnfile,
   (CLOB) return CLOB using RingoPackage.Rxnfile,
   (BLOB) return CLOB using RingoPackage.Rxnfile;

create or replace operator RCML binding
   (VARCHAR2) return CLOB using RingoPackage.RCML,
   (CLOB) return CLOB using RingoPackage.RCML,
   (BLOB) return CLOB using RingoPackage.RCML;

create or replace operator RSMILES binding  
  (VARCHAR2) return VARCHAR2 using RingoPackage.RSMILES,
  (CLOB) return VARCHAR2 using RingoPackage.RSMILES,
  (BLOB) return VARCHAR2 using RingoPackage.RSMILES;

create or replace operator RFingerprint binding
   (VARCHAR2, VARCHAR2) return BLOB using RingoPackage.RFingerprint,
   (CLOB, VARCHAR2) return BLOB using RingoPackage.RFingerprint,
   (BLOB, VARCHAR2) return BLOB using RingoPackage.RFingerprint;   
   
grant execute on RSub to public;
grant execute on RSubHi to public;
grant execute on RExact to public;
grant execute on RSmarts to public;
grant execute on RSmartsHi to public;
grant execute on AAM to public;
grant execute on RSMILES to public;
grant execute on Rxnfile to public;
grant execute on RCML to public;
grant execute on RFingerprint to public;

create or replace indextype ReactionIndex for
   RSub(VARCHAR2, CLOB),
   RSub(VARCHAR2, CLOB, VARCHAR2),
   RSub(VARCHAR2, VARCHAR2),
   RSub(VARCHAR2, VARCHAR2, VARCHAR2),
   RSub(CLOB, CLOB),
   RSub(CLOB, CLOB, VARCHAR2),
   RSub(CLOB, VARCHAR2),
   RSub(CLOB, VARCHAR2, VARCHAR2),
   RSub(BLOB, CLOB),
   RSub(BLOB, CLOB, VARCHAR2),
   RSub(BLOB, VARCHAR2),
   RSub(BLOB, VARCHAR2, VARCHAR2),
   RSmarts(VARCHAR2, VARCHAR2),
   RSmarts(CLOB, VARCHAR2),
   RSmarts(BLOB, VARCHAR2),
   RExact(VARCHAR2, CLOB),
   RExact(VARCHAR2, CLOB, VARCHAR2),
   RExact(VARCHAR2, VARCHAR2),
   RExact(VARCHAR2, VARCHAR2, VARCHAR2),
   RExact(CLOB, CLOB),
   RExact(CLOB, CLOB, VARCHAR2),
   RExact(CLOB, VARCHAR2),
   RExact(CLOB, VARCHAR2, VARCHAR2),
   RExact(BLOB, CLOB),
   RExact(BLOB, CLOB, VARCHAR2),
   RExact(BLOB, VARCHAR2),
   RExact(BLOB, VARCHAR2, VARCHAR2)
   using RingoIndex;

associate statistics with packages RingoPackage using RingoStat;
             
associate statistics with indextypes ReactionIndex using RingoStat;

grant execute on ReactionIndex to public;

spool off;
