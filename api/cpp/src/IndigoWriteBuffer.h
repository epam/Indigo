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
#include <vector>

#include "IndigoObject.h"

namespace indigo_cpp
{
    class IndigoWriteBuffer : public IndigoObject
    {
    public:
        IndigoWriteBuffer(IndigoWriteBuffer&&) = default;
        IndigoWriteBuffer& operator=(IndigoWriteBuffer&&) = default;
        IndigoWriteBuffer(const IndigoWriteBuffer&) = delete;
        IndigoWriteBuffer& operator=(const IndigoWriteBuffer&) = delete;
        ~IndigoWriteBuffer() override = default;

        std::vector<char> toBuffer() const;
        std::string toString() const;

    private:
        IndigoWriteBuffer(int id, IndigoSessionPtr session);
        friend class IndigoSession;
    };
}
