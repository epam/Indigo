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

#ifndef __molecule_json_loader__
#define __molecule_json_loader__

#include "base_c/defs.h"
#include "base_cpp/exception.h"
#include "base_cpp/non_copyable.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Scanner;
    class Molecule;
    class QueryMolecule;

    /*
     * Loader for JSON format
     */

    class DLLEXPORT MoleculeJsonLoader : public NonCopyable
    {
    public:
        DECL_ERROR;

        explicit MoleculeJsonLoader(Scanner& scanner);
        void loadMolecule(Molecule& mol);
        void loadQueryMolecule(QueryMolecule& qmol);

    private:
        Scanner& _scanner;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
