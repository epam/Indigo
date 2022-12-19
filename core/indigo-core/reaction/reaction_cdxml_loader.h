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

#ifndef __reaction_cdxml_loader__
#define __reaction_cdxml_loader__

#include "base_cpp/exception.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/query_molecule.h"
#include "reaction/base_reaction.h"

namespace indigo
{

    class Scanner;
    class Reaction;

    class ReactionCdxmlLoader
    {
    public:
        DECL_ERROR;

        ReactionCdxmlLoader(Scanner& scanner, bool is_binar = false);
        ~ReactionCdxmlLoader();

        void loadReaction(BaseReaction& rxn);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;
        std::set<int> reactants_ids;
        std::set<int> products_ids;
        std::set<int> intermediates_ids;
        std::set<int> arrows_ids;
        std::set<int> agents_ids;

    private:
        ReactionCdxmlLoader(const ReactionCdxmlLoader&); // no implicit copy
        void _initReaction(BaseReaction& rxn);
        void _parseStep(CDXProperty prop);
        Reaction* _prxn;
        QueryReaction* _pqrxn;
        Molecule _mol;
        QueryMolecule _qmol;
        BaseMolecule* _pmol;
        std::map<int, CDXElement> _cdxml_elements;
        Scanner& _scanner;
        bool _is_binary;
    };

}; // namespace indigo

#endif
