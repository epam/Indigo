#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"
#include "catalog/pg_type.h"
#include "executor/spi.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/int8.h"
}

#include "bingo_pg_fix_post.h"

#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "bingo_core_c.h"
#include "bingo_pg_common.h"
#include "bingo_pg_cursor.h"
#include "bingo_pg_text.h"
#include "bingo_postgres.h"
#include "molecule/molecule.h"

extern "C"
{

    BINGO_FUNCTION_EXPORT(importsdf);

    BINGO_FUNCTION_EXPORT(importrdf);

    BINGO_FUNCTION_EXPORT(importsmiles);
}

using namespace indigo;

static void checkImportNull(Datum text_datum, const char* message, BingoPgText& text)
{
    if (text_datum == 0)
        throw BingoPgError("can not import structures: %s is empty", message);
    text.init(text_datum);
    int text_size = 0;
    text.getText(text_size);
    if (text_size == 0)
        throw BingoPgError("can not import structures: %s is empty", message);
}
static void checkImportEmpty(Datum text_datum, BingoPgText& text)
{
    if (text_datum == 0)
        text.initFromString("");
    else
        text.init(text_datum);
}

class BingoImportHandler : public BingoPgCommon::BingoSessionHandler
{
public:
    class ImportColumn
    {
    public:
        ImportColumn()
        {
        }
        ~ImportColumn()
        {
        }

        ArrayChar columnName;
        Oid type;

    private:
        ImportColumn(const ImportColumn&); // no implicit copy
    };

    class ImportData
    {
    public:
        ImportData()
        {
        }
        virtual ~ImportData()
        {
        }

        virtual uintptr_t getDatum() = 0;
        virtual void convert(const char* str) = 0;

    private:
        ImportData(const ImportData&); // no implicit copy
    };
    class ImportTextData : public ImportData
    {
    public:
        ImportTextData()
        {
        }
        virtual ~ImportTextData()
        {
        }

        BingoPgText data;

        virtual uintptr_t getDatum()
        {
            return data.getDatum();
        }
        virtual void convert(const char* str)
        {
            if (str != 0)
                data.initFromString(str);
            else
                data.clear();
        }

    private:
        ImportTextData(const ImportTextData&); // no implicit copy
    };

    class ImportInt8Data : public ImportData
    {
    public:
        ImportInt8Data() : data(0)
        {
        }
        virtual ~ImportInt8Data()
        {
        }
        AutoPtr<int64> data;

        virtual void convert(const char* str)
        {
            if (str == 0)
                data.reset(0);
            else
            {
                BINGO_PG_TRY
                {
                    data.reset(new int64);
                    scanint8(str, false, data.get());
                }
                BINGO_PG_HANDLE(data.reset(0); throw BingoPgError("error while converting to int64: %s", message));
            }
        }

        virtual uintptr_t getDatum()
        {
            if (data.get() == 0)
                return 0;
            else
                return Int64GetDatum(data.ref());
        }

    private:
        ImportInt8Data(const ImportInt8Data&); // no implicit copy
    };
    class ImportInt4Data : public ImportData
    {
    public:
        ImportInt4Data() : data(0)
        {
        }
        virtual ~ImportInt4Data()
        {
        }
        AutoPtr<int32> data;

        virtual void convert(const char* str)
        {
            if (str == 0)
                data.reset(0);
            else
            {
                /*
                 * Pg atoi workaround
                 */
                QS_DEF(ArrayChar, str2);
                str2.readString(str, true);
                BINGO_PG_TRY
                {
                    data.reset(new int32);
                    data.ref() = pg_atoi(str2.ptr(), sizeof(int32), 0);
                }
                BINGO_PG_HANDLE(data.reset(0); throw BingoPgError("error while converting to int32: %s", message));
            }
        }

        virtual uintptr_t getDatum()
        {
            if (data.get() == 0)
                return 0;
            else
                return Int32GetDatum(data.ref());
        }

    private:
        ImportInt4Data(const ImportInt4Data&); // no implicit copy
    };

public:
    BingoImportHandler(unsigned int func_id) : BingoSessionHandler(func_id)
    {
        SPI_connect();
    }
    virtual ~BingoImportHandler()
    {
        _importData.clear();
        SPI_finish();
    }

    virtual bool hasNext() = 0;
    virtual void getNextData() = 0;

