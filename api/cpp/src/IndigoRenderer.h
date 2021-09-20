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

#include <vector>

#include "IndigoSession.h"

namespace indigo_cpp
{
    class IndigoChemicalEntity;

    class IndigoRenderer
    {
    public:
        IndigoRenderer(const IndigoSession& session);
        ~IndigoRenderer();

        IndigoRenderer(IndigoRenderer const&) = delete;
        void operator=(IndigoRenderer const&) = delete;

        std::string svg(const IndigoChemicalEntity& data) const;
        std::vector<char> png(const IndigoChemicalEntity& data) const;

    private:
        const IndigoSession& _session;
    };
} // namespace indigo_cpp
