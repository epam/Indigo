#include "bingo_postgres.h"
#include "bingo_pg_text.h"
#include "bingo_core_c.h"
#include "bingo_pg_common.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
}

CEXPORT {
PG_FUNCTION_INFO_V1(aam);
Datum aam(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(rxnfile);
Datum rxnfile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(rcml);
Datum rcml(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(checkreaction);
Datum checkreaction(PG_FUNCTION_ARGS);
}

Datum aam(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);
   Datum mode_datum = PG_GETARG_DATUM(1);

   BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid, false);
   bingo_handler.setFunctionName("aam");

   BingoPgText react_text(react_datum);
   BingoPgText aam_mode(mode_datum);
   
   int buf_size;
   const char* react_buf = react_text.getText(buf_size);

   char* result = BingoPgCommon::releaseString(ringoAAM(react_buf, buf_size, aam_mode.getString()));

   if(result == 0 || bingo_handler.error_raised)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum rxnfile(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);

   BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid, false);
   bingo_handler.setFunctionName("rxnfile");

   BingoPgText react_text(react_datum);
   int buf_size;
   const char* react_buf = react_text.getText(buf_size);
   char* result = BingoPgCommon::releaseString(ringoRxnfile(react_buf, buf_size));

   if(result == 0 || bingo_handler.error_raised)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum rcml(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);

   BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid, false);
   bingo_handler.setFunctionName("rcml");

   BingoPgText react_text(react_datum);
   int buf_size;
   const char* react_buf = react_text.getText(buf_size);
   char* result = BingoPgCommon::releaseString(ringoRCML(react_buf, buf_size));

   if(result == 0 || bingo_handler.error_raised)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum checkreaction(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);

   BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid, false);
   bingo_handler.setFunctionName("checkreaction");

   BingoPgText react_text(react_datum);
   int buf_size;
   const char* react_buf = react_text.getText(buf_size);

   char* result = BingoPgCommon::releaseString(ringoCheckReaction(react_buf, buf_size));

   if(result == 0 || bingo_handler.error_raised)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

