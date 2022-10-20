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

#ifndef __reaction_auto_loader__
#define __reaction_auto_loader__

#include "base_cpp/array.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_stereocenter_options.h"

namespace indigo
{

    class Scanner;
    class BaseReaction;
    class Reaction;
    class QueryReaction;

    class DLLEXPORT ReactionAutoLoader
    {
    public:
        ReactionAutoLoader(Scanner& scanner);
        ReactionAutoLoader(const Array<char>& arr);
        ReactionAutoLoader(const char*);

        ~ReactionAutoLoader();

        void loadReaction(BaseReaction& reaction);
        // to keep C++ API compatible
        void loadQueryReaction(QueryReaction& qreaction);

        bool treat_x_as_pseudoatom;
        bool ignore_closing_bond_direction_mismatch;
        StereocentersOptions stereochemistry_options;
        bool ignore_cistrans_errors;
        bool ignore_noncritical_query_features;
        bool ignore_no_chiral_flag;
        bool ignore_bad_valence;
        bool dearomatize_on_load;
        AromaticityOptions arom_options;

        DECL_ERROR;

    protected:
        Scanner* _scanner;
        bool _own_scanner;

        void _init();
        void _loadReaction(BaseReaction& reaction);
        bool _isSingleLine();

    private:
        ReactionAutoLoader(const ReactionAutoLoader&); // no implicit copy
    };

} // namespace indigo

#endif
