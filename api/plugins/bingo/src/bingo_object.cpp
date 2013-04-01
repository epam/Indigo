#include "bingo_object.h"

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
using namespace bingo;

BaseMoleculeQuery::BaseMoleculeQuery( BaseMolecule &mol ) : _base_mol(mol)
{
}

void BaseMoleculeQuery::buildFingerprint( const MoleculeFingerprintParameters &fp_params, Array<byte> *sub_fp, Array<byte> *sim_fp )// const
{
   MoleculeFingerprintBuilder fp_builder(_base_mol, fp_params);

   fp_builder.process();

   if (sub_fp)
      sub_fp->copy(fp_builder.get(), fp_params.fingerprintSize());
   if (sim_fp)
      sim_fp->copy(fp_builder.getSim(), fp_params.fingerprintSizeSim());
}
      
const BaseMolecule & BaseMoleculeQuery::getMolecule()
{
   return _base_mol;
}

SubstructureMoleculeQuery::SubstructureMoleculeQuery( /* const */ QueryMolecule &mol ) : BaseMoleculeQuery(_mol)
{
   _mol.clone(mol, 0, 0);
}

SimilarityMoleculeQuery::SimilarityMoleculeQuery( /* const */ Molecule &mol ) : BaseMoleculeQuery(_mol)
{
   _mol.clone(mol, 0, 0);
}

BaseReactionQuery::BaseReactionQuery( BaseReaction &rxn ) : _base_rxn(rxn)
{
}

void BaseReactionQuery::buildFingerprint( const MoleculeFingerprintParameters &fp_params, Array<byte> *sub_fp, Array<byte> *sim_fp ) // const
{
   ReactionFingerprintBuilder fp_builder(_base_rxn, fp_params);

   fp_builder.process();

   if (sub_fp)
      sub_fp->copy(fp_builder.get(), fp_params.fingerprintSize());
   if (sim_fp)
      sim_fp->copy(fp_builder.getSim(), fp_params.fingerprintSizeSim());
}

const BaseReaction & BaseReactionQuery::getReaction()
{
   return _base_rxn;
}

SubstructureReactionQuery::SubstructureReactionQuery( /* const */ QueryReaction &rxn ) : BaseReactionQuery(_rxn)
{
   _rxn.clone(rxn, 0, 0, 0);
}

SimilarityReactionQuery::SimilarityReactionQuery( /* const */ Reaction &rxn ) : BaseReactionQuery(_rxn)
{
   _rxn.clone(rxn, 0, 0, 0);
}
   
IndexMolecule::IndexMolecule( /* const */ Molecule &mol )
{
   _mol.clone(mol, 0, 0);
}

void IndexMolecule::buildFingerprint( const MoleculeFingerprintParameters &fp_params, Array<byte> *sub_fp, Array<byte> *sim_fp ) // const
{
   MoleculeFingerprintBuilder fp_builder(_mol, fp_params);

   fp_builder.process();

   sub_fp->copy(fp_builder.get(), fp_params.fingerprintSize());
   sim_fp->copy(fp_builder.getSim(), fp_params.fingerprintSizeSim());
}

void IndexMolecule::buildCfString( Array<char> &cf )// const
{
   ArrayOutput arr_out(cf);
   CmfSaver cmf_saver(arr_out);

   cmf_saver.saveMolecule(_mol);
}

void IndexMolecule::loadFromCfString( const char *cf, int length )
{
   BufferScanner buf_scn(cf, length);
   CmfLoader cmf_loader(buf_scn);

   cmf_loader.loadMolecule(_mol);
}

