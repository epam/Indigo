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

#ifndef __bingo_index__
#define __bingo_index__

#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"

#include "bingo_error.h"

using namespace indigo;

class BingoContext;

class BingoIndex
{
public:
    BingoIndex()
    {
        _context = 0;
        skip_calculate_fp = false;
    }
    virtual ~BingoIndex()
    {
    }
    void init(BingoContext& context)
    {
        _context = &context;
    };

    virtual void prepare(Scanner& scanner, Output& output, std::mutex* lock_for_exclusive_access) = 0;

    bool skip_calculate_fp;

protected:
    BingoContext* _context;

private:
    BingoIndex(const BingoIndex&); // no implicit copy
};
#endif
