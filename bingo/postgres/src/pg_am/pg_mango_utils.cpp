extern "C" {
#include "postgres.h"
#include "fmgr.h"
}
#ifdef qsort
#undef qsort
#endif
#include "bingo_postgres.h"
#include "bingo_pg_text.h"
#include "bingo_core_c.h"
#include "bingo_pg_common.h"


extern "C" {
PG_FUNCTION_INFO_V1(smiles);
PGDLLEXPORT Datum smiles(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(cansmiles);
PGDLLEXPORT Datum cansmiles(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(molfile);
PGDLLEXPORT Datum molfile(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(cml);
PGDLLEXPORT Datum cml(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(checkmolecule);
PGDLLEXPORT Datum checkmolecule(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(gross);
PGDLLEXPORT Datum gross(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(getweight);
PGDLLEXPORT Datum getweight(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(getmass);
PGDLLEXPORT Datum getmass(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(fingerprint);
PGDLLEXPORT Datum fingerprint(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(compactmolecule);
PGDLLEXPORT Datum compactmolecule(PG_FUNCTION_ARGS);
}


Datum smiles(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("smiles");

      BingoPgText mol_text(mol_datum);
      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);
      const char* bingo_result = mangoSMILES(mol_buf, buf_size, 0);
      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.smiles", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum cansmiles(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("cansmiles");

      BingoPgText mol_text(mol_datum);
      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);
      const char* bingo_result = mangoSMILES(mol_buf, buf_size, 1);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.cansmiles", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum molfile(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("molfile");

      BingoPgText mol_text(mol_datum);
      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);
      const char* bingo_result = mangoMolfile(mol_buf, buf_size);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.molfile", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum cml(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);
   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("cml");

      BingoPgText mol_text(mol_datum);
      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);
      const char* bingo_result = mangoCML(mol_buf, buf_size);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.cml", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum checkmolecule(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);
   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("checkmolecule");

      BingoPgText mol_text(mol_datum);
      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);

      const char* bingo_result = mangoCheckMolecule(mol_buf, buf_size);

      if(bingo_result == 0)
         PG_RETURN_NULL();

      result = BingoPgCommon::releaseString(bingo_result);

   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum gross(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);
   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("gross");

      BingoPgText mol_text(mol_datum);
      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);

      const char* bingo_result = mangoGross(mol_buf, buf_size);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.gross", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum getweight(PG_FUNCTION_ARGS){
   Datum mol_datum = PG_GETARG_DATUM(0);
   Datum options_datum = PG_GETARG_DATUM(1);
   float result = 0;

   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("getweight");

      BingoPgText mol_text(mol_datum);
      BingoPgText mol_options(options_datum);

      int buf_len, bingo_res;
      const char* buf = mol_text.getText(buf_len);

      bingo_res = mangoMass(buf, buf_len, mol_options.getString(), &result);

      if(bingo_res < 1) {
         CORE_HANDLE_WARNING(0, 1, "bingo.getweight", bingoGetError());
         PG_RETURN_NULL();
      }

   }
   PG_BINGO_END

   PG_RETURN_FLOAT4(result);
}

Datum getmass(PG_FUNCTION_ARGS){
   Datum mol_datum = PG_GETARG_DATUM(0);
   
   float result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("getmass");

      BingoPgText mol_text(mol_datum);
      int buf_len, bingo_res;
      const char* buf = mol_text.getText(buf_len);

      bingo_res = mangoMass(buf, buf_len, 0, &result);
      if(bingo_res < 1) {
         CORE_HANDLE_WARNING(0, 1, "bingo.getmass", bingoGetError());
         PG_RETURN_NULL();
      }

   }
   PG_BINGO_END

   PG_RETURN_FLOAT4(result);
}

Datum fingerprint(PG_FUNCTION_ARGS){
   Datum mol_datum = PG_GETARG_DATUM(0);
   Datum options_datum = PG_GETARG_DATUM(1);

   void* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("fingerprint");

      BingoPgText mol_text(mol_datum);
      BingoPgText mol_options(options_datum);
      
      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);

      int res_buf;
      const char* bingo_result = mangoFingerprint(mol_buf, buf_size, mol_options.getString(), &res_buf);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.fingerprint", bingoGetError());
         PG_RETURN_NULL();
      }

      BingoPgText result_data;
      result_data.initFromBuffer(bingo_result, res_buf);

      result = result_data.release();
   }
   PG_BINGO_END

   if(result == 0)
      PG_RETURN_NULL();

   PG_RETURN_BYTEA_P(result);
}

Datum compactmolecule(PG_FUNCTION_ARGS){
   Datum mol_datum = PG_GETARG_DATUM(0);
   bool options_xyz = PG_GETARG_BOOL(1);

   void* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("compactmolecule");

      BingoPgText mol_text(mol_datum);

      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);

      int res_buf;
      const char* bingo_result = mangoICM(mol_buf, buf_size, options_xyz, &res_buf);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.compactmolecule", bingoGetError());
         PG_RETURN_NULL();
      }

      BingoPgText result_data;
      result_data.initFromBuffer(bingo_result, res_buf);

      result = result_data.release();
   }
   PG_BINGO_END

   if(result == 0)
      PG_RETURN_NULL();

   PG_RETURN_BYTEA_P(result);
}
