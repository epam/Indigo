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

#ifndef __mango_index__
#define __mango_index__

#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "bingo_index.h"
#include "mango_matchers.h"

using namespace indigo;

namespace indigo
{
    class Scanner;
}

class BingoContext;

class MangoIndex : public BingoIndex
{
public:
    void prepare(Scanner& molfile, Output& output, OsLock* lock_for_exclusive_access);

    const ArrayChar& getCmf() const;
    const ArrayChar& getXyz() const;

    const MangoExact::Hash& getHash() const;

    const char* getGrossString() const;
    const char* getCountedElementsString() const;
    const ArrayNew<int>& getCountedElements() const;

    const byte* getFingerprint() const;

    const char* getFingerprint_Sim_Str() const;

    float getMolecularMass() const;

    int getFpSimilarityBitsCount() const;

    static const int counted_elements[6];

    void clear();

private:
    // CMF-packed aromatized molecule and coordinates
    ArrayChar _cmf;
    ArrayChar _xyz;

    // hash for exact match
    MangoExact::Hash _hash;

    // gross formula
    ArrayNew<int> _gross;
    ArrayChar _gross_str;

    Array<byte> _fp;
    ArrayChar _fp_sim_str;

    // comma-separated list of selected counters
    // (for non-exact gross formula search)
    ArrayChar _counted_elems_str;
    ArrayNew<int> _counted_elem_counters;

    // Molecular mass
    float _molecular_mass;

    // Number of one bits in similarity fingerprint
    int _fp_sim_bits_count;
};

#endif
