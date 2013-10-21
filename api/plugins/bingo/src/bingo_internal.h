/****************************************************************************
 * Copyright (C) 2010-2013 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __bingo_internal__
#define __bingo_internal__

#include "base_cpp/exception.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo
{

DECL_EXCEPTION_NO_EXP(BingoException);

};

#define BINGO_BEGIN_DB(db_id)                               \
    INDIGO_BEGIN                                            \
    {                                                       \
       if (((db_id) < _bingo_instances.begin()) ||          \
           ((db_id) >= _bingo_instances.end()) ||           \
           !_bingo_instances.hasElement(db_id))             \
         throw BingoException("Incorrect database object"); \
       MMFStorage::database_id = db_id;

#define BINGO_BEGIN_SEARCH(search_id)                       \
    INDIGO_BEGIN                                            \
    {                                                       \
       if (((search_id) < 0) ||                             \
          ((search_id) >= _searches_db.size()) ||           \
          (_searches_db[(search_id)] == -1))                \
          throw BingoException("Incorrect search object");  \
       MMFStorage::database_id = _searches_db[(search_id)]; \

#define BINGO_END(fail)              \
       MMFStorage::database_id = -1; \
    }                                \
    INDIGO_END(fail)    
    
#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __bingo_internal__
