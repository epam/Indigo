#include "bingo_pg_fix_pre.h"

extern "C" {
#include "postgres.h"
#include "fmgr.h"
}

#include "bingo_pg_fix_post.h"

#include "bingo_postgres.h"
#include "bingo_pg_text.h"
#include "bingo_core_c.h"
#include "bingo_pg_common.h"


extern "C" {
BINGO_FUNCTION_EXPORT(smiles);

BINGO_FUNCTION_EXPORT(cansmiles);

BINGO_FUNCTION_EXPORT(molfile);

BINGO_FUNCTION_EXPORT(cml);

BINGO_FUNCTION_EXPORT(checkmolecule);

BINGO_FUNCTION_EXPORT(gross);

BINGO_FUNCTION_EXPORT(getweight);

BINGO_FUNCTION_EXPORT(getmass);

BINGO_FUNCTION_EXPORT(fingerprint);

BINGO_FUNCTION_EXPORT(compactmolecule);

BINGO_FUNCTION_EXPORT(inchi);

BINGO_FUNCTION_EXPORT(inchikey);
}


Datum smiles(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   void* result = 0;
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

      BingoPgText result_text;
      result_text.initFromString(bingo_result);
      result = result_text.release();
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_TEXT_P(result);
}

Datum cansmiles(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   void* result = 0;
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

      BingoPgText result_text;
      result_text.initFromString(bingo_result);
      result = result_text.release();
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_TEXT_P(result);
}

Datum molfile(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   void* result = 0;
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

      BingoPgText result_text;
      result_text.initFromString(bingo_result);
      result = result_text.release();
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_TEXT_P(result);
}

Datum cml(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);
   void* result = 0;
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

      BingoPgText result_text;
      result_text.initFromString(bingo_result);
      result = result_text.release();
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_TEXT_P(result);
}

Datum checkmolecule(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);
   void* result = 0;
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

      BingoPgText result_text;
      result_text.initFromString(bingo_result);
      result = result_text.release();

   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_TEXT_P(result);
}

Datum gross(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);
   void* result = 0;
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

      BingoPgText result_text;
      result_text.initFromString(bingo_result);
      result = result_text.release();
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_TEXT_P(result);
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

Datum inchi(PG_FUNCTION_ARGS){
   Datum mol_datum = PG_GETARG_DATUM(0);
   Datum options_datum = PG_GETARG_DATUM(1);

   void* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("inchi");

      BingoPgText mol_text(mol_datum);
      BingoPgText mol_options(options_datum);

      int buf_size;
      const char* mol_buf = mol_text.getText(buf_size);

      int res_buf;
      const char* bingo_result = mangoInChI(mol_buf, buf_size, mol_options.getString(), &res_buf);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.inchi", bingoGetError());
         PG_RETURN_NULL();
      }

      BingoPgText result_text;
      result_text.initFromString(bingo_result);
      result = result_text.release();
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_TEXT_P(result);
}

Datum inchikey(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);
   void* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("inchikey");

      BingoPgText mol_text(mol_datum);
      const char* mol_buf = mol_text.getString();

      const char* bingo_result = mangoInChIKey(mol_buf);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.inchikey", bingoGetError());
         PG_RETURN_NULL();
      }

      BingoPgText result_text;
      result_text.initFromString(bingo_result);
      result = result_text.release();
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_TEXT_P(result);
}