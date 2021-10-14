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

namespace indigo
{

    DECL_EXCEPTION_NO_EXP(BingoException);

};

#define BINGO_BEGIN_DB(db_id)                                                                                                                                  \
    INDIGO_BEGIN                                                                                                                                               \
    {                                                                                                                                                          \
        {                                                                                                                                                      \
            auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);                                                                                           \
            if (!bingo_indexes->has(db_id))                                                                                                                    \
                throw BingoException("Incorrect database instance");                                                                                             \
        }                                                                                                                                                      \
        MMFStorage::setDatabaseId(db_id);

// Used when we don't need Indigo session, just handle errors
#define BINGO_BEGIN_DB_STATIC(db_id)                                                                                                                           \
    INDIGO_BEGIN_STATIC                                                                                                                                        \
    {                                                                                                                                                          \
        {                                                                                                                                                      \
            auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);                                                                                           \
            if (!bingo_indexes->has(db_id))                                                                                                                    \
                throw BingoException("Incorrect database instance");                                                                                             \
        }                                                                                                                                                      \
        MMFStorage::setDatabaseId(db_id);

#define BINGO_BEGIN_SEARCH(search_id)                                                                                                                          \
    INDIGO_BEGIN                                                                                                                                               \
    {                                                                                                                                                          \
        {                                                                                                                                                      \
            const auto searches_data = sf::slock_safe_ptr(_searches_data);                                                                                     \
            if (((search_id) < 0) || ((search_id) >= searches_data->db.size()) || (searches_data->db[(search_id)] == -1))                                      \
            {                                                                                                                                                  \
                throw BingoException("Incorrect search object");                                                                                               \
            }                                                                                                                                                  \
            MMFStorage::setDatabaseId(searches_data->db[search_id]);                                                                                           \
        }

// Used when we don't need Indigo session, just handle errors
#define BINGO_BEGIN_SEARCH_STATIC(search_id)                                                                                                                   \
    INDIGO_BEGIN_STATIC                                                                                                                                        \
    {                                                                                                                                                          \
        {                                                                                                                                                      \
            const auto searches_data = sf::slock_safe_ptr(_searches_data);                                                                                     \
            if (((search_id) < 0) || ((search_id) >= searches_data->db.size()) || (searches_data->db[(search_id)] == -1))                                      \
            {                                                                                                                                                  \
                throw BingoException("Incorrect search object");                                                                                               \
            }                                                                                                                                                  \
            MMFStorage::setDatabaseId(searches_data->db[search_id]);                                                                                           \
        }

#define BINGO_END(fail)                                                                                                                                        \
    MMFStorage::setDatabaseId(-1);                                                                                                                             \
    }                                                                                                                                                          \
    INDIGO_END(fail)

#endif // __bingo_internal__
