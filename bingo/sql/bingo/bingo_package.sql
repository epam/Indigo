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
   function createContextID(ia sys.ODCIIndexInfo) return NUMBER;
   procedure deleteContextID(i_context_id NUMBER); 
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
   end;
   function createContextID(ia sys.ODCIIndexInfo) return NUMBER IS
      l_schema_name varchar2(30) := ia.IndexCols(1).TableSchema;
      l_table_name varchar2(30)  := ia.IndexCols(1).TableName;
      l_column_name varchar2(30) := trim(both '"' from ia.IndexCols(1).ColName);
      l_context_id  pls_integer;
      l_dummy number;
      l_bingo_name varchar2(30);
   begin
      begin 
         select 1 into l_dummy
         from all_tables t
         where t.owner = l_schema_name
           and t.table_name = l_table_name;
      exception
       when no_data_found then
          select sys_context('USERENV', 'CURRENT_SCHEMA') into l_bingo_name from dual;
          raise_application_error(-20351,   'Table "'||l_schema_name||'"."'||l_table_name
                                          ||'" not accessible for reading (did you forget GRANT SELECT ON "'
                                          ||l_schema_name||'"."'||l_table_name||'" TO "'
                                          ||l_bingo_name||'" ?)');  
      end;
      insert into context (id, schema_name, table_name, column_name)
      values (s_context.nextval, l_schema_name, l_table_name, l_column_name)
      returning id into l_context_id;
      commit;
      return l_context_id;
   end;
   procedure deleteContextID(i_context_id NUMBER) is
   begin
      delete from context where id = i_context_id;
      commit;
   end;
END BingoPackage;
/

spool off;
