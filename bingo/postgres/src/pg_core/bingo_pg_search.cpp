#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "access/genam.h"
#include "access/relscan.h"
#include "fmgr.h"
#include "utils/rel.h"
#include "utils/relcache.h"
}

#include "bingo_pg_fix_post.h"

#include "base_cpp/tlscont.h"

#include "bingo_pg_buffer.h"
#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_search.h"
#include "mango_pg_search_engine.h"
#include "ringo_pg_search_engine.h"

using namespace indigo;

IMPL_ERROR(BingoPgSearch, "bingo search engine");

BingoPgSearch::BingoPgSearch(PG_OBJECT rel) : _initSearch(true), _indexScanDesc(0), _bufferIndex(rel)
{
    Relation rel_idx = (Relation)rel;
    BingoPgWrapper rel_wr;
    const char* rel_name = rel_wr.getRelName(rel_idx->rd_id);

    elog(DEBUG1, "bingo: search: start searching for idx '%s'", rel_name);

    _bufferIndex.setStrategy(BingoPgIndex::READING_STRATEGY);
}

BingoPgSearch::~BingoPgSearch()
{
}
/*
 * Searches for the next match. Return true if search was successfull
 * Sets up item pointer
 */
bool BingoPgSearch::next(PG_OBJECT scan_desc_ptr, PG_OBJECT result_ptr)
{
    /*
     * All the state keeps in the context
     */
    _indexScanDesc = scan_desc_ptr;

    /*
     * Init searching
     */
    if (_initSearch)
    {
        _initScanSearch();
    }

    /*
     * Search and return next element
     */
    return _fpEngine->searchNext(result_ptr);
}

void BingoPgSearch::prepareRescan(PG_OBJECT scan_desc_ptr)
{
    _indexScanDesc = scan_desc_ptr;
    if (_initSearch)
    {
        _initScanSearch();
    }
    else
    {
        /*
         * Prepare query
         */
        _fpEngine->prepareQuerySearch(_bufferIndex, _indexScanDesc);
    }
}

void BingoPgSearch::_initScanSearch()
{
    _initSearch = false;

    Relation index = ((IndexScanDesc)_indexScanDesc)->indexRelation;

    BingoPgWrapper rel_wr;
    const char* rel_name = rel_wr.getRelName(index->rd_id);
    /*
     * Set up search engine
     */
    _bufferIndex.readMetaInfo();
    _bufferIndex.setStrategy(BingoPgIndex::READING_STRATEGY);
    int index_type = _bufferIndex.getIndexType();

    if (index_type == BINGO_INDEX_TYPE_MOLECULE)
        _fpEngine = std::make_unique<MangoPgSearchEngine>(rel_name);
    else if (index_type == BINGO_INDEX_TYPE_REACTION)
        _fpEngine = std::make_unique<RingoPgSearchEngine>(rel_name);
    else
        throw Error("unknown index type %d", index_type);
    /*
     * Read configuration from index tuple
     */
    auto& bingo_core = _fpEngine->bingoCore;

    BingoPgConfig bingo_config(bingo_core);
    _bufferIndex.readConfigParameters(bingo_config);
    /*
     * Set up bingo configuration
     */
    bingo_config.setUpBingoConfiguration();
    bingo_core.bingoTautomerRulesReady(0, 0, 0);
    bingo_core.bingoIndexBegin();

    /*
     * Process query structure with parameters
     */
    _fpEngine->prepareQuerySearch(_bufferIndex, _indexScanDesc);
    _fpEngine->loadDictionary(_bufferIndex);
}
