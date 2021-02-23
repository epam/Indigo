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

#ifndef __temporary_thread_obj__
#define __temporary_thread_obj__

#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    //
    // Various Indigo API methods returns pointers to the temporary buffers
    // This leads to an issues when multiple threads gets the same pointers
    // To avoid this we returns a temporary object that belongs to the current
    // thread for a short time (10 sec).
    //
    template <typename T> class TemporaryThreadObjManager
    {
    public:
        // This method return a temporary object for this thread that lives at least 10 seconds.
        // Method is synchronized
        T& getObject();

    private:
        typedef std::chrono::steady_clock steady_clock;
        struct Container
        {
            Container();
            Container(Container&& that);

            steady_clock::time_point timestamp;
            std::unique_ptr<T> obj;
        };

        typedef std::unordered_map<std::thread::id, Container> Map;
        Map _objects;

        std::mutex access_mutex;
    };

#include "temporary_thread_obj.hpp"

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __tlscont_h__
