-- Copyright (C) 2009-2013 GGA Software Services LLC
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
spool bingo_calls;

create or replace procedure DropTable (tname in varchar2) is
  table_exists boolean;
  shema_name   varchar2(100);
begin
  select SYS_CONTEXT('USERENV', 'CURRENT_SCHEMA') into shema_name from dual;
  table_exists := false;
  for item in (select table_name from user_tables where table_name in (tname)) loop
    table_exists := true;
  end loop;
  for item in (select ttt from (select (shema_name || '.' || table_name) ttt from user_tables) where ttt in (tname)) loop
    table_exists := true;
  end loop;
  if table_exists then
    execute immediate 'DROP TABLE ' || tname;
  end if;
end DropTable;
/
create or replace function FileToCLOB (filename in string) return clob
  AS language C name "oraLoadFileToCLOB" library bingolib
  with context parameters(context, filename, filename indicator short,
                          return indicator short, return OCILobLocator);
/
grant execute on FileToCLOB to public;
/
create or replace function FileToBLOB (filename in string) return blob
  AS language C name "oraLoadFileToBLOB" library bingolib
  with context parameters(context, filename, filename indicator short,
                          return indicator short, return OCILobLocator);
/
grant execute on FileToBLOB to public;
/
create or replace function FileToString(filename in varchar2) return VARCHAR2 as
      language C name "oraLoadFileToString" library bingolib
      with context parameters(context, filename, filename indicator short,
                              return indicator short, return OCIString);
/
grant execute on FileToString to public;
/
create or replace procedure CLOBToFile(lob in CLOB, filename in varchar2) as
      language C name "oraSaveLOBToFile" library bingolib
      with context parameters(context, lob, lob indicator short,
                              filename, filename indicator short);
/
grant execute on CLOBToFile to public;
/
create or replace procedure LogPrint(str in varchar2) as
      language C name "oraLogPrint" library bingolib  
      with context parameters (context, str);
/

create or replace procedure ConfigResetAll (context_id in binary_integer)
  AS language C name "oraConfigResetAll" library bingolib
  with context parameters(context, context_id);
/

create or replace procedure ConfigReset (context_id in binary_integer, key_name in string)
  AS language C name "oraConfigReset" library bingolib
  with context parameters(context, context_id, key_name, key_name indicator short);
/

create or replace procedure ConfigSetInt (context_id in binary_integer, key_name in string, value in Number)
  AS language C name "oraConfigSetInt" library bingolib
  with context parameters(context, context_id, key_name, key_name indicator short, value, value indicator short);
/

create or replace function ConfigGetInt (context_id in binary_integer, key_name in string) return NUMBER
  AS language C name "oraConfigGetInt" library bingolib
  with context parameters(context, context_id, key_name, key_name indicator short, return OCINumber);
/

create or replace procedure ConfigSetFloat (context_id in binary_integer, key_name in string, value in Number)
  AS language C name "oraConfigSetFloat" library bingolib
  with context parameters(context, context_id, key_name, key_name indicator short, value, value indicator short);
/

create or replace function ConfigGetFloat (context_id in binary_integer, key_name in string) return NUMBER
  AS language C name "oraConfigGetFloat" library bingolib
  with context parameters(context, context_id, key_name, key_name indicator short, return OCINumber);
/

create or replace procedure ConfigSetString (context_id in binary_integer, key_name in string, value in string)
  AS language C name "oraConfigSetString" library bingolib
  with context parameters(context, context_id, key_name, key_name indicator short, value, value indicator short);
/

create or replace function ConfigGetString (context_id in binary_integer, key_name in string) return string
  AS language C name "oraConfigGetString" library bingolib
  with context parameters(context, context_id, key_name, key_name indicator short, return OCIString);
/

create or replace function ProfilingGetCount (key_name in string) return NUMBER
  AS language C name "oraProfilingGetCount" library bingolib
  with context parameters(context, key_name, key_name indicator short, return OCINumber);
/

grant execute on ProfilingGetCount to public;
/

create or replace function ProfilingGetTime (key_name in string) return NUMBER
  AS language C name "oraProfilingGetTime" library bingolib
  with context parameters(context, key_name, key_name indicator short, return OCINumber);
/
grant execute on ProfilingGetTime to public;
/

