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

#include <fstream>

#include "BS_thread_pool.hpp"
#include "mio.hpp"

#include "bingo.h"

void allocate_file(const std::string& path, const int size)
{
    std::ofstream file(path, std::ios::binary);
    file.seekp(size - 1);
    file.write("", 1);
}

int main()
{
    BS::thread_pool pool(std::thread::hardware_concurrency());

    mio::mmap_source mmap("molecules.smi");
    const auto& data = mmap.data();
    const size_t size = mmap.size();
    madvise((void*)data, mmap.size(), MADV_WILLNEED);

    int prev_position = 0;
    std::vector<std::future<FingerprintData>> futures;
    for (int i = 0; i < size; i++)
    {
        if (data[i] == '\n')
        {
            futures.emplace_back(pool.submit_task([&data, prev_position, i] { return fp(data, prev_position, i); }));
            prev_position = ++i;
        }
    }
    std::cout << fp_item_size << std::endl;
    std::cout << futures.size() << std::endl;
    allocate_file("molecules.bfp", futures.size() * fp_item_size);

    std::error_code error;
    mio::mmap_sink out = mio::make_mmap_sink(
        "molecules.bfp", 0, mio::map_entire_file, error);

    int i = 0;
    for (auto& future : futures)
    {
        const auto& result = future.get();
        if (result.position == -1)
        {
            continue;
        }
        memcpy(out.data() + i * fp_item_size, result.fingerprint, fingerprintParams.fingerprintSizeSim());
        memcpy(out.data() + i * fp_item_size + fingerprintParams.fingerprintSizeSim(), &result.position, sizeof(int));
        i++;
        if (i % 100000 == 0)
        {
            std::cout << i << std::endl;
        }
    }
}
