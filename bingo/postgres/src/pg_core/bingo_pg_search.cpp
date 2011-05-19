#include "bingo_pg_search.h"

#include "bingo_core_c.h"
#include "base_cpp/tlscont.h"

#include "bingo_pg_common.h"
#include "bingo_pg_buffer.h"
#include "bingo_pg_text.h"
#include "bingo_pg_config.h"
#include "bingo_pg_ext_bitset.h"
#include "mango_pg_search_engine.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "utils/relcache.h"
#include "access/relscan.h"
#include "access/genam.h"
#include "utils/rel.h"
#include "utils/lsyscache.h"
}


using namespace indigo;

BingoPgSearch::BingoPgSearch(PG_OBJECT rel):
_initSearch(true),
_indexScanDesc(0),
_bufferIndex(rel) {
   _bufferIndex.setStrategy(BingoPgIndex::READING_STRATEGY);
}

BingoPgSearch::~BingoPgSearch() {
}
/*
 * Searches for the next match. Return true if search was successfull
 * Sets up item pointer
 */
bool BingoPgSearch::next(PG_OBJECT scan_desc_ptr, PG_OBJECT result_ptr) {
   /*
    * All the state keeps in the context
    */
   _indexScanDesc = scan_desc_ptr;

   /*
    * Init searching
    */
   if(_initSearch) {
      _initScanSearch();
   }

   return _fpEngine->searchNext(result_ptr);

}

void BingoPgSearch::_initScanSearch() {
   _initSearch = false;

   Relation index = ((IndexScanDesc)_indexScanDesc)->indexRelation;

   char* rel_name = get_rel_name(index->rd_id);

   /*
    * Read configuration from index tuple
    */
   BingoPgConfig bingo_config;
   _bufferIndex.readConfigParameters(bingo_config);

   /*
    * Set up search engine
    */
   int index_type = _bufferIndex.getIndexType();

   if (index_type == BINGO_INDEX_TYPE_MOLECULE)
      _fpEngine.reset(new MangoPgSearchEngine(bingo_config, rel_name));
   else if (index_type == BINGO_INDEX_TYPE_REACTION)
      elog(ERROR, "reaction search is not implemented yet");
   else
      elog(ERROR, "unknown index type %d", index_type);

   /*
    * Define search bingo options
    */
//   _defineQueryOptions();
   /*
    * Process query structure with parameters
    */
   _fpEngine->prepareQuerySearch(_bufferIndex, _indexScanDesc);

}

//void BingoPgSearchEngine::_defineQueryOptions() {
//   IndexScanDesc index_scan_desc = (IndexScanDesc) _indexScanDesc;
//
//   char* func_name = get_func_name(index_scan_desc->keyData->sk_func.fn_oid);
//   _funcName.readString(func_name, true);
//
//   /*
//    * Get query info
//    */
//
////   ScanKeyData key_data1 = index_scan_desc->keyData[0];
////   ScanKeyData key_data2 = index_scan_desc->keyData[1];
//
//   HeapTupleHeader query_data = DatumGetHeapTupleHeader(index_scan_desc->keyData->sk_argument);
//   Oid tupType = HeapTupleHeaderGetTypeId(query_data);
//   int32 tupTypmod = HeapTupleHeaderGetTypMod(query_data);
//   TupleDesc tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
//   int ncolumns = tupdesc->natts;
//
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
//   _queryText.init(values[0]);
//   _optionsText.init(values[1]);
//
//   pfree(values);
//   pfree(nulls);
//   ReleaseTupleDesc(tupdesc);
//}


