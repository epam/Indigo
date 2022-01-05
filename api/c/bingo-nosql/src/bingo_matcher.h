#ifndef __bingo_matcher__
#define __bingo_matcher__

#include "bingo_base_index.h"
#include "bingo_object.h"

#include "indigo_fingerprints.h"
#include "indigo_match.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"

#include "math/statistics.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_substructure_matcher.h"
#include "reaction/reaction_exact_matcher.h"

namespace bingo
{
    ///////////////////////////////////////
    // Query data classes
    ///////////////////////////////////////

    class MatcherQueryData
    {
    public:
        virtual /*const*/ QueryObject& getQueryObject() /*const*/ = 0;

        virtual ~MatcherQueryData(){};
    };

    class SimilarityQueryData : public MatcherQueryData
    {
    public:
        virtual float getMin() const = 0;
        virtual float getMax() const = 0;
        virtual void setMin(float min);
    };

    class SubstructureQueryData : public MatcherQueryData
    {
    };

    class ExactQueryData : public MatcherQueryData
    {
    };

    class GrossQueryData : public MatcherQueryData
    {
    public:
        GrossQueryData(Array<char>& gross_str);

        /*const*/ QueryObject& getQueryObject() /*const*/ override;

    private:
        GrossQuery _obj;
    };

    class MoleculeSimilarityQueryData : public SimilarityQueryData
    {
    public:
        MoleculeSimilarityQueryData(/* const */ Molecule& mol, float min_coef, float max_coef);

        /*const*/ QueryObject& getQueryObject() /*const*/ override;

        float getMin() const override;
        float getMax() const override;
        void setMin(float min) override;

    private:
        SimilarityMoleculeQuery _obj;
        float _min;
        float _max;
    };

    class ReactionSimilarityQueryData : public SimilarityQueryData
    {
    public:
        ReactionSimilarityQueryData(/* const */ Reaction& rxn, float min_coef, float max_coef);

        /*const*/ QueryObject& getQueryObject() /*const*/ override;

        float getMin() const override;
        float getMax() const override;
        void setMin(float min) override;

    protected:
        SimilarityReactionQuery _obj;
        float _min;
        float _max;
    };

    class MoleculeExactQueryData : public ExactQueryData
    {
    public:
        MoleculeExactQueryData(/* const */ Molecule& mol);

        /*const*/ QueryObject& getQueryObject() override;

    private:
        SimilarityMoleculeQuery _obj;
    };

    class ReactionExactQueryData : public ExactQueryData
    {
    public:
        ReactionExactQueryData(/* const */ Reaction& rxn);

        /*const*/ QueryObject& getQueryObject() /*const*/ override;

    private:
        SimilarityReactionQuery _obj;
    };

    class MoleculeSubstructureQueryData : public SubstructureQueryData
    {
    public:
        MoleculeSubstructureQueryData(/* const */ QueryMolecule& qmol);

        /*const*/ QueryObject& getQueryObject() /*const*/ override;

    private:
        SubstructureMoleculeQuery _obj;
    };

    class ReactionSubstructureQueryData : public SubstructureQueryData
    {
    public:
        ReactionSubstructureQueryData(/* const */ QueryReaction& qrxn);

        /*const*/ QueryObject& getQueryObject() /*const*/ override;

    private:
        SubstructureReactionQuery _obj;
    };

    ///////////////////////////////////////
    // Matcher classes
    ///////////////////////////////////////

    class IndexCurrentMolecule : public IndigoMolecule
    {
    public:
        IndexCurrentMolecule(IndexCurrentMolecule*& ptr);
        ~IndexCurrentMolecule();

        bool matcher_exist;

    private:
        IndexCurrentMolecule*& _ptr;
    };

    class IndexCurrentReaction : public IndigoReaction
    {
    public:
        IndexCurrentReaction(IndexCurrentReaction*& ptr);
        ~IndexCurrentReaction();

        bool matcher_exist;

    private:
        IndexCurrentReaction*& _ptr;
    };

