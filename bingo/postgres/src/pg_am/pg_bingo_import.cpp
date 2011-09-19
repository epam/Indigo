#include "bingo_postgres.h"
#include "bingo_pg_common.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "bingo_core_c.h"
#include "bingo_pg_text.h"
#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "executor/spi.h"
#include "catalog/pg_type.h"
}

extern "C" {

PG_FUNCTION_INFO_V1(importsdf);
Datum importsdf(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(importrdf);
Datum importrdf(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(importsmiles);
Datum importsmiles(PG_FUNCTION_ARGS);

}

using namespace indigo;

static int _initializeColumnQuery(Datum table_datum, Datum column_datum, Datum other_columns_datum, Array<char>& query_str) {
   BingoPgText tablename_text(table_datum);
   BingoPgText column_text(column_datum);
   BingoPgText other_columns_text(other_columns_datum);
   
   ArrayOutput query_out(query_str);

   query_out.printf("INSERT INTO %s(%s", tablename_text.getString(), column_text.getString());

   int column_count = bingoImportParseFieldList(other_columns_text.getString());

   for (int col_idx = 0; col_idx < column_count; ++col_idx) {
      query_out.printf(", %s", bingoImportGetColumnName(col_idx));
   }
   query_out.printf(") VALUES($1");

   for (int col_idx = 0; col_idx < column_count; ++col_idx) {
      query_out.printf(", $%d", col_idx + 2);
   }
   query_out.printf(")");
   query_out.writeChar(0);
   return column_count;
}

static int _initializeIdQuery(Datum table_datum, Datum column_datum, Datum id_column_datum, Array<char>& query_str) {
   BingoPgText tablename_text(table_datum);
   BingoPgText column_text(column_datum);
   BingoPgText id_column_text(id_column_datum);

   ArrayOutput query_out(query_str);

   query_out.printf("INSERT INTO %s(%s", tablename_text.getString(), column_text.getString());
   int column_count = 0;
   
   if(id_column_datum != 0 ) {
      if(strcmp(id_column_text.getString(), "") !=0)
         column_count = 1;
   }

   for (int col_idx = 0; col_idx < column_count; ++col_idx) {
      query_out.printf(", %s", id_column_text.getString());
   }
   query_out.printf(") VALUES($1");

   for (int col_idx = 0; col_idx < column_count; ++col_idx) {
      query_out.printf(", $%d", col_idx + 2);
   }
   query_out.printf(")");
   query_out.writeChar(0);
   return column_count;

}

class BingoImportSdfHandler : public BingoPgCommon::BingoSessionHandler {
public:
   BingoImportSdfHandler(unsigned int func_id, const char* fname):BingoSessionHandler(func_id, true) {
      setFunctionName("importSDF");
      bingoSDFImportOpen(fname);
      SPI_connect();
   }
   virtual ~BingoImportSdfHandler() {
      SPI_finish();
      bingoSDFImportClose();
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
      QS_DEF(Array<char>, query_str);
      QS_DEF(Array<Datum>, q_values);
      QS_DEF(Array<Oid>, q_oids);
      QS_DEF(Array<char>, q_nulls);
      ObjArray<BingoPgText> q_data;
      int spi_success = 0;

      BingoPgText fname_text(file_name_datum);
      BingoImportSdfHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());

      int column_count = _initializeColumnQuery(table_datum, column_datum, other_columns_datum, query_str);

      q_oids.clear();
      q_nulls.clear();

      for (int col_idx = 0; col_idx < column_count + 1; ++col_idx) {
         q_oids.push(TEXTOID);
         q_nulls.push(0);
      }

      bingo_handler.raise_error = false;
      while (!bingoSDFImportEOF()) {
         q_data.clear();
         const char* data = bingoSDFImportGetNext();
         q_data.push().initFromString(data);

         for (int col_idx = 0; col_idx < column_count; ++col_idx) {
            q_data.push().initFromString(bingoImportGetPropertyValue(col_idx));
         }

         q_values.clear();
         for (int q_idx = 0; q_idx < q_data.size(); ++q_idx) {
            q_values.push(q_data[q_idx].getDatum());
         }

         BINGO_PG_TRY {
            spi_success = SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);
            if(spi_success < 0)
               elog(WARNING, "can not insert a structure into a table");
         } BINGO_PG_HANDLE(throw BingoPgError("can not insert a structure into a table: %s", message));
         /*
          * Return back session id and error handler
          */
         bingo_handler.refresh();

      }

   }
   PG_BINGO_END

   PG_RETURN_VOID();
}

class BingoImportRdfHandler : public BingoPgCommon::BingoSessionHandler {
public:
   BingoImportRdfHandler(unsigned int func_id, const char* fname):BingoSessionHandler(func_id, true) {
      setFunctionName("importRDF");
      bingoRDFImportOpen(fname);
      SPI_connect();
   }
   virtual ~BingoImportRdfHandler() {
      SPI_finish();
      bingoRDFImportClose();
   }

private:
   BingoImportRdfHandler(const BingoImportRdfHandler&); //no implicit copy
};

