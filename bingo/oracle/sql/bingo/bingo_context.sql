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
spool bingo_context;

create table context 
(
  id int, 
  schema_name varchar2(30), 
  table_name  varchar2(30),
  column_name varchar2(30),
  constraint pk_context primary key (id),
  constraint uk_context unique (schema_name, table_name, column_name)
);
column context_id new_value context_id
select nvl(max(id) + 1, 1) context_id from context;
create sequence s_context 
  minvalue 1
  increment by 1
  start with &context_id
  nocache 
  order
  nocycle;
  
spool off;
