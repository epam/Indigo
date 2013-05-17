#include "bingo_index.h"

#include <sstream>
#include <string>

using namespace bingo;

MoleculeIndex::MoleculeIndex() : BaseIndex(MOLECULE)
{
}

Matcher* MoleculeIndex::createMatcher (const char *type, MatcherQueryData *query_data)
{
   if (strcmp(type, "sub") == 0)
   {
      AutoPtr<MoleculeSubMatcher> matcher(new MoleculeSubMatcher(*this));
      matcher->setQueryData(dynamic_cast<SubstructureQueryData *>(query_data));
      return matcher.release();
   }
   else if (strcmp(type, "sim") == 0)
   {
      AutoPtr<MoleculeSimMatcher> matcher(new MoleculeSimMatcher(*this));
      matcher->setQueryData(dynamic_cast<SimilarityQueryData *>(query_data));
      return matcher.release();
   }
   else
      throw Exception("createMatcher: undefined type");

   return 0;
}

ReactionIndex::ReactionIndex () : BaseIndex(REACTION)
{
}

Matcher* ReactionIndex::createMatcher (const char *type, MatcherQueryData *query_data)
{
   if (strcmp(type, "sub") == 0)
   {
      AutoPtr<ReactionSubMatcher> matcher(new ReactionSubMatcher(*this));
      matcher->setQueryData(dynamic_cast<SubstructureQueryData *>(query_data));
      return matcher.release();
   }
   else if (strcmp(type, "sim") == 0)
   {
      AutoPtr<ReactionSimMatcher> matcher(new ReactionSimMatcher(*this));
      matcher->setQueryData(dynamic_cast<SimilarityQueryData *>(query_data));
      return matcher.release();
   }
   else
      throw Exception("createMatcher: undefined type");

   return 0;
}

