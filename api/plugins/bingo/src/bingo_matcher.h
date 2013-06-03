#ifndef __bingo_matcher__
#define __bingo_matcher__

#include "bingo_object.h"
#include "bingo_base_index.h"

#include "indigo_molecule.h"
#include "indigo_reaction.h"

#include "molecule/molecule_substructure_matcher.h"
#include "math/statistics.h"

using namespace indigo;

namespace bingo
{
   ///////////////////////////////////////
   // Query data classes
   ///////////////////////////////////////

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

   ///////////////////////////////////////
   // Matcher classes
   ///////////////////////////////////////

   class IndexCurrentMolecule : public IndigoMolecule
   {
   public:
      IndexCurrentMolecule ( IndexCurrentMolecule *& ptr );
      ~IndexCurrentMolecule ();

      bool matcher_exist;

   private:
      IndexCurrentMolecule *& _ptr;
   };

   class IndexCurrentReaction : public IndigoReaction
   {
   public:
      IndexCurrentReaction ( IndexCurrentReaction *& ptr );
      ~IndexCurrentReaction ();
      
      bool matcher_exist;

   private:
      IndexCurrentReaction *& _ptr;
   };

   class Matcher
   {
   public:
      virtual bool next () = 0;
      virtual int currentId () = 0;
      virtual IndigoObject * currentObject () = 0;
      virtual const Index & getIndex () = 0;

      virtual int esimateRemainingResultsCount (int &delta) = 0;
      virtual float esimateRemainingTime (float &delta) = 0;

      virtual ~Matcher () {};
   };
   
   class BaseMatcher : public Matcher
   {
   public:
      BaseMatcher(BaseIndex &index, IndigoObject *& current_obj);

      virtual int currentId ();

      virtual IndigoObject * currentObject ();

      virtual const Index & getIndex ();

      virtual int esimateRemainingResultsCount (int &delta);
      virtual float esimateRemainingTime (float &delta);

   protected:
      BaseIndex &_index;
      IndigoObject *& _current_obj;
      bool _current_obj_used;
      int _current_id;

      // Variables used for estimation
      MeanEstimator _match_probability_esimate, _match_time_esimate;

      bool _loadCurrentObject();

      ~BaseMatcher ();
   };

   class BaseSubstructureMatcher : public BaseMatcher
   {
   public:
      BaseSubstructureMatcher (/*const */ BaseIndex &index, IndigoObject *& current_obj);
   
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

      IndexCurrentMolecule *_current_mol;
   };
   
   class ReactionSubMatcher : public BaseSubstructureMatcher
   {
   public:
      ReactionSubMatcher(/*const */ BaseIndex &index);

      const ObjArray<Array<int> > & currentMapping ();
   private:
      ObjArray<Array<int> > _mapping;

      virtual bool _tryCurrent () /*const*/;

      IndexCurrentReaction *_current_rxn;
   };

   class BaseSimilarityMatcher : public BaseMatcher
   {
   public:
      BaseSimilarityMatcher (BaseIndex &index, IndigoObject *& current_obj);

      virtual bool next ();
      
      void setQueryData (SimilarityQueryData *query_data);

      ~BaseSimilarityMatcher();

   private:
      /* const */ AutoPtr<SimilarityQueryData> _query_data;
      int _fp_size;

      byte *_current_block;
      const byte *_cur_loc;
      Array<byte> _query_fp;

      float _calcTanimoto (const byte *fp);
   };


   class MoleculeSimMatcher : public BaseSimilarityMatcher
   {
   public:
      MoleculeSimMatcher (/*const */ BaseIndex &index);
   private:
      IndexCurrentMolecule *_current_mol;
   };


   class ReactionSimMatcher : public BaseSimilarityMatcher
   {
   public:
      ReactionSimMatcher(/*const */ BaseIndex &index);
   private:
      IndexCurrentReaction *_current_rxn;
   };
};

#endif // __bingo_matcher__
