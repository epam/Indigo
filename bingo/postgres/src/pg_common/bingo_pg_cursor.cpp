#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"
#include "catalog/pg_type.h"
#include "executor/spi.h"
#include "fmgr.h"
#include "storage/itemptr.h"
}

#include "bingo_pg_fix_post.h"

#include "base_c/nano.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "bingo_pg_common.h"
#include "bingo_pg_cursor.h"
#include "bingo_pg_text.h"
#include "pg_bingo_context.h"

using namespace indigo;

IMPL_ERROR(BingoPgCursor, "bingo cursor access");

BingoPgCursor::BingoPgCursor(const char* format, ...)
{
    Array<char> buf;
    va_list args;
    va_start(args, format);
    ArrayOutput output(buf);
    output.vprintf(format, args);
    output.writeChar(0);
    va_end(args);

    _init(buf);
}

BingoPgCursor::BingoPgCursor(indigo::Array<char>& query_str)
{
    _init(query_str);
}

BingoPgCursor::~BingoPgCursor()
{
    /*
     * JDBC drivers workaround
     */
    BINGO_PG_TRY
    {
        Portal cur_ptr = SPI_cursor_find(_cursorName.ptr());
        if (cur_ptr != NULL)
            SPI_cursor_close((Portal)_cursorPtr);
        SPI_finish();
        SPI_pop_conditional(_pushed);
    }
    BINGO_PG_HANDLE(elog(WARNING, "internal error: can not close the cursor: %s", message));
    // Can not throw error from destructor
}

bool BingoPgCursor::next()
{

    BINGO_PG_TRY
    {
        SPI_cursor_fetch((Portal)_cursorPtr, true, 1);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not fetch the cursor: %s", message));

    if (SPI_processed == 0)
        return false;

    if (SPI_tuptable == NULL)
        return false;

    return true;
}

void BingoPgCursor::getId(int arg_idx, ItemPointerData& data)
{
    BINGO_PG_TRY
    {
        Datum record = getDatum(arg_idx);
        ItemPointer tup = (ItemPointer)DatumGetPointer(record);

        int block_num = ItemPointerGetBlockNumber(tup);
        int off_num = ItemPointerGetOffsetNumber(tup);

        ItemPointerSetBlockNumber(&data, block_num);
        ItemPointerSetOffsetNumber(&data, off_num);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not get the id from the data: %s", message));
}
void BingoPgCursor::getText(int arg_idx, BingoPgText& data)
{
    if (SPI_processed == 0)
        throw Error("internal error: can not get not processed tuple");

    if (SPI_tuptable == NULL)
        throw Error("internal error: can not get null tuple");

    Datum record = 0;

    BINGO_PG_TRY
    {
        TupleDesc tupdesc = SPI_tuptable->tupdesc;

        /*
         * Tuple index is always 0
         */
        int tuple_idx = 0;

        HeapTuple tuple = SPI_tuptable->vals[tuple_idx];

        if (arg_idx > tupdesc->natts)
            elog(ERROR, "internal error: can not get tuple was not in query %d > %d", arg_idx, tupdesc->natts);

        char* result = SPI_getvalue(tuple, tupdesc, arg_idx);
        if (result == NULL)
        {
            if (SPI_result == SPI_ERROR_NOATTRIBUTE)
                elog(ERROR, "internal error: colnumber is out of range (SPI_getvalue)");
            else if (SPI_result == SPI_ERROR_NOOUTFUNC)
                elog(ERROR, "internal error: no output function is available (SPI_getvalue)");
            else
                data.initFromString("\0");
        }
        else
        {
            data.initFromString(result);
            pfree(result);
        }
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not get datum from the tuple: %s", message));
}

uintptr_t BingoPgCursor::getDatum(int arg_idx)
{
    if (SPI_processed == 0)
        throw Error("internal error: can not get not processed tuple");

    if (SPI_tuptable == NULL)
        throw Error("internal error: can not get null tuple");

    Datum record = 0;

    BINGO_PG_TRY
    {
        TupleDesc tupdesc = SPI_tuptable->tupdesc;

        /*
         * Tuple index is always 0
         */
        int tuple_idx = 0;

        HeapTuple tuple = SPI_tuptable->vals[tuple_idx];
        bool isnull;

        if (arg_idx > tupdesc->natts)
            elog(ERROR, "internal error: can not get tuple was not in query %d > %d", arg_idx, tupdesc->natts);

        record = SPI_getbinval(tuple, tupdesc, arg_idx, &isnull);

        if (isnull)
            elog(ERROR, "internal error: can not get null tuple");
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not get datum from the tuple: %s", message));

    return record;
}

unsigned int BingoPgCursor::getArgOid(int arg_idx)
{
    if (SPI_tuptable == NULL)
        throw Error("internal error: can not get null tuple");

    Oid result = 0;

    BINGO_PG_TRY
    {
        TupleDesc tupdesc = SPI_tuptable->tupdesc;
        if (arg_idx >= tupdesc->natts)
            elog(ERROR, "internal error: can not get argument %d natts = %d", arg_idx, tupdesc->natts);
        #if PG_VERSION_NUM / 100 >= 1100
            result = tupdesc->attrs[arg_idx].atttypid;
        #else
            result = tupdesc->attrs[arg_idx]->atttypid;
        #endif
        
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not get datum from the tuple: %s", message));

    return result;
}

void BingoPgCursor::_init(indigo::Array<char>& query_str)
{
    Array<dword> arg_types;

    BINGO_PG_TRY
    {
        _pushed = SPI_push_conditional();
        SPI_connect();
        SPIPlanPtr plan_ptr = SPI_prepare_cursor(query_str.ptr(), arg_types.size(), arg_types.ptr(), 0);

        auto cursor_idx = nanoClock();
        ArrayOutput cursor_name_out(_cursorName);
        cursor_name_out.printf("bingo_cursor_%llu", cursor_idx);
        cursor_name_out.writeChar(0);

        _cursorPtr = SPI_cursor_open(_cursorName.ptr(), plan_ptr, 0, 0, true);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not prepare or open a cursor: %s", message));
}
