/****************************************************************************
 * Copyright (C) from 2024 to Present EPAM Systems.
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

#include "molecule/idt_alias.h"

namespace indigo
{
    IMPL_ERROR(IdtAlias, "IDT alias");

    const std::string& IdtAlias::getModification(IdtModification modification) const
    {
        static std::string empty;
        switch (modification)
        {
        case IdtModification::FIVE_PRIME_END:
            return getFivePrimeEnd();
        case IdtModification::INTERNAL:
            return getInternal();
        case IdtModification::THREE_PRIME_END:
            return getThreePrimeEnd();
        };
        throw Error("Unknown IDT modification: %s.", modification);
        return empty;
    }

    const std::string& IdtAlias::getFivePrimeEnd() const
    {
        if (_five_prime_end != "")
            return _five_prime_end;
        else
            throw Error("IDT alias %s has no five-prime end modification.", _base.c_str());
    }

    const std::string& IdtAlias::getInternal() const
    {
        if (_internal != "")
            return _internal;
        else
            throw Error("IDT alias %s has no internal modification.", _base.c_str());
    }

    const std::string& IdtAlias::getThreePrimeEnd() const
    {
        if (_three_prime_end != "")
            return _three_prime_end;
        else
            throw Error("IDT alias %s has no three-prime end modification.", _base.c_str());
    }

    std::string IdtAlias::getBaseForMod(const std::string& alias)
    {
        if (alias.size() < 2)
            return alias;
        switch (alias[0])
        {
        case '3':
        case 'i':
        case '5':
            return alias.substr(1, alias.size() - 1);
        }
        return alias;
    }

}
