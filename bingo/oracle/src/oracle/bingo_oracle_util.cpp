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

#include "oracle/bingo_oracle.h"

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"

#include "oracle/bingo_oracle_context.h"
#include "oracle/bingo_profiling.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

#include "base_cpp/auto_ptr.h"
#include "base_cpp/profiling.h"
#include "base_cpp/string_pool.h"
#include "gzip/gzip_output.h"
#include "gzip/gzip_scanner.h"
#include "molecule/rdf_loader.h"
#include "molecule/sdf_loader.h"

ORAEXT void oraLogPrint(OCIExtProcContext* ctx, char* str){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

env.dbgPrintf("%s\n", str);
}
ORABLOCK_END
}

ORAEXT OCILobLocator* oraLoadFileToCLOB(OCIExtProcContext* ctx, char* filename, short filename_indicator, short* return_indicator)
{
    OCILobLocator* result = 0;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (filename_indicator == OCI_IND_NULL)
            throw BingoError("Null filename given");

        FileScanner scanner(filename);

        QS_DEF(std::string, buf);

        scanner.readAll(buf);

        OracleLOB lob(env);

        lob.createTemporaryCLOB();
        lob.write(0, buf);
        *return_indicator = OCI_IND_NOTNULL;
        lob.doNotDelete();
        result = lob.get();
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCILobLocator* oraLoadFileToBLOB(OCIExtProcContext* ctx, char* filename, short filename_indicator, short* return_indicator)
{
    OCILobLocator* result = 0;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (filename_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null filename given");

        FileScanner scanner(filename);

        QS_DEF(std::string, buf);

        scanner.readAll(buf);

        OracleLOB lob(env);

        lob.createTemporaryBLOB();
        lob.write(0, buf.c_str(), buf.size());
        *return_indicator = OCI_IND_NOTNULL;
        lob.doNotDelete();
        result = lob.get();
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCIString* oraLoadFileToString(OCIExtProcContext* ctx, char* filename, short filename_indicator, short* return_indicator)
{
    OCIString* result = 0;

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        *return_indicator = OCI_IND_NULL;

        if (filename_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null filename given");

        FileScanner scanner(filename);
        QS_DEF(std::string, buf);

        scanner.readAll(buf);

        env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)buf.c_str(), buf.size(), &result));

        *return_indicator = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}

ORAEXT void oraSaveLOBToFile(OCIExtProcContext* ctx, OCILobLocator* lob_locator, short lob_indicator, char* file_name, short filename_indicator)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        if (lob_indicator == OCI_IND_NULL)
            throw BingoError("Null LOB given");
        if (filename_indicator == OCI_IND_NULL)
            throw BingoError("Null file name given");

        OracleLOB lob(env, lob_locator);

        QS_DEF(std::string, buf);

        lob.readAll(buf, false);

        FileOutput output(file_name);

        output.writeArray(buf);
    }
    ORABLOCK_END
}

void _exportSDF(OracleEnv& env, const char* table, const char* clob_col, const char* other_cols, Output& output)
{
    QS_DEF(StringPool, col_names);
    QS_DEF(std::string, col_values);
    QS_DEF(std::string, word);
    QS_DEF(std::string, lob_value);

    BufferScanner scanner(other_cols);
    int i;
    int max_size = 1024;

    col_names.clear();
    col_values.clear();

    while (1)
    {
        scanner.skipSpace();
        if (scanner.isEOF())
            break;
        word.clear();
        scanner.readWord(word, 0);
        if (word.size() < 2)
            break;
        col_names.add(word.c_str());
    }

    col_values.resize(col_names.end() * max_size);

    OracleStatement statement(env);
    OracleLOB lob(env);

    lob.createTemporaryCLOB();

    statement.append("SELECT %s", clob_col);
    for (i = col_names.begin(); i != col_names.end(); i = col_names.next(i))
        statement.append(", to_char(%s)", col_names.at(i));

    statement.append(" FROM %s", table);
    statement.prepare();

    statement.defineClobByPos(1, lob);
    for (i = col_names.begin(); i != col_names.end(); i = col_names.next(i))
        statement.defineStringByPos(i + 2, &col_values[0] + i * max_size, max_size);

    if (statement.executeAllowNoData())
        do
        {
            lob.readAll(lob_value, false);

            if (lob_value.size() < 4)
                continue;

            // hack to handle molfiles which have newline (and those which have not)
            if (lob_value.back() == '\n')
                lob_value.pop_back();

            // hack to handle molfiles which have $$$$
            if (strncmp(lob_value.c_str() + lob_value.size() - 4, "$$$$", 4) == 0)
                lob_value.resize(lob_value.size() - 4);

            output.writeArray(lob_value);

            for (i = col_names.begin(); i != col_names.end(); i = col_names.next(i))
                output.printf("\n> <%s>\n%s\n", col_names.at(i), col_values.c_str() + i * max_size);
            output.printf("\n$$$$\n");
        } while (statement.fetch());
}

