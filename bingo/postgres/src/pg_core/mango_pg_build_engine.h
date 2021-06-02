/*
 */

#ifndef _MANGO_PG_BUILD_ENGINE_H__
#define _MANGO_PG_BUILD_ENGINE_H__

#include "base_cpp/array.h"
#include "base_cpp/auto_ptr.h"
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
    virtual ~MangoPgBuildEngine();

    virtual bool processStructure(StructCache& struct_cache);
    virtual void processStructures(indigo::ObjArray<StructCache>& struct_caches);

    virtual int getFpSize();
    virtual int getType() const
    {
        return BINGO_INDEX_TYPE_MOLECULE;
    }

    virtual void prepareShadowInfo(const char* schema_name, const char* index_schema);
    virtual void insertShadowInfo(BingoPgFpData&);
    virtual void finishShadowProcessing();

private:
    MangoPgBuildEngine(const MangoPgBuildEngine&); // no implicit copy

    static void _processResultCb(void* context);
    static bool _readPreparedInfo(int* id, MangoPgFpData& data, int fp_size);

    //   void _handleError(int res, int success_res, const char* message, bool only_warn);

    indigo::ArrayChar _relName;
    indigo::ArrayChar _shadowRelName;
    indigo::ArrayChar _shadowHashRelName;

    int _searchType;
};

#endif /* MANGO_PG_BUILD_ENGINE_H */
