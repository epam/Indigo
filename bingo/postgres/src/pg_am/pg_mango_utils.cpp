#include "bingo_postgres.h"
#include "bingo_pg_text.h"
#include "bingo_core_c.h"


#include "bingo_pg_config.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "catalog/namespace.h"
#include "utils/lsyscache.h"
}

CEXPORT {
PG_FUNCTION_INFO_V1(smiles);
Datum smiles(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(cansmiles);
Datum cansmiles(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(molfile);
Datum molfile(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(cml);
Datum cml(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(checkmolecule);
Datum checkmolecule(PG_FUNCTION_ARGS);

}

static void bingoErrorHandler(const char *message, void *context) {
   const char* call_func = (const char*)context;
   elog(ERROR, "error while processing %s: %s", call_func, message);
}

static indigo::Array<char> func_name;

static void _setupBingo(Oid func_id, const char* func_n) {
   qword session_id = bingoAllocateSessionID();
   bingoSetSessionID(session_id);
   bingoSetContext(0);

   const char* schema_name = get_namespace_name(get_func_namespace(func_id));

   BingoPgConfig bingo_config;

   bingo_config.readDefaultConfig(schema_name);
   bingo_config.setUpBingoConfiguration();

   func_name.readString(func_n, true);

   bingoSetErrorHandler(bingoErrorHandler, func_name.ptr());
}

Datum smiles(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   _setupBingo(fcinfo->flinfo->fn_oid, "Molecule SMILES");

   BingoPgText mol_text(mol_datum);
   int buf_size;
   const char* mol_buf = mol_text.getText(buf_size);
   PG_RETURN_CSTRING(mangoSMILES(mol_buf, buf_size, 0));
}

Datum cansmiles(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   _setupBingo(fcinfo->flinfo->fn_oid, "Molecule canonical SMILES");

   BingoPgText mol_text(mol_datum);
   int buf_size;
   const char* mol_buf = mol_text.getText(buf_size);
   PG_RETURN_CSTRING(mangoSMILES(mol_buf, buf_size, 1));
}

Datum molfile(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   _setupBingo(fcinfo->flinfo->fn_oid, "Molecule molfile");

   BingoPgText mol_text(mol_datum);
   int buf_size;
   const char* mol_buf = mol_text.getText(buf_size);
   PG_RETURN_CSTRING(mangoMolfile(mol_buf, buf_size));
}

Datum cml(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   _setupBingo(fcinfo->flinfo->fn_oid, "Molecule CML format");

   BingoPgText mol_text(mol_datum);
   int buf_size;
   const char* mol_buf = mol_text.getText(buf_size);
   PG_RETURN_CSTRING(mangoCML(mol_buf, buf_size));
}

Datum checkmolecule(PG_FUNCTION_ARGS) {
   Datum mol_datum = PG_GETARG_DATUM(0);

   _setupBingo(fcinfo->flinfo->fn_oid, "Molecule check");

   BingoPgText mol_text(mol_datum);
   int buf_size;
   const char* mol_buf = mol_text.getText(buf_size);
   const char* result = mangoCheckMolecule(mol_buf, buf_size);
   if(result == 0)
      PG_RETURN_NULL();
   PG_RETURN_CSTRING(result);
}


