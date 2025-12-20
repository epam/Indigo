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

#ifndef _MANGO_PG_BUILD_ENGINE_H__
#define _MANGO_PG_BUILD_ENGINE_H__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include <memory>

#include "bingo_pg_build_engine.h"
#include "bingo_postgres.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgFpData;
class MangoPgFpData;

/*
 * Class for procession molecule fingerprint data
 */
class MangoPgBuildEngine : public BingoPgBuildEngine
{
public:
    MangoPgBuildEngine(const char* rel_name);
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
