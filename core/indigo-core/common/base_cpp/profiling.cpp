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

#include "profiling.h"

#include <algorithm>
#include <cmath>

#include <safe_ptr.h>

#include "base_cpp/output.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/smart_output.h"

using namespace indigo;

//
// ProfilingTimer
//

ProfilingTimer::ProfilingTimer(int name_index) : _name_index(name_index), _start_time(nanoClock()), _dt(0)
{
}

ProfilingTimer::~ProfilingTimer()
{
    stop();
}

qword ProfilingTimer::stop()
{
    if (_name_index == -1)
    {
        return 0;
    }
    _dt = nanoClock() - _start_time;
    {
        auto inst = sf::xlock_safe_ptr(ProfilingSystem::getInstance());
        inst->addTimer(_name_index, _dt);
    }
    _name_index = -1;
    return _dt;
}

qword ProfilingTimer::getTime() const
{
    if (_name_index == -1)
    {
        return _dt;
    }
    return nanoClock() - _start_time;
}

float ProfilingTimer::getTimeSec() const
{
    return nanoHowManySeconds(getTime());
}

//
// Profiling functionality
//

IMPL_ERROR(ProfilingSystem, "Profiling system");

namespace indigo
{
}

sf::safe_shared_hide_obj<ProfilingSystem>& ProfilingSystem::getInstance()
{
    static sf::safe_shared_hide_obj<ProfilingSystem> _profiling_system;
    return _profiling_system;
}

int ProfilingSystem::getNameIndex(const char* name, bool add_if_not_exists)
{
    for (int i = 0; i < _names.size(); i++)
    {
        if (strcmp(_names.at(i).ptr(), name) == 0)
        {
            return i;
        }
    }
    if (!add_if_not_exists)
    {
        return -1;
    }
    // Add new label
    Array<char>& name_record = _names.push();
    name_record.copy(name, static_cast<int>(strlen(name)) + 1);
    return _names.size() - 1;
}

void ProfilingSystem::addTimer(const int name_index, const qword dt)
{
    _ensureRecordExistanceLocked(name_index);
    Record& rec = _records[name_index];
    rec.type = Record::RecordType::TYPE_TIMER;
    rec.current.add(dt);
    rec.total.add(dt);
}

void ProfilingSystem::addCounter(const int name_index, const int value)
{
    _ensureRecordExistanceLocked(name_index);
    Record& rec = _records[name_index];
    rec.type = Record::RecordType::TYPE_COUNTER;
    rec.current.add(value);
    rec.total.add(value);
}

void ProfilingSystem::reset(const bool all)
{
    for (int i = 0; i < _records.size(); i++)
    {
        _records[i].reset(all);
    }
}

int ProfilingSystem::_recordsCmp(const int idx1, const int idx2, void* context)
{
    auto* this_ = static_cast<ProfilingSystem*>(context);
    return strcmp(this_->_names.at(idx1).ptr(), this_->_names.at(idx2).ptr());
}

void ProfilingSystem::getStatistics(Output& output, const bool get_all)
{
    // Print formatted statistics
    while (_sorted_records.size() < _records.size())
    {
        _sorted_records.push(_sorted_records.size());
    }
    _sorted_records.qsort(_recordsCmp, this);

    // Find maximum name length
    int max_len = 0;
    for (int i = 0; i < _records.size(); i++)
    {
        if (!_hasLabelIndex(i))
        {
            continue;
        }

        if (_names.at(i).size() > max_len)
        {
            max_len = _names.at(i).size();
        }
    }

    SmartTableOutput table_output(output, true);

    table_output.setLineFormat("|c|5c|5c|");
    table_output.printHLine();
    table_output.printf("Name\tStatistics\t\t\t\t\tSession statistics\t\t\t\t\n");
    table_output.setLineFormat("|l|ccccc|ccccc|");
    table_output.printf("\ttotal\tcount\tavg.\tst.dev.\tmax\ttotal\tcount\tavg.\tst.dev.\tmax\n");
    table_output.printHLine();

    table_output.setLineFormat("|l|rrrrr|rrrrr|");

    for (int i = 0; i < _sorted_records.size(); i++)
    {
        int idx = _sorted_records[i];
        if (!_hasLabelIndex(idx))
            continue;
        Record& rec = _records[idx];
        if (!get_all && rec.current.count == 0)
            continue;

        table_output.printf("%s\t", _names.at(idx).ptr());

        if (rec.type == Record::RecordType::TYPE_TIMER)
        {
            _printTimerData(rec.current, table_output);
            table_output.printf("\t");
            _printTimerData(rec.total, table_output);
            table_output.printf("\n");
        }
        else /* rec.type == Record::TYPE_COUNTER */
        {
            _printCounterData(rec.current, table_output);
            table_output.printf("\t");
            _printCounterData(rec.total, table_output);
            table_output.printf("\n");
        }
    }
    table_output.printHLine();

    table_output.flush();
}

