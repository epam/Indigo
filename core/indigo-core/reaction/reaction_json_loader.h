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

#ifndef __reaction_json_loader__
#define __reaction_json_loader__

#include <rapidjson/document.h>

#include "base_cpp/exception.h"
#include "molecule/molecule.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/query_molecule.h"

namespace indigo
{

    class Scanner;
    class BaseReaction;
    class Reaction;
    class QueryReaction;
    class QueryMolecule;

    class ReactionJsonLoader
    {
    public:
        DECL_ERROR;

        ReactionJsonLoader(rapidjson::Document& ket);
        ~ReactionJsonLoader();

        void loadReaction(BaseReaction& rxn);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;
        bool ignore_noncritical_query_features;
        bool treat_x_as_pseudoatom;
        bool ignore_no_chiral_flag;

    protected:
        rapidjson::Value _molecule;
        rapidjson::Value _rgroups;
        rapidjson::Value _pluses;
        rapidjson::Value _arrows;
        rapidjson::Value _simple_objects;

    private:
        ReactionJsonLoader(const ReactionJsonLoader&); // no implicit copy
        Reaction* _prxn;
        QueryReaction* _pqrxn;
        Molecule _mol;
        QueryMolecule _qmol;
        BaseMolecule* _pmol;
    };

} // namespace indigo

#endif /* reaction_json_loader_h */
