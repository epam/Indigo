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
spool mango_package;

CREATE OR REPLACE PACKAGE MangoPackage IS
   function Sub (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sub (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function SubHi (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SubHi (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function Smarts (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Smarts (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Smarts (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function SmartsHi (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SmartsHi (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function SmartsHi (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function Exact (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Exact (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function ExactHi (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function ExactHi (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB;
   function Sim (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Sim (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function GrossCalc (target in VARCHAR2) return VARCHAR2;
   function GrossCalc (target in CLOB) return VARCHAR2;
   function GrossCalc (target in BLOB) return VARCHAR2;
   function Gross (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Gross (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Gross (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Mass (target in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Mass (target in CLOB, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Mass (target in BLOB, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER;
   function Mass (target in VARCHAR2, typee in VARCHAR2) return NUMBER;
   function Mass (target in CLOB, typee in VARCHAR2) return NUMBER;
   function Mass (target in BLOB, typee in VARCHAR2) return NUMBER;
   function Molfile (target in VARCHAR2) return CLOB;
   function Molfile (target in CLOB) return CLOB;
   function Molfile (target in BLOB) return CLOB;
   function CML (target in VARCHAR2) return CLOB;
   function CML (target in CLOB) return CLOB;
   function CML (target in BLOB) return CLOB;
   function SMILES (target in VARCHAR2) return VARCHAR2;
   function SMILES (target in CLOB) return VARCHAR2;
   function SMILES (target in BLOB) return VARCHAR2;
   function CANSMILES (target in VARCHAR2) return VARCHAR2;
   function CANSMILES (target in CLOB) return VARCHAR2;
   function CANSMILES (target in BLOB) return VARCHAR2;
END MangoPackage;
/
CREATE OR REPLACE PACKAGE BODY MangoPackage IS
   function Sub (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(to_clob(target), to_clob(query), null, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(to_clob(target), to_clob(query), params, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(to_clob(target), query, null, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(to_clob(target), query, params, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(target, query, null, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Sub_clob(context_id, target, query, params);
   end Sub;
   function Sub (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sub(target, query, null, indexctx, scanctx, scanflg);
   end Sub;
   function Sub (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Sub_blob(context_id, target, query, params);
   end Sub;
   function SubHi (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(to_clob(target), to_clob(query), null, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(to_clob(target), to_clob(query), params, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(to_clob(target), query, null, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(to_clob(target), query, params, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(target, query, null, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return SubHi_clob(context_id, target, query, params);
   end SubHi;
   function SubHi (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return SubHi(target, query, null, indexctx, scanctx, scanflg);
   end SubHi;
   function SubHi (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return SubHi_blob(context_id, target, query, params);
   end SubHi;

   function Smarts (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      return Smarts(to_clob(target), query, indexctx, scanctx, scanflg);
   end Smarts;
   function Smarts (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Smarts_clob(context_id, target, query);
   end Smarts;
   function Smarts (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Smarts_blob(context_id, target, query);
   end Smarts;

   function SmartsHi (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return SmartsHi_clob(context_id, to_clob(target), query);
   end SmartsHi;
   function SmartsHi (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return SmartsHi_clob(context_id, target, query);
   end SmartsHi;
   function SmartsHi (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return SmartsHi_blob(context_id, target, query);
   end SmartsHi;

   function Exact (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
            scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(to_clob(target), to_clob(query), null, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
            scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(to_clob(target), to_clob(query), params, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
            scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(to_clob(target), query, null, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
            scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(to_clob(target), query, params, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
            scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
            scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
            scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(target, query, null, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Exact_clob(context_id, target, query, params);
   end Exact;
   function Exact (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in BLOB, query in VARCHAR2, params in VARCHAR2,
            indexctx IN sys.ODCIIndexCtx, scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Exact(target, query, null, indexctx, scanctx, scanflg);
   end Exact;
   function Exact (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
            scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Exact_blob(context_id, target, query, params);
   end Exact;

   function ExactHi (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(to_clob(target), to_clob(query), null, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(to_clob(target), to_clob(query), params, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(to_clob(target), query, null, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(to_clob(target), query, params, indexctx, scanctx, scanflg);
   end ExactHi;

   function ExactHi (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(target, query, null, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return ExactHi_clob(context_id, target, query, params);
   end ExactHi;
   function ExactHi (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return ExactHi(target, query, null, indexctx, scanctx, scanflg);
   end ExactHi;
   function ExactHi (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return ExactHi_blob(context_id, target, query, params);
   end ExactHi;

   function Sim (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(to_clob(target), to_clob(query), null, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(to_clob(target), to_clob(query), params, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(to_clob(target), query, null, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in VARCHAR2, query in CLOB, params VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(to_clob(target), query, params, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(target, query, null, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in CLOB, query in CLOB, params VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Sim_clob(context_id, target, query, params);
   end Sim;
   function Sim (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return Sim(target, query, null, indexctx, scanctx, scanflg);
   end Sim;
   function Sim (target in BLOB, query in CLOB, params VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Sim_blob(context_id, target, query, params);
   end Sim;

   function GrossCalc (target in VARCHAR2) return VARCHAR2 IS
   begin
      return GrossCalc(to_clob(target));
   end GrossCalc;
   function GrossCalc (target in CLOB) return VARCHAR2 IS
   begin
      return GrossCalc_clob(target);
   end GrossCalc;
   function GrossCalc (target in BLOB) return VARCHAR2 IS
   begin
      return GrossCalc_blob(target);
   end GrossCalc;

   function Gross (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER is
   begin
      return Gross(to_clob(target), query, indexctx, scanctx, scanflg);
   end Gross;
   function Gross (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER is
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Gross_clob(context_id, target, query);
   end Gross;
   function Gross (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER is
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Gross_blob(context_id, target, query);
   end Gross;

   function Mass (target in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER is
   begin
      return Mass(to_clob(target), indexctx, scanctx, scanflg);
   end Mass;
   function Mass (target in CLOB, indexctx IN sys.ODCIIndexCtx,
                   scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER is
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Mass_clob(context_id, target, NULL);
   end Mass;
   function Mass (target in BLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out MangoIndex, scanflg IN NUMBER) return NUMBER is
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return Mass_blob(context_id, target, NULL);
   end Mass;

   function Mass (target in VARCHAR2, typee in VARCHAR2) return NUMBER is
   begin
      return Mass(to_clob(target), typee);
   end Mass;
   function Mass (target in CLOB, typee in VARCHAR2) return NUMBER is
   begin
      return Mass_clob(0, target, typee);
   end Mass;
   function Mass (target in BLOB, typee in VARCHAR2) return NUMBER is
   begin
      return Mass_blob(0, target, typee);
   end Mass;

   function Molfile (target in VARCHAR2) return CLOB is
   begin
      return Molfile_clob(to_clob(target));
   end Molfile;
   function Molfile (target in CLOB) return CLOB is
   begin
      return Molfile_clob(target);
   end Molfile;
   function Molfile (target in BLOB) return CLOB is
   begin
      return Molfile_blob(target);
   end Molfile;

   function CML (target in VARCHAR2) return CLOB is
   begin
      return CML_clob(to_clob(target));
   end CML;
   function CML (target in CLOB) return CLOB is
   begin
      return CML_clob(target);
   end CML;
   function CML (target in BLOB) return CLOB is
   begin
      return CML_blob(target);
   end CML;

   function SMILES (target in VARCHAR2) return VARCHAR2 is
   begin
      return SMILES_clob(to_clob(target));
   end SMILES;
   function SMILES (target in CLOB) return VARCHAR2 is
   begin
      return SMILES_clob(target);
   end SMILES;
   function SMILES (target in BLOB) return VARCHAR2 is
   begin
      return SMILES_blob(target);
   end SMILES;

   function CANSMILES (target in VARCHAR2) return VARCHAR2 is
   begin
      return CANSMILES_clob(to_clob(target));
   end CANSMILES;
   function CANSMILES (target in CLOB) return VARCHAR2 is
   begin
      return CANSMILES_clob(target);
   end CANSMILES;
   function CANSMILES (target in BLOB) return VARCHAR2 is
   begin
      return CANSMILES_blob(target);
   end CANSMILES;

END MangoPackage;
/
-- necessary for Oracle 9 on Solaris
grant execute on mangopackage to public;
spool off;