void _parseFieldList(const char* str, StringPool& props, StringPool& columns)
{
    QS_DEF(std::string, prop);
    QS_DEF(std::string, column);
    BufferScanner scanner(str);

    props.clear();
    columns.clear();
    scanner.skipSpace();

    while (!scanner.isEOF())
    {
        scanner.readWord(prop, " ,");
        scanner.skipSpace();
        scanner.readWord(column, " ,");
        scanner.skipSpace();

        props.add(prop.c_str());
        columns.add(column.c_str());

        if (scanner.isEOF())
            break;

        if (scanner.readChar() != ',')
            throw BingoError("_parseFieldList(): comma expected");
        scanner.skipSpace();
    }
}

void _importSDF(OracleEnv& env, const char* table, const char* clob_col, const char* other_cols, const char* file_name)
{
    FileScanner scanner(file_name);
    int i, nwritten = 0;
    QS_DEF(std::string, word);
    QS_DEF(StringPool, props);
    QS_DEF(StringPool, columns);

    env.dbgPrintfTS("importing into table %s\n", table);

    SdfLoader loader(scanner);

    _parseFieldList(other_cols, props, columns);

    while (!loader.isEOF())
    {
        profTimerStart(tread, "import.read_next");
        loader.readNext();
        profTimerStop(tread);

        OracleStatement statement(env);
        OracleLOB lob(env);

        lob.createTemporaryCLOB();
        lob.write(0, loader.data);

        statement.append("INSERT INTO %s(%s", table, clob_col);

        for (i = columns.begin(); i != columns.end(); i = columns.next(i))
            statement.append(", %s", columns.at(i));

        statement.append(") VALUES(:clobdata");

        for (i = columns.begin(); i != columns.end(); i = columns.next(i))
        {
            if (loader.properties.contains(props.at(i)))
                statement.append(", NULL");
            else
                statement.append(",:%s", columns.at(i));
        }

        statement.append(")");
        statement.prepare();

        statement.bindClobByName(":clobdata", lob);

        for (i = columns.begin(); i != columns.end(); i = columns.next(i))
        {
            if (loader.properties.contains(props.at(i)))
                continue;

            StringOutput out(word);

            out.printf(":%s", columns.at(i));
            out.writeChar(0);

            const char* val = loader.properties.at(props.at(i));

            statement.bindStringByName(word.c_str(), val, strlen(val));
        }

        profTimerStart(tinsert, "import.sql_insert");
        statement.execute();
        profTimerStop(tinsert);

        nwritten++;
        if (nwritten % 1000 == 0)
        {
            env.dbgPrintfTS("imported %d items, commiting\n", nwritten);
            OracleStatement::executeSingle(env, "COMMIT");
        }
    }
    if (nwritten % 1000 != 0)
    {
        env.dbgPrintfTS("imported %d items, commiting\n", nwritten);
        OracleStatement::executeSingle(env, "COMMIT");
    }
}

