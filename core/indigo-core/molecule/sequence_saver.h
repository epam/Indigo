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

#ifndef __sequence_saver__
#define __sequence_saver__

#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Output;
    class BaseMolecule;

    class DLLEXPORT SequenceSaver
    {
    public:
        DECL_ERROR;

        SequenceSaver(Output& output);
        ~SequenceSaver();

        void saveMolecule(BaseMolecule& mol);

    private:
        SequenceSaver(const SequenceSaver&); // no implicit copy
        Output& _output;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
