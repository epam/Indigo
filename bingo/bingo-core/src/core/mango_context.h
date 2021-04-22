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

#ifndef __mango_context__
#define __mango_context__

#include "base_cpp/tlscont.h"
#include "mango_index.h"
#include "mango_matchers.h"

using namespace indigo;

class MangoContext
{
public:
    explicit MangoContext(BingoContext& context);
    virtual ~MangoContext();

    MangoSubstructure substructure;
    MangoSimilarity similarity;
    MangoExact exact;
    MangoTautomer tautomer;
    MangoGross gross;

    static int begin();
    static int end();
    static int next(int k);

    DECL_ERROR;

    static void remove(int id);

    static MangoContext* get(int id);
    static MangoContext* existing(int id);

protected:
    static MangoContext* _get(int id, BingoContext& context);

    TL_DECL(PtrArray<MangoContext>, _instances);
    static OsLock _instances_lock;

    BingoContext& _context;
};

#endif
