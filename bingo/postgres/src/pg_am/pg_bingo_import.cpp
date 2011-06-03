#include "bingo_postgres.h"
#include "bingo_pg_common.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "bingo_core_c.h"
#include "bingo_pg_text.h"
#include "base_cpp/array.h"
#include "base_cpp/output.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "executor/spi.h"
#include "catalog/pg_type.h"
}

CEXPORT {

//PG_FUNCTION_INFO_V1(bingoimportsdf_begin);
//Datum bingoimportsdf_begin(PG_FUNCTION_ARGS);
//
//PG_FUNCTION_INFO_V1(bingoimportsdf_hasnext);
//Datum bingoimportsdf_hasnext(PG_FUNCTION_ARGS);
//
//PG_FUNCTION_INFO_V1(bingoimportsdf_next);
//Datum bingoimportsdf_next(PG_FUNCTION_ARGS);
//
//PG_FUNCTION_INFO_V1(bingoimportsdf_end);
//Datum bingoimportsdf_end(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(importsdf);
Datum importsdf(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(importrdf);
Datum importrdf(PG_FUNCTION_ARGS);

}

static void bingoPgImportWarningHandler(const char *message, void *context) {
   elog(WARNING, "warning while bingo importing: %s", message);
}
static void bingoPgImportErrorHandler(const char *message, void *context) {
   elog(ERROR, "error while bingo importing: %s", message);
}

using namespace indigo;

//Datum
//bingoimportsdf_begin(PG_FUNCTION_ARGS) {
//   Datum file_name_datum = PG_GETARG_DATUM(0);
//
//   BingoPgText fname_text(file_name_datum);
//
//   qword session_id = bingoAllocateSessionID();
//   bingoSetSessionID(session_id);
//   bingoSetErrorHandler(bingoPgImportErrorHandler, 0);
//
//   bingoSDFImportOpen(fname_text.getString());
//
////   AutoPtr<BingoSdfLoaderWrapper> wrapper;
////   try {
////      wrapper.reset(new BingoSdfLoaderWrapper(file_name.ptr()));
////   } catch (Exception& e) {
////      elog(ERROR, "Error while loading file: %s", e.message());
////   }
//
////   PG_RETURN_POINTER(wrapper.release());
//   PG_RETURN_INT64(session_id);
//}
//
//Datum
//bingoimportsdf_hasnext(PG_FUNCTION_ARGS) {
////   BingoSdfLoaderWrapper* wrapper = (BingoSdfLoaderWrapper*)PG_GETARG_POINTER(0);
////   bool result = !wrapper->sdfLoader->isEOF();
//   qword session_id = PG_GETARG_INT64(0);
//   bingoSetSessionID(session_id);
//   bingoSetErrorHandler(bingoPgImportWarningHandler, 0);
//
//   bool result = bingoSDFImportEOF() == 0;
//
//   PG_RETURN_BOOL(result);
//}
//
//Datum
//bingoimportsdf_next(PG_FUNCTION_ARGS) {
////   BingoSdfLoaderWrapper* wrapper = (BingoSdfLoaderWrapper*)PG_GETARG_POINTER(0);
////   wrapper->sdfLoader->readNext();
////   Array<char>& data = wrapper->sdfLoader->data;
////   text* result = cstring_to_text_with_len(data.ptr(), data.sizeInBytes());
////   PG_RETURN_TEXT_P(result);
//   qword session_id = PG_GETARG_INT64(0);
//   bingoSetSessionID(session_id);
//   bingoSetErrorHandler(bingoPgImportWarningHandler, 0);
//
//   const char* next = bingoSDFImportGetNext();
//
//   text* result = cstring_to_text(next);
//   PG_RETURN_TEXT_P(result);
//
//}
//
//Datum
//bingoimportsdf_end(PG_FUNCTION_ARGS) {
////   BingoSdfLoaderWrapper* wrapper = (BingoSdfLoaderWrapper*)PG_GETARG_POINTER(0);
////   delete  wrapper;
//   qword session_id = PG_GETARG_INT64(0);
//   bingoSetSessionID(session_id);
//   bingoSDFImportClose();
//   bingoReleaseSessionID(session_id);
//   PG_RETURN_VOID();
//}
//
//Datum importsdf(PG_FUNCTION_ARGS) {
//   Oid relOid = PG_GETARG_OID(0);
//   Datum column_datum = PG_GETARG_DATUM(1);
//   Datum file_datum = PG_GETARG_DATUM(2);
//   elog(INFO, "start import");
//
//   Relation rel;
////   ForkNumber forkNum;
//
//   rel = relation_open(relOid, AccessExclusiveLock);
//
//   elog(INFO, "attrs num = %d", rel->rd_att->natts);
//
//   const char* xxx_t = "aaa";
//   Datum map_datum = PointerGetDatum(xxx_t);
//   int size = strlen(xxx_t);
//
//   TupleDesc index_desc = CreateTemplateTupleDesc(1, false);
//   index_desc->attrs[0]->attlen = size;
//   index_desc->attrs[0]->attalign = 'c';
//   index_desc->attrs[0]->attbyval = false;
//   bool isnull = false;
//
//   HeapTuple itup = heap_form_tuple(index_desc, &map_datum, &isnull);
////   int itemsz = IndexTupleDSize(*itup);
////   itemsz = MAXALIGN(itemsz);
//
//   simple_heap_insert(rel, itup);
//   pfree(itup);
//   /*
//    * heap size, including FSM and VM
//    */
////   for (forkNum = 0; forkNum <= MAX_FORKNUM; forkNum++)
////      size += calculate_relation_size(&(rel->rd_node), forkNum);
//
//   /*
//    * Size of toast relation
//    */
////   if (OidIsValid(rel->rd_rel->reltoastrelid))
////      size += calculate_toast_table_size(rel->rd_rel->reltoastrelid);
//
//   relation_close(rel, AccessExclusiveLock);
//
//
//   PG_RETURN_VOID();
//}

Datum importsdf(PG_FUNCTION_ARGS) {
   Datum table_datum = PG_GETARG_DATUM(0);
   Datum file_name_datum = PG_GETARG_DATUM(1);

   BingoPgText fname_text(file_name_datum);
   BingoPgText tablename_text(table_datum);

   qword session_id = bingoAllocateSessionID();
   bingoSetSessionID(session_id);
   bingoSetErrorHandler(bingoPgImportErrorHandler, 0);

   bingoSDFImportOpen(fname_text.getString());

   Array<char> query_str;
   ArrayOutput query_out(query_str);
   query_out.printf("INSERT INTO %s VALUES($1)", tablename_text.getString());
   query_out.writeChar(0);


   Array<Datum> q_values;
   Array<Oid> q_oids;
   Array<char> q_nulls;
   BingoPgText text_data;

   q_nulls.push(0);
   q_oids.push(TEXTOID);
   SPI_connect();
   bingoSetErrorHandler(bingoPgImportWarningHandler, 0);
   
   while(!bingoSDFImportEOF()) {
      const char* data = bingoSDFImportGetNext();
      text_data.initFromString(data);
      q_values.clear();
      q_values.push(text_data.getDatum());
      SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);
      /*
       * Return back session id and error handler
       */
      bingoSetSessionID(session_id);
      bingoSetErrorHandler(bingoPgImportWarningHandler, 0);
   }
   SPI_finish();

   bingoSDFImportClose();
   bingoReleaseSessionID(session_id);
   

   PG_RETURN_BOOL(true);
}
Datum importrdf(PG_FUNCTION_ARGS) {
   Datum table_datum = PG_GETARG_DATUM(0);
   Datum file_name_datum = PG_GETARG_DATUM(1);

   QS_DEF(Array<char>, query_str);
   QS_DEF(Array<Datum>, q_values);
   QS_DEF(Array<Oid>, q_oids);
   QS_DEF(Array<char>, q_nulls);
   BingoPgText text_data;
   BingoPgText fname_text(file_name_datum);
   BingoPgText tablename_text(table_datum);

   qword session_id = bingoAllocateSessionID();
   bingoSetSessionID(session_id);
   bingoSetErrorHandler(bingoPgImportErrorHandler, 0);

   bingoRDFImportOpen(fname_text.getString());
   
   ArrayOutput query_out(query_str);
   query_out.printf("INSERT INTO %s VALUES($1)", tablename_text.getString());
   query_out.writeChar(0);

   q_nulls.clear();
   q_oids.clear();
   q_nulls.push(0);
   q_oids.push(TEXTOID);
   
   SPI_connect();
   while(!bingoRDFImportEOF()) {
      const char* data = bingoRDFImportGetNext();
      text_data.initFromString(data);
      q_values.clear();
      q_values.push(text_data.getDatum());
      SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);
      /*
       * Return back session id and error handler
       */
      bingoSetSessionID(session_id);
      bingoSetErrorHandler(bingoPgImportWarningHandler, 0);
   }
   SPI_finish();

   bingoRDFImportClose();
   bingoReleaseSessionID(session_id);


   PG_RETURN_BOOL(true);
}