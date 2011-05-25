#include "ringo_pg_search_engine.h"

#include "bingo_core_c.h"
#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

#include "bingo_pg_text.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "storage/bufmgr.h"
#include "access/itup.h"
#include "access/htup.h"
#include "access/genam.h"
#include "access/relscan.h"
#include "access/tupdesc.h"
#include "utils/typcache.h"
}

using namespace indigo;

RingoPgSearchEngine::RingoPgSearchEngine(BingoPgConfig& bingo_config, const char* rel_name):
BingoPgSearchEngine(),
_searchType(-1) {
   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);
   /*
    * Set up bingo configuration
    */
   bingo_config.setUpBingoConfiguration();
   bingoTautomerRulesReady(0,0,0);
   bingoIndexBegin();

//   _relName.readString(rel_name, true);
//   _shadowRelName.readString(rel_name, true);
//   _shadowRelName.appendString("_shadow", true);
//   _shadowHashRelName.readString(rel_name, true);
//   _shadowHashRelName.appendString("_shadow_hash", true);
}

RingoPgSearchEngine::~RingoPgSearchEngine() {
   bingoIndexEnd();
}

bool RingoPgSearchEngine::matchTarget(int section_idx, int structure_idx) {
   bool result = false;
   QS_DEF(Array<char>, react_buf);
   react_buf.clear();

   _bufferIndexPtr->readCmfItem(section_idx, structure_idx, react_buf);
   result = ringoMatchTargetBinary(react_buf.ptr(), react_buf.sizeInBytes()) == 1;
   
   return result;
}

void RingoPgSearchEngine::prepareQuerySearch(BingoPgIndex& bingo_idx, PG_OBJECT scan_desc_ptr) {

   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;

   if(scan_desc->numberOfKeys >= 1 && scan_desc->numberOfKeys <= 2) {
      _searchType = scan_desc->keyData[0].sk_strategy;
   } else {
      elog(ERROR, "unsupported number of scan keys %d", scan_desc->numberOfKeys);
   }
   _queryFpData.reset(new RingoPgFpData());

   _setBingoContext();
   bingoSetErrorHandler(_errorHandler, 0);

   BingoPgSearchEngine::prepareQuerySearch(bingo_idx, scan_desc);

   loadDictionary(bingo_idx);

   switch(_searchType) {
      case BingoPgCommon::REACT_SUB:
         _prepareSubSearch(scan_desc);
         break;
      case BingoPgCommon::REACT_EXACT:
         _prepareExactSearch(scan_desc);
         break;
      case BingoPgCommon::REACT_SMARTS:
         _prepareSmartsSearch(scan_desc);
         break;
      default:
         elog(ERROR, "unsupported search type");
         break;
   }

}

bool RingoPgSearchEngine::searchNext(PG_OBJECT result_ptr) {

   bool result = false;
   if (_searchType == BingoPgCommon::REACT_EXACT) {
      result = _searchNextCursor(result_ptr);
   } else if(_searchType == BingoPgCommon::REACT_SUB || _searchType == BingoPgCommon::REACT_SMARTS) {
      result = _searchNextSub(result_ptr);
   }

   return result;
}

void RingoPgSearchEngine::_errorHandler(const char* message, void*) {
   elog(ERROR, "Error while searching a molecule: %s", message);
}

