/*
 */

#ifndef _MANGO_PG_BUILD_ENGINE_H__
#define _MANGO_PG_BUILD_ENGINE_H__

#include "base_cpp/array.h"
#include <memory>
#include "base_cpp/exception.h"

#include "bingo_pg_build_engine.h"
#include "bingo_postgres.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;
class BingoPgFpData;
class MangoPgFpData;

/*
 * Class for procession molecule fingerprint data
 */
class MangoPgBuildEngine : public BingoPgBuildEngine
{
public:
    MangoPgBuildEngine(BingoPgConfig& bingo_config, const char* rel_name);
    ~MangoPgBuildEngine() override;

    bool processStructure(StructCache& struct_cache) override;
    void processStructures(indigo::ObjArray<StructCache>& struct_caches) override;

    int getFpSize() override;
    int getType() const override
    {
        return BINGO_INDEX_TYPE_MOLECULE;
    }

    void prepareShadowInfo(const char* schema_name, const char* index_schema) override;
    void insertShadowInfo(BingoPgFpData&) override;
    void finishShadowProcessing() override;

private:
    MangoPgBuildEngine(const MangoPgBuildEngine&); // no implicit copy

    static void _processResultCb(void* context);
    bool _readPreparedInfo(int* id, MangoPgFpData& data, int fp_size);

    //   void _handleError(int res, int success_res, const char* message, bool only_warn);

    indigo::Array<char> _relName;
    indigo::Array<char> _shadowRelName;
    indigo::Array<char> _shadowHashRelName;

    int _searchType;
};

#endif /* MANGO_PG_BUILD_ENGINE_H */
