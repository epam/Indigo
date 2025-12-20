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

#ifndef __bingo_index__
#define __bingo_index__

#include "bingo_base_index.h"
#include "bingo_matcher.h"

namespace bingo
{
    class MoleculeIndex final : public BaseIndex
    {
    public:
        MoleculeIndex();

        std::unique_ptr<Matcher> createMatcher(const char* type, MatcherQueryData* query_data, const char* options) final;
        std::unique_ptr<Matcher> createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp) final;
        std::unique_ptr<Matcher> createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit) final;
        std::unique_ptr<Matcher> createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit,
                                                            IndigoObject& fp) final;
    };

    class ReactionIndex final : public BaseIndex
    {
    public:
        ReactionIndex();

        std::unique_ptr<Matcher> createMatcher(const char* type, MatcherQueryData* query_data, const char* options) final;
        std::unique_ptr<Matcher> createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp) final;
        std::unique_ptr<Matcher> createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit) final;
        std::unique_ptr<Matcher> createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit,
                                                            IndigoObject& fp) final;
    };
} // namespace bingo

#endif // __bingo_index__
