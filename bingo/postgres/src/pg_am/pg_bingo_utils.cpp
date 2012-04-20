extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "utils/relcache.h"
#include "storage/lock.h"
#include "access/heapam.h"
#include "storage/bufmgr.h"
}


#ifdef qsort
#undef qsort
#endif
#ifdef printf
#undef printf
#endif

#include "bingo_postgres.h"

#include "base_cpp/scanner.h"
#include "bingo_core_c.h"

#include "bingo_pg_common.h"
#include "pg_bingo_context.h"
#include "bingo_pg_config.h"
#include "bingo_pg_text.h"
#include "bingo_pg_buffer.h"
#include "bingo_pg_config.h"
#include "bingo_pg_cursor.h"



extern "C" {
PG_FUNCTION_INFO_V1(bingo_test);
PGDLLEXPORT Datum bingo_test(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_test_tid);
PGDLLEXPORT Datum bingo_test_tid(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_test_select);
PGDLLEXPORT Datum bingo_test_select(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_markpos);
PGDLLEXPORT Datum bingo_markpos(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_restrpos);
PGDLLEXPORT Datum bingo_restrpos(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(getindexstructurescount);
PGDLLEXPORT Datum getindexstructurescount(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(getversion);
PGDLLEXPORT Datum getversion(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(filetotext);
PGDLLEXPORT Datum filetotext(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(filetoblob);
PGDLLEXPORT Datum filetoblob(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(getname);
PGDLLEXPORT Datum getname(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(exportsdf);
PGDLLEXPORT Datum exportsdf(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(exportrdf);
PGDLLEXPORT Datum exportrdf(PG_FUNCTION_ARGS);

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
   elog(NOTICE, "start test function 3");
   PG_RETURN_VOID();
}

Datum bingo_test_tid(PG_FUNCTION_ARGS) {
   elog(NOTICE, "start test function tid");

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
      /*
       * Read file
       */
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
      /*
       * Read file
       */
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

Datum getname(PG_FUNCTION_ARGS) {
   Datum target_datum = PG_GETARG_DATUM(0);
   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid, false);
      bingo_handler.setFunctionName("getname");

      BingoPgText mol_text(target_datum);
      int buf_size;
      const char* target_buf = mol_text.getText(buf_size);

      const char* bingo_result = bingoGetNameCore(target_buf, buf_size);

      if (bingo_handler.error_raised)
         PG_RETURN_NULL();

      result = BingoPgCommon::releaseString(bingo_result);
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

static void _parseQueryFieldList(const char* fields_str, RedBlackStringMap<int, false >& field_list) {
   BufferScanner scanner(fields_str);

   QS_DEF(Array<char>, buf_word);

   scanner.skipSpace();
   int column_idx = field_list.size();

   while (!scanner.isEOF()) {
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

static void checkExportNull(Datum text_datum, const char* message, BingoPgText& text) {
   if(text_datum == 0)
      throw BingoPgError("can not export structures: %s is empty", message);
   text.init(text_datum);
   int text_size = 0;
   text.getText(text_size);
   if(text_size == 0)
      throw BingoPgError("can not export structures: %s is empty", message);
}
static void checkExportEmpty(Datum text_datum, BingoPgText& text) {
   if(text_datum == 0)
      text.initFromString("");
   else
      text.init(text_datum);
}

static int _initializeColumnQuery(Datum table_datum, Datum column_datum, Datum other_column_datum, Array<char>& query_str, RedBlackStringMap<int, false >& field_list) {
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
   
   if(other_column_datum != 0 ) {
      const char* columns_list = other_column_text.getString();
      if(strcmp(columns_list, "") !=0) {
         _parseQueryFieldList(columns_list, field_list);
         query_out.printf(", %s ", columns_list);
      }
   }

   query_out.printf("FROM %s", tablename_text.getString());
   query_out.writeChar(0);
   
   return data_key;
}

//class BingoExportSdfHandler : public BingoPgCommon::BingoSessionHandler {
//public:
//   BingoExportSdfHandler(unsigned int func_id, const char* fname):BingoSessionHandler(func_id, true) {
//      setFunctionName("exportSDF");
//      bingoSDFExportOpen(fname);
////      SPI_connect();
//   }
//   virtual ~BingoImportSdfHandler() {
////      SPI_finish();
//      bingoSDFExportClose();
//   }
//
//private:
//   BingoExportSdfHandler(const BingoExportSdfHandler&); //no implicit copy
//};

Datum exportsdf(PG_FUNCTION_ARGS) {
   Datum table_datum = PG_GETARG_DATUM(0);
   Datum column_datum = PG_GETARG_DATUM(1);
   Datum other_columns_datum = PG_GETARG_DATUM(2);
   Datum file_name_datum = PG_GETARG_DATUM(3);

   PG_BINGO_BEGIN
   {
      QS_DEF(Array<char>, query_str);
      RedBlackStringMap<int, false > field_list;

      BingoPgText fname_text;
      checkExportNull(file_name_datum, "file name", fname_text);
      FileOutput file_output(fname_text.getString());

      int data_key = _initializeColumnQuery(table_datum, column_datum, other_columns_datum, query_str, field_list);

      BingoPgCursor table_cursor(query_str.ptr());
      BingoPgText buf_text;

      while (table_cursor.next()) {
         table_cursor.getText(1, buf_text);
         file_output.writeStringCR(buf_text.getString());
         
         for (int k = field_list.begin(); k != field_list.end(); k = field_list.next(k)) {
            if(data_key == k)
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

Datum exportrdf(PG_FUNCTION_ARGS) {
   Datum table_datum = PG_GETARG_DATUM(0);
   Datum column_datum = PG_GETARG_DATUM(1);
   Datum other_columns_datum = PG_GETARG_DATUM(2);
   Datum file_name_datum = PG_GETARG_DATUM(3);

   PG_BINGO_BEGIN
   {
      QS_DEF(Array<char>, query_str);
      RedBlackStringMap<int, false > field_list;

      BingoPgText fname_text;
      checkExportNull(file_name_datum, "file name", fname_text);
      FileOutput file_output(fname_text.getString());

      int data_key = _initializeColumnQuery(table_datum, column_datum, other_columns_datum, query_str, field_list);

      BingoPgCursor table_cursor(query_str.ptr());
      BingoPgText buf_text;

      file_output.printf("$RDFILE 1\n");

      int str_idx = 0;
      while (table_cursor.next()) {
         ++str_idx;
         table_cursor.getText(1, buf_text);
         file_output.printf("$MFMT $MIREG %d\n", str_idx);
         file_output.writeStringCR(buf_text.getString());

         for (int k = field_list.begin(); k != field_list.end(); k = field_list.next(k)) {
            if(data_key == k)
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
