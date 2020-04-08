#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "access/heapam.h"
#include "access/itup.h"
#include "catalog/namespace.h"
#include "catalog/pg_type.h"
#include "executor/spi.h"
#include "fmgr.h"
#include "postgres.h"
#include "storage/bufmgr.h"
#include "storage/lock.h"
#include "utils/lsyscache.h"
#include "utils/relcache.h"
}

#include "bingo_pg_fix_post.h"

#include "base_c/bitarray.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "bingo_core_c.h"
#include "bingo_pg_buffer.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_ext_bitset.h"
#include "bingo_pg_text.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_fingerprint.h"
#include <math.h>

extern "C"
{
    BINGO_FUNCTION_EXPORT(_internal_func_check);
}

using namespace indigo;

IMPL_ERROR(BingoPgCommon, "bingo postgres");

void BingoPgCommon::getSearchTypeString(int type, indigo::Array<char>& result, bool molecule)
{
    result.clear();
    if (molecule)
    {
        switch (type)
        {
        case (MOL_SUB):
            result.readString("SUB", true);
            break;
        case (MOL_SIM):
            result.readString("SIM", true);
            break;
        case (MOL_SMARTS):
            result.readString("SMARTS", true);
            break;
        case (MOL_EXACT):
            result.readString("EXACT", true);
            break;
        case (MOL_GROSS):
            result.readString("GROSS", true);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (type)
        {
        case (REACT_SUB):
            result.readString("RSUB", true);
            break;
        case (REACT_EXACT):
            result.readString("REXACT", true);
            break;
        case (REACT_SMARTS):
            result.readString("RSMARTS", true);
            break;
        default:
            break;
        }
    }
}

void BingoPgCommon::printBitset(const char* name, BingoPgExternalBitset& bitset)
{
    elog(NOTICE, "bitset = %s", name);
    indigo::Array<char> bits;
    indigo::ArrayOutput ao(bits);
    for (int x = bitset.begin(); x != bitset.end(); x = bitset.next(x))
    {
        ao.printf("%d ", x);
    }
    bits.push(0);
    elog(NOTICE, "%s", bits.ptr());
}

void BingoPgCommon::printFPBitset(const char* name, unsigned char* bitset, int size)
{
    elog(NOTICE, "bitset = %s", name);
    indigo::Array<char> bits;
    indigo::ArrayOutput ao(bits);
    for (int fp_idx = 0; fp_idx < size; fp_idx++)
    {
        if (bitGetBit(bitset, fp_idx))
        {
            ao.printf("%d ", fp_idx);
        }
    }
    bits.push(0);
    elog(NOTICE, "%s", bits.ptr());
}

void BingoPgCommon::setDefaultOptions()
{
    bingoSetConfigInt("treat-x-as-pseudoatom", 0);
    bingoSetConfigInt("ignore-closing-bond-direction-mismatch", 0);

    bingoSetConfigInt("FP_ORD_SIZE", 25);
    bingoSetConfigInt("FP_ANY_SIZE", 15);
    bingoSetConfigInt("FP_TAU_SIZE", 10);
    bingoSetConfigInt("FP_SIM_SIZE", 8);
    bingoSetConfigInt("SUB_SCREENING_MAX_BITS", 8);
    bingoSetConfigInt("SIM_SCREENING_PASS_MARK", 128);

    bingoAddTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te");
    bingoAddTautomerRule(2, "0C", "N,O,P,S");
    bingoAddTautomerRule(3, "1C", "N,O");
}

static int rnd_check = rand();

int BingoPgCommon::executeQuery(indigo::Array<char>& query_str)
{
    int result = 0;
    BINGO_PG_TRY
    {
        SPI_connect();
        int success = SPI_exec(query_str.ptr(), 1);
        result = SPI_processed;

        SPI_finish();
        if (success < 0)
        {
            elog(ERROR, "error (%d) while executing query: %s res", success, query_str.ptr());
        }
    }
    BINGO_PG_HANDLE(throw BingoPgError("internal error: can not execute query: %s", message));
    return result;
}

int BingoPgCommon::executeQuery(const char* format, ...)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput output(buf);
    va_list args;
    va_start(args, format);
    output.vprintf(format, args);
    va_end(args);
    output.writeChar(0);

    return executeQuery(buf);
}

bool BingoPgCommon::tableExists(const char* schema_name, const char* table_name)
{
    return (executeQuery("select * from information_schema.tables where "
                         "table_catalog = CURRENT_CATALOG and table_schema = '%s' "
                         "and table_name = '%s'",
                         schema_name, table_name) > 0);
}

Datum _internal_func_check(PG_FUNCTION_ARGS)
{
    int check_value = PG_GETARG_INT32(0);
    bool result = (check_value == rnd_check);
    rnd_check = rand();
    PG_RETURN_BOOL(result);
}

void BingoPgCommon::createDependency(const char* schema_name, const char* index_schema, const char* child_table, const char* parent_table)
{
    rnd_check = rand();
    executeQuery("SELECT %s._internal_func_011(%d, '%s.%s', '%s.%s')", schema_name, rnd_check, index_schema, child_table, index_schema, parent_table);
}

void BingoPgCommon::dropDependency(const char* schema_name, const char* index_schema, const char* table_name)
{
    rnd_check = rand();
    executeQuery("SELECT %s._internal_func_012(%d, '%s.%s')", schema_name, rnd_check, index_schema, table_name);
}