void RingoPgSearchEngine::_prepareExactQueryStrings(indigo::Array<char>& what_clause_str, indigo::Array<char>& from_clause_str, indigo::Array<char>& where_clause_str) {
//   int hash_elements_count, count;
//   dword hash;
//
//   ArrayOutput what_clause(what_clause_str);
//   ArrayOutput from_clause(from_clause_str);
//   ArrayOutput where_clause(where_clause_str);
//
//   what_clause.printf("sh.b_id");
//
//   ringoGetHash(false, -1, &hash_elements_count, &hash);
//
//   from_clause.printf("%s sh", _shadowRelName.ptr());
//
//   for (int i = 0; i < hash_elements_count; i++)
//      from_clause.printf(", %s t%d", _shadowHashRelName.ptr(), i);
//
//   /*
//    * Create complex WHERE clause
//    */
//
//   bool where_was_added = false;
//   if (hash_elements_count > 0) {
//      where_was_added = true;
//      /*
//       * molecule ids must be same
//       */
//      where_clause.printf("sh.b_id = t0.b_id AND ");
//      for (int i = 1; i < hash_elements_count; i++)
//         where_clause.printf("t%d.b_id = t%d.b_id AND ", i - 1, i);
//      /*
//       * query components must match target components
//       */
//      for (int i = 0; i < hash_elements_count; i++) {
//         ringoGetHash(false, i, &count, &hash);
//         where_clause.printf("t%d.ex_hash = %d AND ", i, hash);
//      }
//
//      /*
//       * components count mast must target components count
//       */
//      Array<char> rel;
//      if (ringoExactNeedComponentMatching())
//         rel.readString(">=", true);
//      else
//         rel.readString("=", true);
//
//      for (int i = 0; i < hash_elements_count; i++) {
//         if (i != 0)
//            where_clause.printf("AND ");
//         ringoGetHash(false, i, &count, &hash);
//         where_clause.printf("t%d.f_count %s %d ", i, rel.ptr(), count);
//      }
//   }
//   if (!ringoExactNeedComponentMatching()) {
//      if (where_was_added)
//         where_clause.printf("AND ");
//
//      /*
//       * There must be no other components in target
//       */
//      int query_fragments_count = 0;
//      for (int i = 0; i < hash_elements_count; i++) {
//         ringoGetHash(false, i, &count, &hash);
//         query_fragments_count += count;
//      }
//      where_clause.printf("sh.fragments = %d", query_fragments_count);
//   }
//   what_clause_str.push(0);
//   from_clause_str.push(0);
//   where_clause_str.push(0);

}
void RingoPgSearchEngine::_prepareExactTauStrings(indigo::Array<char>& what_clause_str, indigo::Array<char>& from_clause_str, indigo::Array<char>& where_clause_str) {
//   ArrayOutput what_clause(what_clause_str);
//   ArrayOutput from_clause(from_clause_str);
//   ArrayOutput where_clause(where_clause_str);
//
//   what_clause.printf("b_id");
//   from_clause.printf("%s", _shadowRelName.ptr());
//
//   const char* query_gross = ringoTauGetQueryGross();
//   where_clause.printf("gross='%s' OR gross LIKE '%s H%%'", query_gross, query_gross);
//
//   what_clause_str.push(0);
//   from_clause_str.push(0);
//   where_clause_str.push(0);

}

void RingoPgSearchEngine::_prepareSubSearch(PG_OBJECT scan_desc_ptr) {
   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;
   QS_DEF(Array<char>, search_type);
   BingoPgText search_query;
   BingoPgText search_options;
   BingoPgFpData& data = _queryFpData.ref();

   BingoPgCommon::getSearchTypeString(_searchType, search_type, false);

   _getScanQueries(scan_desc->keyData[0].sk_argument, search_query, search_options);

   /*
    * Set up matching parameters
    */
   if(ringoSetupMatch(search_type.ptr(), search_query.getString(), search_options.getString()) < 0)
      elog(ERROR, "Can not set search context: %s", bingoGetError());


   const char* fingerprint_buf;
   int fp_len;

   ringoGetQueryFingerprint(&fingerprint_buf, &fp_len);

   int size_bits = fp_len * 8;
   data.setFingerPrints(fingerprint_buf, size_bits);

}

