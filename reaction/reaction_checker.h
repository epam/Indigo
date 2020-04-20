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

#ifndef __reaction_checker__
#define __reaction_checker__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo
{

    class Output;
    class Reaction;
    class BaseReaction;
    class QueryReaction;
    class StructureChecker;

    class ReactionChecker
    {
    public:
        ReactionChecker(Output& output);
        ~ReactionChecker();

        void checkBaseReaction(BaseReaction& reaction);
        void checkReaction(Reaction& reaction);
        void checkQueryReaction(QueryReaction& reaction);

        void setCheckTypes(const char* params);

        DECL_ERROR;

    protected:
        void _checkReaction();
        void _checkReactionComponent(StructureChecker& checker, int idx);
        void _addCheckResult(StructureChecker& checker, int idx);

        BaseReaction* _brxn;
        QueryReaction* _qrxn;
        Reaction* _rxn;

        Output& _output;

        Array<char> _check_types;
    };

} // namespace indigo

#endif
