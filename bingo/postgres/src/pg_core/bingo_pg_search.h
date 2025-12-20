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

#ifndef _BINGO_PG_SEARCH_H__
#define _BINGO_PG_SEARCH_H__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include <memory>

#include "bingo_pg_index.h"
#include "bingo_pg_text.h"
#include "bingo_postgres.h"

#include "bingo_pg_search_engine.h"

class BingoPgText;
class BingoPgBuffer;
/*
 * Class for searcing molecular structures
 */
class BingoPgSearch
{
public:
    BingoPgSearch(PG_OBJECT rel);
    ~BingoPgSearch();
    /*
     * Searches for the next match. Return true if search was successfull
     * Sets up item pointer
     */
    bool next(PG_OBJECT scan_desc_ptr, PG_OBJECT result_item);

    void setItemPointer(PG_OBJECT result_ptr);
    void readCmfItem(indigo::Array<char>& cmf_buf);

    BingoPgIndex& getIndex()
    {
        return _bufferIndex;
    }

    const char* getFuncName() const
    {
        return _funcName.ptr();
    }

    void prepareRescan(PG_OBJECT scan_desc_ptr, bool deferred_finish);

    DECL_ERROR;

private:
    BingoPgSearch(const BingoPgSearch&); // no implicit copy

    void _initScanSearch(bool deferred_finish = false);
    //   void _defineQueryOptions();

    bool _initSearch;

    PG_OBJECT _indexScanDesc;

    BingoPgIndex _bufferIndex;
    std::unique_ptr<BingoPgSearchEngine> _fpEngine;

    indigo::Array<char> _funcName;
};

#endif /* BINGO_PG_SEARCH_H */