void ProfilingSystem::_printTimerData(const Record::Data& data, Output& output) const
{
    if (data.count == 0)
    {
        output.printf("-\t0\t\t\t");
        return;
    }
    float total_sec = nanoHowManySeconds(data.value);
    float avg_ms = nanoHowManySeconds(data.value / data.count) * 1000;
    float max_ms = nanoHowManySeconds(data.max_value) * 1000;

    double avg_value = (double)data.value / data.count;
    double sigma_sq = data.square_sum / data.count - avg_value * avg_value;
    float sigma_ms = nanoHowManySeconds((qword)sqrt(sigma_sq)) * 1000;

    output.printf("%0.2fs\t%0.0lf\t%0.1fms\t%0.1lfms\t%0.1fms", total_sec, (double)data.count, avg_ms, sigma_ms, max_ms);
}

void ProfilingSystem::_printCounterData(const Record::Data& data, Output& output)
{
    if (data.count == 0)
    {
        output.printf("-\t0\t\t\t");
        return;
    }
    float avg_value = (float)data.value / data.count;

    // To avoid platform-specific code qwords were casted to doubles
    double sigma_sq = data.square_sum / data.count - avg_value * avg_value;
    output.printf("%0.0lf\t%0.0lf\t%0.1f\t%0.1lf\t%0.0lf", (double)data.value, (double)data.count, avg_value, sqrt(sigma_sq), (double)data.max_value);
}

bool ProfilingSystem::_hasLabelIndex(const int name_index) const
{
    if (name_index >= _records.size())
    {
        return false;
    }
    return _records[name_index].total.count > 0;
}

bool ProfilingSystem::hasLabel(const char* name)
{
    int name_index = getNameIndex(name, false);
    if (name_index == -1)
    {
        return false;
    }
    return _hasLabelIndex(name_index);
}

void ProfilingSystem::_ensureRecordExistanceLocked(const int name_index)
{
    while (_records.size() <= name_index)
    {
        _records.push();
    }
}

float ProfilingSystem::getLabelExecTime(const char* name, const bool total)
{
    int idx = getNameIndex(name);
    _ensureRecordExistanceLocked(idx);

    if (total)
    {
        return nanoHowManySeconds(_records[idx].total.value);
    }
    return nanoHowManySeconds(_records[idx].current.value);
}

qword ProfilingSystem::getLabelValue(const char* name, const bool total)
{
    int idx = getNameIndex(name);
    _ensureRecordExistanceLocked(idx);
    if (total)
    {
        return _records[idx].total.value;
    }
    return _records[idx].current.value;
}

qword ProfilingSystem::getLabelCallCount(const char* name, const bool total)
{
    int idx = getNameIndex(name);
    _ensureRecordExistanceLocked(idx);
    if (total)
    {
        return _records[idx].total.count;
    }
    return _records[idx].current.count;
}

//
// ProfilingSystem::Record
//
void ProfilingSystem::Record::reset(const bool all)
{
    current.reset();
    if (all)
    {
        total.reset();
    }
}

//
// ProfilingSystem::Record::Data
//

ProfilingSystem::Record::Data::Data() : count(0), value(0), max_value(0), square_sum(0.0)
{
    reset();
}

void ProfilingSystem::Record::Data::reset()
{
    count = value = max_value = 0;
    square_sum = 0;
}

void ProfilingSystem::Record::Data::add(const qword adding_value)
{
    count++;
    value += adding_value;
    max_value = std::max(max_value, adding_value);

    const auto adding_value_dbl = static_cast<double>(adding_value);
    square_sum += adding_value_dbl * adding_value_dbl;
}
