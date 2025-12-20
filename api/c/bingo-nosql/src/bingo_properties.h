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

#ifndef __bingo_parameters__
#define __bingo_parameters__

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "mmf/mmf_array.h"
#include "mmf/mmf_ptr.h"

namespace bingo
{
    class Properties
    {
    public:
        Properties();

        static MMFAddress create(MMFPtr<Properties>& ptr);

        static void load(MMFPtr<Properties>& ptr, MMFAddress offset);

        static void parseOptions(const char* options, std::map<std::string, std::string>& option_map, std::vector<std::string>* allowed_props = 0);

        void add(const char* prop_name, const char* value);

        void add(const char* prop_name, unsigned long value);

        const char* get(const char* prop_name);

        const char* getNoThrow(const char* prop_name) const;

        unsigned long getULong(const char* prop_name);

        unsigned long getULongNoThrow(const char* prop_name);

    private:
        struct _PropertyPair
        {
            MMFPtr<char> name;
            MMFPtr<char> value;
        };

        void _rewritePropFile();

        static void _parseProperty(const std::string& line, std::string& prop_out, std::string& value_out);

        static const int max_prop_len = 1024;
        MMFArray<_PropertyPair> _props;
    };
}; // namespace bingo

#endif /* __bingo_parameters__ */
