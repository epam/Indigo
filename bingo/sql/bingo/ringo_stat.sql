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
spool ringo_stat;

create or replace type RingoStat as object
(
   dummy number,
   static function ODCIGetInterfaces(ifclist out sys.ODCIObjectList) return NUMBER,
   static function ODCIStatsCollect(col sys.ODCIColInfo, options sys.ODCIStatsOptions,
                                   rawstats out RAW, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsDelete(col sys.ODCIColInfo, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsCollect(ia sys.ODCIIndexInfo, options sys.ODCIStatsOptions,
                                   rawstats out RAW, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsDelete(ia sys.ODCIIndexInfo, env sys.ODCIEnv) return NUMBER,

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target VARCHAR2, query VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target VARCHAR2, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target VARCHAR2, query CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target VARCHAR2, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target CLOB, query VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target CLOB, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target CLOB, query CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target CLOB, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target BLOB, query VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target BLOB, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target BLOB, query CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target BLOB, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,

   static function Selectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
     
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target VARCHAR2, query VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target VARCHAR2, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target VARCHAR2, query CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target VARCHAR2, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target CLOB, query VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target CLOB, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target CLOB, query CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target CLOB, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target BLOB, query VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target BLOB, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target BLOB, query CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target BLOB, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,

   static function FunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER,

   static function ODCIStatsIndexCost(ia sys.ODCIIndexInfo, sel NUMBER, cost out sys.ODCICost,
      qi sys.ODCIQueryInfo, pred sys.ODCIPredInfo, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, query VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsIndexCost(ia sys.ODCIIndexInfo, sel NUMBER, cost out sys.ODCICost,
      qi sys.ODCIQueryInfo, pred sys.ODCIPredInfo, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsIndexCost(ia sys.ODCIIndexInfo, sel NUMBER, cost out sys.ODCICost,
      qi sys.ODCIQueryInfo, pred sys.ODCIPredInfo, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, query CLOB, env sys.ODCIEnv) return NUMBER,
   static function ODCIStatsIndexCost(ia sys.ODCIIndexInfo, sel NUMBER, cost out sys.ODCICost,
      qi sys.ODCIQueryInfo, pred sys.ODCIPredInfo, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER
);
/
create or replace type body RingoStat is
   static function ODCIGetInterfaces (ifclist out sys.ODCIObjectList)
      return NUMBER is
   begin
      ifclist := sys.ODCIObjectList(sys.ODCIObject('SYS','ODCISTATS2'));
      return ODCIConst.Success;
   end ODCIGetInterfaces;
  
   static function ODCIStatsCollect (col sys.ODCIColInfo,
                                     options sys.ODCIStatsOptions,
                                     rawstats out RAW, env sys.ODCIEnv) return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(col);

      sys.ODCIColInfoDump(col);
      sys.ODCIStatsOptionsDump(options);

      ringoCollectStatistics(context_id);
      return ODCIConst.Success;
   end;
  
   static function ODCIStatsCollect (ia sys.ODCIIndexInfo,
                                     options sys.ODCIStatsOptions,
                                     rawstats out RAW, env sys.ODCIEnv) return NUMBER is
      context_id binary_integer;
   begin
      context_id := BingoPackage.getContextID(ia);
      ringoCollectStatistics(context_id);
      return ODCIConst.Success;
   end;   
  
   static function ODCIStatsDelete(col sys.ODCIColInfo, env sys.ODCIEnv) 
      return NUMBER is
   begin
      return ODCIConst.Success;
   end;
  
   static function ODCIStatsDelete(ia sys.ODCIIndexInfo, env sys.ODCIEnv) 
      return NUMBER is
   begin
      return ODCIConst.Success;
   end;
  
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target VARCHAR2, query VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, to_clob(query), null, env);
   end;
   
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target VARCHAR2, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, to_clob(query), params, env);
   end;

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target VARCHAR2, query CLOB, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, query, null, env);
   end;

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
               strt NUMBER, stop NUMBER, target VARCHAR2, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, query, params, env);
   end;

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target CLOB, query VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, to_clob(query), null, env);
   end;
   
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target CLOB, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, to_clob(query), params, env);
   end;

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target CLOB, query CLOB, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, query, null, env);
   end;

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
               strt NUMBER, stop NUMBER, target CLOB, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, query, params, env);
   end;

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target BLOB, query VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, to_clob(query), null, env);
   end;
   
   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target BLOB, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, to_clob(query), params, env);
   end;

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, target BLOB, query CLOB, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, query, null, env);
   end;

   static function ODCIStatsSelectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
               strt NUMBER, stop NUMBER, target BLOB, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return Selectivity(pred, sel, args, strt, stop, query, params, env);
   end;

   static function Selectivity (pred sys.ODCIPredInfo, sel out NUMBER, args sys.ODCIArgDescList,
               strt NUMBER, stop NUMBER, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
      colname VARCHAR2(30);
      context_id binary_integer;
   begin
      if  (args(3).ArgType != ODCIConst.ArgCol) THEN
         LogPrint('ODCIStatsSelectivity: args(3) type mismatch, returning error');
         return ODCIConst.Error;
      end if;
      
      colname := rtrim(ltrim(args(3).colName, '"'), '"');
      context_id := BingoPackage.getContextID(args(3).tableSchema, args(3).tableName, colname);
      
      if bitand(pred.Flags, ODCIConst.PredExactMatch) != 0 then
         sel := ringoIndexSelectivity(context_id, pred.MethodName, query, strt, strt, params);
      else
         sel := ringoIndexSelectivity(context_id, pred.MethodName, query, strt, stop, params);
      end if;
      
      sel := least(100, ceil(100*sel));
      return ODCIConst.Success;
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target VARCHAR2, query VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, to_clob(query), null, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target VARCHAR2, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, to_clob(query), params, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target VARCHAR2, query CLOB, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, query, null, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target VARCHAR2, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, query, params, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target CLOB, query VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, to_clob(query), null, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target CLOB, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, to_clob(query), params, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target CLOB, query CLOB, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, query, null, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target CLOB, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, query, params, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target BLOB, query VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, to_clob(query), null, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target BLOB, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, to_clob(query), params, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target BLOB, query CLOB, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, query, null, env);
   end;

   static function ODCIStatsFunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, target BLOB, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return FunctionCost(func, cost, args, query, params, env);
   end;


   static function FunctionCost(func sys.ODCIFuncInfo, cost out sys.ODCICost,
      args sys.ODCIArgDescList, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      cost := sys.ODCICost(NULL, NULL, NULL, NULL);
      cost.IOCost := 1;
      cost.CPUCost := 1;
      return ODCIConst.Success;
   end;

   
   static function ODCIStatsIndexCost(ia sys.ODCIIndexInfo, sel NUMBER, cost out sys.ODCICost,
      qi sys.ODCIQueryInfo, pred sys.ODCIPredInfo, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, query VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return ODCIStatsIndexCost(ia, sel, cost, qi, pred, args, strt, stop, to_clob(query), null, env);
   end;

   static function ODCIStatsIndexCost(ia sys.ODCIIndexInfo, sel NUMBER, cost out sys.ODCICost,
      qi sys.ODCIQueryInfo, pred sys.ODCIPredInfo, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, query VARCHAR2, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
   begin
      return ODCIStatsIndexCost(ia, sel, cost, qi, pred, args, strt, stop, to_clob(query), params, env);
   end;

   static function ODCIStatsIndexCost(ia sys.ODCIIndexInfo, sel NUMBER, cost out sys.ODCICost,
      qi sys.ODCIQueryInfo, pred sys.ODCIPredInfo, args sys.ODCIArgDescList,
      strt NUMBER, stop NUMBER, query CLOB, env sys.ODCIEnv) return NUMBER is
   begin
      return ODCIStatsIndexCost(ia, sel, cost, qi, pred, args, strt, stop, query, null, env);
   end;

   static function ODCIStatsIndexCost(ia sys.ODCIIndexInfo, sel NUMBER, cost out sys.ODCICost,
                qi sys.ODCIQueryInfo, pred sys.ODCIPredInfo, args sys.ODCIArgDescList,
                strt NUMBER, stop NUMBER, query CLOB, params VARCHAR2, env sys.ODCIEnv) return NUMBER is
      colname VARCHAR2(30);
      context_id binary_integer;
      iocost binary_integer;
      cpucost binary_integer;
   begin
      if args(3).ArgType != ODCIConst.ArgCol then
         LogPrint('ODCIStatsIndexCost: args(3) type mismatch, returning error');
         return ODCIConst.Error;
      end if;
    
      colname := rtrim(ltrim(args(3).colName, '"'), '"');
      context_id := BingoPackage.getContextID(args(3).tableSchema, args(3).tableName, colname);
      
      ringoIndexCost(context_id, sel / 100.0, pred.ObjectName, query, strt, stop, params, iocost, cpucost);
      cost := sys.ODCICost(NULL, NULL, NULL, NULL);
      cost.IOCost := iocost;
      cost.CPUCost := cpucost;
      return ODCIConst.Success;
   end;
end;
/
grant execute on RingoStat to public;

spool off;