create or replace procedure ProfilingPrint (print_all in Number)
  AS language C name "oraProfilingPrint" library bingolib
  with context parameters(context, print_all, print_all indicator short);
/
grant execute on ProfilingPrint to public;
/

create or replace procedure ExportSDF (table_name in string, clob_col in string, other_cols in string, filename in string)
   AS language C name "oraExportSDF" library bingolib
  with context parameters(context,
                          table_name, table_name indicator short,
                          clob_col, clob_col indicator short,
                          other_cols, other_cols indicator short,
                          filename, filename indicator short);
/
grant execute on ExportSDF to public;
/
create or replace procedure ExportSDFZip (table_name in string, clob_col in string, other_cols in string, filename in string)
   AS language C name "oraExportSDFZip" library bingolib
  with context parameters(context,
                          table_name, table_name indicator short,
                          clob_col, clob_col indicator short,
                          other_cols, other_cols indicator short,
                          filename, filename indicator short);
/
grant execute on ExportSDFZip to public;
/
create or replace procedure ImportSDF (table_name in string, clob_col in string, other_cols in string, filename in string)
   AS language C name "oraImportSDF" library bingolib
  with context parameters(context,
                          table_name, table_name indicator short,
                          clob_col, clob_col indicator short,
                          other_cols, other_cols indicator short,
                          filename, filename indicator short);
/
grant execute on ImportSDF to public;
create or replace procedure ImportRDF (table_name in string, clob_col in string, other_cols in string, filename in string)
   AS language C name "oraImportRDF" library bingolib
  with context parameters(context,
                          table_name, table_name indicator short,
                          clob_col, clob_col indicator short,
                          other_cols, other_cols indicator short,
                          filename, filename indicator short);
/
grant execute on ImportRDF to public;
create or replace procedure ImportSMILES (table_name in string, smiles_col in string, id_col in string, filename in string)
   AS language C name "oraImportSMILES" library bingolib
  with context parameters(context,
                          table_name, table_name indicator short,
                          smiles_col, smiles_col indicator short,
                          id_col, id_col indicator short,
                          filename, filename indicator short);
/
grant execute on ImportSMILES to public;
create or replace procedure RingoFlushInserts (docommit in binary_integer)
   AS language C name "oraRingoFlushInserts" library bingolib
  with context parameters(context, docommit);
/
create or replace procedure MangoFlushInserts (docommit in binary_integer)
   AS language C name "oraMangoFlushInserts" library bingolib
  with context parameters(context, docommit);
/
create or replace procedure Zip2 (src in CLOB, dest in BLOB) 
  AS language C name "oraBingoZip" library bingolib  
  with context parameters (context, src, src indicator short, 
                           dest, dest indicator short);
/
grant execute on Zip2 to public;
/
create or replace function Zip (src in CLOB) return BLOB IS
  lob BLOB;
BEGIN
  dbms_lob.createtemporary(lob, TRUE, dbms_lob.call);
  Zip2(src, lob);
  return lob;
END Zip;
/
grant execute on Zip to public;
/
create or replace procedure Unzip2 (src in BLOB, dest in CLOB)
  AS language C name "oraBingoUnzip" library bingolib  
  with context parameters (context, src, src indicator short, 
                           dest, dest indicator short);
/
grant execute on Unzip2 to public;
/
create or replace function Unzip (src in BLOB) return CLOB IS
  lob CLOB;
BEGIN
  dbms_lob.createtemporary(lob, TRUE, dbms_lob.call);
  Unzip2(src, lob);
  return lob;
END Unzip;
/
grant execute on Unzip to public;
/
create or replace procedure FlushInserts is
begin
   RingoFlushInserts(1);
   MangoFlushInserts(1);
end FlushInserts;
/
grant execute on FlushInserts to public;
/
create or replace trigger LogoffTrigger BEFORE LOGOFF ON DATABASE
begin
   RingoFlushInserts(0);
   MangoFlushInserts(0);
exception
   when others then
     null;
end;
/
create or replace function GetVersion return VARCHAR2
  AS language C name "oraGetVersion" library bingolib
  with context parameters(context, return indicator short, return OCIString);
/
grant execute on GetVersion to public;
/
create or replace function Name (target in CLOB) return VARCHAR2
  AS language C name "oraBingoGetName" library bingolib
  with context parameters(context, target, target indicator short,
                          return indicator short, return OCIString);
/
grant execute on Name to public;
spool off;
