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

        virtual bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/;

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
        ArrayChar _gross_str;

        virtual bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/;

    public:
        GrossQuery(/* const */ ArrayChar& str);

        ArrayChar& getGrossString();
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

        virtual bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/;

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

        virtual bool buildGrossString(ArrayChar& cf) /* const */ = 0;

        virtual bool buildCfString(ArrayChar& cf) /* const */ = 0;

        virtual bool buildHash(dword& hash) /* const */ = 0;

        virtual ~IndexObject(){};
    };

    class IndexMolecule : public IndexObject
    {
    protected:
        Molecule _mol;

    public:
        IndexMolecule(/* const */ Molecule& mol);

        virtual bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/;

        virtual bool buildGrossString(ArrayChar& gross_string) /* const */;

        virtual bool buildCfString(ArrayChar& cf) /*const*/;

        virtual bool buildHash(dword& hash) /* const */;
    };

    class IndexReaction : public IndexObject
    {
    protected:
        Reaction _rxn;

    public:
        IndexReaction(/* const */ Reaction& rxn);

        virtual bool buildFingerprint(const MoleculeFingerprintParameters& fp_params, Array<byte>* sub_fp, Array<byte>* sim_fp) /*const*/;

        virtual bool buildGrossString(ArrayChar& gross_string) /* const */;

        virtual bool buildCfString(ArrayChar& cf) /*const*/;

        virtual bool buildHash(dword& hash) /* const */;
    };
}; // namespace bingo

#endif //__bingo_object__
