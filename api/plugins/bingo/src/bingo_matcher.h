#ifndef __bingo_matcher__
#define __bingo_matcher__

#include "bingo_object.h"
#include "bingo_base_index.h"

#include "molecule/molecule_substructure_matcher.h"

using namespace indigo;

namespace bingo
{
   class Matcher
   {
   public:
      virtual bool next () = 0;
      virtual int currentId () = 0;
      virtual const char * currentCf ( int &len ) = 0;
      virtual const Index & getIndex () = 0;

      virtual ~Matcher () {};
   };
   
   class BaseMatcher : public Matcher
   {
   public:
      BaseMatcher(BaseIndex &index);

      virtual int currentId ();

      virtual const char * currentCf ( int &len );

      virtual const Index & getIndex ();

   protected:
      BaseIndex &_index;
      int _current_id;

      virtual ~BaseMatcher () {};
   };

   class MatcherQueryData
   {
   public:
      virtual /*const*/ QueryObject &getQueryObject () /*const*/ = 0;

      virtual ~MatcherQueryData () {};
   };

   class SimilarityQueryData : public MatcherQueryData
   {
   public:
      virtual float getMin () const = 0;
      virtual float getMax () const = 0;
   };

   class SubstructureQueryData : public MatcherQueryData
   {
   };

   class MoleculeSimilarityQueryData : public SimilarityQueryData
   {
   public:
      MoleculeSimilarityQueryData (/* const */ Molecule &mol, float min_coef, float max_coef);

      virtual /*const*/ QueryObject &getQueryObject () /*const*/ ;

      virtual float getMin () const ;

      virtual float getMax () const ;

   private:
      SimilarityMoleculeQuery _obj;
      float _min;
      float _max;
   };

   class ReactionSimilarityQueryData : public SimilarityQueryData
   {
   public:
      ReactionSimilarityQueryData (/* const */ Reaction &rxn, float min_coef, float max_coef);

      virtual /*const*/ QueryObject &getQueryObject () /*const*/ ;

      virtual float getMin () const ;

      virtual float getMax () const ;

   protected:
      SimilarityReactionQuery _obj;
      float _min;
      float _max;
   };

   class MoleculeSubstructureQueryData : public SubstructureQueryData
   {
   public:
      MoleculeSubstructureQueryData (/* const */ QueryMolecule &qmol);

      virtual /*const*/ QueryObject &getQueryObject () /*const*/;

   private:
      SubstructureMoleculeQuery _obj;
   };
   
   class ReactionSubstructureQueryData : public SubstructureQueryData
   {
   public:
      ReactionSubstructureQueryData (/* const */ QueryReaction &qrxn);

      virtual /*const*/ QueryObject &getQueryObject () /*const*/;

   private:
      SubstructureReactionQuery _obj;
   };

   class BaseSubstructureMatcher : public BaseMatcher
   {
   public:
      BaseSubstructureMatcher (/*const */ BaseIndex &index);
   
      virtual bool next ();

      void setQueryData (SubstructureQueryData *query_data);

   protected:
      int _fp_size;
      int _cand_count;
      /*const*/ AutoPtr<SubstructureQueryData> _query_data;
      Array<byte> _query_fp;

      void _findPackCandidates (int pack_idx);

      void _findIncCandidates ();

      virtual bool _tryCurrent ()/* const */ = 0;

   private:
      Array<int> _candidates;
      int _current_cand_id;
      int _current_pack;
      const TranspFpStorage &_fp_storage;
   };

   class MoleculeSubMatcher : public BaseSubstructureMatcher
   {
   public:
      MoleculeSubMatcher (/*const */ BaseIndex &index);

      const Array<int> & currentMapping ();

   private:
      Array<int> _mapping;

      virtual bool _tryCurrent () /*const*/;
   };
   
   class ReactionSubMatcher : public BaseSubstructureMatcher
   {
   public:
      ReactionSubMatcher(/*const */ BaseIndex &index);

      const ObjArray<Array<int> > & currentMapping ();

   private:
      ObjArray<Array<int> > _mapping;

      virtual bool _tryCurrent () /*const*/;
   };
   
   class SimMatcher : public BaseMatcher
   {
   public:
      SimMatcher (BaseIndex &index);

      virtual bool next ();
      
      void setQueryData (SimilarityQueryData *query_data);

      ~SimMatcher();

   private:
      /* const */ AutoPtr<SimilarityQueryData> _query_data;
      int _fp_size;

      byte *_current_block;
      const byte *_cur_loc;
      Array<byte> _query_fp;

      float _calcTanimoto (const byte *fp);
   };
};

#endif // __bingo_matcher__