void _importSMILES(OracleEnv& env, const char* table, const char* smiles_col, const char* id_col, const char* file_name)
{
    FileScanner fscanner(file_name);
    AutoPtr<GZipScanner> gzscanner;
    Scanner* scanner;

    int nwritten = 0;
    QS_DEF(std::string, id);
    QS_DEF(std::string, str);

    env.dbgPrintfTS("importing into table %s\n", table);

    // detect if input is gzipped
    byte magic[2];
    int pos = fscanner.tell();

    fscanner.readCharsFix(2, (char*)magic);
    fscanner.seek(pos, SEEK_SET);

    if (magic[0] == 0x1f && magic[1] == 0x8b)
    {
        gzscanner.reset(new GZipScanner(fscanner));
        scanner = gzscanner.get();
    }
    else
        scanner = &fscanner;

    while (!scanner->isEOF())
    {
        id.clear();
        scanner->readLine(str);
        BufferScanner strscan(str);

        strscan.skipSpace();
        while (!strscan.isEOF() && !isspace(strscan.readChar()))
            ;
        strscan.skipSpace();
        if (strscan.lookNext() == '|')
        {
            strscan.readChar();
            while (!strscan.isEOF() && strscan.readChar() != '|')
                ;
            strscan.skipSpace();
        }

        if (!strscan.isEOF() && id_col != 0)
            strscan.readLine(id);

        OracleStatement statement(env);

        statement.append("INSERT INTO %s(%s", table, smiles_col);

        if (id_col != 0)
            statement.append(", %s", id_col);

        statement.append(") VALUES(:smiles");

        if (id_col != 0)
        {
            if (id.size() > 1)
                statement.append(", :id");
            else
                statement.append(", NULL");
        }
        statement.append(")");
        statement.prepare();

        statement.bindStringByName(":smiles", str.c_str(), str.size());
        if (id.size() > 1)
            statement.bindStringByName(":id", id.c_str(), id.size());

        statement.execute();
        nwritten++;
        if (nwritten % 1000 == 0)
        {
            env.dbgPrintfTS("imported %d items, commiting\n", nwritten);
            OracleStatement::executeSingle(env, "COMMIT");
        }
    }
    if (nwritten % 1000 != 0)
    {
        env.dbgPrintfTS("imported %d items, commiting\n", nwritten);
        OracleStatement::executeSingle(env, "COMMIT");
    }
}

void _importRDF(OracleEnv& env, const char* table, const char* clob_col, const char* other_cols, const char* file_name)
{
    FileScanner scanner(file_name);
    int i, nwritten = 0;
    QS_DEF(std::string, word);
    QS_DEF(StringPool, props);
    QS_DEF(StringPool, columns);

    env.dbgPrintfTS("importing into table %s\n", table);

    _parseFieldList(other_cols, props, columns);

    RdfLoader loader(scanner);

    while (!loader.isEOF())
    {
        loader.readNext();

        OracleStatement statement(env);
        OracleLOB lob(env);

        lob.createTemporaryCLOB();
        lob.write(0, loader.data);

        statement.append("INSERT INTO %s(%s", table, clob_col);

        for (i = columns.begin(); i != columns.end(); i = columns.next(i))
            statement.append(", %s", columns.at(i));

        statement.append(") VALUES(:clobdata");

        for (i = columns.begin(); i != columns.end(); i = columns.next(i))
        {
            if (loader.properties.contains(props.at(i)))
                statement.append(", NULL");
            else
                statement.append(",:%s", columns.at(i));
        }

        statement.append(")");
        statement.prepare();

        statement.bindClobByName(":clobdata", lob);

        for (i = columns.begin(); i != columns.end(); i = columns.next(i))
        {
            if (loader.properties.contains(props.at(i)))
                continue;

            StringOutput out(word);

            out.printf(":%s", columns.at(i));
            out.writeChar(0);

            const char* val = loader.properties.at(props.at(i));

            statement.bindStringByName(word.c_str(), val, strlen(val));
        }

        statement.execute();
        nwritten++;
        if (nwritten % 1000 == 0)
        {
            env.dbgPrintfTS("imported %d items, commiting\n", nwritten);
            OracleStatement::executeSingle(env, "COMMIT");
        }
    }
    if (nwritten % 1000 != 0)
    {
        env.dbgPrintfTS("imported %d items, commiting\n", nwritten);
        OracleStatement::executeSingle(env, "COMMIT");
    }
}

