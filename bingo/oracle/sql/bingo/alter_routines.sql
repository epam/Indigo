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
