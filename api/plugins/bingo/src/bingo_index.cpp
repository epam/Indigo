#include "bingo_index.h"

#include <sstream>
#include <string>

using namespace bingo;

MoleculeIndex::MoleculeIndex() : BaseIndex(MOLECULE)
{
}

Matcher* MoleculeIndex::createMatcher (const char *type, const MatcherQueryData *query_data)
{
   if (strcmp(type, "sub") == 0)
   {
      // TODO: AutPtr<MoleculeSubMatcher> to avoid memory leak in 
      //   case of exception in matcher->setQueryData.

      // MR TODO: type cast with type checking based on dynamic_cast

      MoleculeSubMatcher *matcher = new MoleculeSubMatcher(*this);
      matcher->setQueryData((SubstructureQueryData *)query_data);
      return matcher;
   }
   else if (strcmp(type, "sim") == 0)
   {
      SimMatcher *matcher = new SimMatcher(*this);
      matcher->setQueryData((SimilarityQueryData *)query_data);
      return matcher;
   }
   else
      throw Exception("createMatcher: undefined type");

   return 0;
}
   
ReactionIndex::ReactionIndex() : BaseIndex(REACTION)
{
}

Matcher* ReactionIndex::createMatcher (const char *type, const MatcherQueryData *query_data)
{
   if (strcmp(type, "sub") == 0)
   {
      ReactionSubMatcher *matcher = new ReactionSubMatcher(*this);
      matcher->setQueryData((SubstructureQueryData *)query_data);
      return matcher;
   }
   else if (strcmp(type, "sim") == 0)
   {
      SimMatcher *matcher = new SimMatcher(*this);
      matcher->setQueryData((SimilarityQueryData *)query_data);
      return matcher;
   }
   else
      throw Exception("createMatcher: undefined type");

   return 0;
}

