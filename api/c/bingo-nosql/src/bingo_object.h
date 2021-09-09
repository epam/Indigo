#ifndef __bingo_object__
#define __bingo_object__

//#include "base_c/defs.h"
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

using namespace indigo;
namespace bingo
{
    class QueryObject
    {
    public:
        virtual bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /* const */ = 0;
        virtual ~QueryObject(){};
    };

    //////////////////////////
    // Molecule query objects
    //////////////////////////

    class BaseMoleculeQuery : public QueryObject
    {
    private:
        BaseMolecule& _base_mol;

        bool _needs_query_fingerprint;

    public:
        BaseMoleculeQuery(BaseMolecule& mol, bool needs_query_fingerprint);

        bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/ override;

        const BaseMolecule& getMolecule();
    };

    class SubstructureMoleculeQuery : public BaseMoleculeQuery
    {
    private:
        QueryMolecule _mol;

    public:
        SubstructureMoleculeQuery(/* const */ QueryMolecule& mol);
    };

    class SimilarityMoleculeQuery : public BaseMoleculeQuery
    {
    private:
        Molecule _mol;

    public:
        SimilarityMoleculeQuery(/* const */ Molecule& mol);
    };

    class GrossQuery : public QueryObject
    {
    private:
        Array<char> _gross_str;

        bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/ override;

    public:
        GrossQuery(/* const */ Array<char>& str);

        Array<char>& getGrossString();
    };

    //////////////////////////
    // Reaction query objects
    //////////////////////////

    class BaseReactionQuery : public QueryObject
    {
    private:
        BaseReaction& _base_rxn;

    public:
        BaseReactionQuery(BaseReaction& rxn);

        bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/ override;

        const BaseReaction& getReaction();
    };

    class SubstructureReactionQuery : public BaseReactionQuery
    {
    public:
        SubstructureReactionQuery(/* const */ QueryReaction& rxn);

    private:
        QueryReaction _rxn;
    };

    class SimilarityReactionQuery : public BaseReactionQuery
    {
    public:
        SimilarityReactionQuery(/* const */ Reaction& rxn);

    private:
        Reaction _rxn;
    };

    class IndexObject
    {
    public:
        virtual bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /* const */ = 0;

        virtual bool buildGrossString(Array<char>& cf) /* const */ = 0;

        virtual bool buildCfString(Array<char>& cf) /* const */ = 0;

        virtual bool buildHash(dword& hash) /* const */ = 0;

        virtual ~IndexObject(){};
    };

    class IndexMolecule : public IndexObject
    {
    protected:
        Molecule _mol;

    public:
        IndexMolecule(/* const */ Molecule& mol);

        bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/ override;

        bool buildGrossString(Array<char>& gross_string) /* const */ override;

        bool buildCfString(Array<char>& cf) /*const*/ override;

        bool buildHash(dword& hash) /* const */ override;
    };

    class IndexReaction : public IndexObject
    {
    protected:
        Reaction _rxn;

    public:
        IndexReaction(/* const */ Reaction& rxn);

        bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/ override;

        bool buildGrossString(Array<char>& gross_string) /* const */ override;

        bool buildCfString(Array<char>& cf) /*const*/ override;

        bool buildHash(dword& hash) /* const */ override;
    };
}; // namespace bingo

#endif //__bingo_object__
