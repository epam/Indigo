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

#ifndef __molecule_cdx_saver_h__
#define __molecule_cdx_saver_h__

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"


namespace indigo
{

    class Molecule;
    class Output;

    class DLLEXPORT MoleculeCdxSaver
    {
    public:
        explicit MoleculeCdxSaver(Output& output);

        void saveMolecule(Molecule& mol);

        DECL_ERROR;

    protected:
        Molecule* _mol;
        Output& _output;

    private:
        MoleculeCdxSaver(const MoleculeCdxSaver&); // no implicit copy
    };

} // namespace indigo

#endif