void RingoPgSearchEngine::_prepareExactSearch(PG_OBJECT scan_desc_ptr) {
//   IndexScanDesc scan_desc = (IndexScanDesc) scan_desc_ptr;
//   QS_DEF(Array<char>, what_clause);
//   QS_DEF(Array<char>, from_clause);
//   QS_DEF(Array<char>, where_clause);
//   QS_DEF(Array<char>, search_type);
//   BingoPgText search_query;
//   BingoPgText search_options;
//
//   BingoPgCommon::getSearchTypeString(_searchType, search_type);
//
//   _getScanQueries(scan_desc->keyData[0].sk_argument, search_query, search_options);
//
//   /*
//    * Set up matching parameters
//    */
//   if(ringoSetupMatch(search_type.ptr(), search_query.getString(), search_options.getString()) < 0)
//      elog(ERROR, "Can not set search context: %s", bingoGetError());
//
//   if (strcasestr(search_options.getString(), "TAU") != 0) {
//      _prepareExactTauStrings(what_clause, from_clause, where_clause);
//   } else {
//      _prepareExactQueryStrings(what_clause, from_clause, where_clause);
//   }
//
//   _searchCursor.reset(new BingoPgCursor("SELECT %s FROM %s WHERE %s",what_clause.ptr(), from_clause.ptr(), where_clause.ptr()));
}

void RingoPgSearchEngine::_prepareSmartsSearch(PG_OBJECT scan_desc_ptr) {
   _prepareSubSearch(scan_desc_ptr);
}

void RingoPgSearchEngine::_getScanQueries(dword arg_datum, BingoPgText& str1, BingoPgText& str2) {
   /*
    * Get query info
    */
   HeapTupleHeader query_data = DatumGetHeapTupleHeader(arg_datum);
   Oid tupType = HeapTupleHeaderGetTypeId(query_data);
   int32 tupTypmod = HeapTupleHeaderGetTypMod(query_data);
   TupleDesc tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
   int ncolumns = tupdesc->natts;

   if(ncolumns != 2)
      elog(ERROR, "internal error: expecting two columns in query but was %d", ncolumns);

   HeapTupleData tuple;
   /*
    * Build a temporary HeapTuple control structure
    */
   tuple.t_len = HeapTupleHeaderGetDatumLength(query_data);
   ItemPointerSetInvalid(&(tuple.t_self));
   tuple.t_tableOid = InvalidOid;
   tuple.t_data = query_data;

   Datum *values = (Datum *) palloc(ncolumns * sizeof (Datum));
   bool *nulls = (bool *) palloc(ncolumns * sizeof (bool));

   /*
    *  Break down the tuple into fields
    */
   heap_deform_tuple(&tuple, tupdesc, values, nulls);

   /*
    * Query tuple consist of query and options
    */
   str1.init(values[0]);
   str2.init(values[1]);

   pfree(values);
   pfree(nulls);
   ReleaseTupleDesc(tupdesc);
}

//void RingoPgSearchEngine::_getScanQueries(dword arg_datum, float& min_bound, float& max_bound, BingoPgText& str1, BingoPgText& str2) {
//   /*
//    * Get query info
//    */
//   HeapTupleHeader query_data = DatumGetHeapTupleHeader(arg_datum);
//   Oid tupType = HeapTupleHeaderGetTypeId(query_data);
//   int32 tupTypmod = HeapTupleHeaderGetTypMod(query_data);
//   TupleDesc tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
//   int ncolumns = tupdesc->natts;
//
//   if(ncolumns != 4)
//      elog(ERROR, "internal error: expecting four columns in query but was %d", ncolumns);
//
//   HeapTupleData tuple;
//   /*
//    * Build a temporary HeapTuple control structure
//    */
//   tuple.t_len = HeapTupleHeaderGetDatumLength(query_data);
//   ItemPointerSetInvalid(&(tuple.t_self));
//   tuple.t_tableOid = InvalidOid;
//   tuple.t_data = query_data;
//
//   Datum *values = (Datum *) palloc(ncolumns * sizeof (Datum));
//   bool *nulls = (bool *) palloc(ncolumns * sizeof (bool));
//
//   /*
//    *  Break down the tuple into fields
//    */
//   heap_deform_tuple(&tuple, tupdesc, values, nulls);
//
//   /*
//    * Query tuple consist of query and options
//    */
//   min_bound = DatumGetFloat4(values[0]);
//   max_bound = DatumGetFloat4(values[1]);
//   str1.init(values[2]);
//   str2.init(values[3]);
//
//   pfree(values);
//   pfree(nulls);
//   ReleaseTupleDesc(tupdesc);
//}

