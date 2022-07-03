#include "bingo_object.h"
#include "bingo_exact_storage.h"
#include "bingo_gross_storage.h"

#include "reaction/crf_loader.h"
#include "reaction/crf_saver.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_fingerprint.h"
#include "reaction/reaction_substructure_matcher.h"

#include "molecule/cmf_loader.h"
#include "molecule/cmf_saver.h"
#include "molecule/molecule.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_substructure_matcher.h"

using namespace indigo;
using namespace bingo;

namespace
{
    constexpr const int _fp_calc_timeout = 10000;
}

BaseMoleculeQuery::BaseMoleculeQuery(BaseMolecule& mol, bool needs_query_fingerprint) : _base_mol(mol), _needs_query_fingerprint(needs_query_fingerprint)
{
}

bool BaseMoleculeQuery::buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) // const
{
    MoleculeFingerprintBuilder fp_builder(_base_mol, fp_params);
    fp_builder.query = _needs_query_fingerprint;

    fp_builder.process();

    if (sub_fp)
        sub_fp->copy(fp_builder.get(), fp_params.fingerprintSize());
    if (sim_fp)
        sim_fp->copy(fp_builder.getSim(), fp_params.fingerprintSizeSim());

    return true;
}

const BaseMolecule& BaseMoleculeQuery::getMolecule()
{
    return _base_mol;
}

SubstructureMoleculeQuery::SubstructureMoleculeQuery(/* const */ QueryMolecule& mol) : BaseMoleculeQuery(_mol, true)
{
    _mol.clone(mol, 0, 0);
}

SimilarityMoleculeQuery::SimilarityMoleculeQuery(/* const */ Molecule& mol) : BaseMoleculeQuery(_mol, false)
{
    _mol.clone(mol, 0, 0);
}

GrossQuery::GrossQuery(/* const */ Array<char>& str)
{
    _gross_str.copy(str);
}

bool GrossQuery::buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/
{
    throw Exception("GrossQuery::buildFingerprint can\t be called");
}

Array<char>& GrossQuery::getGrossString()
{
    return _gross_str;
}

BaseReactionQuery::BaseReactionQuery(BaseReaction& rxn) : _base_rxn(rxn)
{
}

bool BaseReactionQuery::buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) // const
{
    ReactionFingerprintBuilder fp_builder(_base_rxn, fp_params);

    fp_builder.process();

    if (sub_fp)
        sub_fp->copy(fp_builder.get(), fp_params.fingerprintSize());
    if (sim_fp)
        sim_fp->copy(fp_builder.getSim(), fp_params.fingerprintSizeSim());

    return true;
}

const BaseReaction& BaseReactionQuery::getReaction()
{
    return _base_rxn;
}

SubstructureReactionQuery::SubstructureReactionQuery(/* const */ QueryReaction& rxn) : BaseReactionQuery(_rxn)
{
    _rxn.clone(rxn, 0, 0, 0);
}

SimilarityReactionQuery::SimilarityReactionQuery(/* const */ Reaction& rxn) : BaseReactionQuery(_rxn)
{
    _rxn.clone(rxn, 0, 0, 0);
}

IndexMolecule::IndexMolecule(/* const */ Molecule& mol, const AromaticityOptions& arom_options)
{
    _mol.clone(mol, nullptr, nullptr);
    // _mol.aromatize(arom_options);
}

bool IndexMolecule::buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) // const
{
    MoleculeFingerprintBuilder fp_builder(_mol, fp_params);

    fp_builder.process();

    if (sub_fp)
        sub_fp->copy(fp_builder.get(), fp_params.fingerprintSize());
    if (sim_fp)
        sim_fp->copy(fp_builder.getSim(), fp_params.fingerprintSizeSim());

    return true;
}

bool IndexMolecule::buildGrossString(Array<char>& gross) /* const */
{
    GrossStorage::calculateMolFormula(_mol, gross);

    return true;
}

bool IndexMolecule::buildCfString(Array<char>& cf) // const
{
    ArrayOutput arr_out(cf);
    CmfSaver cmf_saver(arr_out);

    cmf_saver.saveMolecule(_mol);

    return true;
}

bool IndexMolecule::buildHash(dword& hash)
{
    hash = ExactStorage::calculateMolHash(_mol);

    return true;
}

IndexReaction::IndexReaction(/* const */ Reaction& rxn, const AromaticityOptions& arom_options)
{
    _rxn.clone(rxn);
    // _rxn.aromatize(arom_options);
}

bool IndexReaction::buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) // const
{
    ReactionFingerprintBuilder fp_builder(_rxn, fp_params);

    fp_builder.process();

    if (sub_fp)
        sub_fp->copy(fp_builder.get(), fp_params.fingerprintSize());
    if (sim_fp)
        sim_fp->copy(fp_builder.getSim(), fp_params.fingerprintSizeSim());

    return true;
}

bool IndexReaction::buildGrossString(Array<char>& gross) /* const */
{
    GrossStorage::calculateRxnFormula(_rxn, gross);

    return true;
}

bool IndexReaction::buildCfString(Array<char>& cf) // const
{
    ArrayOutput arr_out(cf);
    CrfSaver crf_saver(arr_out);

    crf_saver.saveReaction(_rxn);

    return true;
}

bool IndexReaction::buildHash(dword& hash)
{
    hash = ExactStorage::calculateRxnHash(_rxn);

    return true;
}
