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
