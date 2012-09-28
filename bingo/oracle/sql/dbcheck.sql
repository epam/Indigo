-- Copyright (C) 2009-2012 GGA Software Services LLC
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
