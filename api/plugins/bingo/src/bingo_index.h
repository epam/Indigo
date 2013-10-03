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
      
      virtual Matcher* createMatcher (const char *type, MatcherQueryData *query_data, const char *options);
   };
   
   class ReactionIndex : public BaseIndex
   {
   public:
      ReactionIndex();

      virtual Matcher* createMatcher (const char *type, MatcherQueryData *query_data, const char *options);
   };
};

#endif // __bingo_index__
