#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"
#include "fmgr.h"
}

#include "bingo_pg_fix_post.h"

#include "bingo_core_c.h"
#include "bingo_pg_build_engine.h"

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"

#include "bingo_pg_index.h"

using namespace indigo;

BingoPgBuildEngine::BingoPgBuildEngine() : _bufferIndexPtr(0)
{
    // _bingoSession = bingoAllocateSessionID();
    _bingoContext = std::make_unique<BingoContext>(0);
    _mangoContext = std::make_unique<MangoContext>(*_bingoContext.get());
    _ringoContext = std::make_unique<RingoContext>(*_bingoContext.get());

    bingoCore.bingo_context = _bingoContext.get();
    bingoCore.mango_context = _mangoContext.get();
    bingoCore.ringo_context = _ringoContext.get();
}

BingoPgBuildEngine::~BingoPgBuildEngine()
{
    // bingoReleaseSessionID(_bingoSession);
}

// void BingoPgBuildEngine::_setBingoContext()
// {
    // bingoSetSessionID(_bingoSession);
    // bingoSetContext(0);
// }

void BingoPgBuildEngine::loadDictionary(BingoPgIndex& bingo_index)
{
    // _setBingoContext();

    QS_DEF(Array<char>, dict);
    bingo_index.readDictionary(dict);
    bingoCore.bingoSetConfigBin("cmf_dict", dict.ptr(), dict.sizeInBytes());
}

const char* BingoPgBuildEngine::getDictionary(int& size)
{
    // _setBingoContext();

    const char* dict_buf;

    bingoCore.bingoGetConfigBin("cmf-dict", &dict_buf, &size);

    return dict_buf;
}

int BingoPgBuildEngine::getNthreads()
{
    // TO DISABLE THREADS UNCOMMENT THIS
    //   return 1;

    if (!nThreads.hasValue())
    {
        // _setBingoContext();
        int result;
        bingoCore.bingoGetConfigInt("nthreads", &result);
        nThreads.set(result);
    }

    return nThreads.get();
}

int BingoPgBuildEngine::_getNextRecordCb(void* context)
{
    BingoPgBuildEngine* engine = (BingoPgBuildEngine*)context;

    int& cache_idx = engine->_currentCache;
    ObjArray<StructCache>& struct_caches = *(engine->_structCaches);
    if (cache_idx >= struct_caches.size())
        return 0;

    StructCache& struct_cache = struct_caches[cache_idx];

    int struct_size;
    const char* struct_ptr = struct_cache.text->getText(struct_size);

    /*
     * Set target data. There is no need to handle errors
     */
    engine->bingoCore.bingoSetIndexRecordData(cache_idx, struct_ptr, struct_size);
    ++cache_idx;
    return 1;
}

void BingoPgBuildEngine::_processErrorCb(int id, void* context)
{
    BingoPgBuildEngine* engine = (BingoPgBuildEngine*)context;
    ObjArray<StructCache>& struct_caches = *(engine->_structCaches);
    ItemPointer item_ptr = &(struct_caches[id].ptr);
    int block_number = ItemPointerGetBlockNumber(item_ptr);
    int offset_number = ItemPointerGetOffsetNumber(item_ptr);
    elog(WARNING, "build engine: error while processing record with ctid='(%d,%d)'::tid: %s", block_number, offset_number, bingoGetWarning());
}