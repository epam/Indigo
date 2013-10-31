#include "bingo_index.h"

#include <sstream>
#include <string>

using namespace bingo;

MoleculeIndex::MoleculeIndex() : BaseIndex(MOLECULE)
{
}

Matcher* MoleculeIndex::createMatcher (const char *type, MatcherQueryData *query_data, const char *options)
{
   if (strcmp(type, "sub") == 0)
   {
      AutoPtr<MoleculeSubMatcher> matcher(new MoleculeSubMatcher(*this));
      matcher->setOptions(options);
      matcher->setQueryData(dynamic_cast<SubstructureQueryData *>(query_data));
      return matcher.release();
   }
   else if (strcmp(type, "sim") == 0)
   {
      AutoPtr<MoleculeSimMatcher> matcher(new MoleculeSimMatcher(*this));
      matcher->setOptions(options);
      matcher->setQueryData(dynamic_cast<SimilarityQueryData *>(query_data));
      return matcher.release();
   }
   else if (strcmp(type, "exact") == 0)
   {
      AutoPtr<MolExactMatcher> matcher(new MolExactMatcher(*this));
      matcher->setOptions(options);
      matcher->setQueryData(dynamic_cast<ExactQueryData *>(query_data));
      return matcher.release();
   }
   else
      throw Exception("createMatcher: undefined type");

   return 0;
}

ReactionIndex::ReactionIndex () : BaseIndex(REACTION)
{
}

Matcher* ReactionIndex::createMatcher (const char *type, MatcherQueryData *query_data, const char *options)
{
   if (strcmp(type, "sub") == 0)
   {
      AutoPtr<ReactionSubMatcher> matcher(new ReactionSubMatcher(*this));
      matcher->setOptions(options);
      matcher->setQueryData(dynamic_cast<SubstructureQueryData *>(query_data));
      return matcher.release();
   }
   else if (strcmp(type, "sim") == 0)
   {
      AutoPtr<ReactionSimMatcher> matcher(new ReactionSimMatcher(*this));
      matcher->setOptions(options);
      matcher->setQueryData(dynamic_cast<SimilarityQueryData *>(query_data));
      return matcher.release();
   }
   else if (strcmp(type, "exact") == 0)
   {
      AutoPtr<RxnExactMatcher> matcher(new RxnExactMatcher(*this));
      matcher->setOptions(options);
      matcher->setQueryData(dynamic_cast<ExactQueryData *>(query_data));
      return matcher.release();
   }
   else
      throw Exception("createMatcher: undefined type");

   return 0;
}

