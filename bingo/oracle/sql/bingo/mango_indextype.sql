-- Copyright (C) 2009-2012 GGA Software Services LLC
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
spool mango_indextype;

create or replace operator Sub binding
  (VARCHAR2,  CLOB) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (VARCHAR2,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (VARCHAR2,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (VARCHAR2,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (CLOB,  CLOB) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (CLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (CLOB,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (CLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (BLOB,  CLOB) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (BLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (BLOB,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub,
  (BLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Sub;

create or replace operator Smarts binding
  (VARCHAR2,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Smarts,
  (CLOB,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Smarts,
  (BLOB,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Smarts;

create or replace operator Exact binding
  (VARCHAR2,  CLOB) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (VARCHAR2,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (VARCHAR2,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (VARCHAR2,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (CLOB,  CLOB) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (CLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (CLOB,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (CLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (BLOB,  CLOB) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (BLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (BLOB,  VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact,
  (BLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex
                                         compute ancillary data using MangoPackage.Exact;
                                         
-- 'Sim' and 'Gross' operators never return ancillary data
create or replace operator Sim binding
  (VARCHAR2,  CLOB) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (VARCHAR2,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (VARCHAR2,  VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (VARCHAR2,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (CLOB,  CLOB) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (CLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (CLOB,  VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (CLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (BLOB,  CLOB) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (BLOB,  CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (BLOB,  VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim,
  (BLOB,  VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Sim;

-- 'Gross' operator can be used in two ways  
-- (1) select Gross(structure) from table
-- (2) select * from table where Gross(structure, '>= C6H6')=1
create or replace operator Gross binding 
   (VARCHAR2) return VARCHAR2 using MangoPackage.GrossCalc,
   (VARCHAR2, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Gross,
   (CLOB) return VARCHAR2 using MangoPackage.GrossCalc,
   (CLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Gross,
   (BLOB) return VARCHAR2 using MangoPackage.GrossCalc,
   (BLOB, VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Gross;

-- 'Mass' operator
create or replace operator Mass binding 
   (VARCHAR2) return NUMBER with index context, scan context MangoIndex using MangoPackage.Mass,
   (CLOB) return NUMBER with index context, scan context MangoIndex using MangoPackage.Mass,
   (BLOB) return NUMBER with index context, scan context MangoIndex using MangoPackage.Mass,
   (VARCHAR2, VARCHAR2) return NUMBER using MangoPackage.Mass,
   (CLOB, VARCHAR2) return NUMBER using MangoPackage.Mass,
   (BLOB, VARCHAR2) return NUMBER using MangoPackage.Mass;
   

create or replace operator Molfile binding
   (VARCHAR2) return CLOB using MangoPackage.Molfile,
   (CLOB) return CLOB using MangoPackage.Molfile,
   (BLOB) return CLOB using MangoPackage.Molfile;

create or replace operator CML binding
   (VARCHAR2) return CLOB using MangoPackage.CML,
   (CLOB) return CLOB using MangoPackage.CML,
   (BLOB) return CLOB using MangoPackage.CML;

create or replace operator SMILES binding
   (VARCHAR2) return VARCHAR2 using MangoPackage.SMILES,
   (CLOB) return VARCHAR2 using MangoPackage.SMILES,
   (BLOB) return VARCHAR2 using MangoPackage.SMILES;

create or replace operator CANSMILES binding
   (VARCHAR2) return VARCHAR2 using MangoPackage.CANSMILES,
   (CLOB) return VARCHAR2 using MangoPackage.CANSMILES,
   (BLOB) return VARCHAR2 using MangoPackage.CANSMILES;

create or replace operator SubHi binding (NUMBER) return CLOB ANCILLARY TO 
   Sub (VARCHAR2, CLOB),
   Sub (VARCHAR2, CLOB, VARCHAR2),
   Sub (VARCHAR2, VARCHAR2),
   Sub (VARCHAR2, VARCHAR2, VARCHAR2),
   Sub (CLOB, CLOB),
   Sub (CLOB, CLOB, VARCHAR2),
   Sub (CLOB, VARCHAR2),
   Sub (CLOB, VARCHAR2, VARCHAR2),
   Sub (BLOB, CLOB),
   Sub (BLOB, CLOB, VARCHAR2),
   Sub (BLOB, VARCHAR2),
   Sub (BLOB, VARCHAR2, VARCHAR2) using MangoPackage.SubHi;

create or replace operator SmartsHi binding (NUMBER) return CLOB ANCILLARY TO 
   Smarts (VARCHAR2, VARCHAR2),
   Smarts (CLOB, VARCHAR2),
   Smarts (BLOB, VARCHAR2) using MangoPackage.SmartsHi;
   
create or replace operator ExactHi binding (NUMBER) return CLOB ANCILLARY TO 
   Exact (VARCHAR2, CLOB),
   Exact (VARCHAR2, CLOB, VARCHAR2),
   Exact (VARCHAR2, VARCHAR2),
   Exact (VARCHAR2, VARCHAR2, VARCHAR2),
   Exact (CLOB, CLOB),
   Exact (CLOB, CLOB, VARCHAR2),
   Exact (CLOB, VARCHAR2),
   Exact (CLOB, VARCHAR2, VARCHAR2),
   Exact (BLOB, CLOB),
   Exact (BLOB, CLOB, VARCHAR2),
   Exact (BLOB, VARCHAR2),
   Exact (BLOB, VARCHAR2, VARCHAR2) using MangoPackage.ExactHi;
   
grant execute on Sub to public;
grant execute on Smarts to public;
grant execute on Exact to public;
grant execute on Sim to public;
grant execute on Gross to public;
grant execute on Mass to public;
grant execute on Molfile to public;
grant execute on CML to public;
grant execute on SMILES to public;
grant execute on CANSMILES to public;
grant execute on SubHi to public;
grant execute on SmartsHi to public;
grant execute on ExactHi to public;

create or replace indextype MoleculeIndex for
   Sub(VARCHAR2, CLOB),
   Sub(VARCHAR2, CLOB, VARCHAR2),
   Sub(VARCHAR2, VARCHAR2),
   Sub(VARCHAR2, VARCHAR2, VARCHAR2),
   Sub(CLOB, CLOB),
   Sub(CLOB, CLOB, VARCHAR2),
   Sub(CLOB, VARCHAR2),
   Sub(CLOB, VARCHAR2, VARCHAR2),
   Sub(BLOB, CLOB),
   Sub(BLOB, CLOB, VARCHAR2),
   Sub(BLOB, VARCHAR2),
   Sub(BLOB, VARCHAR2, VARCHAR2),
   Smarts(VARCHAR2, VARCHAR2),
   Smarts(CLOB, VARCHAR2),
   Smarts(BLOB, VARCHAR2),
   Exact(VARCHAR2, CLOB),
   Exact(VARCHAR2, CLOB, VARCHAR2),
   Exact(VARCHAR2, VARCHAR2),
   Exact(VARCHAR2, VARCHAR2, VARCHAR2),
   Exact(CLOB, CLOB),
   Exact(CLOB, CLOB, VARCHAR2),
   Exact(CLOB, VARCHAR2),
   Exact(CLOB, VARCHAR2, VARCHAR2),
   Exact(BLOB, CLOB),
   Exact(BLOB, CLOB, VARCHAR2),
   Exact(BLOB, VARCHAR2),
   Exact(BLOB, VARCHAR2, VARCHAR2),
   Sim(VARCHAR2, CLOB),
   Sim(VARCHAR2, CLOB, VARCHAR2),
   Sim(VARCHAR2, VARCHAR2),
   Sim(VARCHAR2, VARCHAR2, VARCHAR2),
   Sim(CLOB, CLOB),
   Sim(CLOB, CLOB, VARCHAR2),
   Sim(CLOB, VARCHAR2),
   Sim(CLOB, VARCHAR2, VARCHAR2),
   Sim(BLOB, CLOB),
   Sim(BLOB, CLOB, VARCHAR2),
   Sim(BLOB, VARCHAR2),
   Sim(BLOB, VARCHAR2, VARCHAR2),
   Gross(VARCHAR2, VARCHAR2),
   Gross(CLOB, VARCHAR2),
   Gross(BLOB, VARCHAR2),
   Mass(VARCHAR2),
   Mass(CLOB),
   Mass(BLOB)
   using MangoIndex;

associate statistics with packages MangoPackage using MangoStat;
             
associate statistics with indextypes MoleculeIndex using MangoStat;

grant execute on MoleculeIndex to public;

spool off;
