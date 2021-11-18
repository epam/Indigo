#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"
#include "fmgr.h"
}

#include "bingo_pg_fix_post.h"

#include "ringo_pg_build_engine.h"

#include "base_c/bitarray.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "bingo_core_c.h"

#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"
#include "bingo_pg_text.h"
#include "ringo_pg_search_engine.h"
#include <double.h>

using namespace indigo;

RingoPgBuildEngine::RingoPgBuildEngine(BingoPgConfig& bingo_config, const char* rel_name) : BingoPgBuildEngine(), _searchType(-1)
{
    _setBingoContext();
    //   bingoSetErrorHandler(_errorHandler, 0);
    /*
     * Set up bingo configuration
     */
    bingo_config.setUpBingoConfiguration();
    bingoTautomerRulesReady(0, 0, 0);
    bingoIndexBegin();

    _relName.readString(rel_name, true);
    _shadowRelName.readString(rel_name, true);
    _shadowRelName.appendString("_shadow", true);
    elog(DEBUG1, "bingo: ringo build: start building '%s'", _relName.ptr());
}

RingoPgBuildEngine::~RingoPgBuildEngine()
{
    elog(DEBUG1, "bingo: ringo build: finish building '%s'", _relName.ptr());
    bingoIndexEnd();
}

bool RingoPgBuildEngine::processStructure(StructCache& struct_cache)
{

    _setBingoContext();

    ItemPointer item_ptr = &struct_cache.ptr;
    int block_number = ItemPointerGetBlockNumber(item_ptr);
    int offset_number = ItemPointerGetOffsetNumber(item_ptr);

    int struct_size, bingo_res;
    const char* struct_ptr = struct_cache.text->getText(struct_size);
    /*
     * Set target data
     */
    bingoSetIndexRecordData(0, struct_ptr, struct_size);
    /*
     * Process target
     */
    bingo_res = ringoIndexProcessSingleRecord();
    CORE_HANDLE_ERROR_TID_NO_INDEX(bingo_res, 0, "reaction build engine: error while processing records", block_number, offset_number, bingoGetError());
    CORE_HANDLE_WARNING_TID_NO_INDEX(bingo_res, 1, "reaction build engine: error while processing record", block_number, offset_number, bingoGetWarning());
    if (bingo_res < 1)
        return false;

    std::unique_ptr<RingoPgFpData> fp_data = std::make_unique<RingoPgFpData>();
    if (_readPreparedInfo(0, *fp_data, getFpSize()))
    {
        struct_cache.data.reset(fp_data.release());
        struct_cache.data->setTidItem(item_ptr);
    }
    else
    {
        elog(WARNING, "reaction build engine: internal error while processing record with ctid='(%d,%d)'::tid: see at the previous warning", block_number,
             offset_number);
        return false;
    }
    return true;
}

void RingoPgBuildEngine::processStructures(ObjArray<StructCache>& struct_caches)
{
    _setBingoContext();
    int bingo_res;

    _currentCache = 0;
    _structCaches = &struct_caches;
    _fpSize = getFpSize();

    /*
     * Process target
     */
    bingo_res = bingoIndexProcess(true, _getNextRecordCb, _processResultCb, _processErrorCb, this);
    CORE_HANDLE_ERROR(bingo_res, 0, "reaction build engine: error while processing records", bingoGetError());
    _setBingoContext();
}
void RingoPgBuildEngine::insertShadowInfo(BingoPgFpData& item_data)
{
    RingoPgFpData& data = (RingoPgFpData&)item_data;

    const char* shadow_rel_name = _shadowRelName.ptr();
    ItemPointerData* tid_ptr = &data.getTidItem();

    BingoPgCommon::executeQuery("INSERT INTO %s(b_id,tid_map,ex_hash) VALUES ("
                                "'(%d, %d)'::tid, '(%d, %d)'::tid, %d)",
                                shadow_rel_name, data.getSectionIdx(), data.getStructureIdx(), ItemPointerGetBlockNumber(tid_ptr),
                                ItemPointerGetOffsetNumber(tid_ptr), data.getHash());
}

