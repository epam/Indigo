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

#ifndef __bingo_internal__
#define __bingo_internal__

#include "base_cpp/exception.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    DECL_EXCEPTION_NO_EXP(BingoException);

};

#define BINGO_BEGIN_DB(db_id)                                                                                                                                  \
    INDIGO_BEGIN                                                                                                                                               \
    {                                                                                                                                                          \
        if (((db_id) < _bingo_instances.begin()) || ((db_id) >= _bingo_instances.end()) || !_bingo_instances.hasElement(db_id))                                \
            throw BingoException("Incorrect database object");                                                                                                 \
        MMFStorage::setDatabaseId(db_id);

#define BINGO_BEGIN_SEARCH(search_id)                                                                                                                          \
    INDIGO_BEGIN                                                                                                                                               \
    {                                                                                                                                                          \
        if (((search_id) < 0) || ((search_id) >= _searches_db.size()) || (_searches_db[(search_id)] == -1))                                                    \
            throw BingoException("Incorrect search object");                                                                                                   \
        MMFStorage::setDatabaseId(_searches_db[(search_id)]);

#define BINGO_END(fail)                                                                                                                                        \
    MMFStorage::setDatabaseId(-1);                                                                                                                             \
    }                                                                                                                                                          \
    INDIGO_END(fail)

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __bingo_internal__
