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

#ifndef __ket_annotation__
#define __ket_annotation__

#include "molecule/ket_obj_with_props.h"
#include <optional>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT KetObjectAnnotation : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        const std::map<std::string, int>& getStringPropStrToIdx() const override;

        enum class StringProps
        {
            text
        };
    };

    class DLLEXPORT KetAnnotation : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        void copy(const KetAnnotation& other);

        void setExtended(const rapidjson::Document& extended);

        const std::optional<rapidjson::Document>& extended() const
        {
            return _extended;
        };

    private:
        std::optional<rapidjson::Document> _extended;
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif