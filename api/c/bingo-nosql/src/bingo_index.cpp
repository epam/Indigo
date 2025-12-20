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

#include "bingo_index.h"

#include <sstream>
#include <string>

using namespace bingo;

MoleculeIndex::MoleculeIndex() : BaseIndex(IndexType::MOLECULE)
{
}

std::unique_ptr<Matcher> MoleculeIndex::createMatcher(const char* type, MatcherQueryData* query_data, const char* options)
{
    if (strcmp(type, "sub") == 0)
    {
        std::unique_ptr<MoleculeSubMatcher> matcher = std::make_unique<MoleculeSubMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SubstructureQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "sim") == 0)
    {
        std::unique_ptr<MoleculeSimMatcher> matcher = std::make_unique<MoleculeSimMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "exact") == 0)
    {
        std::unique_ptr<MolExactMatcher> matcher = std::make_unique<MolExactMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<ExactQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "formula") == 0)
    {
        std::unique_ptr<MolGrossMatcher> matcher = std::make_unique<MolGrossMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<GrossQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "enum") == 0)
    {
        return std::make_unique<EnumeratorMatcher>(*this);
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

std::unique_ptr<Matcher> MoleculeIndex::createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        std::unique_ptr<MoleculeSimMatcher> matcher = std::make_unique<MoleculeSimMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

std::unique_ptr<Matcher> MoleculeIndex::createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit)
{
    if (strcmp(type, "sim") == 0)
    {
        std::unique_ptr<MoleculeTopNSimMatcher> matcher = std::make_unique<MoleculeTopNSimMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        matcher->setLimit(limit);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

std::unique_ptr<Matcher> MoleculeIndex::createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit,
                                                                   IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        std::unique_ptr<MoleculeTopNSimMatcher> matcher = std::make_unique<MoleculeTopNSimMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        matcher->setLimit(limit);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

ReactionIndex::ReactionIndex() : BaseIndex(IndexType::REACTION)
{
}

std::unique_ptr<Matcher> ReactionIndex::createMatcher(const char* type, MatcherQueryData* query_data, const char* options)
{
    if (strcmp(type, "sub") == 0)
    {
        std::unique_ptr<ReactionSubMatcher> matcher = std::make_unique<ReactionSubMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SubstructureQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "sim") == 0)
    {
        std::unique_ptr<ReactionSimMatcher> matcher = std::make_unique<ReactionSimMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "exact") == 0)
    {
        std::unique_ptr<RxnExactMatcher> matcher = std::make_unique<RxnExactMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<ExactQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "enum") == 0)
    {
        return std::make_unique<EnumeratorMatcher>(*this);
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

std::unique_ptr<Matcher> ReactionIndex::createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        std::unique_ptr<ReactionSimMatcher> matcher = std::make_unique<ReactionSimMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

std::unique_ptr<Matcher> ReactionIndex::createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit)
{
    if (strcmp(type, "sim") == 0)
    {
        std::unique_ptr<ReactionTopNSimMatcher> matcher = std::make_unique<ReactionTopNSimMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        matcher->setLimit(limit);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

std::unique_ptr<Matcher> ReactionIndex::createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit,
                                                                   IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        std::unique_ptr<ReactionTopNSimMatcher> matcher = std::make_unique<ReactionTopNSimMatcher>(*this);
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        matcher->setLimit(limit);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}
