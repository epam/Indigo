
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
}

CEXPORT {
PG_FUNCTION_INFO_V1(bingo_sub_internal);
Datum bingo_sub_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_smarts_internal);
Datum bingo_smarts_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_exact_internal);
Datum bingo_exact_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingosim);
Datum bingosim(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_gross_internal);
Datum bingo_gross_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_sim_internal);
Datum bingo_sim_internal(PG_FUNCTION_ARGS);

}

using namespace indigo;

/*
 * Helper class for searching setup and perfoming
 */
class _MangoContextHandler {
public:
   _MangoContextHandler(int type) :_type(type) {
      _bingoSession = bingoAllocateSessionID();
      bingoSetSessionID(_bingoSession);
      bingoSetContext(0);

      BingoPgCommon::getSearchTypeString(_type, _typeStr);

      bingoSetErrorHandler(bingoErrorHandler, _typeStr.ptr());

      BingoPgConfig bingo_config;
      bingo_config.readDefaultConfig();
      bingo_config.setUpBingoConfiguration();
      bingoTautomerRulesReady(0,0,0);
   }

   ~_MangoContextHandler() {
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
      mangoSetupMatch(_typeStr.ptr(), query_text.getString(), options_text.getString());

      int target_size;
      const char* target_data = target_text.getText(target_size);
      
      if(_type == BingoPgCommon::MOL_GROSS)
         target_data = mangoGross(target_data, target_size);
      
      int res = mangoMatchTarget(target_data, target_size);

      if (res < 0)
         elog(ERROR, "Error while bingo%s loading molecule: %s", _typeStr.ptr(), bingoGetWarning());

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
   _MangoContextHandler(const _MangoContextHandler&);//no implicit copy
   qword _bingoSession;
   int _type;
   indigo::Array<char> _typeStr;
};


Datum bingo_sub_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_SUB);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}

Datum bingo_smarts_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_SMARTS);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}

Datum bingo_exact_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_EXACT);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}

Datum bingosim(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_SIM);

   float res;

   if(bingo_context.matchInternal(query_datum, target_datum, options_datum))
      mangoSimilarityGetScore(&res);
   else
      res = 0;

   PG_RETURN_FLOAT4(res);
}

Datum bingo_gross_internal(PG_FUNCTION_ARGS) {
   Datum query_sign = PG_GETARG_DATUM(0);
   Datum query_datum = PG_GETARG_DATUM(1);
   Datum target_datum = PG_GETARG_DATUM(2);

   BingoPgText query_text(query_datum);
   BingoPgText sign_text(query_sign);
   indigo::Array<char> bingo_query;
   bingo_query.readString(sign_text.getString(), false);
   bingo_query.appendString(" ", false);
   bingo_query.appendString(query_text.getString(), false);

   query_text.initFromArray(bingo_query);
   
   _MangoContextHandler bingo_context(BingoPgCommon::MOL_GROSS);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_text.getDatum(), target_datum, 0));
}

Datum bingo_sim_internal(PG_FUNCTION_ARGS){
   float min_bound = PG_GETARG_FLOAT4(0);
   float max_bound = PG_GETARG_FLOAT4(1);
   Datum query_datum = PG_GETARG_DATUM(2);
   Datum target_datum = PG_GETARG_DATUM(3);
   Datum options_datum = PG_GETARG_DATUM(4);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_SIM);

   float mol_sim;

   if(bingo_context.matchInternal(query_datum, target_datum, options_datum))
      mangoSimilarityGetScore(&mol_sim);
   else
      mol_sim = 0;

   bool result = (mol_sim <= max_bound) && (mol_sim >= min_bound);

   PG_RETURN_BOOL(result);
}

