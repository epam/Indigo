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

#ifndef __molecule_stereocenter_iterator__
#define __molecule_stereocenter_iterator__

#include "base_c/defs.h"
#include "molecule/molecule_stereocenters.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT StereocenterIterator
    {
        StereocenterIterator(MoleculeStereocenters::Container::const_iterator it);

        MoleculeStereocenters::Container::const_iterator _iterator;

        friend class MoleculeStereocenters;

    public:
        bool operator==(const StereocenterIterator& rhs) const;

        bool operator!=(const StereocenterIterator& rhs) const;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_stereocenter_iterator__
