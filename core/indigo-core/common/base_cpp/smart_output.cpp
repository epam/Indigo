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

#include <algorithm>
#include <string.h>

#include "base_cpp/smart_output.h"

using namespace indigo;

IMPL_ERROR(SmartTableOutput, "smart table output");

CP_DEF(SmartTableOutput);

SmartTableOutput::SmartTableOutput(Output& output, bool use_smart_printing)
    : CP_INIT, TL_CP_GET(_lines), TL_CP_GET(_line_formats), TL_CP_GET(_line_format_index), _output(output)
{
    _lines.clear();
    _line_format_index.clear();
    _line_formats.clear();

    _active_line = &_lines.push();
    _line_format_index.push(-1);
    _use_smart_printing = use_smart_printing;
}

SmartTableOutput::~SmartTableOutput()
{
    flush();
}

void SmartTableOutput::write(const void* data, int size)
{
    if (!_use_smart_printing)
    {
        _output.write(data, size);
        return;
    }
    // split data into lines
    const char* data_char = (const char*)data;
    int start = 0;
    int end = 0;
    while (end <= size)
    {
        if (end == size || data_char[end] == '\n')
        {
            for (int i = start; i < end; i++)
                _active_line->push(data_char[i]);

            if (end < size)
            {
                _active_line = &_lines.push();
                _line_format_index.push(_line_formats.size() - 1);
            }

            start = end + 1;
        }
        end++;
    }
}

void SmartTableOutput::flush()
{
    // TODO: MR: merge this with flushTable?
    if (!_use_smart_printing || _lines.size() == 0)
        return;

    // Smart table printing
    QS_DEF(Array<int>, column_widths);
    column_widths.clear();

    for (int i = 0; i < _lines.size(); i++)
        _updateColumnWidths(i, column_widths);
    for (int i = 0; i < _lines.size(); i++)
        _printLineSmart(i, column_widths);
    _lines.clear();
}

void SmartTableOutput::setLineFormat(const char* line_format)
{
    Array<char>& format = _line_formats.push();
    format.copy(line_format, strlen(line_format));
    _line_format_index.top() = _line_formats.size() - 1;
}

void SmartTableOutput::_updateColumnWidths(int index, Array<int>& widths)
{
    const Array<char>& line = _lines[index];

    if (line.size() == 0 || line[0] == HLINE_CHAR)
        return;

    QS_DEF(Array<int>, cur_widths);
    cur_widths.clear();
    cur_widths.push(0);

    for (int i = 0; i < line.size(); i++)
    {
        if (line[i] == '\t')
            cur_widths.push(0);
        else
            cur_widths.top()++;
    }

    // Check merged columns
    Array<char>& format = _line_formats[_line_format_index[index]];
    int cur_column = 0;
    for (int i = 0; i < format.size(); i++)
    {
        if (format[i] > '0' && format[i] < '9')
        {
            int merge_count = format[i] - '0';

            int width = cur_widths[cur_column];
            for (int j = 0; j < merge_count; j++)
                cur_widths[cur_column + j] = width / merge_count + 1;

            cur_column += merge_count;
            i++;
        }
        else if (format[i] == 'l' || format[i] == 'r' || format[i] == 'c')
            cur_column++;
    }

    // Update total widths
    while (widths.size() < cur_widths.size())
        widths.push(0);

    for (int i = 0; i < cur_widths.size(); i++)
        widths[i] = std::max(widths[i], cur_widths[i] + 2);
}

void SmartTableOutput::_printLineSmart(int index, const Array<int>& widths)
{
    const Array<char>& line = _lines[index];
    if (line.size() == 0)
        return;

    Array<char>& format = _line_formats[_line_format_index[index]];

    bool hline = false;

    int field_begin = 0, field_end = 0, cur_column = 0;
    for (int i = 0; i < format.size(); i++)
    {
        int width = 0;
        if (cur_column < widths.size())
            width = widths[cur_column];
        int skip_tabs = 1;

        if (format[i] > '0' && format[i] < '9')
        {
            int merge_count = format[i] - '0';

            for (int j = 1; j < merge_count; j++)
                width += widths[cur_column + j];

            cur_column += merge_count - 1;
            skip_tabs = merge_count;
            i++;
        }

        if (format[i] == 'l' || format[i] == 'r' || format[i] == 'c')
        {
            if (hline || line[field_end] == HLINE_CHAR)
            {
                hline = true;
                while (width-- > 0)
                    _output.writeChar('-');
                cur_column++;
                continue;
            }

            // Print current column
            while (field_end < line.size() && line[field_end] != '\t')
                field_end++;

            int field_width = field_end - field_begin;
            width -= 2;

            int space_begin = 0, space_end = 0;
            if (format[i] == 'l')
                space_end = width - field_width;
            else if (format[i] == 'r')
                space_begin = width - field_width;
            else // format[i] == 'c'
            {
                space_begin = (width - field_width) / 2;
                space_end = width - field_width - space_begin;
            }
            space_begin++;
            space_end++;

            while (space_begin-- > 0)
                _output.writeChar(' ');
            if (field_width != 0)
                _output.write(line.ptr() + field_begin, field_width);
            while (space_end-- > 0)
                _output.writeChar(' ');

            // Go to next column
            cur_column++;
            while (skip_tabs-- > 0)
            {
                while (field_end < line.size() && line[field_end] != '\t')
                    field_end++;
                if (field_end < line.size() - 1)
                    field_end++;
            }
            field_begin = field_end;
        }
        else
            _output.writeChar(format[i]);
    }
    _output.printf("\n");
}

void SmartTableOutput::printHLine()
{
    if (!_use_smart_printing)
        return;
    if (_active_line->size() != 0)
        _active_line = &_lines.push();

    _active_line->push((char)HLINE_CHAR);
    _active_line = &_lines.push();
    _line_format_index.push(_line_formats.size() - 1);
}
