#include "bingo_index.h"

#include <sstream>
#include <string>

using namespace bingo;

MoleculeIndex::MoleculeIndex() : BaseIndex()
{
   _type = IND_MOL;
}

Matcher* MoleculeIndex::createMatcher (const char *type, const MatcherQueryData *query_data)
{
   if (strcmp(type, "sub") == 0)
   {
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
   
ReactionIndex::ReactionIndex() : BaseIndex()
{
   _type = IND_RXN;
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

