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

#ifndef __reaction_cml_loader__
#define __reaction_cml_loader__

#include "base_cpp/exception.h"
#include "molecule/molecule_stereocenter_options.h"

namespace indigo
{

    class Scanner;
    class Reaction;

    class ReactionCmlLoader
    {
    public:
        DECL_ERROR;

        ReactionCmlLoader(Scanner& scanner);
        ~ReactionCmlLoader();

        void loadReaction(Reaction& rxn);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;

    protected:
        Scanner& _scanner;

    private:
        ReactionCmlLoader(const ReactionCmlLoader&); // no implicit copy
    };

}; // namespace indigo

#endif
