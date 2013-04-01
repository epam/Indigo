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

      virtual Matcher* createMatcher (const char *type, const MatcherQueryData *query_data);
   };
   
   class ReactionIndex : public BaseIndex
   {
   public:
      ReactionIndex();

      virtual Matcher* createMatcher (const char *type, const MatcherQueryData *query_data);
   };
};

#endif // __bingo_index__
