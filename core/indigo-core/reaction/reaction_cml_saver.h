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

#ifndef __reaction_cml_saver__
#define __reaction_cml_saver__

#include "base_cpp/exception.h"

namespace indigo
{

    class Output;
    class Reaction;
    class BaseReaction;

    class ReactionCmlSaver
    {
    public:
        explicit ReactionCmlSaver(Output& output);
        ~ReactionCmlSaver();

        void saveReaction(BaseReaction& rxn);
        bool skip_cml_tag; // skips <?xml> and <cml> tags

        DECL_ERROR;

    protected:
        Reaction* _rxn;
        Output& _output;

    private:
        ReactionCmlSaver(const ReactionCmlSaver&); // no implicit copy
    };

} // namespace indigo

#endif