int RingoPgBuildEngine::getFpSize()
{
    int result;
    _setBingoContext();

    bingoGetConfigInt("reaction-fp-size-bytes", &result);

    return result * 8;
}

void RingoPgBuildEngine::prepareShadowInfo(const char* schema_name, const char* index_schema)
{
    /*
     * Create auxialiry tables
     */
    const char* rel_name = _relName.ptr();
    const char* shadow_rel_name = _shadowRelName.ptr();

    /*
     * Drop table if exists (in case of truncate index)
     */
    if (BingoPgCommon::tableExists(index_schema, shadow_rel_name))
    {
        BingoPgCommon::dropDependency(schema_name, index_schema, shadow_rel_name);
        BingoPgCommon::executeQuery("DROP TABLE %s.%s", index_schema, shadow_rel_name);
    }

    BingoPgCommon::executeQuery("CREATE TABLE %s.%s ("
                                "b_id tid,"
                                "tid_map tid,"
                                "ex_hash integer)",
                                index_schema, shadow_rel_name);

    /*
     * Create dependency for new tables
     */
    BingoPgCommon::createDependency(schema_name, index_schema, shadow_rel_name, rel_name);
}

void RingoPgBuildEngine::finishShadowProcessing()
{
    /*
     * Create shadow indexes
     */
    const char* shadow_rel_name = _shadowRelName.ptr();

    BingoPgCommon::executeQuery("CREATE INDEX %s_hash_idx ON %s using hash(ex_hash)", shadow_rel_name, shadow_rel_name);
}

void RingoPgBuildEngine::_processResultCb(void* context)
{
    RingoPgBuildEngine* engine = (RingoPgBuildEngine*)context;
    ObjArray<StructCache>& struct_caches = *(engine->_structCaches);
    int cache_idx = -1;
    std::unique_ptr<RingoPgFpData> fp_data = std::make_unique<RingoPgFpData>();
    /*
     * Prepare info
     */
    if (_readPreparedInfo(&cache_idx, *fp_data, engine->_fpSize))
    {
        StructCache& struct_cache = struct_caches[cache_idx];
        struct_cache.data.reset(fp_data.release());
        struct_cache.data->setTidItem(&struct_cache.ptr);
    }
    else
    {
        if (cache_idx != -1)
        {
            ItemPointer item_ptr = &(struct_caches[cache_idx].ptr);
            int block_number = ItemPointerGetBlockNumber(item_ptr);
            int offset_number = ItemPointerGetOffsetNumber(item_ptr);
            elog(WARNING, "reaction build engine: internal error while processing record with ctid='(%d,%d)'::tid: see at the previous warning", block_number,
                 offset_number);
        }
        else
        {
            elog(WARNING, "reaction build engine: internal error while processing record: see at the previous warning");
        }
    }
}

bool RingoPgBuildEngine::_readPreparedInfo(int* id, RingoPgFpData& data, int fp_size)
{
    int bingo_res;
    const char* crf_buf;
    int crf_len;
    const char* fp_buf;
    int fp_len;
    /*
     * Get prepared data
     */
    bingo_res = ringoIndexReadPreparedReaction(id, &crf_buf, &crf_len, &fp_buf, &fp_len);
    CORE_HANDLE_WARNING(bingo_res, 1, "reaction build engine: error while prepare record", bingoGetError());
    if (bingo_res < 1)
        return false;

    /*
     * Set hash information
     */
    dword ex_hash;
    bingo_res = ringoGetHash(1, &ex_hash);
    CORE_HANDLE_WARNING(bingo_res, 1, "reaction build engine: error while get hash", bingoGetError());
    if (bingo_res < 1)
        return false;
    data.setHash(ex_hash);

    /*
     * Set common info
     */
    data.setCmf(crf_buf, crf_len);
    data.setFingerPrints(fp_buf, fp_size);

    return true;
}
