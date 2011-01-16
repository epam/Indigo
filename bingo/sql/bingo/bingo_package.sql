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
spool bingo_package;

CREATE OR REPLACE PACKAGE BingoPackage IS
   function GetContextID (ia sys.ODCIIndexInfo) return NUMBER;
   function GetContextID (col sys.ODCIColInfo) return NUMBER;
   function GetContextID (schema_name in varchar2, table_name in varchar2, column_name in varchar2) return NUMBER;
   function GetContextID (full_table_name_ in varchar2, column_name_ in varchar2) return NUMBER;
END BingoPackage;
/
CREATE OR REPLACE PACKAGE BODY BingoPackage IS
   function GetContextID (ia sys.ODCIIndexInfo) return NUMBER IS
   begin
      return GetContextID(ia.IndexCols(1).TableSchema || '.' || ia.IndexCols(1).TableName, ia.IndexCols(1).ColName);
   end GetContextID;
   function GetContextID (col sys.ODCIColInfo) return NUMBER IS
   begin
      return GetContextID(col.tableSchema, col.tableName, col.colName);
   end GetContextID;
   function GetContextID (schema_name in varchar2, table_name in varchar2, column_name in varchar2) return NUMBER IS
   begin
      return GetContextID(schema_name || '.' || table_name, column_name);
   end GetContextID;
   function GetContextID (full_table_name_ in varchar2, column_name_ in varchar2) return NUMBER IS
		context_id number;
    n          number;
    full_table_name varchar2(1000);
		column_name     varchar2(1000);
		stmt1 varchar2(1000);
		stmt2 varchar2(1000);
		cnum1 integer;       
		cnum2 integer;       
		junk  integer;
   begin
      if (full_table_name_ is null) or (column_name_ is null) then
         return 0;
      end if;
      full_table_name := upper(replace(full_table_name_, '"'));
      column_name := upper(replace(column_name_, '"'));
      stmt1 := 'SELECT id FROM context WHERE full_table_name = ' || '''' || full_table_name || '''' ||
                                       ' AND     column_name = ' || '''' || column_name     || '''';
      cnum1 := dbms_sql.open_cursor;
      dbms_sql.parse(cnum1, stmt1, dbms_sql.native);
      dbms_sql.define_column(cnum1, 1, context_id);
      junk := dbms_sql.execute(cnum1);
      if dbms_sql.fetch_rows(cnum1) > 0 then
         dbms_sql.column_value(cnum1, 1, context_id); 
			   dbms_sql.close_cursor(cnum1);
      else
         execute immediate 'LOCK TABLE context IN EXCLUSIVE MODE';
         context_id := 1;
         dbms_sql.close_cursor(cnum1);
         stmt2 := 'SELECT id FROM context ORDER BY id';
         cnum2 := dbms_sql.open_cursor;
         dbms_sql.parse(cnum2, stmt2, dbms_sql.native);
         dbms_sql.define_column(cnum2, 1, n);
         junk := dbms_sql.execute(cnum2);
         while dbms_sql.fetch_rows(cnum2) > 0 loop
            dbms_sql.column_value(cnum2, 1, n);
            if n = context_id then
               context_id := context_id + 1;
            end if;
         end loop;
         dbms_sql.close_cursor(cnum2);
         execute immediate 'INSERT INTO context VALUES(' || context_id || ',''' ||
                                               full_table_name || ''',''' || column_name || ''')';
         execute immediate 'COMMIT';
      end if;
      return context_id;
   end GetContextID;
END BingoPackage;
/
spool off;
