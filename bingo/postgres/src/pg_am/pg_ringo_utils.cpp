#include "bingo_postgres.h"
#include "bingo_pg_text.h"
#include "bingo_core_c.h"
#include "bingo_pg_index.h"
#include "bingo_pg_config.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "catalog/namespace.h"
#include "utils/lsyscache.h"
}

CEXPORT {
PG_FUNCTION_INFO_V1(aam);
Datum aam(PG_FUNCTION_ARGS);
}

static void bingoErrorHandler(const char *message, void *context) {
   const char* call_func = (const char*)context;
   elog(ERROR, "error while processing %s: %s", call_func, message);
}

Datum aam(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);
   Datum mode_datum = PG_GETARG_DATUM(1);

   qword session_id = bingoAllocateSessionID();
   bingoSetSessionID(session_id);
   bingoSetContext(0);

   const char* schema_name = get_namespace_name(get_func_namespace(fcinfo->flinfo->fn_oid));
   
   BingoPgConfig bingo_config;

   bingo_config.readDefaultConfig(schema_name);
   bingo_config.setUpBingoConfiguration();
   indigo::Array<char> func_name;
   func_name.readString("Reaction AAM", true);

   bingoSetErrorHandler(bingoErrorHandler, func_name.ptr());

   BingoPgText react_text(react_datum);
   BingoPgText aam_mode(mode_datum);
   int buf_size;
   const char* react_buf = react_text.getText(buf_size);
   PG_RETURN_CSTRING(ringoAAM(react_buf, buf_size, aam_mode.getString()));
}

