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

Define USER_NAME = &1
Define USER_PASS = &2

Set Verify Off
spool bingo_init;

column TBS_TYPE new_value TBS_TYPE
select case when version like '9.%' then null else 'BIGFILE' end TBS_TYPE from v$instance;

CREATE &TBS_TYPE TABLESPACE &USER_NAME
    NOLOGGING 
    DATAFILE '&USER_NAME..ora' SIZE 64M 
    REUSE AUTOEXTEND 
    ON NEXT 64M 
    MAXSIZE UNLIMITED 
    EXTENT MANAGEMENT LOCAL 
;

CREATE 
    TEMPORARY TABLESPACE &USER_NAME._TEMP 
    TEMPFILE '&USER_NAME._temp.ora' SIZE 64M
    REUSE AUTOEXTEND
    ON NEXT 64M 
    MAXSIZE UNLIMITED 
    EXTENT MANAGEMENT LOCAL 
    UNIFORM SIZE 1M
;

create user &USER_NAME
  identified by &USER_PASS
  default tablespace &USER_NAME
  temporary tablespace &USER_NAME._TEMP
;

grant connect to &USER_NAME;
grant create type to &USER_NAME;
grant create table to &USER_NAME;
grant create library to &USER_NAME;
grant create operator to &USER_NAME;
grant create procedure to &USER_NAME;
grant create sequence to &USER_NAME;
grant create indextype to &USER_NAME;
grant unlimited tablespace to &USER_NAME;
grant create trigger to &USER_NAME;
grant administer database trigger to &USER_NAME;
grant select any table to &USER_NAME;

spool off;

exit;