    class Matcher
    {
    public:
        virtual bool next() = 0;
        virtual int currentId() const = 0;
        virtual IndigoObject* currentObject() = 0;
        virtual const BaseIndex& getIndex() = 0;
        virtual float currentSimValue() const = 0;
        virtual void setOptions(const char* options) = 0;
        virtual void resetThresholdLimit(float min) = 0;

        virtual int esimateRemainingResultsCount(int& delta) = 0;
        virtual float esimateRemainingTime(float& delta) = 0;
        virtual int containersCount() const = 0;
        virtual int cellsCount() const = 0;
        virtual int currentCell() const = 0;
        virtual int minCell() const = 0;
        virtual int maxCell() const = 0;

        virtual ~Matcher(){};
    };

    class BaseMatcher : public Matcher
    {
    public:
        BaseMatcher(BaseIndex& index, IndigoObject*& current_obj);

        int currentId() const override;

        IndigoObject* currentObject() override;

        const BaseIndex& getIndex() override;

        float currentSimValue() const override;

        void setOptions(const char* options) override;
        void resetThresholdLimit(float min) override;

        int esimateRemainingResultsCount(int& delta) override;
        float esimateRemainingTime(float& delta) override;
        int containersCount() const override;
        int cellsCount() const override;
        int currentCell() const override;
        int minCell() const override;
        int maxCell() const override;

    protected:
        BaseIndex& _index;
        IndigoObject*& _current_obj;
        bool _current_obj_used;
        int _current_id;
        int _part_id;
        int _part_count;

        // Variables used for estimation
        MeanEstimator _match_probability_esimate, _match_time_esimate;

        bool _isCurrentObjectExist();

        bool _loadCurrentObject();

        virtual void _setParameters(const char* params) = 0;
        virtual void _initPartition() = 0;

        ~BaseMatcher() override;
    };

    class BaseSubstructureMatcher : public BaseMatcher
    {
    public:
        BaseSubstructureMatcher(/*const */ BaseIndex& index, IndigoObject*& current_obj);

        bool next() override;

        void setQueryData(SubstructureQueryData* query_data);

    protected:
        int _fp_size;
        int _cand_count;
        /*const*/ std::unique_ptr<SubstructureQueryData> _query_data;
        Array<byte> _query_fp;
        Array<int> _query_fp_bits_used;

        void _findPackCandidates(int pack_idx);

        void _findIncCandidates();

        virtual bool _tryCurrent() /* const */ = 0;

        void _setParameters(const char* params) override;

        void _initPartition() override;

    private:
        Array<int> _candidates;
        int _current_cand_id;
        int _current_pack;
        int _final_pack;
        const TranspFpStorage& _fp_storage;
        int sub_cnt;
    };

    class MoleculeSubMatcher : public BaseSubstructureMatcher
    {
    public:
        MoleculeSubMatcher(/*const */ BaseIndex& index);

        const Array<int>& currentMapping();

    private:
        Array<int> _mapping;

        bool _tryCurrent() /*const*/ override;

        IndexCurrentMolecule* _current_mol;
    };

    class ReactionSubMatcher : public BaseSubstructureMatcher
    {
    public:
        ReactionSubMatcher(/*const */ BaseIndex& index);

        const ObjArray<Array<int>>& currentMapping();

    private:
        ObjArray<Array<int>> _mapping;

        bool _tryCurrent() /*const*/ override;

        IndexCurrentReaction* _current_rxn;
    };

    class BaseSimilarityMatcher : public BaseMatcher
    {
    public:
        BaseSimilarityMatcher(BaseIndex& index, IndigoObject*& current_obj);

        bool next() override;

        void setQueryData(SimilarityQueryData* query_data);

        void setQueryDataWithExtFP(SimilarityQueryData* query_data, IndigoObject& fp);

        ~BaseSimilarityMatcher() override;

        int esimateRemainingResultsCount(int& delta) override;
        float esimateRemainingTime(float& delta) override;
        void resetThresholdLimit(float min) override;

        int containersCount() const override;
        int cellsCount() const override;
        int currentCell() const override;
        int minCell() const override;
        int maxCell() const override;

        float currentSimValue() const override;

    protected:
        float _current_sim_value;
        std::unique_ptr<SimilarityQueryData> _query_data;

    private:
        int _fp_size;

        int _min_cell;
        int _max_cell;
        int _first_cell;
        int _containers_count;

