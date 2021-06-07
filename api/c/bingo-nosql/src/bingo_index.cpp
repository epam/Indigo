#include "bingo_index.h"

#include <sstream>
#include <string>

using namespace bingo;

MoleculeIndex::MoleculeIndex() : BaseIndex(MOLECULE)
{
}

Matcher* MoleculeIndex::createMatcher(const char* type, MatcherQueryData* query_data, const char* options)
{
    if (strcmp(type, "sub") == 0)
    {
        MoleculeSubMatcher* matcher = new MoleculeSubMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SubstructureQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "sim") == 0)
    {
        MoleculeSimMatcher* matcher = new MoleculeSimMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "exact") == 0)
    {
        MolExactMatcher* matcher = new MolExactMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<ExactQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "formula") == 0)
    {
        MolGrossMatcher* matcher = new MolGrossMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<GrossQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "enum") == 0)
    {
        EnumeratorMatcher* matcher = new EnumeratorMatcher(*this);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

Matcher* MoleculeIndex::createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        MoleculeSimMatcher* matcher = new MoleculeSimMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

Matcher* MoleculeIndex::createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit)
{
    if (strcmp(type, "sim") == 0)
    {
        MoleculeTopNSimMatcher* matcher = new MoleculeTopNSimMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        matcher->setLimit(limit);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

Matcher* MoleculeIndex::createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        MoleculeTopNSimMatcher* matcher = new MoleculeTopNSimMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        matcher->setLimit(limit);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

ReactionIndex::ReactionIndex() : BaseIndex(REACTION)
{
}

Matcher* ReactionIndex::createMatcher(const char* type, MatcherQueryData* query_data, const char* options)
{
    if (strcmp(type, "sub") == 0)
    {
        ReactionSubMatcher* matcher = new ReactionSubMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SubstructureQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "sim") == 0)
    {
        ReactionSimMatcher* matcher = new ReactionSimMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "exact") == 0)
    {
        RxnExactMatcher* matcher = new RxnExactMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<ExactQueryData*>(query_data));
        return matcher;
    }
    else if (strcmp(type, "enum") == 0)
    {
        return new EnumeratorMatcher(*this);
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

Matcher* ReactionIndex::createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        ReactionSimMatcher* matcher = new ReactionSimMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

Matcher* ReactionIndex::createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit)
{
    if (strcmp(type, "sim") == 0)
    {
        ReactionTopNSimMatcher* matcher = new ReactionTopNSimMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        matcher->setLimit(limit);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}

Matcher* ReactionIndex::createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        ReactionTopNSimMatcher* matcher = new ReactionTopNSimMatcher(*this);
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        matcher->setLimit(limit);
        return matcher;
    }
    else
        throw Exception("createMatcher: undefined type");

    return nullptr;
}
