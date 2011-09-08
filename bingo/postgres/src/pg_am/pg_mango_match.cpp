
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
PG_FUNCTION_INFO_V1(_sub_internal);
Datum _sub_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_smarts_internal);
Datum _smarts_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_exact_internal);
Datum _exact_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(getsimilarity);
Datum getsimilarity(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_gross_internal);
Datum _gross_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_sim_internal);
Datum _sim_internal(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_match_mass_less);
Datum _match_mass_less(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_match_mass_great);
Datum _match_mass_great(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_mass_in);
Datum _mass_in(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(_mass_out);
Datum _mass_out(PG_FUNCTION_ARGS);

}

using namespace indigo;

/*
 * Helper class for searching setup and perfoming
 */
class _MangoContextHandler {
public:
   _MangoContextHandler(int type, unsigned int func_oid) :_type(type), _sessionHandler(func_oid, true){
      BingoPgCommon::getSearchTypeString(_type, _typeStr, true);
      _sessionHandler.setFunctionName(_typeStr.ptr());
   }

   ~_MangoContextHandler() {
   }


   /*
    * Match method
    * Returns true if matching is successfull
    * Throws an error if query can not be loaded
    */
   bool matchInternal(Datum query_datum, Datum target_datum, Datum options_datum) {
      BingoPgText query_text(query_datum);
      BingoPgText target_text(target_datum);
      BingoPgText options_text(options_datum);

      /*
       * Set up match parameters
       */
      int res = mangoSetupMatch(_typeStr.ptr(), query_text.getString(), options_text.getString());
      if (res < 0)
         elog(ERROR, "Error while bingo%s loading molecule: %s", _typeStr.ptr(), bingoGetError());

      int target_size;
      const char* target_data = target_text.getText(target_size);
      
      if(_type == BingoPgCommon::MOL_GROSS)
         target_data = mangoGross(target_data, target_size);
      
      res = mangoMatchTarget(target_data, target_size);

      if (res < 0)
         elog(WARNING, "Warning while bingo%s loading molecule: %s", _typeStr.ptr(), bingoGetWarning());

      return res > 0;
   }
private:
   _MangoContextHandler(const _MangoContextHandler&);//no implicit copy
   qword _bingoSession;
   int _type;
   indigo::Array<char> _typeStr;
   BingoPgCommon::BingoSessionHandler _sessionHandler;
};


Datum _sub_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_SUB, fcinfo->flinfo->fn_oid);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}

Datum _smarts_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_SMARTS, fcinfo->flinfo->fn_oid);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}

Datum _exact_internal(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_EXACT, fcinfo->flinfo->fn_oid);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_datum, target_datum, options_datum));
}

Datum getsimilarity(PG_FUNCTION_ARGS) {
   Datum query_datum = PG_GETARG_DATUM(0);
   Datum target_datum = PG_GETARG_DATUM(1);
   Datum options_datum = PG_GETARG_DATUM(2);

   _MangoContextHandler bingo_context(BingoPgCommon::MOL_SIM, fcinfo->flinfo->fn_oid);

   float res;

   if(bingo_context.matchInternal(query_datum, target_datum, options_datum))
      mangoSimilarityGetScore(&res);
   else
      res = 0;

   PG_RETURN_FLOAT4(res);
}

Datum _gross_internal(PG_FUNCTION_ARGS) {
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
   
   _MangoContextHandler bingo_context(BingoPgCommon::MOL_GROSS, fcinfo->flinfo->fn_oid);

   PG_RETURN_BOOL(bingo_context.matchInternal(query_text.getDatum(), target_datum, 0));
}

Datum _sim_internal(PG_FUNCTION_ARGS){
   float min_bound = PG_GETARG_FLOAT4(0);
   float max_bound = PG_GETARG_FLOAT4(1);
   Datum query_datum = PG_GETARG_DATUM(2);
   Datum target_datum = PG_GETARG_DATUM(3);
   Datum options_datum = PG_GETARG_DATUM(4);
   _MangoContextHandler bingo_context(BingoPgCommon::MOL_SIM, fcinfo->flinfo->fn_oid);

   float mol_sim;

   if(bingo_context.matchInternal(query_datum, target_datum, options_datum))
      mangoSimilarityGetScore(&mol_sim);
   else
      mol_sim = 0;

   bool result = (mol_sim <= max_bound) && (mol_sim >= min_bound);

   PG_RETURN_BOOL(result);
}


Datum _match_mass_less(PG_FUNCTION_ARGS){
   Datum mol_datum = PG_GETARG_DATUM(0);
   char* mass_datum = PG_GETARG_CSTRING(1);

   BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid, true);
   bingo_handler.setFunctionName("mass less");

   BufferScanner scanner(mass_datum);
   float usr_mass = scanner.readFloat();

   BingoPgText mol_text(mol_datum);

   float mol_mass = 0;

   int buf_len;
   const char* buf = mol_text.getText(buf_len);

   mangoMass(buf, buf_len, 0, &mol_mass);

   bool result = mol_mass < usr_mass;

   PG_RETURN_BOOL(result);
}
Datum _match_mass_great(PG_FUNCTION_ARGS){
   Datum mol_datum = PG_GETARG_DATUM(0);
   char* mass_datum = PG_GETARG_CSTRING(1);

   BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid, true);
   bingo_handler.setFunctionName("mass great");

   BufferScanner scanner(mass_datum);
   float usr_mass = scanner.readFloat();

   BingoPgText mol_text(mol_datum);

   float mol_mass = 0;

   int buf_len;
   const char* buf = mol_text.getText(buf_len);

   mangoMass(buf, buf_len, 0, &mol_mass);

   bool result = mol_mass > usr_mass;

   PG_RETURN_BOOL(result);
}

Datum _mass_in(PG_FUNCTION_ARGS) {
   char *str = PG_GETARG_CSTRING(0);
   int size = strlen(str) + 1;
   char* result = (char*)palloc(size);
   memcpy(result, str, size);
   PG_RETURN_POINTER(result);
}

Datum _mass_out(PG_FUNCTION_ARGS) {
   char *str = PG_GETARG_CSTRING(0);
   int size = strlen(str) + 1;
   char* result = (char*)palloc(size);
   memcpy(result, str, size);
   PG_RETURN_CSTRING(result);
}