        int _current_cell;
        int _current_container;
        Array<SimResult> _current_portion;
        int _current_portion_id;

        // float _current_sim_value;

        std::unique_ptr<SimCoef> _sim_coef;

        Array<byte> _current_block;
        const byte* _cur_loc;
        Array<byte> _query_fp;

        void _setParameters(const char* params) override;

        void _initPartition() override;
    };

    class MoleculeSimMatcher : public BaseSimilarityMatcher
    {
    public:
        MoleculeSimMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentMolecule* _current_mol;
    };

    class ReactionSimMatcher : public BaseSimilarityMatcher
    {
    public:
        ReactionSimMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentReaction* _current_rxn;
    };

    class TopNSimMatcher : public BaseSimilarityMatcher
    {
    public:
        TopNSimMatcher(/*const */ BaseIndex& index, IndigoObject*& current_obj);

        bool next() override;
        void setLimit(int limit);

        ~TopNSimMatcher() override;

    protected:
        void _findTopN();
        void _initModelDistribution(Array<float>& thrs, Array<int>& nhits_per_block);
        static int _cmp_sim_res(SimResult& res1, SimResult& res2, void* context);

    private:
        int _idx;
        int _limit;
        Array<SimResult> _current_results;
        Array<int> _result_ids;
        Array<float> _result_sims;
    };

    class MoleculeTopNSimMatcher : public TopNSimMatcher
    {
    public:
        MoleculeTopNSimMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentMolecule* _current_mol;
    };

    class ReactionTopNSimMatcher : public TopNSimMatcher
    {
    public:
        ReactionTopNSimMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentReaction* _current_rxn;
    };

    class BaseExactMatcher : public BaseMatcher
    {
    public:
        BaseExactMatcher(BaseIndex& index, IndigoObject*& current_obj);

        bool next() override;

        void setQueryData(ExactQueryData* query_data);

        ~BaseExactMatcher() override;

    protected:
        int _current_cand_id;
        dword _query_hash;
        int _flags;
        Array<int> _candidates;
        /* const */ std::unique_ptr<ExactQueryData> _query_data;

        virtual dword _calcHash() = 0;

        virtual bool _tryCurrent() /* const */ = 0;

        void _initPartition() override;
    };

    class MolExactMatcher : public BaseExactMatcher
    {
    public:
        MolExactMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentMolecule* _current_mol;
        float _rms_threshold;

        dword _calcHash() override;

        bool _tryCurrent() /* const */ override;

        void _setParameters(const char* params) override;

        bool _tautomer;
        IndigoTautomerParams _tautomer_params;
    };

    class RxnExactMatcher : public BaseExactMatcher
    {
    public:
        RxnExactMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentReaction* _current_rxn;

        dword _calcHash() override;

        bool _tryCurrent() /* const */ override;

        void _setParameters(const char* params) override;
    };

    class BaseGrossMatcher : public BaseMatcher
    {
    public:
        BaseGrossMatcher(BaseIndex& index, IndigoObject*& current_obj);

        bool next() override;

        void setQueryData(GrossQueryData* query_data);

        ~BaseGrossMatcher() override;

    protected:
        int _current_cand_id;
        Array<int> _query_array;
        Array<int> _candidates;
        /* const */ std::unique_ptr<GrossQueryData> _query_data;

        virtual void _calcFormula() = 0;

        virtual bool _tryCurrent() /* const */ = 0;

        virtual void _initPartition() override;
    };

    class MolGrossMatcher : public BaseGrossMatcher
    {
    public:
        MolGrossMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentMolecule* _current_mol;

        void _calcFormula() override;

        bool _tryCurrent() /* const */ override;

        void _setParameters(const char* params) override;
    };

    class EnumeratorMatcher : public BaseMatcher
    {
    public:
        EnumeratorMatcher(BaseIndex& index);

        bool next() override;

        ~EnumeratorMatcher() override
        {
        }

    protected:
        void _setParameters(const char* params) override;
        void _initPartition() override;

    private:
        IndigoObject* _indigoObject;
        int _id_numbers;
    };
}; // namespace bingo

#endif // __bingo_matcher__
