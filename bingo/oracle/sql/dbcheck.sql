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

Set Verify Off
Set Heading Off
Set Feedback Off
set arraysize 1
set maxdata 6000

Create Or Replace Function CHECK_DB_OBJECTS Return Varchar2 Is

nTables Number ;
nViews Number ;
nProcedures Number ;
nFunctions Number ;
nPackages Number ;
nSequences Number ;
nInvalids Number ;

Begin

   Select Count (*)
      Into nTables
      From USER_OBJECTS
      Where OBJECT_TYPE = 'TABLE';

   Select Count (*)
      Into nViews
      From USER_OBJECTS
      Where OBJECT_TYPE = 'VIEW';

   Select Count (*)
      Into nProcedures
      From USER_OBJECTS
      Where OBJECT_TYPE = 'PROCEDURE';

   Select Count (*)
      Into nFunctions
      From USER_OBJECTS
      Where OBJECT_TYPE = 'FUNCTION';

   Select Count (*)
      Into nPackages
      From USER_OBJECTS
      Where OBJECT_TYPE = 'PACKAGE';

   Select Count (*)
      Into nSequences
      From USER_OBJECTS
      Where OBJECT_TYPE = 'SEQUENCE';

   Select Count (*)
      Into nInvalids
      From USER_OBJECTS
      Where STATUS = 'INVALID';


   Return ( '--------- REPORT ---------' || chr (13) || chr (10) ||
            'Tables: ' || chr (09) || chr (09) || nTables || chr (13) || chr (10) ||
            'Views: ' || chr (09) || chr (09) || chr (09) || nViews || chr (13) || chr (10) ||
            'Packages: ' || chr (09) || chr (09) || nPackages || chr (13) || chr (10) ||
            'Procedures: ' || chr (09) || chr (09) || nProcedures || chr (13) || chr (10) ||
            'Functions: ' || chr (09) || chr (09) || nFunctions || chr (13) || chr (10) ||
            'Sequences: ' || chr (09) || chr (09) || nSequences || chr (13) || chr (10) ||
            '--------------------------' ) ;

End ;

/
Select CHECK_DB_OBJECTS From Sys.Dual ;
Drop Function  CHECK_DB_OBJECTS;
Select OBJECT_TYPE || ' ' || OBJECT_NAME
   From USER_OBJECTS
   Where STATUS = 'INVALID'
   Order By OBJECT_TYPE ;
Select * From USER_ERRORS;
Prompt DB check executed.
--Prompt Type QUIT to leave SQLPLUS command prompt.
exit;
