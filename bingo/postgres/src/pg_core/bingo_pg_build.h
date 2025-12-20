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

/*
 */

#ifndef _BINGO_PG_BUILD_H__
#define _BINGO_PG_BUILD_H__

#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"
#include <memory>

#include "bingo_pg_build_engine.h"
#include "bingo_pg_index.h"
#include "bingo_postgres.h"

class BingoPgText;
class BingoPgConfig;

/*
 * Class for building and updating the bingo index
 */
class BingoPgBuild
{
public:
    enum
    {
        MAX_CACHE_SIZE = 1000
    };
    BingoPgBuild(PG_OBJECT index, const char* schema_name, const char* index_schema, bool new_index);
    ~BingoPgBuild();

    /*
     * Inserts a new structure into the index
     * Returns true if insertion was successfull
     */
    void insertStructure(PG_OBJECT item_ptr, uintptr_t text_ptr);
    bool insertStructureSingle(PG_OBJECT item_ptr, uintptr_t text_ptr);
    void insertStructureParallel(PG_OBJECT item_ptr, uintptr_t text_ptr);
    void flush();

    DECL_ERROR;

private:
    BingoPgBuild(const BingoPgBuild&); // no implicit copy

    //   static void _errorHandler(const char* message, void* context);

    void _prepareBuilding(const char* schema_name, const char* index_schema);
    void _prepareUpdating();

    /*
     * Index relation
     */
    PG_OBJECT _index;

    /*
     * Buffers section handler
     */
    BingoPgIndex _bufferIndex;

    std::unique_ptr<BingoPgBuildEngine> fp_engine;

    /*
     * There are two possible uses - build(true) and update(false)
     */
    bool _buildingState;

    indigo::ObjArray<BingoPgBuildEngine::StructCache> _parrallelCache;

    // #ifdef BINGO_PG_INTEGRITY_DEBUG
    //    indigo::std::unique_ptr<FileOutput> debug_fileoutput;
    // #endif
};

#endif /* BINGO_PG_BUILD_H */
