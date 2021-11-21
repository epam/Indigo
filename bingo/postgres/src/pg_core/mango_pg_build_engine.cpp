#include <float.h>

#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"
#include "fmgr.h"
}

#include "bingo_pg_fix_post.h"

#include "mango_pg_build_engine.h"

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "bingo_core_c.h"

#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"
#include "bingo_pg_text.h"
#include "mango_pg_search_engine.h"


using namespace indigo;

MangoPgBuildEngine::MangoPgBuildEngine(const char* rel_name) : BingoPgBuildEngine(), _searchType(-1)
{
    // _setBingoContext();
    _relName.readString(rel_name, true);
    _shadowRelName.readString(rel_name, true);
    _shadowRelName.appendString("_shadow", true);
    _shadowHashRelName.readString(rel_name, true);
    _shadowHashRelName.appendString("_shadow_hash", true);

    elog(DEBUG1, "bingo: mango build: start building '%s'", _relName.ptr());
}

MangoPgBuildEngine::~MangoPgBuildEngine()
{
    elog(DEBUG1, "bingo: mango build: finish building '%s'", _relName.ptr());
    // _setBingoContext();
    bingoCore.bingoIndexEnd();
}

bool MangoPgBuildEngine::processStructure(StructCache& struct_cache)
{

    // _setBingoContext();
    int bingo_res;

    BingoPgText& struct_text = *struct_cache.text;
    ItemPointer item_ptr = &struct_cache.ptr;
    int block_number = ItemPointerGetBlockNumber(item_ptr);
    int offset_number = ItemPointerGetOffsetNumber(item_ptr);

    int struct_size;
    const char* struct_ptr = struct_text.getText(struct_size);
    /*
     * Set target data
     */
    bingoCore.bingoSetIndexRecordData(0, struct_ptr, struct_size);
    /*
     * Process target
     */
    bingo_res = bingoCore.mangoIndexProcessSingleRecord();
    CORE_HANDLE_ERROR_TID_NO_INDEX(bingo_res, 0, "molecule build engine: error while processing records", block_number, offset_number, bingoGetError());
    CORE_HANDLE_WARNING_TID_NO_INDEX(bingo_res, 1, "molecule build engine: error while processing record", block_number, offset_number, bingoGetWarning());
    if (bingo_res < 1)
        return false;

    std::unique_ptr<MangoPgFpData> fp_data = std::make_unique<MangoPgFpData>();
    if (_readPreparedInfo(0, *fp_data, getFpSize()))
    {
        struct_cache.data.reset(fp_data.release());
        struct_cache.data->setTidItem(item_ptr);
    }
    else
    {
        elog(WARNING, "molecule build engine: internal error while processing record with ctid='(%d,%d)'::tid: see at the previous warning", block_number,
             offset_number);
        return false;
    }
    return true;
}

void MangoPgBuildEngine::processStructures(ObjArray<StructCache>& struct_caches)
{
    // _setBingoContext();
    int bingo_res;

    _currentCache = 0;
    _structCaches = &struct_caches;
    _fpSize = getFpSize();

    /*
     * Process target
     */
    bingo_res = bingoCore.bingoIndexProcess(false, _getNextRecordCb, _processResultCb, _processErrorCb, this);
    /*
     * If error on structure, try to parse ids
     */
    if (bingo_res < 0)
    {
        const char* mes = bingoGetError();
        const char* ERR_MES = "ERROR ON id=";
        const char* id_s = strstr(mes, ERR_MES);
        if (id_s != NULL)
        {
            BufferScanner sc(id_s);
            sc.skip(strlen(ERR_MES));
            int id_n = -1;
            try
            {
                id_n = sc.readInt();
            }
            catch (Exception&)
            {
            }
            if (id_n < struct_caches.size() && id_n >= 0)
            {
                ItemPointer item_ptr = &(struct_caches[id_n].ptr);
                int block_number = ItemPointerGetBlockNumber(item_ptr);
                int offset_number = ItemPointerGetOffsetNumber(item_ptr);
                CORE_HANDLE_ERROR_TID_NO_INDEX(bingo_res, 0, "molecule build engine: error while processing records", block_number, offset_number,
                                               bingoGetError());
            }
        }
    }
    CORE_HANDLE_ERROR(bingo_res, 0, "molecule build engine: error while processing records", bingoGetError());
    // _setBingoContext();
}

void MangoPgBuildEngine::insertShadowInfo(BingoPgFpData& item_data)
{
    MangoPgFpData& data = (MangoPgFpData&)item_data;

    const char* shadow_rel_name = _shadowRelName.ptr();
    const char* shadow_hash_name = _shadowHashRelName.ptr();
    ItemPointerData* tid_ptr = &data.getTidItem();

    BingoPgCommon::executeQuery("INSERT INTO %s(b_id,tid_map,mass,fragments,gross,cnt_C,cnt_N,cnt_O,cnt_P,cnt_S,cnt_H) VALUES ("
                                "'(%d, %d)'::tid, '(%d, %d)'::tid, %f, %d, %s)",
                                shadow_rel_name, data.getSectionIdx(), data.getStructureIdx(), ItemPointerGetBlockNumber(tid_ptr),
                                ItemPointerGetOffsetNumber(tid_ptr), data.getMass(), data.getFragmentsCount(), data.getGrossStr());

    const RedBlackMap<dword, int>& hashes = data.getHashes();
    for (int h_idx = hashes.begin(); h_idx != hashes.end(); h_idx = hashes.next(h_idx))
    {
        BingoPgCommon::executeQuery("INSERT INTO %s(b_id, ex_hash, f_count) VALUES ('(%d, %d)'::tid, %d, %d)", shadow_hash_name, data.getSectionIdx(),
                                    data.getStructureIdx(), hashes.key(h_idx), hashes.value(h_idx));
    }
}