ORAEXT void oraExportSDF(OCIExtProcContext* ctx, const char* table_name, short table_name_ind, const char* clob_col_name, short clob_col_ind,
                         const char* other_col_names, short other_col_ind, const char* file_name, short file_name_ind)
{
    logger.initIfClosed(log_filename);

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        if (table_name_ind != OCI_IND_NOTNULL)
            throw BingoError("null table name");

        if (clob_col_ind != OCI_IND_NOTNULL)
            throw BingoError("null CLOB column name");

        if (file_name_ind != OCI_IND_NOTNULL)
            throw BingoError("null file name");

        if (other_col_ind != OCI_IND_NOTNULL)
            other_col_names = 0;

        FileOutput output(file_name);

        _exportSDF(env, table_name, clob_col_name, other_col_names, output);
    }
    ORABLOCK_END
}

ORAEXT void oraExportSDFZip(OCIExtProcContext* ctx, const char* table_name, short table_name_ind, const char* clob_col_name, short clob_col_ind,
                            const char* other_col_names, short other_col_ind, const char* file_name, short file_name_ind)
{
    logger.initIfClosed(log_filename);

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        if (table_name_ind != OCI_IND_NOTNULL)
            throw BingoError("null table name");

        if (clob_col_ind != OCI_IND_NOTNULL)
            throw BingoError("null CLOB column name");

        if (file_name_ind != OCI_IND_NOTNULL)
            throw BingoError("null file name");

        if (other_col_ind != OCI_IND_NOTNULL)
            other_col_names = 0;

        FileOutput output(file_name);
        GZipOutput gzoutput(output, 9);

        _exportSDF(env, table_name, clob_col_name, other_col_names, gzoutput);
    }
    ORABLOCK_END
}

ORAEXT void oraImportSDF(OCIExtProcContext* ctx, const char* table_name, short table_name_ind, const char* clob_col_name, short clob_col_ind,
                         const char* other_col_names, short other_col_ind, const char* file_name, short file_name_ind)
{
    logger.initIfClosed(log_filename);

    ORABLOCK_BEGIN
    {
        profTimersReset();
        profTimerStart(ttotal, "total");

        OracleEnv env(ctx, logger);

        if (table_name_ind != OCI_IND_NOTNULL)
            throw BingoError("null table name");

        if (clob_col_ind != OCI_IND_NOTNULL)
            throw BingoError("null table name");

        if (clob_col_ind != OCI_IND_NOTNULL)
            throw BingoError("null file name");

        if (other_col_ind != OCI_IND_NOTNULL)
            other_col_names = 0;

        _importSDF(env, table_name, clob_col_name, other_col_names, file_name);

        profTimerStop(ttotal);
        bingoProfilingPrintStatistics(false);
    }
    ORABLOCK_END
}

ORAEXT void oraImportRDF(OCIExtProcContext* ctx, const char* table_name, short table_name_ind, const char* clob_col_name, short clob_col_ind,
                         const char* other_col_names, short other_col_ind, const char* file_name, short file_name_ind)
{
    logger.initIfClosed(log_filename);

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        if (table_name_ind != OCI_IND_NOTNULL)
            throw BingoError("null table name");

        if (clob_col_ind != OCI_IND_NOTNULL)
            throw BingoError("null table name");

        if (clob_col_ind != OCI_IND_NOTNULL)
            throw BingoError("null file name");

        if (other_col_ind != OCI_IND_NOTNULL)
            other_col_names = 0;

        _importRDF(env, table_name, clob_col_name, other_col_names, file_name);
    }
    ORABLOCK_END
}

