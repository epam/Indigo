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

#ifndef __icr_saver__
#define __icr_saver__

#include "base_cpp/exception.h"

namespace indigo
{
    class Reaction;
    class Output;

    class IcrSaver
    {
    public:
        static const char *VERSION1, *VERSION2;

        static bool checkVersion(const char* prefix);

        explicit IcrSaver(Output& output);

        void saveReaction(Reaction& reaction);

        bool save_xyz;
        bool save_bond_dirs;
        bool save_highlighting;
        bool save_ordering;

        DECL_ERROR;

    protected:
        Output& _output;

    private:
        IcrSaver(const IcrSaver&); // no implicit copy
    };

} // namespace indigo

#endif
