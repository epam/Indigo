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

#ifndef __molecule_savers_h__
#define __molecule_savers_h__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "molecule/base_molecule.h"

namespace indigo
{

    class MoleculeSavers
    {
    public:
        static int getHCount(BaseMolecule& mol, int index, int atom_number, int atom_charge);
        static bool getRingBondCountFlagValue(QueryMolecule& qmol, int idx, int& value);
        static bool getSubstitutionCountFlagValue(QueryMolecule& qmol, int idx, int& value);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
