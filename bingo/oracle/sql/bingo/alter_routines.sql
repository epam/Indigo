-- Copyright (C) 2009-2015 EPAM Systems
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
