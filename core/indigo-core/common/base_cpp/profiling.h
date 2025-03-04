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

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <atomic>
#include <chrono>
#include <safe_ptr.h>

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/os_sync_wrapper.h"

#define PROF_GET_NAME_INDEX(var_name, name)                                                                                                                    \
    static std::atomic<int> var_name##_name_index;                                                                                                             \
    if (var_name##_name_index == 0)                                                                                                                            \
    {                                                                                                                                                          \
        auto inst = sf::xlock_safe_ptr(indigo::ProfilingSystem::getInstance());                                                                                \
        var_name##_name_index = inst->getNameIndex(name);                                                                                                      \
    }

#define profTimerStart(var_name, name)                                                                                                                         \
    PROF_GET_NAME_INDEX(var_name, name)                                                                                                                        \
    indigo::ProfilingTimer var_name##_timer(var_name##_name_index)

#define profTimerStop(var_name) var_name##_timer.stop()

#define profTimerGetTime(var_name) var_name##_timer.getTime()

#define profTimerGetTimeSec(var_name) var_name##_timer.getTimeSec()

#define profIncTimer(name, dt)                                                                                                                                 \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        PROF_GET_NAME_INDEX(var_name, name)                                                                                                                    \
        auto inst = sf::xlock_safe_ptr(indigo::ProfilingSystem::getInstance());                                                                                \
        inst->addTimer(var_name##_name_index, dt);                                                                                                             \
    } while (false)

#define profIncCounter(name, count)                                                                                                                            \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        PROF_GET_NAME_INDEX(var_name, name)                                                                                                                    \
        auto inst = sf::xlock_safe_ptr(indigo::ProfilingSystem::getInstance());                                                                                \
        inst->addCounter(var_name##_name_index, count);                                                                                                        \
    } while (false)

#define profTimersReset() sf::xlock_safe_ptr(indigo::ProfilingSystem::getInstance())->reset(false)
#define profTimersResetSession() sf::xlock_safe_ptr(indigo::ProfilingSystem::getInstance())->reset(true)

#define profGetStatistics(output, all) sf::xlock_safe_ptr(indigo::ProfilingSystem::getInstance())->getStatistics(output, all)

namespace indigo
{
    class Output;

    class DLLEXPORT ProfilingSystem
    {
    public:
        static sf::safe_shared_hide_obj<ProfilingSystem>& getInstance();

        int getNameIndex(const char* name, bool add_if_not_exists = true);

        void addTimer(int name_index, qword dt);
        void addCounter(int name_index, int value);
        void reset(bool all);
        void getStatistics(Output& output, bool get_all);

        bool hasLabel(const char* name);
        float getLabelExecTime(const char* name, bool total = false);
        qword getLabelValue(const char* name, bool total = false);
        qword getLabelCallCount(const char* name, bool total = false);

        DECL_ERROR;

    private:
        struct Record
        {
            enum class RecordType
            {
                TYPE_TIMER,
                TYPE_COUNTER
            };

            struct Data
            {
                qword count, value, max_value;
                double square_sum;

                Data();

                void reset();

                void add(qword value);
            };

            Data current, total;

            RecordType type;

            void reset(bool all);
        };

        static int _recordsCmp(int idx1, int idx2, void* context);

        void _printTimerData(const Record::Data& data, Output& output) const;
        static void _printCounterData(const Record::Data& data, Output& output);

        bool _hasLabelIndex(int name_index) const;
        void _ensureRecordExistanceLocked(int name_index);

        ObjArray<Array<char>> _names;
        ObjArray<Record> _records;
        Array<int> _sorted_records;
    };

    class DLLEXPORT ProfilingTimer
    {
    public:
        explicit ProfilingTimer(int name_index);
        ProfilingTimer(ProfilingTimer&&) = delete;
        ProfilingTimer& operator=(ProfilingTimer&&) = delete;
        ProfilingTimer(const ProfilingTimer&) = delete;
        ProfilingTimer& operator=(const ProfilingTimer&) = delete;
        ~ProfilingTimer();

        qword stop();
        qword getTime() const;
        float getTimeSec() const;

    private:
        int _name_index;
        std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
        qword _dt;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