int MangoPgBuildEngine::getFpSize()
{
    int result;
    // _setBingoContext();

    bingoCore.bingoGetConfigInt("fp-size-bytes", &result);

    return result * 8;
}

void MangoPgBuildEngine::prepareShadowInfo(const char* schema_name, const char* index_schema)
{
    /*
     * Create auxialiry tables
     */
    const char* rel_name = _relName.ptr();
    const char* shadow_rel_name = _shadowRelName.ptr();
    const char* shadow_hash_name = _shadowHashRelName.ptr();

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
                                "mass real,"
                                "fragments integer,"
                                "gross text,"
                                "cnt_C integer,"
                                "cnt_N integer,"
                                "cnt_O integer,"
                                "cnt_P integer,"
                                "cnt_S integer,"
                                "cnt_H integer,"
                                "xyz bytea)",
                                index_schema, shadow_rel_name);

    if (BingoPgCommon::tableExists(index_schema, shadow_hash_name))
    {
        BingoPgCommon::dropDependency(schema_name, index_schema, shadow_hash_name);
        BingoPgCommon::executeQuery("DROP TABLE %s.%s", index_schema, shadow_hash_name);
    }
    BingoPgCommon::executeQuery("CREATE TABLE %s.%s (b_id tid, ex_hash integer, f_count integer)", index_schema, shadow_hash_name);
    /*
     * Create dependency for new tables
     */
    BingoPgCommon::createDependency(schema_name, index_schema, shadow_rel_name, rel_name);
    BingoPgCommon::createDependency(schema_name, index_schema, shadow_hash_name, rel_name);
}

void MangoPgBuildEngine::finishShadowProcessing()
{
    /*
     * Create shadow indexes
     */
    const char* shadow_rel_name = _shadowRelName.ptr();
    const char* shadow_hash_rel_name = _shadowHashRelName.ptr();

    BingoPgCommon::executeQuery("CREATE INDEX %s_mass_idx ON %s (mass)", shadow_rel_name, shadow_rel_name);
    BingoPgCommon::executeQuery("CREATE INDEX %s_cmf_idx ON %s(b_id)", shadow_rel_name, shadow_rel_name);
    BingoPgCommon::executeQuery("CREATE INDEX %s_hash_idx ON %s (ex_hash)", shadow_hash_rel_name, shadow_hash_rel_name);
}

void MangoPgBuildEngine::_processResultCb(void* context)
{
    MangoPgBuildEngine* engine = (MangoPgBuildEngine*)context;
    ObjArray<StructCache>& struct_caches = *(engine->_structCaches);
    int cache_idx = -1;
    std::unique_ptr<MangoPgFpData> fp_data = std::make_unique<MangoPgFpData>();
    /*
     * Prepare info
     */
    if (engine->_readPreparedInfo(&cache_idx, *fp_data, engine->_fpSize))
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
            elog(WARNING, "molecule build engine: internal error while processing record with ctid='(%d,%d)'::tid: see at the previous warning", block_number,
                 offset_number);
        }
        else
        {
            elog(WARNING, "molecule build engine: internal error while processing record: see at the previous warning");
        }
    }
}

bool MangoPgBuildEngine::_readPreparedInfo(int* id, MangoPgFpData& data, int fp_size)
{
    int bingo_res;
    const char* cmf_buf;
    int cmf_len;
    const char* xyz_buf;
    int xyz_len;
    const char* gross_str;
    const char* counter_elements_str;
    const char* fp_buf;
    int fp_len;
    const char* fp_sim_str;
    float mass;
    int sim_fp_bits_count;
    /*
     * Get prepared data
     */
    bingo_res = bingoCore.mangoIndexReadPreparedMolecule(id, &cmf_buf, &cmf_len, &xyz_buf, &xyz_len, &gross_str, &counter_elements_str, &fp_buf, &fp_len, &fp_sim_str,
                                               &mass, &sim_fp_bits_count);

    CORE_HANDLE_WARNING(bingo_res, 1, "molecule build engine: error while prepare record", bingoGetError());
    if (bingo_res < 1)
        return false;

    /*
     * Set gross formula
     */
    data.setGrossStr(gross_str, counter_elements_str);

    /*
     * Set hash information
     */
    dword ex_hash;
    int ex_hash_count;
    bingo_res = bingoCore.mangoGetHash(1, -1, &ex_hash_count, &ex_hash);

    CORE_HANDLE_WARNING(bingo_res, 1, "molecule build engine: error while calculating hash for a record", bingoGetError());
    if (bingo_res < 1)
        return false;

    int target_fragments = 0;
    for (int comp_idx = 0; comp_idx < ex_hash_count; ++comp_idx)
    {
        int comp_count;
        bingo_res = bingoCore.mangoGetHash(1, comp_idx, &comp_count, &ex_hash);

        CORE_HANDLE_WARNING(bingo_res, 1, "molecule build engine: error while calculating hash for a record", bingoGetError());
        if (bingo_res < 1)
            return false;
        data.insertHash(ex_hash, comp_count);
        target_fragments += comp_count;
    }
    data.setFragmentsCount(target_fragments);

    /*
     * Set common info
     */
    data.setCmf(cmf_buf, cmf_len);
    data.setXyz(xyz_buf, xyz_len);
    data.setFingerPrints(fp_buf, fp_size);
    data.setMass(mass);
    data.setBitsCount(sim_fp_bits_count);
    return true;
}
