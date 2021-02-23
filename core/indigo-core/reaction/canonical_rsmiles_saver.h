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

#ifndef __canonical_rsmiles_saver__
#define __canonical_rsmiles_saver__

#include "reaction/rsmiles_saver.h"

#include "base_cpp/exception.h"

namespace indigo
{

    class Output;
    class BaseReaction;
    class CanonicalSmilesSaver;
    class Reaction;

    class DLLEXPORT CanonicalRSmilesSaver : public RSmilesSaver
    {
    public:
        explicit CanonicalRSmilesSaver(Output& output);
        ~CanonicalRSmilesSaver();

        void saveReaction(Reaction& react);

        DECL_ERROR;

    protected:
        void _saveReaction();
        void _writeMolecule(int i, CanonicalSmilesSaver& saver);
    };

} // namespace indigo

#endif
