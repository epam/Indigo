#include "bingo_postgres.h"
#include "bingo_pg_common.h"
#include "pg_bingo_context.h"
#include "bingo_core_c.h"
#include "bingo_pg_config.h"
#include "bingo_pg_text.h"
#include "bingo_pg_buffer.h"
#include "bingo_pg_config.h"
#include "base_cpp/scanner.h"


CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "utils/relcache.h"
#include "storage/lock.h"
#include "access/heapam.h"
#include "storage/bufmgr.h"
#include "catalog/pg_type.h"
#include "parser/parse_func.h"
#include "catalog/namespace.h"
#include "utils/lsyscache.h"
}

CEXPORT {
PG_FUNCTION_INFO_V1(bingo_test);
Datum bingo_test(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_test_tid);
Datum bingo_test_tid(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_test_select);
Datum bingo_test_select(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_markpos);
Datum bingo_markpos(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_restrpos);
Datum bingo_restrpos(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(getindexstructurescount);
Datum getindexstructurescount(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(getversion);
Datum getversion(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(filetotext);
Datum filetotext(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(filetoblob);
Datum filetoblob(PG_FUNCTION_ARGS);

}

CEXPORT {
#ifdef PG_MODULE_MAGIC
   PG_MODULE_MAGIC;
#endif

}

using namespace indigo;

Datum getindexstructurescount(PG_FUNCTION_ARGS){
   Oid relOid = PG_GETARG_OID(0);

   int result = 0;
   Relation rel;

   rel = relation_open(relOid, AccessShareLock);

   BingoPgBuffer meta_buffer;
   meta_buffer.readBuffer(rel, BINGO_METAPAGE, BINGO_PG_READ);
   BingoMetaPage meta_page = BingoPageGetMeta(BufferGetPage(meta_buffer.getBuffer()));

   result = meta_page->n_molecules;

//   elog(INFO, "attrs num = %d", rel->rd_att->natts);

   relation_close(rel, AccessShareLock);

   PG_RETURN_INT32(result);
}



Datum bingo_test(PG_FUNCTION_ARGS) {
   elog(INFO, "start test function 3");
   PG_RETURN_VOID();
}

Datum bingo_test_tid(PG_FUNCTION_ARGS) {
   elog(INFO, "start test function tid");

   ItemPointer pp = (ItemPointer) palloc0(sizeof(ItemPointerData));

   ItemPointerSet(pp, 1, 2);

   PG_RETURN_POINTER(pp);
}

//static Oid getFunc(const char* name, Array<Oid>& types) {
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

//Datum bingo_test_select(PG_FUNCTION_ARGS) {
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
Datum
bingo_markpos(PG_FUNCTION_ARGS) {
   elog(ERROR, "bingo does not support mark/restore");
   PG_RETURN_VOID();
}

/*
 * Restore scan to last saved position
 */
Datum
bingo_restrpos(PG_FUNCTION_ARGS) {
   elog(ERROR, "bingo does not support mark/restore");
   PG_RETURN_VOID();
}

void
bingo_redo(XLogRecPtr lsn, XLogRecord *record) {
   elog(PANIC, "bingo_redo: unimplemented");
}

void
bingo_desc(StringInfo buf, uint8 xl_info, char *rec) {
}

Datum getversion(PG_FUNCTION_ARGS) {
   PG_RETURN_CSTRING(bingoGetVersion());
}

Datum filetotext(PG_FUNCTION_ARGS) {
   Datum file_name_datum = PG_GETARG_DATUM(0);

   void* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgText fname_text(file_name_datum);
      BingoPgText result_text;

      QS_DEF(Array<char>, buffer);
      buffer.clear();
      FileScanner f_scanner(fname_text.getString());
      f_scanner.readAll(buffer);

      result_text.initFromArray(buffer);

      result = result_text.release();
   }
   PG_BINGO_END

   if(result == 0)
      PG_RETURN_NULL();
   
   PG_RETURN_TEXT_P(result);
}

Datum filetoblob(PG_FUNCTION_ARGS) {
   Datum file_name_datum = PG_GETARG_DATUM(0);
   void* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgText fname_text(file_name_datum);
      BingoPgText result_text;
      QS_DEF(Array<char>, buffer);
      buffer.clear();
      FileScanner f_scanner(fname_text.getString());
      f_scanner.readAll(buffer);

      result_text.initFromArray(buffer);
      
      result = result_text.release();

   }
   PG_BINGO_END

   if(result == 0)
      PG_RETURN_NULL();
   
   PG_RETURN_BYTEA_P(result);
}
