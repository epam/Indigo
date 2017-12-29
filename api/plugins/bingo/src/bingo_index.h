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
      virtual Matcher* createMatcherWithExtFP (const char *type, MatcherQueryData *query_data, const char *options, IndigoObject &fp);
      virtual Matcher* createMatcherTopN (const char *type, MatcherQueryData *query_data, const char *options, int limit);
   };
   
   class ReactionIndex : public BaseIndex
   {
   public:
      ReactionIndex();

      virtual Matcher* createMatcher (const char *type, MatcherQueryData *query_data, const char *options);
      virtual Matcher* createMatcherWithExtFP (const char *type, MatcherQueryData *query_data, const char *options, IndigoObject &fp);
      virtual Matcher* createMatcherTopN (const char *type, MatcherQueryData *query_data, const char *options, int limit);
   };
};

#endif // __bingo_index__
