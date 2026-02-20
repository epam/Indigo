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

#ifndef __macro_properties_calculator__
#define __macro_properties_calculator__

#include "common/base_cpp/output.h"
#include "molecule/ket_document.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class DLLEXPORT MacroPropertiesCalculator
    {
    public:
        DECL_ERROR;
        void CalculateMacroProps(KetDocument& document, Output& output, float upc, float nac, bool pretty_json = false) const;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
