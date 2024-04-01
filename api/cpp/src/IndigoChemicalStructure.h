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

#include <string>

#include "IndigoCopyableObject.h"
#include "IndigoObject.h"

namespace indigo_cpp
{
    class IndigoChemicalStructure : public IndigoCopyableObject
    {
    public:
        void aromatize();

        void dearomatize();

        void layout();

        void clean2d();

        std::string canonicalSmiles() const;

        std::string smiles() const;

        std::string smarts() const;

        std::string cml() const;

        std::string inchi() const;

        virtual std::string ctfile() const = 0;

        std::string rawData() const;

        std::string name() const;

    protected:
        IndigoChemicalStructure(int id, IndigoSessionPtr session);
        IndigoChemicalStructure(IndigoChemicalStructure&&) = default;
        IndigoChemicalStructure& operator=(IndigoChemicalStructure&&) = default;
        IndigoChemicalStructure(const IndigoChemicalStructure& other);
        IndigoChemicalStructure& operator=(const IndigoChemicalStructure&) = default;
        ~IndigoChemicalStructure() override = default;
    };
} // namespace indigo_cpp
