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

#ifndef __canonical_smiles_saver__
#define __canonical_smiles_saver__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <map>

#include "base_cpp/exception.h"
#include "molecule/smiles_saver.h"

namespace indigo
{

    class DLLEXPORT CanonicalSmilesSaver : public SmilesSaver
    {
    public:
        explicit CanonicalSmilesSaver(Output& output);
        ~CanonicalSmilesSaver();

        bool find_invalid_stereo;

        void saveMolecule(Molecule& mol);

        DECL_ERROR;

    protected:
        typedef std::map<int, int> MapIntInt;

        TL_CP_DECL(Array<int>, _actual_atom_atom_mapping);
        TL_CP_DECL(MapIntInt, _initial_to_actual);
        int _aam_counter;
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
