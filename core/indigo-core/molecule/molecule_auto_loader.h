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

#ifndef __molecule_auto_loader__
#define __molecule_auto_loader__

#include "base_cpp/array.h"
#include "base_cpp/properties_map.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_stereocenter_options.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Scanner;
    class Molecule;
    class QueryMolecule;
    class BaseMolecule;

    class DLLEXPORT MoleculeAutoLoader
    {
    public:
        MoleculeAutoLoader(Scanner& scanner);
        MoleculeAutoLoader(const Array<char>& arr);
        MoleculeAutoLoader(const char* str);

        ~MoleculeAutoLoader();

        void loadMolecule(BaseMolecule& mol);
        // to keep C++ API compatible
        void loadQueryMolecule(QueryMolecule& qmol);

        StereocentersOptions stereochemistry_options;
        bool ignore_cistrans_errors;
        bool ignore_closing_bond_direction_mismatch;
        bool ignore_noncritical_query_features;
        bool treat_x_as_pseudoatom;
        bool skip_3d_chirality;
        bool ignore_no_chiral_flag;
        bool ignore_bad_valence;
        int treat_stereo_as;
        bool dearomatize_on_load;
        AromaticityOptions arom_options;

        // Loaded properties
        // CP_DECL;
        // TL_CP_DECL(PropertiesMap, properties);
        PropertiesMap properties;

        DECL_ERROR;

        static bool tryMDLCT(Scanner& scanner, Array<char>& outbuf);
        static void readAllDataToString(Scanner& scanner, Array<char>& dataBuf);

    protected:
        Scanner* _scanner;
        bool _own_scanner;

        void _init();
        bool _isSingleLine();
        void _loadMolecule(BaseMolecule& mol);

    private:
        MoleculeAutoLoader(const MoleculeAutoLoader&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
