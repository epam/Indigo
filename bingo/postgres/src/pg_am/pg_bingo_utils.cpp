#include "bingo_pg_fix_pre.h"

extern "C"

{
#include "postgres.h"
#include "access/heapam.h"
#include "fmgr.h"
#include "storage/bufmgr.h"
#include "storage/lock.h"
#include "utils/relcache.h"
#if PG_VERSION_NUM / 100 >= 903
#include "access/xlog_internal.h"
#include "lib/stringinfo.h"
#endif
}

#include "bingo_pg_fix_post.h"

#include "bingo_postgres.h"

#include "base_cpp/scanner.h"
#include "bingo_core_c.h"

#include "bingo_pg_buffer.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_cursor.h"
#include "bingo_pg_index.h"
#include "bingo_pg_text.h"
#include "pg_bingo_context.h"

extern "C"
{
    BINGO_FUNCTION_EXPORT(bingo_markpos);

    BINGO_FUNCTION_EXPORT(bingo_restrpos);

    BINGO_FUNCTION_EXPORT(getindexstructurescount);

    BINGO_FUNCTION_EXPORT(_get_structures_count);

    BINGO_FUNCTION_EXPORT(_get_block_count);

    BINGO_FUNCTION_EXPORT(_precache_database);

    BINGO_FUNCTION_EXPORT(getversion);

    BINGO_FUNCTION_EXPORT(filetotext);

    BINGO_FUNCTION_EXPORT(filetoblob);

    BINGO_FUNCTION_EXPORT(getname);

    BINGO_FUNCTION_EXPORT(exportsdf);

    BINGO_FUNCTION_EXPORT(exportrdf);

    BINGO_FUNCTION_EXPORT(_reset_profiling_info);

    BINGO_FUNCTION_EXPORT(_get_profiling_info);

    BINGO_FUNCTION_EXPORT(_print_profiling_info);
}

using namespace indigo;

Datum getindexstructurescount(PG_FUNCTION_ARGS)
{
    Oid relOid = PG_GETARG_OID(0);

    int result = 0;
    Relation rel;

    rel = relation_open(relOid, AccessShareLock);
    PG_BINGO_BEGIN
    {
        BingoPgBuffer meta_buffer;
        meta_buffer.readBuffer(rel, BINGO_METAPAGE, BINGO_PG_READ);
        BingoMetaPage meta_page = BingoPageGetMeta(BufferGetPage(meta_buffer.getBuffer()));
        result = meta_page->n_molecules;
    }
    PG_BINGO_END

    relation_close(rel, AccessShareLock);

    PG_RETURN_INT32(result);
}

Datum _get_structures_count(PG_FUNCTION_ARGS)
{
    Oid relOid = PG_GETARG_OID(0);

    int result = 0;
    Relation rel;

    rel = relation_open(relOid, AccessShareLock);
    PG_BINGO_BEGIN
    {
        BingoPgBuffer meta_buffer;
        meta_buffer.readBuffer(rel, BINGO_METAPAGE, BINGO_PG_READ);
        BingoMetaPage meta_page = BingoPageGetMeta(BufferGetPage(meta_buffer.getBuffer()));
        result = meta_page->n_molecules;
    }
    PG_BINGO_END

    relation_close(rel, AccessShareLock);

    PG_RETURN_INT32(result);
}

Datum _get_block_count(PG_FUNCTION_ARGS)
{
    Oid relOid = PG_GETARG_OID(0);

    int result = 0;
    Relation rel;

    rel = relation_open(relOid, AccessShareLock);
    PG_BINGO_BEGIN
    {
        BingoPgBuffer meta_buffer;
        meta_buffer.readBuffer(rel, BINGO_METAPAGE, BINGO_PG_READ);
        BingoMetaPage meta_page = BingoPageGetMeta(BufferGetPage(meta_buffer.getBuffer()));
        result = meta_page->n_sections;
    }
    PG_BINGO_END

    relation_close(rel, AccessShareLock);

    PG_RETURN_INT32(result);
}

class CacheParams
{
public:
    enum
    {
        SIZE_IN_BYTES,
        SIZE_IN_KB,
        SIZE_IN_MB,
        SIZE_IN_GB
    };

