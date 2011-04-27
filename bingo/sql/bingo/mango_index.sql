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
spool mango_index;

create or replace type MangoIndex as object
(  
   fetch_id number,
   static function ODCIGetInterfaces (ifclist OUT sys.ODCIObjectList) return NUMBER, 
   static function ODCIIndexCreate   (ia sys.ODCIIndexInfo, parms VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexAlter    (ia sys.ODCIIndexInfo, parms VARCHAR2, alter_option VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexDrop     (ia sys.ODCIIndexInfo, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexTruncate (ia sys.ODCIIndexInfo, env sys.ODCIEnv) return NUMBER,

   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, query VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, query CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
          
   static function ODCIIndexInsert (ia sys.ODCIIndexInfo, rid VARCHAR2, newval CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexDelete (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexUpdate (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval CLOB, newval CLOB, env sys.ODCIEnv) return NUMBER,

   static function ODCIIndexInsert (ia sys.ODCIIndexInfo, rid VARCHAR2, newval VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexDelete (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexUpdate (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval VARCHAR2, newval VARCHAR2, env sys.ODCIEnv) return NUMBER,

   static function ODCIIndexInsert (ia sys.ODCIIndexInfo, rid VARCHAR2, newval BLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexDelete (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval BLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIIndexUpdate (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval BLOB, newval BLOB, env sys.ODCIEnv) return NUMBER,

   member function ODCIIndexFetch (nrows NUMBER, rids OUT sys.ODCIRidList, env sys.ODCIEnv) return NUMBER,
   member function ODCIIndexClose (env sys.ODCIEnv) return NUMBER
);
/
create or replace type body MangoIndex is
static function ODCIGetInterfaces(ifclist OUT sys.ODCIObjectList) return NUMBER is
   begin
      ifclist := sys.ODCIObjectList(sys.ODCIObject('SYS','ODCIINDEX2'));
      return ODCIConst.Success;
   end;
		
   static function ODCIIndexCreate (ia sys.ODCIIndexInfo, parms VARCHAR2, env sys.ODCIEnv) return NUMBER is
      context_id binary_integer;
      col        sys.ODCIColInfo := ia.IndexCols(1);
   begin
      context_id := BingoPackage.createContextID(ia);
      mangoCreateIndex(context_id, parms, '"'||col.TableSchema||'"."'||col.TableName||'"', col.ColName, col.ColTypeName);
      return ODCICONST.Success;
   end;
   
   static function ODCIIndexAlter (ia sys.ODCIIndexInfo, parms VARCHAR2, alter_option VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      AlterPackage.AlterIndex(ia, parms, alter_option, AlterPackage.mangoIndexType);
      return ODCICONST.Success;
   end;
   
   static function ODCIIndexDrop (ia sys.ODCIIndexInfo, env sys.ODCIEnv) return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      if context_id is not null then
         mangoDropIndex(context_id);
         BingoPackage.deleteContextID(context_id);
      end if;
      return ODCICONST.Success;
   end;

   static function ODCIIndexTruncate (ia sys.ODCIIndexInfo, env sys.ODCIEnv) return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoTruncateIndex(context_id);
      return ODCICONST.Success;
   end;
	 
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, query VARCHAR2, env sys.ODCIEnv)
          return NUMBER is
   begin                              
      return ODCIIndexStart(sctx, ia, op, qi, strt, stop, to_clob(query), null, env);
   end;
   
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv)
          return NUMBER is
   begin                              
      return ODCIIndexStart(sctx, ia, op, qi, strt, stop, to_clob(query), params, env);
   end;
   
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, query CLOB, env sys.ODCIEnv)
          return NUMBER is
   begin                              
      return ODCIIndexStart(sctx, ia, op, qi, strt, stop, query, null, env);
   end;
	 
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, env sys.ODCIEnv)
          return NUMBER is
   begin                              
      return ODCIIndexStart(sctx, ia, op, qi, strt, stop, to_clob('<null>'), null, env);
   end;
	 
   static function ODCIIndexStart (sctx IN OUT MangoIndex, ia sys.ODCIIndexInfo, op sys.ODCIPredInfo,
          qi sys.ODCIQueryInfo, strt NUMBER, stop NUMBER, query CLOB, params VARCHAR2, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
      fetch_id binary_integer;
      flags binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);

      flags := 0;

      if bitand(op.Flags, ODCIConst.PredIncludeStart) != 0 then
         flags := flags + 16 - bitand(flags, 16);
      end if;      
      if bitand(op.Flags, ODCIConst.PredIncludeStop) != 0 then
         flags := flags + 32 - bitand(flags, 32);
      end if;      
      if bitand(op.Flags, ODCIConst.PredExactMatch) != 0 then
         flags := flags + 64 - bitand(flags, 64);
      end if;

      fetch_id := mangoIndexStart(context_id, op.ObjectName, query,
                                  strt, stop, flags, params);
         
      sctx := MangoIndex(fetch_id);

      return ODCICONST.Success;
   end;
			
   static function ODCIIndexInsert (ia sys.ODCIIndexInfo, rid VARCHAR2, newval CLOB, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexInsert_clob(context_id, rid, newval);
      return ODCICONST.Success;
   end;
   
   static function ODCIIndexDelete (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval CLOB, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexDelete(context_id, rid);
      return ODCICONST.Success;
   end;
          
   static function ODCIIndexUpdate (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval CLOB, newval CLOB, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexDelete(context_id, rid);
      mangoIndexInsert_clob(context_id, rid, newval);
      return ODCICONST.Success;
   end;

   static function ODCIIndexInsert (ia sys.ODCIIndexInfo, rid VARCHAR2, newval VARCHAR2, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexInsert_clob(context_id, rid, to_clob(newval));
      return ODCICONST.Success;
   end;
   
   static function ODCIIndexDelete (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval VARCHAR2, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexDelete(context_id, rid);
      return ODCICONST.Success;
   end;
          
   static function ODCIIndexUpdate (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval VARCHAR2, newval VARCHAR2, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexDelete(context_id, rid);
      mangoIndexInsert_clob(context_id, rid, to_clob(newval));
      return ODCICONST.Success;
   end;
          
   static function ODCIIndexInsert (ia sys.ODCIIndexInfo, rid VARCHAR2, newval BLOB, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexInsert_blob(context_id, rid, newval);
      return ODCICONST.Success;
   end;
   
   static function ODCIIndexDelete (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval BLOB, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexDelete(context_id, rid);
      return ODCICONST.Success;
   end;
          
   static function ODCIIndexUpdate (ia sys.ODCIIndexInfo, rid VARCHAR2, oldval BLOB, newval BLOB, env sys.ODCIEnv)
          return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      mangoIndexDelete(context_id, rid);
      mangoIndexInsert_blob(context_id, rid, newval);
      return ODCICONST.Success;
   end;

   member function ODCIIndexFetch (nrows NUMBER, rids OUT NOCOPY sys.ODCIRidList, env sys.ODCIEnv) return NUMBER is
      res int;
   begin   
      rids := sys.odciridlist();
      res := mangoIndexFetch(fetch_id, nrows, rids);
      if res = 0 then -- fetch is complete
         rids.extend; -- add zero to the end of fetched list
      end if;
      return ODCICONST.Success;
   end;
   
   member function ODCIIndexClose (env sys.ODCIEnv) return NUMBER is
   begin     
      mangoIndexClose(fetch_id);
      return ODCICONST.Success;
   end;
end;
/
-- this is necessary for Oracle 9
grant execute on MangoIndex to public;
spool off;
