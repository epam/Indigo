#ifndef __bingo_index__
#define __bingo_index__

#include "bingo_base_index.h"
#include "bingo_matcher.h"

namespace bingo
{
    class MoleculeIndex : public BaseIndex
    {
    public:
        MoleculeIndex();

        Matcher* createMatcher(const char* type, MatcherQueryData* query_data, const char* options) override;
        Matcher* createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp) override;
        Matcher* createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit) override;
        Matcher* createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit, IndigoObject& fp) override;
    };

    class ReactionIndex : public BaseIndex
    {
    public:
        ReactionIndex();

        Matcher* createMatcher(const char* type, MatcherQueryData* query_data, const char* options) override;
        Matcher* createMatcherWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, IndigoObject& fp) override;
        Matcher* createMatcherTopN(const char* type, MatcherQueryData* query_data, const char* options, int limit) override;
        Matcher* createMatcherTopNWithExtFP(const char* type, MatcherQueryData* query_data, const char* options, int limit, IndigoObject& fp) override;
    };
}; // namespace bingo

#endif // __bingo_index__
