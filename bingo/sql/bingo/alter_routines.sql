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
spool alter_routines;

CREATE OR REPLACE PACKAGE AlterPackage IS
   mangoIndexType constant pls_integer := 1;
   ringoIndexType constant pls_integer := 2;
   procedure AlterIndex(ia sys.ODCIIndexInfo, parms VARCHAR2, alter_option VARCHAR2, index_type PLS_INTEGER);
   procedure renameColumn(i_schema_name in varchar2, i_table_name in varchar2, 
                          i_old_column_name in varchar2, i_new_column_name in varchar2);
   procedure renameTable(i_schema_name in varchar2,
                         i_old_table_name in varchar2, i_new_table_name in varchar2);
END AlterPackage;
/
CREATE OR REPLACE PACKAGE BODY AlterPackage IS
   /* column rename */
   procedure renameColumn(i_schema_name in varchar2, i_table_name in varchar2, 
                          i_old_column_name in varchar2, i_new_column_name in varchar2) is
      l_old_column_name varchar2(30) := trim(both '"' from i_old_column_name);
      l_new_column_name varchar2(30) := trim(both '"' from i_new_column_name);
   begin
      update context
      set column_name = l_new_column_name
      where schema_name = i_schema_name
        and table_name  = i_table_name
        and column_name = i_old_column_name;
      commit;
   end;
   procedure renameColumn(ia sys.ODCIIndexInfo, i_new_column_name VARCHAR2) is
   begin
      renameColumn(ia.IndexCols(1).TableSchema, ia.IndexCols(1).TableName, ia.IndexCols(1).ColName, i_new_column_name);
   end;
   /* table rename */
   procedure renameTable(i_schema_name in varchar2,
                         i_old_table_name in varchar2, i_new_table_name in varchar2) is
   begin
      update context
      set table_name = i_new_table_name
      where schema_name = i_schema_name
        and table_name  = i_old_table_name;
      commit;
   end;   
   procedure renameTable(ia sys.ODCIIndexInfo, i_new_table_name VARCHAR2) is      
   begin
      renameTable(ia.IndexCols(1).TableSchema, ia.IndexCols(1).TableName, i_new_table_name);
   end;
   /* rebuild index */
   procedure rebuildIndex(ia sys.ODCIIndexInfo, parms VARCHAR2, index_type PLS_INTEGER) is
     context_id number;
     col sys.ODCIColInfo := ia.IndexCols(1);
     full_table_name varchar2(100) := '"'||col.TableSchema||'"."'||col.TableName||'"';
   begin
     context_id := BingoPackage.getContextID(ia);     
     case index_type 
       when mangoIndexType then
         mangoDropIndex(context_id);
         mangoCreateIndex(context_id, parms, full_table_name, col.ColName, col.ColTypeName);
       when ringoIndexType then
         ringoDropIndex(context_id);
         ringoCreateIndex(context_id, parms, full_table_name, col.ColName, col.ColTypeName);
       else
         raise_application_error(-20356, 'Index type is not supported');
     end case;       
   end;
   /* common alter */
   procedure AlterIndex(ia sys.ODCIIndexInfo, parms VARCHAR2, alter_option VARCHAR2, index_type PLS_INTEGER) is
   begin
     case alter_option
       when ODCIConst.AlterIndexNone then
         raise_application_error(-20356, 'Alter option is not supported, please use REBUILD INDEX instead');
       when ODCIConst.AlterIndexRename then
         null;
       when ODCIConst.AlterIndexRebuild then
         rebuildIndex(ia, parms, index_type);
$if dbms_db_version.version >= 11 $then
       when ODCIConst.AlterIndexRenameCol then
         renameColumn(ia, parms);
       when ODCIConst.AlterIndexRenameTab then
         renameTable(ia, parms);
$end
       else
         raise_application_error(-20356, 'Alter option is not supported');
       end case;
   end;
end AlterPackage;
/

create or replace procedure renameColumn(i_schema_name in varchar2, i_table_name in varchar2, 
                                         i_old_column_name in varchar2, i_new_column_name in varchar2) is
begin
  AlterPackage.renameColumn(i_schema_name, i_table_name, i_old_column_name, i_new_column_name);
end;
/
                          
create or replace procedure renameTable(i_schema_name in varchar2,
                                        i_old_table_name in varchar2, i_new_table_name in varchar2) is
begin
  AlterPackage.renameTable(i_schema_name, i_old_table_name, i_new_table_name);
end;
/

begin
  $if dbms_db_version.version < 11 $then
    execute immediate 'grant execute on renameColumn to public';
    execute immediate 'grant execute on renameTable  to public';
  $else
    null;
  $end
end;
/

spool off;