    void init(Datum table_datum, Datum column_datum, Datum other_columns_datum)
    {
        BingoPgText tablename_text;
        BingoPgText column_text;
        BingoPgText other_columns_text;

        checkImportNull(table_datum, "table name", tablename_text);
        checkImportNull(column_datum, "column name", column_text);
        checkImportEmpty(other_columns_datum, other_columns_text);
        /*
         * Add the data column
         */
        _importColumns.clear();
        _importColumns.push().columnName.readString(column_text.getString(), true);

        /*
         * Prepare query table with column name
         */
        ArrayOutput column_names(_columnNames);
        column_names.printf("%s(%s", tablename_text.getString(), column_text.getString());
        /*
         * Add additional columns
         */
        if (_parseColumns)
        {
            int column_count = bingoImportParseFieldList(other_columns_text.getString());
            for (int col_idx = 0; col_idx < column_count; ++col_idx)
            {
                ImportColumn& idCol = _importColumns.push();
                idCol.columnName.readString(bingoImportGetColumnName(col_idx), true);
                column_names.printf(", %s", idCol.columnName.ptr());
            }
        }
        else
        {
            if (other_columns_datum != 0)
            {
                if (strcmp(other_columns_text.getString(), "") != 0)
                {
                    ImportColumn& idCol = _importColumns.push();
                    idCol.columnName.readString(other_columns_text.getString(), true);
                    column_names.printf(", %s", other_columns_text.getString());
                }
            }
        }
        column_names.printf(")");
        column_names.writeChar(0);

        _defineColumnTypes(tablename_text.getString());
    }

    void _defineColumnTypes(const char* table_name)
    {
        /*
         * Read column names for query
         */
        ArrayChar column_names;
        for (int i = 0; i < _importColumns.size(); ++i)
        {
            if (i != 0)
                column_names.appendString(", ", true);
            column_names.appendString(_importColumns[i].columnName.ptr(), true);
        }
        /*
         * Make a query for types definition
         */
        BingoPgCursor table_first("select %s from %s", column_names.ptr(), table_name);
        table_first.next();
        /*
         * Set up all types
         */
        for (int i = 0; i < _importColumns.size(); ++i)
        {
            int arg_type = table_first.getArgOid(i);
            ImportColumn& dataCol = _importColumns.at(i);

            if ((arg_type != INT4OID) && (arg_type != INT8OID) && (arg_type != TEXTOID) && (arg_type != BYTEAOID))
                throw BingoPgError("can not import a structure: unsupported column '%s' type; supported values: 'text', 'bytea', 'integer'",
                                   dataCol.columnName.ptr());

            dataCol.type = arg_type;
        }
    }

    void _addData(const char* data, int col_idx)
    {
        AutoPtr<ImportData> import_data;
        ImportColumn& import_column = _importColumns[col_idx];
        /*
         * Detect the types and correspond class
         */
        switch (import_column.type)
        {
        case INT4OID:
            import_data.reset(new ImportInt4Data());
            break;
        case INT8OID:
            import_data.reset(new ImportInt8Data());
            break;
        case TEXTOID:
            import_data.reset(new ImportTextData());
            break;
        case BYTEAOID:
            import_data.reset(new ImportTextData());
            break;
        default:
            throw BingoPgError("internal error: unknown data type %d", import_column.type);
        }
        /*
         * Convert the string
         */
        import_data->convert(data);
        /*
         * Add a data to the array
         */
        _importData.add(import_data.release());
    }