ORAEXT void oraImportSMILES(OCIExtProcContext* ctx, const char* table_name, short table_name_ind, const char* smiles_col_name, short smiles_col_ind,
                            const char* id_col_name, short id_col_ind, const char* file_name, short file_name_ind)
{
    logger.initIfClosed(log_filename);

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        if (table_name_ind != OCI_IND_NOTNULL)
            throw BingoError("null table name");

        if (smiles_col_ind != OCI_IND_NOTNULL)
            throw BingoError("null SMILES column name");

        if (file_name_ind != OCI_IND_NOTNULL)
            throw BingoError("null file name");

        if (id_col_ind != OCI_IND_NOTNULL)
            id_col_name = 0;

        _importSMILES(env, table_name, smiles_col_name, id_col_name, file_name);
    }
    ORABLOCK_END
}

ORAEXT OCIString* oraGetVersion(OCIExtProcContext* ctx, short* return_indicator)
{
    OCIString* result = 0;

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        // env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (const oratext *)bingo_version_string,
        //   strlen(bingo_version_string), &result));
        env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (const oratext*)BINGO_VERSION, strlen(BINGO_VERSION), &result));

        *return_indicator = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}

ORAEXT void oraBingoZip(OCIExtProcContext* ctx, OCILobLocator* source_locator, short source_indicator, OCILobLocator* dest_locator,
                        short dest_indicator){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

if (source_indicator == OCI_IND_NULL)
    throw BingoError("null source given");
if (dest_indicator == OCI_IND_NULL)
    throw BingoError("null destination given");

OracleLOB source_lob(env, source_locator);

QS_DEF(std::string, source);
QS_DEF(std::string, dest);

source_lob.readAll(source, false);

{
    StringOutput output(dest);
    GZipOutput gzoutput(output, 9);

    gzoutput.write(source.c_str(), source.size());
    gzoutput.flush();
}

OracleLOB result_lob(env, dest_locator);

result_lob.write(0, dest);
result_lob.trim(dest.size());
}
ORABLOCK_END
}

ORAEXT void oraBingoUnzip(OCIExtProcContext* ctx, OCILobLocator* source_locator, short source_indicator, OCILobLocator* dest_locator,
                          short dest_indicator){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

if (source_indicator == OCI_IND_NULL)
    throw BingoError("null source given");
if (dest_indicator == OCI_IND_NULL)
    throw BingoError("null destination given");

OracleLOB source_lob(env, source_locator);

QS_DEF(std::string, source);
QS_DEF(std::string, dest);

source_lob.readAll(source, false);

{
    BufferScanner scanner(source);
    GZipScanner gzscanner(scanner);

    gzscanner.readAll(dest);
}

OracleLOB result_lob(env, dest_locator);

result_lob.write(0, dest);
result_lob.trim(dest.size());
}
ORABLOCK_END
}

ORAEXT OCIString* oraBingoGetName(OCIExtProcContext* ctx, OCILobLocator* source_locator, short source_indicator, short* return_indicator)
{
    OCIString* result = NULL;

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        if (source_indicator == OCI_IND_NULL)
            throw BingoError("null source given");

        QS_DEF(std::string, source);
        QS_DEF(std::string, name);
        OracleLOB source_lob(env, source_locator);

        source_lob.readAll(source, false);

        BufferScanner scanner(source);
        bingoGetName(scanner, name);

        if (name.size() < 1)
        {
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            OCIStringAssignText(env.envhp(), env.errhp(), (text*)"nil", 3, &result);
            *return_indicator = OCI_IND_NULL;
        }
        else
        {
            OCIStringAssignText(env.envhp(), env.errhp(), (text*)name.c_str(), strlen(name.c_str()), &result);
            *return_indicator = OCI_IND_NOTNULL;
        }
    }
    ORABLOCK_END

    return result;
}
