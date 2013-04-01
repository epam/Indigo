#ifndef __bingo_object__
#define __bingo_object__

//#include "base_c\defs.h"
#include "base_cpp\output.h"
#include "base_cpp\scanner.h"

#include "reaction\reaction.h"
#include "reaction\query_reaction.h"
#include "reaction\crf_saver.h"
#include "reaction\crf_loader.h"
#include "reaction\reaction_fingerprint.h"
#include "reaction\reaction_substructure_matcher.h"

#include "molecule\molecule.h"
#include "molecule\cmf_saver.h"
#include "molecule\cmf_loader.h"
#include "molecule\molecule_fingerprint.h"
#include "molecule\molecule_substructure_matcher.h"

using namespace indigo;
namespace bingo
{
   class QueryObject
   {
   public:
      virtual void buildFingerprint( const MoleculeFingerprintParameters &fp_params, Array<byte> *sub_fp, Array<byte> *sim_fp )/* const */ = 0;
   };

   //////////////////////////
   // Molecule query objects
   //////////////////////////

   class BaseMoleculeQuery : public QueryObject
   {
   private:
      BaseMolecule &_base_mol;
      
   public:
      BaseMoleculeQuery( BaseMolecule &mol );

      virtual void buildFingerprint( const MoleculeFingerprintParameters &fp_params, Array<byte> *sub_fp, Array<byte> *sim_fp ) /*const*/;
      
      const BaseMolecule &getMolecule();
   };

   class SubstructureMoleculeQuery : public BaseMoleculeQuery
   {
   private:
      QueryMolecule _mol;
      
   public:
      SubstructureMoleculeQuery( /* const */ QueryMolecule &mol );
   };

   class SimilarityMoleculeQuery : public BaseMoleculeQuery
   {
   private:
      Molecule _mol;
      
   public:
      SimilarityMoleculeQuery( /* const */ Molecule &mol );
   };

   //////////////////////////
   // Reaction query objects
   //////////////////////////

   class BaseReactionQuery : public QueryObject
   {
   private:
      BaseReaction &_base_rxn;
      
   public:
      BaseReactionQuery( BaseReaction &rxn );

      virtual void buildFingerprint( const MoleculeFingerprintParameters &fp_params, Array<byte> *sub_fp, Array<byte> *sim_fp ) /*const*/;

      const BaseReaction &getReaction();
   };

   class SubstructureReactionQuery : public BaseReactionQuery
   {
   public:
      SubstructureReactionQuery( /* const */ QueryReaction &rxn );

   private:
      QueryReaction _rxn;
   };

   class SimilarityReactionQuery : public BaseReactionQuery
   {
   public:
      SimilarityReactionQuery( /* const */ Reaction &rxn );

   private:
      Reaction _rxn;
   };
   
   class IndexObject
   {
   public:
      virtual void buildFingerprint( const MoleculeFingerprintParameters &fp_params, Array<byte> *sub_fp, Array<byte> *sim_fp ) /* const */ = 0;

      virtual void buildCfString( Array<char> &cf )/* const */ = 0;

      virtual void loadFromCfString( const char *cf, int length ) = 0;
   };

   class IndexMolecule : public IndexObject
   {
   protected:
      Molecule _mol;

   public:
      IndexMolecule( /* const */ Molecule &mol );

      virtual void buildFingerprint( const MoleculeFingerprintParameters &fp_params, Array<byte> *sub_fp, Array<byte> *sim_fp ) /*const*/;

      virtual void buildCfString( Array<char> &cf ) /*const*/;

      virtual void loadFromCfString( const char *cf, int length );
   };
};

#endif //__bingo_object__
