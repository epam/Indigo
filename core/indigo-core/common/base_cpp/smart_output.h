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

#ifndef __smart_output_h__
#define __smart_output_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/output.h"
#include "base_cpp/reusable_obj_array.h"
#include "base_cpp/tlscont.h"

namespace indigo
{

    class SmartTableOutput : public Output
    {
    public:
        SmartTableOutput(Output& output, bool use_smart_printing);
        ~SmartTableOutput() override;

        void write(const void* data, int size) override;
        void flush() override;

        void setLineFormat(const char* line_format);
        void printHLine();

        enum
        {
            HLINE_CHAR = '\a'
        };

        DECL_ERROR;

    private:
        void _updateColumnWidths(int index, Array<int>& widths);
        void _printLineSmart(int index, const Array<int>& widths);

        CP_DECL;
        TL_CP_DECL(ReusableObjArray<Array<char>>, _lines);
        TL_CP_DECL(ReusableObjArray<Array<char>>, _line_formats);
        TL_CP_DECL(Array<int>, _line_format_index);

        Array<char>* _active_line;
        bool _use_smart_printing;
        Output& _output;
    };

} // namespace indigo

#endif // __smart_output_h__
