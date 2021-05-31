/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "oracle/warnings_table.h"

#include "base_cpp/scanner.h"

void WarningsTable::reset()
{
    _table_name.clear();
    _rowid_column.clear();
    _message_column.clear();
}

void WarningsTable::setTableNameAndColumns(OracleEnv& env, const char* table_name_with_columns)
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
    catch (Scanner::Error& ex)
    {
        if (strlen(table_name_with_columns) > 0)
            env.dbgPrintfTS("Cannot parse %s table name with columns. The format is <name>(<rowid_column>,<message>)", table_name_with_columns);
        reset();
    }
}

void WarningsTable::add(OracleEnv& env, const char* rowid, const char* message)
{
    if (_table_name.size() != 0)
    {
        OracleStatement statement(env);

        statement.append("insert into %s (%s, %s) values (:id, :msg)", _table_name.c_str(), _rowid_column.c_str(), _message_column.c_str());
        statement.prepare();
        statement.bindStringByName(":id", rowid, strlen(rowid) + 1);
        statement.bindStringByName(":msg", message, strlen(message) + 1);

        statement.executeAllowNoData();
    }
}