    void import()
    {
        QS_DEF(ArrayChar, query_str);
        QS_DEF(Array<Datum>, q_values);
        QS_DEF(Array<Oid>, q_oids);
        QS_DEF(ArrayChar, q_nulls);
        int spi_success = 0;

        /*
         * Prepare query string
         */
        ArrayOutput query_string(query_str);
        query_string.printf("INSERT INTO ");
        query_string.printf("%s", _columnNames.ptr());
        query_string.printf(" VALUES (");

        q_nulls.clear();
        q_oids.clear();
        for (int col_idx = 0; col_idx < _importColumns.size(); ++col_idx)
        {
            q_nulls.push(0);
            q_oids.push(_importColumns[col_idx].type);

            if (col_idx != 0)
                query_string.printf(", ");
            query_string.printf("$%d", col_idx + 1);
        }
        query_string.printf(")");
        query_string.writeChar(0);

        int debug_idx = 0;
        /*
         * Loop through the data
         */

        while (hasNext())
        {
            ++debug_idx;
            elog(DEBUG1, "bingo: %s: processing data entry with index %d", getFunctionName(), debug_idx);

            /*
             * Initialize the data
             */
            try
            {
                getNextData();
            }
            catch (Exception& e)
            {
                /*
                 * Handle incorrect format errors
                 */
                if (strstr(e.message(), "data size exceeded the acceptable size") != 0)
                    throw BingoPgError(e.message());
                elog(WARNING, "can not import a structure: %s", e.message());
                continue;
            }

            /*
             * Initialize values for the query
             */

            q_values.clear();
            for (int q_idx = 0; q_idx < _importData.size(); ++q_idx)
            {
                q_values.push(_importData[q_idx]->getDatum());
                if (q_values[q_idx] == 0)
                {
                    q_nulls[q_idx] = 'n';
                }
                else
                {
                    q_nulls[q_idx] = 0;
                }
            }
            BINGO_PG_TRY
            {
                /*
                 * Execute query
                 */
                spi_success = SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);
                if (spi_success < 0)
                    elog(WARNING, "can not insert a structure into a table: %s", SPI_result_code_string(spi_success));
            }
            BINGO_PG_HANDLE(throw BingoPgError("can not import all the structures: SQL error: %s", message));

            if (debug_idx % 1000 == 0)
                elog(NOTICE, "bingo.import: %d structures processed", debug_idx);
            /*
             * Return back session id and error handler
             */
            refresh();
        }
    }

protected:
    int bingo_res;
    bool _parseColumns;
    ArrayChar _columnNames;

    ObjArray<ImportColumn> _importColumns;
    PtrArray<ImportData> _importData;

private:
    BingoImportHandler(const BingoImportHandler&); // no implicit copy
};

class BingoImportSdfHandler : public BingoImportHandler
{
public:
    BingoImportSdfHandler(unsigned int func_id, const char* fname) : BingoImportHandler(func_id)
    {
        _parseColumns = true;
        setFunctionName("importSDF");
        bingo_res = bingoSDFImportOpen(fname);
        CORE_HANDLE_ERROR(bingo_res, 1, "importSDF", bingoGetError());
    }
    virtual ~BingoImportSdfHandler()
    {
        bingo_res = bingoSDFImportClose();
        CORE_HANDLE_WARNING(bingo_res, 0, "importSDF close", bingoGetError());
    }

    virtual bool hasNext()
    {
        bingo_res = bingoSDFImportEOF();
        CORE_HANDLE_ERROR(bingo_res, 0, "importSDF", bingoGetError());
        return !bingo_res;
    }

    virtual void getNextData()
    {

        const char* data = 0;
        _importData.clear();

        for (int col_idx = 0; col_idx < _importColumns.size(); ++col_idx)
        {
            if (col_idx == 0)
                data = bingoSDFImportGetNext();
            else
                data = bingoImportGetPropertyValue(col_idx - 1);

            if (data == 0)
            {
                /*
                 * Handle incorrect format errors
                 */
                if (strstr(bingoGetError(), "data size exceeded the acceptable size") != 0)
                    throw BingoPgError(bingoGetError());
                CORE_HANDLE_WARNING(0, 1, "importSDF", bingoGetError());
            }

            _addData(data, col_idx);
        }
    }

private:
    BingoImportSdfHandler(const BingoImportSdfHandler&); // no implicit copy
};

Datum importsdf(PG_FUNCTION_ARGS)
{
    Datum table_datum = PG_GETARG_DATUM(0);
    Datum column_datum = PG_GETARG_DATUM(1);
    Datum other_columns_datum = PG_GETARG_DATUM(2);
    Datum file_name_datum = PG_GETARG_DATUM(3);

    PG_BINGO_BEGIN
    {
        /*
         * Check file name
         */
        BingoPgText fname_text;
        checkImportNull(file_name_datum, "file name", fname_text);
        /*
         * Initialize import
         */
        BingoImportSdfHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());
        bingo_handler.init(table_datum, column_datum, other_columns_datum);
        /*
         * Perform import
         */
        bingo_handler.import();
    }
    PG_BINGO_END

    PG_RETURN_VOID();
}
class BingoImportRdfHandler : public BingoImportHandler
{
public:
    BingoImportRdfHandler(unsigned int func_id, const char* fname) : BingoImportHandler(func_id)
    {
        _parseColumns = true;
        setFunctionName("importRDF");
        bingo_res = bingoRDFImportOpen(fname);
        CORE_HANDLE_ERROR(bingo_res, 1, "importRDF", bingoGetError());
    }
    virtual ~BingoImportRdfHandler()
    {
        bingo_res = bingoRDFImportClose();
        CORE_HANDLE_WARNING(bingo_res, 0, "importRDF close", bingoGetError());
    }

