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

#ifndef __ringo_context__
#define __ringo_context__

#include "base_cpp/tlscont.h"
#include "ringo_index.h"
#include "ringo_matchers.h"

using namespace indigo;

namespace ingido
{
    class BingoContext;
}

class RingoContext
{
public:
    explicit RingoContext(BingoContext& context);
    virtual ~RingoContext();

    RingoSubstructure substructure;
    RingoExact exact;
    RingoAAM ringoAAM;

    DECL_ERROR;

    static int begin();
    static int end();
    static int next(int k);

    static void remove(int id);

    static RingoContext* existing(int id);
    static RingoContext* get(int id);

protected:
    static RingoContext* _get(int id, BingoContext& context);

    TL_DECL(PtrArray<RingoContext>, _instances);
    static std::mutex _instances_lock;

    BingoContext& _context;
};

#endif
