#include "bingo_pg_common.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_fingerprint.h"
#include "base_c/bitarray.h"
#include "base_cpp/output.h"
#include "bingo_pg_ext_bitset.h"
#include "bingo_pg_buffer.h"
#include "bingo_pg_text.h"
#include "bingo_core_c.h"
#include "base_cpp/tlscont.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "storage/bufmgr.h"
#include "access/itup.h"
#include "utils/relcache.h"
#include "storage/lock.h"
#include "access/heapam.h"
#include "storage/bufmgr.h"
#include "catalog/pg_type.h"
//#include "parser/parse_func.h"
//#include "funcapi.h"
#include "executor/spi.h"
}


//dword BingoPgCommon::getFunctionOid(const char* name, indigo::Array<dword>& types) {
//   indigo::Array<char> fname;
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

using namespace indigo;

void BingoPgCommon::getSearchTypeString(int type, indigo::Array<char>& result) {
   result.clear();
   switch(type) {
      case(MOL_SUB):
         result.readString("SUB", true);
         break;
      case(MOL_SIM):
         result.readString("SIM", true);
         break;
      case(MOL_SMARTS):
         result.readString("SMARTS", true);
         break;
      case(MOL_EXACT):
         result.readString("EXACT", true);
         break;
      case(MOL_GROSS):
         result.readString("GROSS", true);
         break;
      default:
         break;

   }
}

void BingoPgCommon::printBitset(const char* name, BingoPgExternalBitset& bitset) {
   elog(INFO, "bitset = %s", name);
   indigo::Array<char> bits;
   indigo::ArrayOutput ao(bits);
   for (int x = bitset.begin(); x != bitset.end(); x = bitset.next(x)) {
      ao.printf("%d ", x);
   }
   bits.push(0);
   elog(INFO, "%s", bits.ptr());

}

void BingoPgCommon::printFPBitset(const char* name, unsigned char* bitset, int size) {
   elog(INFO, "bitset = %s", name);
   indigo::Array<char> bits;
   indigo::ArrayOutput ao(bits);
   for (int fp_idx = 0; fp_idx < size; fp_idx++) {
      if (bitGetBit(bitset, fp_idx)) {
         ao.printf("%d ", fp_idx);
      }
   }
   bits.push(0);
   elog(INFO, "%s", bits.ptr());
}

//dword BingoPgCommon::getFunctionOid1(const char* name, dword type1) {
//   QS_DEF(indigo::Array<dword>, types);
//   types.clear();
//   types.push(type1);
//   return getFunctionOid(name, types);
//}
//
//dword BingoPgCommon::callFunction(dword functionId, indigo::Array<dword>& args) {
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
////      elog(ERROR, "function %u returned NULL", flinfo.fn_oid);
//
//   return result;
//}

//dword BingoPgCommon::callFunction1(dword oid, dword arg1) {
//   QS_DEF(indigo::Array<dword>, args);
//   args.clear();
//   args.push(arg1);
//   return callFunction(oid, args);
//}


void BingoPgCommon::setDefaultOptions() {
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

//void BingoPgCommon::executeQuery(const char* query_str) {
//   int result = SPI_exec(query_str, 1);
//   if(result < 0)
//      elog(ERROR, "error while executing query: %s", query_str);
////   Oid func_oid = getFunctionOid1("bingo_execute_func", TEXTOID);
////
////   BingoPgText text_query;
////   text_query.initFromString(query_str);
////
////   OidFunctionCall1(func_oid, text_query.getDatum());
//}

int BingoPgCommon::executeQuery(indigo::Array<char>& query_str) {
   SPI_connect();
   int success = SPI_exec(query_str.ptr(), 1);
   int result = SPI_processed;
   SPI_finish();
   if(success < 0) {
      elog(ERROR, "error (%d) while executing query: %s res",  success, query_str.ptr());
   }
   return result;
}

int BingoPgCommon::executeQuery(const char *format, ...) {
   QS_DEF(Array<char>, buf);
   ArrayOutput output(buf);
   va_list args;
   va_start(args, format);
   output.vprintf(format, args);
   va_end(args);
   output.writeChar(0);

   return executeQuery(buf);
}

bool BingoPgCommon::tableExists(const char* table_name) {
   return (executeQuery("select * from information_schema.tables where "
           "table_catalog = CURRENT_CATALOG and table_schema = CURRENT_SCHEMA "
           "and table_name = '%s'", table_name) > 0);
   
}

void BingoPgCommon::createDependency(const char* child_table, const char* parent_table) {
   QS_DEF(Array<char>, query_str);
   ArrayOutput query_out(query_str);
   query_out.clear();
   query_out.printf("INSERT INTO pg_depend (classid, objid, objsubid, refclassid, refobjid, refobjsubid, deptype) VALUES (");
   query_out.printf("'pg_class'::regclass::oid, '%s'::regclass::oid, 0, ", child_table);
   query_out.printf("'pg_class'::regclass::oid, '%s'::regclass::oid, 0, 'i')", parent_table);
   query_out.writeChar(0);
   
   executeQuery(query_str);
}

void BingoPgCommon::dropDependency(const char* table_name) {
   executeQuery("DELETE FROM pg_depend WHERE objid='%s'::regclass::oid", table_name);
}