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

#ifndef __indigo_arreviations__
#define __indigo_arreviations__

#include <string>
#include <vector>

#include "base_cpp/ptr_array.h"

namespace indigo
{

    namespace abbreviations
    {

        struct Abbreviation
        {
            std::string name, expansion;
            std::vector<std::string> left_aliases, right_aliases, left_aliases2, right_aliases2;

            int connections;
        };

        class IndigoAbbreviations
        {
        public:
            IndigoAbbreviations();

            void clear();

            PtrArray<Abbreviation> abbreviations;

        private:
            void loadDefault();
        };

        IndigoAbbreviations& indigoGetAbbreviationsInstance();
        IndigoAbbreviations& indigoCreateAbbreviationsInstance();

    } // namespace abbreviations
} // namespace indigo

#endif // __indigo_arreviations__