    int size_in;

    CacheParams()
    {
        size_in = SIZE_IN_MB;
    }
    void getSizeIn(Array<char>& out)
    {
        switch (size_in)
        {
        case SIZE_IN_BYTES:
            out.readString("B", true);
            break;
        case SIZE_IN_KB:
            out.readString("KB", true);
            break;
        case SIZE_IN_MB:
            out.readString("MB", true);
            break;
        case SIZE_IN_GB:
            out.readString("GB", true);
            break;
        default:
            break;
        }
    }
    qword getSize(qword size_b)
    {
        qword res = 0;
        switch (size_in)
        {
        case SIZE_IN_BYTES:
            res = size_b;
            break;
        case SIZE_IN_KB:
            res = size_b >> 10;
            break;
        case SIZE_IN_MB:
            res = size_b >> 20;
            break;
        case SIZE_IN_GB:
            res = size_b >> 30;
            break;
        default:
            break;
        }
        if (res == 0 && size_b > 0)
            res = 1;
        return res;
    }
    void parseParameters(const char* params_str)
    {
        BufferScanner scanner(params_str);

        QS_DEF(Array<char>, buf_word);

        scanner.skipSpace();

        while (!scanner.isEOF())
        {
            scanner.readWord(buf_word, 0);
            scanner.skipSpace();

            if (strcasecmp(buf_word.ptr(), "B") == 0)
            {
                size_in = CacheParams::SIZE_IN_BYTES;
            }
            else if (strcasecmp(buf_word.ptr(), "KB") == 0)
            {
                size_in = CacheParams::SIZE_IN_KB;
            }
            else if (strcasecmp(buf_word.ptr(), "MB") == 0)
            {
                size_in = CacheParams::SIZE_IN_MB;
            }
            else if (strcasecmp(buf_word.ptr(), "GB") == 0)
            {
                size_in = CacheParams::SIZE_IN_GB;
            }
            else
            {
                throw BingoPgError("unknown parameter: %s", buf_word.ptr());
            }

            if (scanner.isEOF())
                break;
            scanner.skipSpace();
        }
    }
    ~CacheParams()
    {
    }

private:
    CacheParams(const CacheParams&);
};

