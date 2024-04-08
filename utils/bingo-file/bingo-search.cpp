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

#include <base_cpp/scanner.h>
#include <molecule/molecule_fingerprint.h>

using namespace indigo;

#include "bingo.h"

#include "mio.hpp"
#include "libpopcnt.h"

namespace
{
    double tanimoto_similarity(const std::byte* a, const std::byte* b, const int size)
    {
        const auto a_ones = popcnt(a, size);
        const auto b_ones = popcnt(b, size);

        std::byte ab[size];
#pragma omp simd
        for (int i = 0; i < size; i++)
        {
            ab[i] = static_cast<std::byte>(a[i] & b[i]);
        }
        const auto ab_ones = popcnt(ab, size);

        return static_cast<double>(ab_ones) / static_cast<double>(a_ones + b_ones - ab_ones);
    }
}

int main(int argc, char* argv[])
{
    FingerprintData queryData = fp(argv[1], 0, strlen(argv[1]));


    // read the database file
    mio::mmap_source mmap("molecules.bfp");
    const auto& data = mmap.data();
    const size_t size = mmap.size();
    madvise((void*)data, mmap.size(), MADV_WILLNEED);


    // count items
    const int items = size / fp_item_size;
    std::cout << "items: " << items << std::endl;

    // compare the query fingerprint with the database fingerprints using tanimoto similarity and popcnt
    for (int i = 0; i < items; i++)
    {
        const std::byte* db_fingerprint = reinterpret_cast<const std::byte*>(&data[i * fp_item_size]);
        const auto db_position = *reinterpret_cast<const int*>(&data[i * fp_item_size + fingerprintParams.fingerprintSizeSim()]);
        const auto similarity = tanimoto_similarity(reinterpret_cast<const std::byte*>(queryData.fingerprint), db_fingerprint, fingerprintParams.fingerprintSizeSim());
        if (similarity >= 0.7)
        {
            std::cout << "similarity: " << similarity << ", position: " << db_position << std::endl;
        }
    }
}