Datum importrdf(PG_FUNCTION_ARGS) {
   Datum table_datum = PG_GETARG_DATUM(0);
   Datum column_datum = PG_GETARG_DATUM(1);
   Datum other_columns_datum = PG_GETARG_DATUM(2);
   Datum file_name_datum = PG_GETARG_DATUM(3);

   PG_BINGO_BEGIN
   {

      QS_DEF(Array<char>, query_str);
      QS_DEF(Array<Datum>, q_values);
      QS_DEF(Array<Oid>, q_oids);
      QS_DEF(Array<char>, q_nulls);
      ObjArray<BingoPgText> q_data;
      int spi_success = 0;

      BingoPgText fname_text(file_name_datum);
      BingoImportRdfHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());

      int column_count = _initializeColumnQuery(table_datum, column_datum, other_columns_datum, query_str);

      q_oids.clear();
      q_nulls.clear();

      for (int col_idx = 0; col_idx < column_count + 1; ++col_idx) {
         q_oids.push(TEXTOID);
         q_nulls.push(0);
      }

      bingo_handler.raise_error = false;
      while (!bingoRDFImportEOF()) {
         q_data.clear();
         const char* data = bingoRDFImportGetNext();
         q_data.push().initFromString(data);

         for (int col_idx = 0; col_idx < column_count; ++col_idx) {
            q_data.push().initFromString(bingoImportGetPropertyValue(col_idx));
         }

         q_values.clear();
         for (int q_idx = 0; q_idx < q_data.size(); ++q_idx) {
            q_values.push(q_data[q_idx].getDatum());
         }

         BINGO_PG_TRY
         {
            spi_success = SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);

            if (spi_success < 0)
               elog(WARNING, "can not insert a structure into a table");
         }
         BINGO_PG_HANDLE(throw BingoPgError("can not insert a structure into a table: %s", message));
         /*
          * Return back session id and error handler
          */
         bingo_handler.refresh();
      }
   }
   PG_BINGO_END


   PG_RETURN_VOID();
}

class BingoImportSmilesHandler : public BingoPgCommon::BingoSessionHandler {
public:
   BingoImportSmilesHandler(unsigned int func_id, const char* fname):BingoSessionHandler(func_id, true) {
      setFunctionName("importSMILES");
      bingoSMILESImportOpen(fname);
      SPI_connect();
   }
   virtual ~BingoImportSmilesHandler() {
      SPI_finish();
      bingoSMILESImportClose();
   }

private:
   BingoImportSmilesHandler(const BingoImportSmilesHandler&); //no implicit copy
};

Datum importsmiles(PG_FUNCTION_ARGS) {
   Datum table_datum = PG_GETARG_DATUM(0);
   Datum column_datum = PG_GETARG_DATUM(1);
   Datum id_column_datum = PG_GETARG_DATUM(2);
   Datum file_name_datum = PG_GETARG_DATUM(3);

   PG_BINGO_BEGIN
   {

      QS_DEF(Array<char>, query_str);
      QS_DEF(Array<Datum>, q_values);
      QS_DEF(Array<Oid>, q_oids);
      QS_DEF(Array<char>, q_nulls);
      ObjArray<BingoPgText> q_data;
      int spi_success = 0;

      BingoPgText fname_text(file_name_datum);
      BingoImportSmilesHandler bingo_handler(fcinfo->flinfo->fn_oid, fname_text.getString());

      int column_count = _initializeIdQuery(table_datum, column_datum, id_column_datum, query_str);

      q_oids.clear();
      q_nulls.clear();

      for (int col_idx = 0; col_idx < column_count + 1; ++col_idx) {
         q_oids.push(TEXTOID);
         q_nulls.push(0);
      }

      bingo_handler.raise_error = false;
      while (!bingoSMILESImportEOF()) {
         q_data.clear();
         const char* data = bingoSMILESImportGetNext();
         q_data.push().initFromString(data);

         for (int col_idx = 0; col_idx < column_count; ++col_idx) {
            const char* id_string = bingoSMILESImportGetId();
            BingoPgText& id_data = q_data.push();
            if (id_string == 0) {
               q_nulls[col_idx + 1] = 'n';
            } else {
               id_data.initFromString(id_string);
               q_nulls[col_idx + 1] = 0;
            }
         }

         q_values.clear();
         for (int q_idx = 0; q_idx < q_data.size(); ++q_idx) {
            q_values.push(q_data[q_idx].getDatum());
         }

         BINGO_PG_TRY
         {
            spi_success = SPI_execute_with_args(query_str.ptr(), q_values.size(), q_oids.ptr(), q_values.ptr(), q_nulls.ptr(), false, 1);
            if (spi_success < 0)
               elog(WARNING, "can not insert a structure into a table");
         }
         BINGO_PG_HANDLE(throw BingoPgError("can not insert a structure into a table: %s", message));
         /*
          * Return back session id and error handler
          */
         bingo_handler.refresh();
      }
   }
   PG_BINGO_END


   PG_RETURN_VOID();
}