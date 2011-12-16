extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "executor/spi.h"
#include "catalog/pg_type.h"
}
#ifdef qsort
#undef qsort
#endif
#ifdef printf
#undef printf
#endif
#include "bingo_postgres.h"
#include "bingo_pg_common.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "bingo_core_c.h"
#include "bingo_pg_text.h"
#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "bingo_pg_cursor.h"


extern "C" {

PG_FUNCTION_INFO_V1(importsdf);
PGDLLEXPORT Datum importsdf(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(importrdf);
PGDLLEXPORT Datum importrdf(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(importsmiles);
PGDLLEXPORT Datum importsmiles(PG_FUNCTION_ARGS);

}

using namespace indigo;

static void checkImportNull(Datum text_datum, const char* message, BingoPgText& text) {
   if(text_datum == 0)
      throw BingoPgError("can not import structures: %s is empty", message);
   text.init(text_datum);
   int text_size = 0;
   text.getText(text_size);
   if(text_size == 0)
      throw BingoPgError("can not import structures: %s is empty", message);
}
static void checkImportEmpty(Datum text_datum, BingoPgText& text) {
   if(text_datum == 0)
      text.initFromString("");
   else
      text.init(text_datum);
}

//static int _initializeColumnQuery(Datum table_datum, Datum column_datum, Datum other_columns_datum, Array<char>& query_str) {
//   BingoPgText tablename_text;
//   BingoPgText column_text;
//   BingoPgText other_columns_text;
//
//   checkImportNull(table_datum, "table name", tablename_text);
//   checkImportNull(column_datum, "column name", column_text);
//   checkImportEmpty(other_columns_datum, other_columns_text);
//
//   ArrayOutput query_out(query_str);
//
//   query_out.printf("INSERT INTO %s(%s", tablename_text.getString(), column_text.getString());
//
//   int column_count = bingoImportParseFieldList(other_columns_text.getString());
//
//   for (int col_idx = 0; col_idx < column_count; ++col_idx) {
//      query_out.printf(", %s", bingoImportGetColumnName(col_idx));
//   }
//   query_out.printf(") VALUES($1");
//
//   for (int col_idx = 0; col_idx < column_count; ++col_idx) {
//      query_out.printf(", $%d", col_idx + 2);
//   }
//   query_out.printf(")");
//   query_out.writeChar(0);
//   return column_count;
//}
//
//static int _initializeIdQuery(Datum table_datum, Datum column_datum, Datum id_column_datum, Array<char>& query_str) {
//   BingoPgText tablename_text;
//   BingoPgText column_text;
//   BingoPgText id_column_text;
//
//   checkImportNull(table_datum, "table name", tablename_text);
//   checkImportNull(column_datum, "column name", column_text);
//   checkImportEmpty(id_column_datum, id_column_text);
//
//   ArrayOutput query_out(query_str);
//
//   query_out.printf("INSERT INTO %s(%s", tablename_text.getString(), column_text.getString());
//   int column_count = 0;
//
//   if(id_column_datum != 0 ) {
//      if(strcmp(id_column_text.getString(), "") !=0)
//         column_count = 1;
//   }
//
//   for (int col_idx = 0; col_idx < column_count; ++col_idx) {
//      query_out.printf(", %s", id_column_text.getString());
//   }
//   query_out.printf(") VALUES($1");
//
//   for (int col_idx = 0; col_idx < column_count; ++col_idx) {
//      query_out.printf(", $%d", col_idx + 2);
//   }
//   query_out.printf(")");
//   query_out.writeChar(0);
//   return column_count;
//
//}



class BingoImportHandler : public BingoPgCommon::BingoSessionHandler {
public:
   class ImportColumn {
   public:
      ImportColumn() {}
      ~ImportColumn() {}

      Array<char> columnName;
      Oid type;
   private:
      ImportColumn(const ImportColumn&); //no implicit copy
   };
   
   class ImportData {
   public:
      ImportData() {}
      virtual ~ImportData() {}

      virtual uintptr_t getDatum() = 0;
      virtual void convert(const char* str) = 0;
   private:
      ImportData(const ImportData&); //no implicit copy
   };
   class ImportTextData : public ImportData {
   public:
      ImportTextData() {}
      virtual ~ImportTextData() {}

      BingoPgText data;

      virtual uintptr_t getDatum() {
         return data.getDatum();
      }
      virtual void convert(const char* str) {
         if(str != 0)
            data.initFromString(str);
         else
            data.clear();
      }
   private:
      ImportTextData(const ImportTextData&); //no implicit copy
   };
   
   class ImportInt8Data : public ImportData{
   public:
      ImportInt8Data():data(0) {}
      virtual ~ImportInt8Data() {}
      AutoPtr<int64> data;

      virtual void convert(const char* str) {

      }

      virtual uintptr_t getDatum() {
         if(data.get() == 0)
            return 0;
         else
            return Int64GetDatum(data.ref());
      }
   private:
      ImportInt8Data(const ImportInt8Data&); //no implicit copy
   };
   class ImportInt4Data : public ImportData{
   public:
      ImportInt4Data():data(0) {}
      virtual ~ImportInt4Data() {}
      AutoPtr<int32> data;

      virtual void convert(const char* str) {
         
      }

      virtual uintptr_t getDatum() {
         if(data.get() == 0)
            return 0;
         else
            return Int32GetDatum(data.ref());
      }
   private:
      ImportInt4Data(const ImportInt4Data&); //no implicit copy
   };
   
public:
   BingoImportHandler(unsigned int func_id):BingoSessionHandler(func_id, true) {
      SPI_connect();
   }
   virtual ~BingoImportHandler() {
      SPI_finish();
   }
   
   virtual bool hasNext() = 0;
   virtual void getNextData() = 0;

   void init(Datum table_datum, Datum column_datum, Datum other_columns_datum) {
      BingoPgText tablename_text;
      BingoPgText column_text;
      BingoPgText other_columns_text;

      checkImportNull(table_datum, "table name", tablename_text);
      checkImportNull(column_datum, "column name", column_text);
      checkImportEmpty(other_columns_datum, other_columns_text);

      ArrayOutput column_names(_columnNames);
      _importColumns.clear();

      _importColumns.push().columnName.readString(column_text.getString(), true);
      
      column_names.printf("%s(%s", tablename_text.getString(), column_text.getString());

      if (_parseColumns) {
         int column_count = bingoImportParseFieldList(other_columns_text.getString());
         for (int col_idx = 0; col_idx < column_count; ++col_idx) {
            ImportColumn& idCol = _importColumns.push();
            idCol.columnName.readString(bingoImportGetColumnName(col_idx), true);
            column_names.printf(", %s", idCol.columnName.ptr());
         }
      } else {
         if (other_columns_datum != 0) {
            if (strcmp(other_columns_text.getString(), "") != 0) {
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

   void _defineColumnTypes(const char* table_name) {
      Array<char> column_names;
      for (int i = 0; i < _importColumns.size(); ++i) {
         if(i != 0)
            column_names.appendString(", ", true);
         column_names.appendString(_importColumns[i].columnName.ptr(), true);
      }

      BingoPgCursor table_first("select %s from %s", column_names.ptr(), table_name);
      table_first.next();

      for (int i = 0; i < _importColumns.size(); ++i) {
         int arg_type = table_first.getArgOid(i);
         ImportColumn& dataCol = _importColumns.at(i);
         
         if((arg_type != INT4OID) &&
                 (arg_type != INT8OID) &&
                 (arg_type != TEXTOID) &&
                 (arg_type != BYTEAOID))
            throw BingoPgError("can not import a structure: unsupported column '%s' type; supported values: 'text', 'bytea', 'integer'", dataCol.columnName.ptr());

         dataCol.type = arg_type;
      }
      
   }

   void _addData(const char* data, int col_idx) {
      AutoPtr<ImportData> import_data;
      ImportColumn& import_column = _importColumns[col_idx];
      
      switch(import_column.type) {
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

      import_data->convert(data);

      _importData.add(import_data.release());
   }
//   void _addData(const char* data, int col_idx, ArrayOutput& column_values, ObjArray<BingoPgText>& q_data) {
//      ImportColumn& dataCol = _importColumns[col_idx];
//      if (dataCol.isText) {
//         BingoPgText& col_data = q_data.push();
//
//         if(data != 0)
//            col_data.initFromString(data);
//
//         column_values.printf(", $%d", q_data.size());
//      } else {
//         if(data == 0)
//            column_values.printf(", null");
//         else
//            column_values.printf(", '%s'::%s", data, dataCol.type.ptr());
//      }
//   }
   
   void import() {
      QS_DEF(Array<char>, query_str);
      QS_DEF(Array<Datum>, q_values);
      QS_DEF(Array<Oid>, q_oids);
      QS_DEF(Array<char>, q_nulls);
//      ObjArray<BingoPgText> q_data;
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
      for (int col_idx = 0; col_idx < _importColumns.size(); ++col_idx) {
         q_nulls.push(0);
         q_oids.push(_importColumns[col_idx].type);

         if(col_idx != 0)
            query_string.printf(", ");
         query_string.printf("$%d", col_idx + 1);
      }
      query_string.printf(")");
      query_string.writeChar(0);
      
      raise_error = false;
      
      /*
       * Loop through data 
       */
      while (hasNext()) {
//         q_data.clear();
         /*
          * Initialize data
          */
         try {
            getNextData();
         } catch (Exception& e) {
            elog(WARNING, "%s", e.message());
         }
         /*
          * Initialize values for the query
          */
         q_values.clear();
         for (int q_idx = 0; q_idx < _importData.size(); ++q_idx) {
            q_values.push(_importData[q_idx]->getDatum());
            if(q_values[q_idx] == 0) {
               q_nulls[q_idx] = 'n';
            } else {
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
         BINGO_PG_HANDLE(throw BingoPgError("can not insert a structure into a table: %s", message));

         /*
          * Return back session id and error handler
          */
         refresh();
      }

   }
protected:
   bool _parseColumns;
   Array<char> _columnNames;
   
   ObjArray<ImportColumn> _importColumns;
   PtrArray<ImportData> _importData;
   
private:
   BingoImportHandler(const BingoImportHandler&); //no implicit copy
};

class BingoImportSdfHandler : public BingoImportHandler {
public:
   BingoImportSdfHandler(unsigned int func_id, const char* fname):
   BingoImportHandler(func_id) {
      _parseColumns = true;
      setFunctionName("importSDF");
      bingoSDFImportOpen(fname);
   }
   virtual ~BingoImportSdfHandler() {
      bingoSDFImportClose();
   }

   virtual bool hasNext() {
      return !bingoSDFImportEOF();
   }

   virtual void getNextData() {
      _importData.clear();
      
      const char* data = 0;
       _importData.clear();
//      q_data.clear();
//      ArrayOutput column_values(_columnValues);
//
//      q_data.push().initFromString();
//      column_values.printf("($1");
      
      for (int col_idx = 0; col_idx < _importColumns.size(); ++col_idx) {
         if(col_idx == 0)
            data = bingoSDFImportGetNext();
         else
            data = bingoImportGetPropertyValue(col_idx - 1);
         _addData(data, col_idx);
//         _addData(data, col_idx, column_values, q_data);
      }
//      column_values.printf(")");
//      column_values.writeChar(0);
   }

private:
   BingoImportSdfHandler(const BingoImportSdfHandler&); //no implicit copy
};

Datum importsdf(PG_FUNCTION_ARGS) {
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
//Datum importsdf(PG_FUNCTION_ARGS) {
//   Datum table_datum = PG_GETARG_DATUM(0);
//   Datum column_datum = PG_GETARG_DATUM(1);
//   Datum other_columns_datum = PG_GETARG_DATUM(2);
//   Datum file_name_datum = PG_GETARG_DATUM(3);
//
//   PG_BINGO_BEGIN
//   {
//      QS_DEF(Array<char>, query_str);
//      QS_DEF(Array<Datum>, q_values);
//      QS_DEF(Array<Oid>, q_oids);
//      QS_DEF(Array<char>, q_nulls);
//      ObjArray<BingoPgText> q_data;
//      int spi_success = 0;
//
//      BingoPgText fname_text;
//      checkImportNull(file_name_datum, "file name", fname_text);
//
//      BingoImportSdfHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());
//
//      int column_count = _initializeColumnQuery(table_datum, column_datum, other_columns_datum, query_str);
//
//      q_oids.clear();
//      q_nulls.clear();
//
//      for (int col_idx = 0; col_idx < column_count + 1; ++col_idx) {
//         q_oids.push(TEXTOID);
//         q_nulls.push(0);
//      }
//
//      bingo_handler.raise_error = false;
//      while (!bingoSDFImportEOF()) {
//         q_data.clear();
//         const char* data = bingoSDFImportGetNext();
//         q_data.push().initFromString(data);
//
//         for (int col_idx = 0; col_idx < column_count; ++col_idx) {
//            q_data.push().initFromString(bingoImportGetPropertyValue(col_idx));
//         }
//
//         q_values.clear();
//         for (int q_idx = 0; q_idx < q_data.size(); ++q_idx) {
//            q_values.push(q_data[q_idx].getDatum());
//         }
//
//         BINGO_PG_TRY {
//            spi_success = SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);
//            if(spi_success < 0)
//               elog(WARNING, "can not insert a structure into a table");
//         } BINGO_PG_HANDLE(throw BingoPgError("can not insert a structure into a table: %s", message));
//         /*
//          * Return back session id and error handler
//          */
//         bingo_handler.refresh();
//
//      }
//
//   }
//   PG_BINGO_END
//
//   PG_RETURN_VOID();
//}

//class BingoImportRdfHandler : public BingoPgCommon::BingoSessionHandler {
//public:
//   BingoImportRdfHandler(unsigned int func_id, const char* fname):BingoSessionHandler(func_id, true) {
//      setFunctionName("importRDF");
//      bingoRDFImportOpen(fname);
//      SPI_connect();
//   }
//   virtual ~BingoImportRdfHandler() {
//      SPI_finish();
//      bingoRDFImportClose();
//   }
//
//private:
//   BingoImportRdfHandler(const BingoImportRdfHandler&); //no implicit copy
//};

class BingoImportRdfHandler : public BingoImportHandler {
public:
   BingoImportRdfHandler(unsigned int func_id, const char* fname):
   BingoImportHandler(func_id) {
      _parseColumns = true;
      setFunctionName("importRDF");
      bingoRDFImportOpen(fname);
   }
   virtual ~BingoImportRdfHandler() {
      bingoRDFImportClose();
   }

   virtual bool hasNext() {
      return !bingoRDFImportEOF();
   }

   virtual void getNextData() {
//      q_data.clear();
//      ArrayOutput column_values(_columnValues);
//
//      q_data.push().initFromString(bingoRDFImportGetNext());
//      column_values.printf("($1");

      const char* data = 0;
       _importData.clear();

      for (int col_idx = 0; col_idx < _importColumns.size(); ++col_idx) {
         if(col_idx == 0)
            data = bingoRDFImportGetNext();
         else
            data = bingoImportGetPropertyValue(col_idx - 1);
         _addData(data, col_idx);
//         const char* data = bingoImportGetPropertyValue(col_idx);
//         _addData(data, col_idx, column_values, q_data);
      }
//      column_values.printf(")");
//      column_values.writeChar(0);
   }

private:
   BingoImportRdfHandler(const BingoImportSdfHandler&); //no implicit copy
};

Datum importrdf(PG_FUNCTION_ARGS) {
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

//Datum importrdf(PG_FUNCTION_ARGS) {
//   Datum table_datum = PG_GETARG_DATUM(0);
//   Datum column_datum = PG_GETARG_DATUM(1);
//   Datum other_columns_datum = PG_GETARG_DATUM(2);
//   Datum file_name_datum = PG_GETARG_DATUM(3);
//
//   PG_BINGO_BEGIN
//   {
//
//      QS_DEF(Array<char>, query_str);
//      QS_DEF(Array<Datum>, q_values);
//      QS_DEF(Array<Oid>, q_oids);
//      QS_DEF(Array<char>, q_nulls);
//      ObjArray<BingoPgText> q_data;
//      int spi_success = 0;
//
//      BingoPgText fname_text;
//      checkImportNull(file_name_datum, "file name", fname_text);
//
//      BingoImportRdfHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());
//
//      int column_count = _initializeColumnQuery(table_datum, column_datum, other_columns_datum, query_str);
//
//      q_oids.clear();
//      q_nulls.clear();
//
//      for (int col_idx = 0; col_idx < column_count + 1; ++col_idx) {
//         q_oids.push(TEXTOID);
//         q_nulls.push(0);
//      }
//
//      bingo_handler.raise_error = false;
//      while (!bingoRDFImportEOF()) {
//         q_data.clear();
//         const char* data = bingoRDFImportGetNext();
//         q_data.push().initFromString(data);
//
//         for (int col_idx = 0; col_idx < column_count; ++col_idx) {
//            q_data.push().initFromString(bingoImportGetPropertyValue(col_idx));
//         }
//
//         q_values.clear();
//         for (int q_idx = 0; q_idx < q_data.size(); ++q_idx) {
//            q_values.push(q_data[q_idx].getDatum());
//         }
//
//         BINGO_PG_TRY
//         {
//            spi_success = SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);
//
//            if (spi_success < 0)
//               elog(WARNING, "can not insert a structure into a table");
//         }
//         BINGO_PG_HANDLE(throw BingoPgError("can not insert a structure into a table: %s", message));
//         /*
//          * Return back session id and error handler
//          */
//         bingo_handler.refresh();
//      }
//   }
//   PG_BINGO_END
//
//
//   PG_RETURN_VOID();
//}

class BingoImportSmilesHandler : public BingoImportHandler {
public:
   BingoImportSmilesHandler(unsigned int func_id, const char* fname):BingoImportHandler(func_id) {
      _parseColumns = false;
      setFunctionName("importSMILES");
      bingoSMILESImportOpen(fname);
   }
   virtual ~BingoImportSmilesHandler() {
      bingoSMILESImportClose();
   }

   virtual bool hasNext() {
      return !bingoSMILESImportEOF();
   }

   virtual void getNextData() {
//      q_data.clear();
//      ArrayOutput column_values(_columnValues);
//
//      q_data.push().initFromString(bingoSMILESImportGetNext());
//      column_values.printf("($1");
//      int arg_idx = 1;

      const char* data = 0;
      _importData.clear();

      for (int col_idx = 0; col_idx < _importColumns.size(); ++col_idx) {
         if(col_idx == 0)
            data = bingoSMILESImportGetNext();
         else
            data = bingoSMILESImportGetId();
         _addData(data, col_idx);
         
//         const char* data = bingoSMILESImportGetId();
//         _addData(data, col_idx, column_values, q_data);
      }
//      column_values.printf(")");
//      column_values.writeChar(0);
   }
private:
   BingoImportSmilesHandler(const BingoImportSmilesHandler&); //no implicit copy
};
//class BingoImportSmilesHandler : public BingoPgCommon::BingoSessionHandler {
//public:
//   BingoImportSmilesHandler(unsigned int func_id, const char* fname):BingoSessionHandler(func_id, true) {
//      setFunctionName("importSMILES");
//      bingoSMILESImportOpen(fname);
//      SPI_connect();
//   }
//   virtual ~BingoImportSmilesHandler() {
//      SPI_finish();
//      bingoSMILESImportClose();
//   }
//
//private:
//   BingoImportSmilesHandler(const BingoImportSmilesHandler&); //no implicit copy
//};

Datum importsmiles(PG_FUNCTION_ARGS) {
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
//Datum importsmiles(PG_FUNCTION_ARGS) {
//   Datum table_datum = PG_GETARG_DATUM(0);
//   Datum column_datum = PG_GETARG_DATUM(1);
//   Datum id_column_datum = PG_GETARG_DATUM(2);
//   Datum file_name_datum = PG_GETARG_DATUM(3);
//
//   PG_BINGO_BEGIN
//   {
//
//      QS_DEF(Array<char>, query_str);
//      QS_DEF(Array<Datum>, q_values);
//      QS_DEF(Array<Oid>, q_oids);
//      QS_DEF(Array<char>, q_nulls);
//      ObjArray<BingoPgText> q_data;
//      int spi_success = 0;
//
//      BingoPgText fname_text;
//      checkImportNull(file_name_datum, "file name", fname_text);
//
//      BingoImportSmilesHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());
//
//      int column_count = _initializeIdQuery(table_datum, column_datum, id_column_datum, query_str);
//
//      q_oids.clear();
//      q_nulls.clear();
//
//      for (int col_idx = 0; col_idx < column_count + 1; ++col_idx) {
//         q_oids.push(TEXTOID);
//         q_nulls.push(0);
//      }
//
//      bingo_handler.raise_error = false;
//      while (!bingoSMILESImportEOF()) {
//         q_data.clear();
//         const char* data = bingoSMILESImportGetNext();
//         q_data.push().initFromString(data);
//
//         for (int col_idx = 0; col_idx < column_count; ++col_idx) {
//            const char* id_string = bingoSMILESImportGetId();
//            BingoPgText& id_data = q_data.push();
//            if (id_string == 0) {
//               q_nulls[col_idx + 1] = 'n';
//            } else {
//               id_data.initFromString(id_string);
//               q_nulls[col_idx + 1] = 0;
//            }
//         }
//
//         q_values.clear();
//         for (int q_idx = 0; q_idx < q_data.size(); ++q_idx) {
//            q_values.push(q_data[q_idx].getDatum());
//         }
//
//         BINGO_PG_TRY
//         {
//            spi_success = SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);
//            if (spi_success < 0)
//               elog(WARNING, "can not insert a structure into a table");
//         }
//         BINGO_PG_HANDLE(throw BingoPgError("can not insert a structure into a table: %s", message));
//         /*
//          * Return back session id and error handler
//          */
//         bingo_handler.refresh();
//      }
//   }
//   PG_BINGO_END
//
//
//   PG_RETURN_VOID();
//}