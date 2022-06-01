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

#include "IndigoObject.h"

namespace indigo_cpp
{
    class IndigoCopyableObject : public IndigoObject
    {
    protected:
        IndigoCopyableObject(int id, IndigoSessionPtr session);
        IndigoCopyableObject(IndigoCopyableObject&&) = default;
        IndigoCopyableObject& operator=(IndigoCopyableObject&&) = default;
        IndigoCopyableObject(const IndigoCopyableObject&);
        IndigoCopyableObject& operator=(const IndigoCopyableObject&);
        ~IndigoCopyableObject() override = default;

        static int _cloneAndReturnId(const IndigoObject& other);
    };
}
