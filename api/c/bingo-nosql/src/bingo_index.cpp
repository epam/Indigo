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
        AutoPtr<MoleculeSubMatcher> matcher(new MoleculeSubMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SubstructureQueryData*>(query_data));
        return matcher.release();
    }
    else if (strcmp(type, "sim") == 0)
    {
        AutoPtr<MoleculeSimMatcher> matcher(new MoleculeSimMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        return matcher.release();
    }
    else if (strcmp(type, "exact") == 0)
    {
        AutoPtr<MolExactMatcher> matcher(new MolExactMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<ExactQueryData*>(query_data));
        return matcher.release();
    }
    else if (strcmp(type, "formula") == 0)
    {
        AutoPtr<MolGrossMatcher> matcher(new MolGrossMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<GrossQueryData*>(query_data));
        return matcher.release();
    }
    else if (strcmp(type, "enum") == 0)
    {
        AutoPtr<EnumeratorMatcher> matcher(new EnumeratorMatcher(*this));
        return matcher.release();
    }
    else
        throw Exception("createMatcher: undefined type");

    return 0;
}

Matcher* MoleculeIndex::createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        AutoPtr<MoleculeSimMatcher> matcher(new MoleculeSimMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        return matcher.release();
    }
    else
        throw Exception("createMatcher: undefined type");

    return 0;
}

Matcher* MoleculeIndex::createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit)
{
    if (strcmp(type, "sim") == 0)
    {
        AutoPtr<MoleculeTopNSimMatcher> matcher(new MoleculeTopNSimMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        matcher->setLimit(limit);
        return matcher.release();
    }
    else
        throw Exception("createMatcher: undefined type");

    return 0;
}

Matcher* MoleculeIndex::createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        AutoPtr<MoleculeTopNSimMatcher> matcher(new MoleculeTopNSimMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        matcher->setLimit(limit);
        return matcher.release();
    }
    else
        throw Exception("createMatcher: undefined type");

    return 0;
}

ReactionIndex::ReactionIndex() : BaseIndex(REACTION)
{
}

Matcher* ReactionIndex::createMatcher(const char* type, MatcherQueryData* query_data, const char* options)
{
    if (strcmp(type, "sub") == 0)
    {
        AutoPtr<ReactionSubMatcher> matcher(new ReactionSubMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SubstructureQueryData*>(query_data));
        return matcher.release();
    }
    else if (strcmp(type, "sim") == 0)
    {
        AutoPtr<ReactionSimMatcher> matcher(new ReactionSimMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        return matcher.release();
    }
    else if (strcmp(type, "exact") == 0)
    {
        AutoPtr<RxnExactMatcher> matcher(new RxnExactMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<ExactQueryData*>(query_data));
        return matcher.release();
    }
    else if (strcmp(type, "enum") == 0)
    {
        AutoPtr<EnumeratorMatcher> matcher(new EnumeratorMatcher(*this));
        return matcher.release();
    }
    else
        throw Exception("createMatcher: undefined type");

    return 0;
}

Matcher* ReactionIndex::createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        AutoPtr<ReactionSimMatcher> matcher(new ReactionSimMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        return matcher.release();
    }
    else
        throw Exception("createMatcher: undefined type");

    return 0;
}

Matcher* ReactionIndex::createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit)
{
    if (strcmp(type, "sim") == 0)
    {
        AutoPtr<ReactionTopNSimMatcher> matcher(new ReactionTopNSimMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryData(dynamic_cast<SimilarityQueryData*>(query_data));
        matcher->setLimit(limit);
        return matcher.release();
    }
    else
        throw Exception("createMatcher: undefined type");

    return 0;
}

Matcher* ReactionIndex::createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit, IndigoObject& fp)
{
    if (strcmp(type, "sim") == 0)
    {
        AutoPtr<ReactionTopNSimMatcher> matcher(new ReactionTopNSimMatcher(*this));
        matcher->setOptions(options);
        matcher->setQueryDataWithExtFP(dynamic_cast<SimilarityQueryData*>(query_data), fp);
        matcher->setLimit(limit);
        return matcher.release();
    }
    else
        throw Exception("createMatcher: undefined type");

    return 0;
}
