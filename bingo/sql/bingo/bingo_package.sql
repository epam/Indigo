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
   function GetContextID (i_schema_name in varchar2, i_table_name in varchar2, i_column_name in varchar2) return NUMBER;
END BingoPackage;
/
CREATE OR REPLACE PACKAGE BODY BingoPackage IS
   function GetContextID (ia sys.ODCIIndexInfo) return NUMBER IS
   begin
      return GetContextID(ia.IndexCols(1));
   end GetContextID;
   function GetContextID (col sys.ODCIColInfo) return NUMBER IS
   begin
      return GetContextID(col.TableSchema, col.TableName, col.ColName);
   end GetContextID;
   function GetContextID (i_schema_name in varchar2, i_table_name in varchar2, i_column_name in varchar2) return NUMBER is
      l_column_name varchar2(30) := trim(both '"' from i_column_name);
      l_context_id  pls_integer;
   begin
      select id into l_context_id
      from context
      where schema_name = i_schema_name 
        and table_name  = i_table_name
        and column_name = l_column_name;
      return l_context_id;
   exception
      when no_data_found then
         insert into context (id, schema_name, table_name, column_name)
         values (s_context.nextval, i_schema_name, i_table_name, l_column_name)
         returning id into l_context_id;
         commit;
         return l_context_id;
   end;   
END BingoPackage;
/

spool off;
