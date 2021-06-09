#ifndef _RINGO_PG_BUILD_ENGINE_H__
#define _RINGO_PG_BUILD_ENGINE_H__

#include "base_cpp/array.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/obj_array.h"

#include "bingo_pg_build_engine.h"
#include "bingo_postgres.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;
class BingoPgFpData;
class RingoPgFpData;

/*
 * Class for procession reaction fingerprint data
 */
class RingoPgBuildEngine : public BingoPgBuildEngine
{
public:
    RingoPgBuildEngine(BingoPgConfig& bingo_config, const char* rel_name);
    ~RingoPgBuildEngine() override;

    bool processStructure(StructCache& struct_cache) override;
    void processStructures(indigo::ObjArray<StructCache>& struct_cache) override;

    int getFpSize() override;
    int getType() const override
    {
        return BINGO_INDEX_TYPE_REACTION;
    }

    void prepareShadowInfo(const char* schema_name, const char* index_schema) override;
    void insertShadowInfo(BingoPgFpData&) override;
    void finishShadowProcessing() override;

    // hardcode return single threading for reactions due to an instable state
    int getNthreads()
    {
        return 1;
    }

private:
    RingoPgBuildEngine(const RingoPgBuildEngine&); // no implicit copy

    static void _processResultCb(void* context);
    static bool _readPreparedInfo(int* id, RingoPgFpData& data, int fp_size);

    indigo::ArrayChar _relName;
    indigo::ArrayChar _shadowRelName;

    int _searchType;
};

#endif /* RINGO_PG_BUILD_ENGINE_H */
