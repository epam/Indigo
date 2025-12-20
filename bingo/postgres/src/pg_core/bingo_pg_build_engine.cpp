/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "fmgr.h"
}

#include "bingo_pg_fix_post.h"

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "gzip/gzip_scanner.h"

#include "bingo_pg_build_engine.h"

#include "bingo_pg_config.h"
#include "bingo_pg_index.h"

using namespace indigo;

BingoPgBuildEngine::BingoPgBuildEngine() : _bufferIndexPtr(0)
{
    _bingoContext = std::make_unique<BingoContext>(0);
    _mangoContext = std::make_unique<MangoContext>(*_bingoContext.get());
    _ringoContext = std::make_unique<RingoContext>(*_bingoContext.get());

    bingoCore.bingo_context = _bingoContext.get();
    bingoCore.mango_context = _mangoContext.get();
    bingoCore.ringo_context = _ringoContext.get();
}

BingoPgBuildEngine::~BingoPgBuildEngine()
{
}

void BingoPgBuildEngine::setUpConfiguration(BingoPgConfig& bingo_config)
{
    /*
     * Set up bingo configuration
     */
    bingo_config.setUpBingoConfiguration();
    bingoCore.bingoTautomerRulesReady(0, 0, 0);
    bingoCore.bingoIndexBegin();
}

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
    elog(WARNING, "build engine: error while processing record with ctid='(%d,%d)'::tid: %s", block_number, offset_number, engine->bingoCore.warning.ptr());
}