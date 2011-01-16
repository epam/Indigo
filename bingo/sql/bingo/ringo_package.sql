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
spool ringo_package;

CREATE OR REPLACE PACKAGE RingoPackage IS
   function RSub (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;
   function RSub (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER;

   function RSubHi (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;
   function RSubHi (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB;

   function AAM (target in VARCHAR2, params in VARCHAR2) return CLOB;
   function AAM (target in CLOB, params in VARCHAR2) return CLOB;
   function AAM (target in BLOB, params in VARCHAR2) return CLOB;

   function Rxnfile (target in VARCHAR2) return CLOB;
   function Rxnfile (target in CLOB) return CLOB;
   function Rxnfile (target in BLOB) return CLOB;

   function RSMILES (target in VARCHAR2) return VARCHAR2;
   function RSMILES (target in CLOB) return VARCHAR2;
   function RSMILES (target in BLOB) return VARCHAR2;

END RingoPackage;
/
CREATE OR REPLACE PACKAGE BODY RingoPackage IS
   function RSub (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(to_clob(target), to_clob(query), null, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(to_clob(target), to_clob(query), params, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(to_clob(target), query, null, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(to_clob(target), query, params, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(target, query, null, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return RSub_clob(context_id, target, query, params);
   end RSub;
   function RSub (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
   begin
      return RSub(target, query, null, indexctx, scanctx, scanflg);
   end RSub;
   function RSub (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                 scanctx in out RingoIndex, scanflg IN NUMBER) return NUMBER IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return RSub_blob(context_id, target, query, params);
   end RSub;

   function RSubHi (target in VARCHAR2, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(to_clob(target), to_clob(query), null, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in VARCHAR2, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(to_clob(target), to_clob(query), params, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in VARCHAR2, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(to_clob(target), query, null, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in VARCHAR2, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(to_clob(target), query, params, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in CLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in CLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in CLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(target, query, null, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in CLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return RSubHi_clob(context_id, target, query, params);
   end RSubHi;
   function RSubHi (target in BLOB, query in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(target, to_clob(query), null, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in BLOB, query in VARCHAR2, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(target, to_clob(query), params, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in BLOB, query in CLOB, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
   begin
      return RSubHi(target, query, null, indexctx, scanctx, scanflg);
   end RSubHi;
   function RSubHi (target in BLOB, query in CLOB, params in VARCHAR2, indexctx IN sys.ODCIIndexCtx,
                  scanctx in out RingoIndex, scanflg IN NUMBER) return CLOB IS
      context_id binary_integer := 0;
   begin
      if indexctx.IndexInfo is not null then
         context_id := BingoPackage.getContextID(indexctx.IndexInfo);
      end if;
      return RSubHi_blob(context_id, target, query, params);
   end RSubHi;

   function AAM (target in VARCHAR2, params in VARCHAR2) return CLOB IS
   begin
      return AutoAAM_clob(to_clob(target), params);
   end AAM;
   function AAM (target in CLOB, params in VARCHAR2) return CLOB IS
   begin
      return AutoAAM_clob(target, params);
   end AAM;
   function AAM (target in BLOB, params in VARCHAR2) return CLOB IS
   begin
      return AutoAAM_blob(target, params);
   end AAM;

   function Rxnfile (target in VARCHAR2) return CLOB is
   begin
      return Rxnfile_clob(to_clob(target));
   end Rxnfile;
   function Rxnfile (target in CLOB) return CLOB is
   begin
      return Rxnfile_clob(target);
   end Rxnfile;
   function Rxnfile (target in BLOB) return CLOB is
   begin
      return Rxnfile_blob(target);
   end Rxnfile;

   function RSMILES (target in VARCHAR2) return VARCHAR2 is
   begin
      return RSMILES_clob(to_clob(target));
   end RSMILES;
   function RSMILES (target in CLOB) return VARCHAR2 is
   begin
      return RSMILES_clob(target);
   end RSMILES;
   function RSMILES (target in BLOB) return VARCHAR2 is
   begin
      return RSMILES_blob(target);
   end RSMILES;

END RingoPackage;
/
-- necessary for Oracle 9 on Solaris
grant execute on ringopackage to public;
spool off;
