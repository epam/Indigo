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
