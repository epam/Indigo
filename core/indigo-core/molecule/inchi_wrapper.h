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

#ifndef __inchi_wrapper_h__
#define __inchi_wrapper_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

#include "inchi_api.h"

#include <string>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    // Forward declaration
    class Molecule;
    struct InchiOutput;

    class DLLEXPORT InchiWrapper
    {
    public:
        InchiWrapper();

        void clear();

        // Input parameters
        void setOptions(const char* opt);
        void getOptions(Array<char>& value);

        // Output additional results
        Array<char> warning, log, auxInfo;

        void loadMoleculeFromInchi(const char* inchi, Molecule& mol);
        void loadMoleculeFromAux(const char* aux, Molecule& mol);

        void saveMoleculeIntoInchi(Molecule& mol, Array<char>& inchi);

        void parseInchiOutput(const InchiOutput& inchi_output, Molecule& mol);

        void generateInchiInput(Molecule& mol, inchi_Input& input, Array<inchi_Atom>& atoms, Array<inchi_Stereo0D>& stereo);

        void neutralizeV5Nitrogen(Molecule& mol);

        static const char* version();

        static void InChIKey(const char* inchi, Array<char>& output);

        DECL_EXCEPTION_NO_EXP(Error);

    protected:
        enum
        {
            _STEREO_ABS = 1,
            _STEREO_REL,
            _STEREO_RAC
        };

    private:
        Array<char> options;
        int _stereo_opt;
    };

} // namespace indigo

#endif // __inchi_wrapper_h__
