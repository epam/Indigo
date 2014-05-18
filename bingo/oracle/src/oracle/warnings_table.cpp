/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "oracle/warnings_table.h"

#include "base_cpp/scanner.h"

void WarningsTable::reset ()
{
   _table_name.clear();
   _rowid_column.clear();
   _message_column.clear();
}

void WarningsTable::setTableNameAndColumns (OracleEnv &env, const char *table_name_with_columns)
{
   try
   {
      BufferScanner scanner(table_name_with_columns);
      scanner.readWord(_table_name, "(");
      scanner.skip(1);
      scanner.readWord(_rowid_column, ",");
      scanner.skip(1);
      scanner.readWord(_message_column, ")");
   }
   catch (Scanner::Error &ex)
   {
      if (strlen(table_name_with_columns) > 0)
         env.dbgPrintfTS("Cannot parse %s table name with columns. The format is <name>(<rowid_column>,<message>)", table_name_with_columns);
      reset();
   }
}
 
void WarningsTable::add (OracleEnv &env, const char *rowid, const char *message)
{
   if (_table_name.size() != 0)
   {
      OracleStatement statement(env);

      statement.append("insert into %s (%s, %s) values (:id, :msg)", _table_name.ptr(), _rowid_column.ptr(), _message_column.ptr());
      statement.prepare();
      statement.bindStringByName(":id", rowid, strlen(rowid) + 1);
      statement.bindStringByName(":msg", message, strlen(message) + 1);

      statement.executeAllowNoData();
   }
}
