#ifndef __bingo_object__
#define __bingo_object__

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"

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
#include "molecule/molecule_substructure_matcher.h"

namespace bingo
{
    class QueryObject
    {
    public:
        virtual bool buildFingerprint(const indigo::MoleculeFingerprintParameters& fp_params, indigo::Array<byte>* sub_fp,
                                      indigo::Array<byte>* sim_fp) /* const */
            = 0;
        virtual ~QueryObject(){};
    };

    //////////////////////////
    // Molecule query objects
    //////////////////////////

    class BaseMoleculeQuery : public QueryObject
    {
    private:
        indigo::BaseMolecule& _base_mol;

        bool _needs_query_fingerprint;

    public:
        BaseMoleculeQuery(indigo::BaseMolecule& mol, bool needs_query_fingerprint);

        bool buildFingerprint(const indigo::MoleculeFingerprintParameters& fp_params, indigo::Array<byte>* sub_fp,
                              indigo::Array<byte>* sim_fp) /*const*/ override;

        const indigo::BaseMolecule& getMolecule();
    };

    class SubstructureMoleculeQuery : public BaseMoleculeQuery
    {
    private:
        indigo::QueryMolecule _mol;

    public:
        SubstructureMoleculeQuery(/* const */ indigo::QueryMolecule& mol);
    };

    class SimilarityMoleculeQuery : public BaseMoleculeQuery
    {
    private:
        indigo::Molecule _mol;

    public:
        SimilarityMoleculeQuery(/* const */ indigo::Molecule& mol);
    };

    class GrossQuery : public QueryObject
    {
    private:
        indigo::Array<char> _gross_str;

        bool buildFingerprint(const indigo::MoleculeFingerprintParameters& fp_params, indigo::Array<byte>* sub_fp,
                              indigo::Array<byte>* sim_fp) /*const*/ override;

    public:
        GrossQuery(/* const */ indigo::Array<char>& str);

        indigo::Array<char>& getGrossString();
    };

    //////////////////////////
    // Reaction query objects
    //////////////////////////

    class BaseReactionQuery : public QueryObject
    {
    private:
        indigo::BaseReaction& _base_rxn;

    public:
        BaseReactionQuery(indigo::BaseReaction& rxn);

        bool buildFingerprint(const indigo::MoleculeFingerprintParameters& fp_params, indigo::Array<byte>* sub_fp,
                              indigo::Array<byte>* sim_fp) /*const*/ override;

        const indigo::BaseReaction& getReaction();
    };

    class SubstructureReactionQuery : public BaseReactionQuery
    {
    public:
        SubstructureReactionQuery(/* const */ indigo::QueryReaction& rxn);

    private:
        indigo::QueryReaction _rxn;
    };

    class SimilarityReactionQuery : public BaseReactionQuery
    {
    public:
        SimilarityReactionQuery(/* const */ indigo::Reaction& rxn);

    private:
        indigo::Reaction _rxn;
    };

    class IndexObject
    {
    public:
        virtual bool buildFingerprint(const indigo::MoleculeFingerprintParameters& fp_params, indigo::Array<byte>* sub_fp,
                                      indigo::Array<byte>* sim_fp) /* const */
            = 0;

        virtual bool buildGrossString(indigo::Array<char>& cf) /* const */ = 0;

        virtual bool buildCfString(indigo::Array<char>& cf) /* const */ = 0;

        virtual bool buildHash(dword& hash) /* const */ = 0;

        virtual ~IndexObject(){};
    };

    class IndexMolecule : public IndexObject
    {
    protected:
        indigo::Molecule _mol;

    public:
        IndexMolecule(/* const */ indigo::Molecule& mol, const indigo::AromaticityOptions& arom_options);

        bool buildFingerprint(const indigo::MoleculeFingerprintParameters& fp_params, indigo::Array<byte>* sub_fp,
                              indigo::Array<byte>* sim_fp) /*const*/ override;

        bool buildGrossString(indigo::Array<char>& gross_string) /* const */ override;

        bool buildCfString(indigo::Array<char>& cf) /*const*/ override;

        bool buildHash(dword& hash) /* const */ override;
    };

    class IndexReaction : public IndexObject
    {
    protected:
        indigo::Reaction _rxn;

    public:
        IndexReaction(/* const */ indigo::Reaction& rxn, const indigo::AromaticityOptions& arom_options);

        bool buildFingerprint(const indigo::MoleculeFingerprintParameters& fp_params, indigo::Array<byte>* sub_fp,
                              indigo::Array<byte>* sim_fp) /*const*/ override;

        bool buildGrossString(indigo::Array<char>& gross_string) /* const */ override;

        bool buildCfString(indigo::Array<char>& cf) /*const*/ override;

        bool buildHash(dword& hash) /* const */ override;
    };
}; // namespace bingo

#endif //__bingo_object__