    virtual bool hasNext()
    {
        bingo_res = bingoRDFImportEOF();
        CORE_HANDLE_ERROR(bingo_res, 0, "importRDF", bingoGetError());

        return !bingo_res;
    }

    virtual void getNextData()
    {
        const char* data = 0;
        _importData.clear();

        for (int col_idx = 0; col_idx < _importColumns.size(); ++col_idx)
        {
            if (col_idx == 0)
                data = bingoRDFImportGetNext();
            else
                data = bingoImportGetPropertyValue(col_idx - 1);

            if (data == 0)
            {
                /*
                 * Handle incorrect format errors
                 */
                if (strstr(bingoGetError(), "data size exceeded the acceptable size") != 0)
                    throw BingoPgError(bingoGetError());
                CORE_HANDLE_WARNING(0, 1, "importRDF", bingoGetError());
            }
            _addData(data, col_idx);
        }
    }

private:
    BingoImportRdfHandler(const BingoImportSdfHandler&); // no implicit copy
};

Datum importrdf(PG_FUNCTION_ARGS)
{
    Datum table_datum = PG_GETARG_DATUM(0);
    Datum column_datum = PG_GETARG_DATUM(1);
    Datum other_columns_datum = PG_GETARG_DATUM(2);
    Datum file_name_datum = PG_GETARG_DATUM(3);

    PG_BINGO_BEGIN
    {
        /*
         * Check file name
         */
        BingoPgText fname_text;
        checkImportNull(file_name_datum, "file name", fname_text);
        /*
         * Initialize import
         */
        BingoImportRdfHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());
        bingo_handler.init(table_datum, column_datum, other_columns_datum);
        /*
         * Perform import
         */
        bingo_handler.import();
    }
    PG_BINGO_END

    PG_RETURN_VOID();
}

class BingoImportSmilesHandler : public BingoImportHandler
{
public:
    BingoImportSmilesHandler(unsigned int func_id, const char* fname) : BingoImportHandler(func_id)
    {
        _parseColumns = false;
        setFunctionName("importSMILES");
        bingo_res = bingoSMILESImportOpen(fname);
        CORE_HANDLE_ERROR(bingo_res, 1, "importSmiles", bingoGetError());
    }
    virtual ~BingoImportSmilesHandler()
    {
        bingo_res = bingoSMILESImportClose();
        CORE_HANDLE_WARNING(bingo_res, 0, "importSmiles close", bingoGetError());
    }

    virtual bool hasNext()
    {
        bingo_res = bingoSMILESImportEOF();
        CORE_HANDLE_ERROR(bingo_res, 0, "importSmiles", bingoGetError());
        return !bingo_res;
    }

    virtual void getNextData()
    {

        const char* data = 0;
        _importData.clear();

        for (int col_idx = 0; col_idx < _importColumns.size(); ++col_idx)
        {
            if (col_idx == 0)
            {
                data = bingoSMILESImportGetNext();
                if (data == 0)
                    CORE_HANDLE_WARNING(0, 1, "importSMILES", bingoGetError());
            }
            else
            {
                data = bingoSMILESImportGetId();
                if (data == 0)
                    CORE_HANDLE_WARNING(0, 1, "importSMILES", "can not get smiles id");
            }

            _addData(data, col_idx);
        }
    }

private:
    BingoImportSmilesHandler(const BingoImportSmilesHandler&); // no implicit copy
};

Datum importsmiles(PG_FUNCTION_ARGS)
{
    Datum table_datum = PG_GETARG_DATUM(0);
    Datum column_datum = PG_GETARG_DATUM(1);
    Datum other_column_datum = PG_GETARG_DATUM(2);
    Datum file_name_datum = PG_GETARG_DATUM(3);

    PG_BINGO_BEGIN
    {
        /*
         * Check file name
         */
        BingoPgText fname_text;
        checkImportNull(file_name_datum, "file name", fname_text);
        /*
         * Initialize import
         */
        BingoImportSmilesHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());
        bingo_handler.init(table_datum, column_datum, other_column_datum);
        /*
         * Perform import
         */
        bingo_handler.import();
    }
    PG_BINGO_END

    PG_RETURN_VOID();
}