void BingoPgCommon::appendPath(const char* schema_name)
{
    executeQuery("SELECT set_config('search_path', current_setting('search_path') ||',%s'::text , true)", schema_name);
}

char* BingoPgCommon::releaseString(const char* str)
{
    if (str == 0)
        return 0;
    int size = strlen(str) + 1;
    char* result = (char*)palloc(size);
    memcpy(result, str, size);
    return result;
}

BingoPgCommon::BingoSessionHandler::BingoSessionHandler(Oid func_id)
{
    char* schema_name = 0;

    BINGO_PG_TRY
    {
        schema_name = get_namespace_name(get_func_namespace(func_id));
        pfree(schema_name);
    }
    BINGO_PG_HANDLE(throw Error("internal error while trying get namespace name: %s", message));

    BingoPgConfig bingo_config;
    bingo_config.readDefaultConfig(schema_name);

    _sessionId = bingoAllocateSessionID();
    refresh();

    bingo_config.setUpBingoConfiguration();
    bingoTautomerRulesReady(0, 0, 0);
}

BingoPgCommon::BingoSessionHandler::~BingoSessionHandler()
{
    bingoReleaseSessionID(_sessionId);
}

void BingoPgCommon::BingoSessionHandler::refresh()
{
    bingoSetSessionID(_sessionId);
    bingoSetContext(0);
    //   bingoSetErrorHandler(bingoErrorHandler, this);
}

// void BingoPgCommon::BingoSessionHandler::bingoErrorHandler(const char* message, void* self_ptr) {
//   BingoSessionHandler* self = (BingoSessionHandler*)self_ptr;
//
//   const char* func = self->getFunctionName();
//
//   if(self->raise_error) {
//      if (func)
//         throw BingoPgError("runtime error in bingo.'%s': %s", func, message);
//      else
//         throw BingoPgError("runtime error: %s", message);
//   } else {
//      self->error_raised = true;
//      if (func)
//         elog(WARNING, "warning in bingo.'%s': %s", func, message);
//      else
//         elog(WARNING, "warning: %s", message);
//   }
//
//
//}
// dword BingoPgCommon::getFunctionOid(const char* name, indigo::Array<dword>& types) {
//   indigo::Array<char> fname;
//   fname.readString(name, true);
//   Value* func_name = makeString(fname.ptr());
//
//   List* func_list = list_make1(func_name);
//   Oid func_oid = LookupFuncName(func_list, types.size(), types.ptr(), false);
//
//   if(func_oid == InvalidOid)
//      throw Error( "can not find the function %s", name);
//
//   list_free(func_list);
//   return func_oid;
//}

// void BingoPgCommon::executeQuery(const char* query_str) {
//   int result = SPI_exec(query_str, 1);
//   if(result < 0)
//      throw Error( "error while executing query: %s", query_str);
////   Oid func_oid = getFunctionOid1("bingo_execute_func", TEXTOID);
////
////   BingoPgText text_query;
////   text_query.initFromString(query_str);
////
////   OidFunctionCall1(func_oid, text_query.getDatum());
//}

// dword BingoPgCommon::getFunctionOid1(const char* name, dword type1) {
//   QS_DEF(indigo::Array<dword>, types);
//   types.clear();
//   types.push(type1);
//   return getFunctionOid(name, types);
//}
//
// dword BingoPgCommon::callFunction(dword functionId, indigo::Array<dword>& args) {
//   FmgrInfo flinfo;
//   FunctionCallInfoData fcinfo;
//
//   int args_len = args.size();
//
//   fmgr_info(functionId, &flinfo);
//   InitFunctionCallInfoData(fcinfo, &flinfo, args_len, NULL, NULL);
//
//   for (int arg_idx = 0; arg_idx < args_len; ++arg_idx) {
//      fcinfo.arg[arg_idx] = args[arg_idx];
//      fcinfo.argnull[arg_idx] = false;
//   }
//
//   Datum result = FunctionCallInvoke(&fcinfo);
//
//   /* Do not Check for */
////   if (fcinfo.isnull)
////      throw Error( "function %u returned NULL", flinfo.fn_oid);
//
//   return result;
//}

// dword BingoPgCommon::callFunction1(dword oid, dword arg1) {
//   QS_DEF(indigo::Array<dword>, args);
//   args.clear();
//   args.push(arg1);
//   return callFunction(oid, args);
//}

BingoPgWrapper::BingoPgWrapper() : _ptr(0)
{
}

BingoPgWrapper::~BingoPgWrapper()
{
    clear();
}

void BingoPgWrapper::clear()
{
    if (_ptr != 0)
        pfree(_ptr);
    _ptr = 0;
}

const char* BingoPgWrapper::getFuncNameSpace(dword oid_func)
{
    clear();
    _ptr = get_namespace_name(get_func_namespace(oid_func));
    return (const char*)_ptr;
}

const char* BingoPgWrapper::getRelNameSpace(dword oid_rel)
{
    clear();
    _ptr = get_namespace_name(get_rel_namespace(oid_rel));
    return (const char*)_ptr;
}
const char* BingoPgWrapper::getFuncName(dword oid_func)
{
    clear();
    _ptr = get_func_name(oid_func);
    return (const char*)_ptr;
}

const char* BingoPgWrapper::getRelName(dword oid_rel)
{
    clear();
    _ptr = get_rel_name(oid_rel);
    return (const char*)_ptr;
}