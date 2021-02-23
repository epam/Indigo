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

#ifndef __icm_saver__
#define __icm_saver__

#include "base_cpp/exception.h"

namespace indigo
{

    class Molecule;
    class Output;

    class IcmSaver
    {
    public:
        static const char *VERSION1, *VERSION2;
        static bool checkVersion(const char* prefix);

        explicit IcmSaver(Output& output);

        void saveMolecule(Molecule& mol);

        bool save_xyz;
        bool save_bond_dirs;
        bool save_highlighting;
        bool save_ordering;

        DECL_ERROR;

    protected:
        Output& _output;

    private:
        IcmSaver(const IcmSaver&); // no implicit copy
    };

} // namespace indigo

#endif