Datum _precache_database(PG_FUNCTION_ARGS)
{
    Oid relOid = PG_GETARG_OID(0);
    Datum parameters_datum = PG_GETARG_DATUM(1);

    void* res = 0;
    Relation rel;
    /*
     *
     */

    rel = relation_open(relOid, AccessShareLock);
    PG_BINGO_BEGIN
    {
        BingoPgText parameters_text(parameters_datum);
        /*
         * Parse parameters
         */
        CacheParams params;
        params.parseParameters(parameters_text.getString());

        Array<char> tmp_buffer;
        Array<int> tmp_buffer2;
        Array<char> result_buf;
        ArrayOutput result(result_buf);
        ItemPointerData item_buf;

        int buf_size = 0;
        int processed_num = 0;

        qword total_cache_size = 0;
        qword total_index_size = 0;

        qword index_metapages_size = 0;
        qword block_metapages_size = 0;

        qword cmf_real_size = 0;
        qword cmf_cache_size = 0;
        qword sim_real_size = 0;
        qword sim_cache_size = 0;
        qword xyz_real_size = 0;
        qword xyz_cache_size = 0;
        qword fp_cache_size = 0;

        result.printfCR("{");

        BingoPgIndex bingo_index(rel);
        int section_idx = bingo_index.readBegin();
        int section_num = bingo_index.readEnd();

        result.printfCR("structures_number : %d,", bingo_index.getStructuresNumber());
        result.printfCR("blocks_number : %d,", bingo_index.getSectionNumber());

        params.getSizeIn(tmp_buffer);
        result.printfCR("size_in : '%s',", tmp_buffer.ptr());

        /*
         * Calc dictionary buf size
         */
        buf_size = bingo_index.getDictCount() * BingoPgBufferCacheBin::BUFFER_SIZE;
        result.printfCR("dict_cache_size : %d,", params.getSize(buf_size));
        total_cache_size += buf_size;

        total_index_size += BINGO_DICTIONARY_BLOCKS_NUM * BingoPgBufferCacheBin::BUFFER_SIZE;
        /*
         * Read config and offset blocks as meta
         */
        auto bingoContext = std::make_unique<BingoContext>(0);
        auto mangoContext = std::make_unique<MangoContext>(*bingoContext.get());
        auto ringoContext = std::make_unique<RingoContext>(*bingoContext.get());
        bingo_core::BingoCore bingoCore;

        bingoCore.bingo_context = bingoContext.get();
        bingoCore.mango_context = mangoContext.get();
        bingoCore.ringo_context = ringoContext.get();
        BingoPgConfig bingo_config(bingoCore);
        bingo_index.readConfigParameters(bingo_config);
        index_metapages_size += BINGO_METABLOCKS_NUM * BingoPgBufferCacheBin::BUFFER_SIZE;

        total_cache_size += index_metapages_size;
        total_index_size += index_metapages_size;

        buf_size = section_num / BINGO_SECTION_OFFSET_PER_BLOCK + 1;

        index_metapages_size += buf_size * BingoPgBufferCacheBin::BUFFER_SIZE;
        total_cache_size += buf_size * BingoPgBufferCacheBin::BUFFER_SIZE;

        total_index_size += BINGO_SECTION_OFFSET_BLOCKS_NUM * BingoPgBufferCacheBin::BUFFER_SIZE;

        BingoPgExternalBitset bitset_buf;
        int str_num = 0;

        for (; section_idx < bingo_index.readEnd(); section_idx = bingo_index.readNext(section_idx))
        {

            str_num = bingo_index.getSectionStructuresNumber(section_idx);
            /*
             * Exist structures as section metapages
             */
            bingo_index.getSectionBitset(section_idx, bitset_buf);
            block_metapages_size += 2 * BingoPgBufferCacheBin::BUFFER_SIZE;

            cmf_real_size = 0;
            for (int str_idx = 0; str_idx < str_num; ++str_idx)
            {
                /*
                 * Cmf real size
                 */
                bingo_index.readCmfItem(section_idx, str_idx, tmp_buffer);
                cmf_real_size += tmp_buffer.sizeInBytes();

                /*
                 * Mapping buffers
                 */
                bingo_index.readTidItem(section_idx, str_idx, &item_buf);
            }
            /*
             * Sim buffers
             */
            // bingo_index.getSectionBitsCount(section_idx, tmp_buffer2);
            // sim_real_size += tmp_buffer2.sizeInBytes();

            /*
             * Cmf buffer size
             */
            cmf_cache_size += ((cmf_real_size / BingoPgBufferCacheBin::MAX_SIZE) + 1) * BingoPgBufferCacheBin::BUFFER_SIZE;

            /*
             * Block mapping as metapages size
             */
            buf_size = str_num / BINGO_MOLS_PER_MAPBLOCK + 1;
            block_metapages_size += buf_size * BingoPgBufferCacheBin::BUFFER_SIZE;

            /*
             * Sim buffers
             */
            buf_size = str_num / BingoPgSection::SECTION_BITS_PER_BLOCK + 1;
            sim_cache_size += buf_size * BingoPgBufferCacheBin::BUFFER_SIZE;

            int fp_size = bingo_index.getSectionInfo(section_idx).n_blocks_for_fp;

            bitset_buf.clear();
            for (int fp_idx = 0; fp_idx < fp_size; ++fp_idx)
            {
                bingo_index.andWithBitset(section_idx, fp_idx, bitset_buf);
            }
            fp_cache_size += (fp_size * BingoPgBufferCacheBin::BUFFER_SIZE);

            /*
             * Total section size
             */

            buf_size = bingo_index.getSectionInfo(section_idx).section_size;
            total_index_size += buf_size * BingoPgBufferCacheBin::BUFFER_SIZE;

            elog(NOTICE, "%d blocks processed", section_idx + 1);
        }
        total_cache_size += sim_cache_size;
        total_cache_size += fp_cache_size;
        total_cache_size += block_metapages_size;
        total_cache_size += cmf_cache_size;

        result.printfCR("index_metapages_size : %d,", params.getSize(index_metapages_size));
        result.printfCR("block_metapages_size : %d,", params.getSize(block_metapages_size));
        result.printfCR("cmf_cache_size : %d,", params.getSize(cmf_cache_size));
        result.printfCR("sim_cache_size : %d,", params.getSize(sim_cache_size));
        result.printfCR("fp_cache_size : %d,", params.getSize(fp_cache_size));
        result.printfCR("total_cache_size : %d", params.getSize(total_cache_size));
        result.printfCR("total_index_size : %d", params.getSize(total_index_size));
        result.printf("}");
        result_buf.push(0);
        elog(NOTICE, "%s", result_buf.ptr());
        BingoPgText res_text;
        res_text.initFromString(result_buf.ptr());
        res = res_text.release();
    }
    PG_BINGO_END

    relation_close(rel, AccessShareLock);

    if (res == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(res);
}

// Datum bingo_test(PG_FUNCTION_ARGS) {
//   elog(NOTICE, "start test function 3");
//   PG_RETURN_VOID();
//}
//
// Datum bingo_test_tid(PG_FUNCTION_ARGS) {
//   elog(NOTICE, "start test function tid");
//
//   ItemPointer pp = (ItemPointer) palloc0(sizeof(ItemPointerData));
//
//   ItemPointerSet(pp, 1, 2);
//
//   PG_RETURN_POINTER(pp);
//}
//
// static Oid getFunc(const char* name, Array<Oid>& types) {
//   Array<char> fname;
//   fname.readString(name, true);
//   Value* func_name = makeString(fname.ptr());
//
//   List* func_list = list_make1(func_name);
//   Oid func_oid = LookupFuncName(func_list, types.size(), types.ptr(), false);
//
//   if(func_oid == InvalidOid)
//      elog(ERROR, "can not find the function %s", name);
//
//   list_free(func_list);
//   return func_oid;
//}

// Datum bingo_test_select(PG_FUNCTION_ARGS) {
//   elog(INFO, "start test select");
//
//   Array<Oid> func_type;
//   func_type.push(TEXTOID);
//
//   Oid func_begin_oid = getFunc("bingo_test_cur_begin", func_type);
//
//   FmgrInfo f_begin_info;
//   fmgr_info(func_begin_oid, &f_begin_info);
//
//   func_type.clear();
//   func_type.push(REFCURSOROID);
//
//   Oid func_next_oid = getFunc("bingo_test_cur_next", func_type);
//
//   FmgrInfo f_next_info;
//   fmgr_info(func_next_oid, &f_next_info);
//
//   elog(INFO, "func = %d", func_begin_oid);
//
//   BingoPgText test_select;
//   test_select.initFromString("btest_shadow");
//
//   Datum cursor_ref = FunctionCall1(&f_begin_info, PointerGetDatum(test_select.ptr()));
//
//   BingoPgText res_text(cursor_ref);
//   elog(INFO, "res text = %s", res_text.getString());
//
//
//   Datum record;
//   ItemPointer tup;
//   for (int i = 0; i < 5; ++i) {
//      record = FunctionCall1(&f_next_info, cursor_ref);
//      if(record == 0) {
//         elog(INFO, "Rec is null");
//         continue;
//      }
//      tup = (ItemPointer) DatumGetPointer(record);
//      elog(INFO, "block = %d off = %d", ItemPointerGetBlockNumber(tup), ItemPointerGetOffsetNumber(tup));
//   }
//
//   PG_RETURN_VOID();
//}

/*
 * Save current scan position
 */
Datum bingo_markpos(PG_FUNCTION_ARGS)
{
    elog(ERROR, "bingo does not support mark/restore");
    PG_RETURN_VOID();
}

/*
 * Restore scan to last saved position
 */
Datum bingo_restrpos(PG_FUNCTION_ARGS)
{
    elog(ERROR, "bingo does not support mark/restore");
    PG_RETURN_VOID();
}

void bingo_redo(XLogRecPtr lsn, XLogRecord* record)
{
    elog(PANIC, "bingo_redo: unimplemented");
}

void bingo_desc(StringInfo buf, uint8 xl_info, char* rec)
{
}

Datum getversion(PG_FUNCTION_ARGS)
{
    BingoPgText result_text;
    result_text.initFromString(BINGO_VERSION);

    PG_RETURN_TEXT_P(result_text.release());
}

Datum filetotext(PG_FUNCTION_ARGS)
{
    Datum file_name_datum = PG_GETARG_DATUM(0);

    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgText fname_text(file_name_datum);
        BingoPgText result_text;

        QS_DEF(Array<char>, buffer);
        buffer.clear();
        /*
         * Read file
         */
        FileScanner f_scanner(fname_text.getString());
        f_scanner.readAll(buffer);

        result_text.initFromArray(buffer);

        result = result_text.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

Datum filetoblob(PG_FUNCTION_ARGS)
{
    Datum file_name_datum = PG_GETARG_DATUM(0);
    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgText fname_text(file_name_datum);
        BingoPgText result_text;
        QS_DEF(Array<char>, buffer);
        buffer.clear();
        /*
         * Read file
         */
        FileScanner f_scanner(fname_text.getString());
        f_scanner.readAll(buffer);

        result_text.initFromArray(buffer);

        result = result_text.release();
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_BYTEA_P(result);
}

Datum getname(PG_FUNCTION_ARGS)
{
    Datum target_datum = PG_GETARG_DATUM(0);
    void* result = 0;
    PG_BINGO_BEGIN
    {
        BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
        bingo_handler.setFunctionName("getname");

        BingoPgText mol_text(target_datum);
        int buf_size;
        const char* target_buf = mol_text.getText(buf_size);

        try {
            const char* bingo_result = bingo_handler.bingoCore.bingoGetNameCore(target_buf, buf_size);

            BingoPgText result_text;
            result_text.initFromString(bingo_result);
            result = result_text.release();
        } CORE_CATCH_WARNING_RETURN("bingo.getname", PG_RETURN_NULL())
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(result);
}

static void _parseQueryFieldList(const char* fields_str, RedBlackStringMap<int, false>& field_list)
{
    BufferScanner scanner(fields_str);

    QS_DEF(Array<char>, buf_word);

    scanner.skipSpace();
    int column_idx = field_list.size();

    while (!scanner.isEOF())
    {
        scanner.readWord(buf_word, " ,");
        scanner.skipSpace();

        if (field_list.find(buf_word.ptr()))
            throw BingoPgError("parseQueryFieldList(): key %s is already presented in the query list", buf_word.ptr());

        ++column_idx;
        field_list.insert(buf_word.ptr(), column_idx);

        if (scanner.isEOF())
            break;

        if (scanner.readChar() != ',')
            throw BingoPgError("parseQueryFieldList(): comma expected");

        scanner.skipSpace();
    }
}

static void checkExportNull(Datum text_datum, const char* message, BingoPgText& text)
{
    if (text_datum == 0)
        throw BingoPgError("can not export structures: %s is empty", message);
    text.init(text_datum);
    int text_size = 0;
    text.getText(text_size);
    if (text_size == 0)
        throw BingoPgError("can not export structures: %s is empty", message);
}
static void checkExportEmpty(Datum text_datum, BingoPgText& text)
{
    if (text_datum == 0)
        text.initFromString("");
    else
        text.init(text_datum);
}

static int _initializeColumnQuery(Datum table_datum, Datum column_datum, Datum other_column_datum, Array<char>& query_str,
                                  RedBlackStringMap<int, false>& field_list)
{
    BingoPgText tablename_text;
    BingoPgText column_text;
    BingoPgText other_column_text;

    checkExportNull(table_datum, "table name", tablename_text);
    checkExportNull(column_datum, "column name", column_text);
    checkExportEmpty(other_column_datum, other_column_text);

    field_list.clear();

    field_list.insert(column_text.getString(), 0);
    int data_key = field_list.begin();

    ArrayOutput query_out(query_str);
    query_out.printf("SELECT %s", column_text.getString());

    if (other_column_datum != 0)
    {
        const char* columns_list = other_column_text.getString();
        if (strcmp(columns_list, "") != 0)
        {
            _parseQueryFieldList(columns_list, field_list);
            query_out.printf(", %s", columns_list);
        }
    }

    query_out.printf(" FROM %s", tablename_text.getString());
    query_out.writeChar(0);

    return data_key;
}

Datum exportsdf(PG_FUNCTION_ARGS)
{
    Datum table_datum = PG_GETARG_DATUM(0);
    Datum column_datum = PG_GETARG_DATUM(1);
    Datum other_columns_datum = PG_GETARG_DATUM(2);
    Datum file_name_datum = PG_GETARG_DATUM(3);

    PG_BINGO_BEGIN
    {
        QS_DEF(Array<char>, query_str);
        RedBlackStringMap<int, false> field_list;

        BingoPgText fname_text;
        checkExportNull(file_name_datum, "file name", fname_text);
        FileOutput file_output(fname_text.getString());

        int data_key = _initializeColumnQuery(table_datum, column_datum, other_columns_datum, query_str, field_list);

        BingoPgCursor table_cursor(query_str.ptr());
        BingoPgText buf_text;

        while (table_cursor.next())
        {
            table_cursor.getText(1, buf_text);
            file_output.writeStringCR(buf_text.getString());

            for (int k = field_list.begin(); k != field_list.end(); k = field_list.next(k))
            {
                if (data_key == k)
                    continue;

                int col_idx = field_list.value(k);
                const char* col_name = field_list.key(k);
                table_cursor.getText(col_idx, buf_text);
                file_output.printf(">  <%s>\n", col_name);
                file_output.printf("%s\n\n", buf_text.getString());
            }
            file_output.printf("\n$$$$\n");
        }
    }
    PG_BINGO_END

    PG_RETURN_VOID();
}

Datum exportrdf(PG_FUNCTION_ARGS)
{
    Datum table_datum = PG_GETARG_DATUM(0);
    Datum column_datum = PG_GETARG_DATUM(1);
    Datum other_columns_datum = PG_GETARG_DATUM(2);
    Datum file_name_datum = PG_GETARG_DATUM(3);

    PG_BINGO_BEGIN
    {
        QS_DEF(Array<char>, query_str);
        RedBlackStringMap<int, false> field_list;

        BingoPgText fname_text;
        checkExportNull(file_name_datum, "file name", fname_text);
        FileOutput file_output(fname_text.getString());

        int data_key = _initializeColumnQuery(table_datum, column_datum, other_columns_datum, query_str, field_list);

        BingoPgCursor table_cursor(query_str.ptr());
        BingoPgText buf_text;

        file_output.printf("$RDFILE 1\n");

        int str_idx = 0;
        while (table_cursor.next())
        {
            ++str_idx;
            table_cursor.getText(1, buf_text);
            file_output.printf("$MFMT $MIREG %d\n", str_idx);
            file_output.writeStringCR(buf_text.getString());

            for (int k = field_list.begin(); k != field_list.end(); k = field_list.next(k))
            {
                if (data_key == k)
                    continue;

                int col_idx = field_list.value(k);
                const char* col_name = field_list.key(k);
                table_cursor.getText(col_idx, buf_text);
                file_output.printf("$DTYPE %s\n", col_name);
                file_output.printf("$DATUM %s\n", buf_text.getString());
            }
        }
    }
    PG_BINGO_END

    PG_RETURN_VOID();
}

Datum _reset_profiling_info(PG_FUNCTION_ARGS)
{
    bingoProfilingReset(true);
    PG_RETURN_VOID();
}

Datum _get_profiling_info(PG_FUNCTION_ARGS)
{
    char* result = 0;
    PG_BINGO_BEGIN
    {
        const char* bingo_result = bingoProfilingGetStatistics(true);
        result = BingoPgCommon::releaseString(bingo_result);
    }
    PG_BINGO_END

    if (result == 0)
        PG_RETURN_NULL();

    PG_RETURN_CSTRING(result);
}

Datum _print_profiling_info(PG_FUNCTION_ARGS)
{
    PG_BINGO_BEGIN
    {
        const char* bingo_result = bingoProfilingGetStatistics(true);
        elog(NOTICE, "\n%s", bingo_result);
    }
    PG_BINGO_END
    PG_RETURN_VOID();
}