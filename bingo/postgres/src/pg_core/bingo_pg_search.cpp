extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "utils/relcache.h"
#include "access/genam.h"
#include "access/relscan.h"
#include "utils/rel.h"
}
#ifdef qsort
#undef qsort
#endif

#include "bingo_pg_search.h"

#include "bingo_core_c.h"
#include "base_cpp/tlscont.h"

#include "bingo_pg_common.h"
#include "bingo_pg_buffer.h"
#include "bingo_pg_text.h"
#include "bingo_pg_config.h"
#include "bingo_pg_ext_bitset.h"
#include "mango_pg_search_engine.h"
#include "ringo_pg_search_engine.h"



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

   /*
    * Search and return next element
    */
   return _fpEngine->searchNext(result_ptr);

}

void BingoPgSearch::_initScanSearch() {
   _initSearch = false;

   Relation index = ((IndexScanDesc)_indexScanDesc)->indexRelation;

   BingoPgWrapper rel_wr;
   const char* rel_name = rel_wr.getRelName(index->rd_id);

   /*
    * Read configuration from index tuple
    */
   BingoPgConfig bingo_config;
   _bufferIndex.readConfigParameters(bingo_config);

   /*
    * Set up search engine
    */
   _bufferIndex.readMetaInfo();
   int index_type = _bufferIndex.getIndexType();

   if (index_type == BINGO_INDEX_TYPE_MOLECULE)
      _fpEngine.reset(new MangoPgSearchEngine(bingo_config, rel_name));
   else if (index_type == BINGO_INDEX_TYPE_REACTION)
      _fpEngine.reset(new RingoPgSearchEngine(bingo_config, rel_name));
   else
      throw Error("unknown index type %d", index_type);

   /*
    * Process query structure with parameters
    */
   _fpEngine->prepareQuerySearch(_bufferIndex, _indexScanDesc);

}


