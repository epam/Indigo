

#include "bingo_postgres.h"
#include "bingo_pg_common.h"


#include "pg_bingo_context.h"
#include "bingo_core_c.h"
#include "bingo_pg_config.h"
#include "bingo_pg_text.h"


CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "utils/relcache.h"
#include "storage/lock.h"
#include "access/heapam.h"
#include "storage/bufmgr.h"
#include "catalog/namespace.h"
#include "utils/lsyscache.h"
}

CEXPORT {
PG_FUNCTION_INFO_V1(_rsub_internal);
Datum _rsub_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_rsmarts_internal);
Datum _rsmarts_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_rexact_internal);
Datum _rexact_internal(PG_FUNCTION_ARGS);

}

using namespace indigo;

/*
 * Helper class for searching setup and perfoming
 */
class _RingoContextHandler {
public:
   _RingoContextHandler(int type, unsigned int func_oid) :_type(type) {
      _bingoSession = bingoAllocateSessionID();
      bingoSetSessionID(_bingoSession);
      bingoSetContext(0);

      BingoPgCommon::getSearchTypeString(_type, _typeStr);

      bingoSetErrorHandler(bingoErrorHandler, _typeStr.ptr());

      const char* schema_name = get_namespace_name(get_func_namespace(func_oid));

      BingoPgConfig bingo_config;
      bingo_config.readDefaultConfig(schema_name);
      bingo_config.setUpBingoConfiguration();
      bingoTautomerRulesReady(0,0,0);
   }

   ~_RingoContextHandler() {
      bingoReleaseSessionID(_bingoSession);
   }


   /*
    * Match method
    * Returns true if matching is successfull
    * Throws an error if query or target can not be loaded
    */
   bool matchInternal(Datum query_datum, Datum target_datum, Datum options_datum) {
      BingoPgText query_text(query_datum);
      BingoPgText target_text(target_datum);
      BingoPgText options_text(options_datum);

      /*
       * Set up match parameters
       */
      int res = ringoSetupMatch(_typeStr.ptr(), query_text.getString(), options_text.getString());
      if (res < 0)
         elog(WARNING, "Warning while bingo%s loading molecule: %s", _typeStr.ptr(), bingoGetWarning());

      int target_size;
      const char* target_data = target_text.getText(target_size);

      res = ringoMatchTarget(target_data, target_size);

      if (res < 0)
         elog(WARNING, "Warning while bingo%s loading molecule: %s", _typeStr.ptr(), bingoGetWarning());

      return res > 0;
   }
   static void bingoErrorHandler(const char* message, void* func_str) {
      char* func = (char*) func_str;
      if (func)
         elog(ERROR, "Error in bingo%s: %s", func, message);
      else
         elog(ERROR, "Error %s", message);
   }
private:
   _RingoContextHandler(const _RingoContextHandler&);//no implicit copy
   qword _bingoSession;
   int _type;
   indigo::Array<char> _typeStr;
};


Datum _rsub_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _RingoContextHandler bingo_context(BingoPgCommon::REACT_SUB, fcinfo->flinfo->fn_oid);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}

Datum _rsmarts_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _RingoContextHandler bingo_context(BingoPgCommon::REACT_SMARTS, fcinfo->flinfo->fn_oid);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}

Datum _rexact_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _RingoContextHandler bingo_context(BingoPgCommon::REACT_EXACT, fcinfo->flinfo->fn_oid);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}
