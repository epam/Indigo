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

#ifndef __molecule_stereocenter_options__
#define __molecule_stereocenter_options__

#include "base_c/defs.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT StereocentersOptions
    {
    public:
        StereocentersOptions();

        void reset();

        // Ignore all stereocenter errors. Default is false.
        bool ignore_errors;

        // Treat stereobond direction bond not only for a pointed stereocenter, but for the
        // neighbour as well. Default is false.
        bool bidirectional_mode;

        // Detect Haworth projection. Default is false.
        bool detect_haworth_projection;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_stereocenter_options__
