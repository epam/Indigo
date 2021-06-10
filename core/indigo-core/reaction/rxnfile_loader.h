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

#ifndef __rxnfile_loader__
#define __rxnfile_loader__

#include "base_cpp/exception.h"
#include "molecule/molecule_stereocenter_options.h"

namespace indigo
{

    class Scanner;
    class BaseReaction;
    class Reaction;
    class QueryReaction;
    class MolfileLoader;

    class DLLEXPORT RxnfileLoader
    {
    public:
        RxnfileLoader(Scanner& scanner);
        ~RxnfileLoader();

        void loadReaction(Reaction& reaction);
        void loadQueryReaction(QueryReaction& reaction);

        bool treat_x_as_pseudoatom;
        StereocentersOptions stereochemistry_options;
        bool ignore_noncritical_query_features;
        bool ignore_no_chiral_flag;
        int treat_stereo_as;
        bool ignore_bad_valence;

        DECL_ERROR;

    protected:
        BaseReaction* _brxn;
        QueryReaction* _qrxn;
        Reaction* _rxn;

        void _loadReaction();

        Scanner& _scanner;
        void _readRxnHeader();
        void _readReactantsHeader();
        void _readProductsHeader();
        void _readCatalystsHeader();
        void _readReactantsFooter();
        void _readProductsFooter();
        void _readCatalystsFooter();
        void _readMolHeader();
        void _readMol(MolfileLoader& loader, int index);
        int _n_reactants;
        int _n_products;
        int _n_catalysts;
        bool _v3000;
    };

} // namespace indigo

#endif
