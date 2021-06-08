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

using namespace indigo;

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
        GrossQueryData(ArrayChar& gross_str);

        virtual /*const*/ QueryObject& getQueryObject() /*const*/;

    private:
        GrossQuery _obj;
    };

    class MoleculeSimilarityQueryData : public SimilarityQueryData
    {
    public:
        MoleculeSimilarityQueryData(/* const */ Molecule& mol, float min_coef, float max_coef);

        virtual /*const*/ QueryObject& getQueryObject() /*const*/;

        virtual float getMin() const;
        virtual float getMax() const;
        virtual void setMin(float min);

    private:
        SimilarityMoleculeQuery _obj;
        float _min;
        float _max;
    };

    class ReactionSimilarityQueryData : public SimilarityQueryData
    {
    public:
        ReactionSimilarityQueryData(/* const */ Reaction& rxn, float min_coef, float max_coef);

        virtual /*const*/ QueryObject& getQueryObject() /*const*/;

        virtual float getMin() const;
        virtual float getMax() const;
        virtual void setMin(float min);

    protected:
        SimilarityReactionQuery _obj;
        float _min;
        float _max;
    };

    class MoleculeExactQueryData : public ExactQueryData
    {
    public:
        MoleculeExactQueryData(/* const */ Molecule& mol);

        virtual /*const*/ QueryObject& getQueryObject();

    private:
        SimilarityMoleculeQuery _obj;
    };

    class ReactionExactQueryData : public ExactQueryData
    {
    public:
        ReactionExactQueryData(/* const */ Reaction& rxn);

        virtual /*const*/ QueryObject& getQueryObject() /*const*/;

    private:
        SimilarityReactionQuery _obj;
    };

    class MoleculeSubstructureQueryData : public SubstructureQueryData
    {
    public:
        MoleculeSubstructureQueryData(/* const */ QueryMolecule& qmol);

        virtual /*const*/ QueryObject& getQueryObject() /*const*/;

    private:
        SubstructureMoleculeQuery _obj;
    };

    class ReactionSubstructureQueryData : public SubstructureQueryData
    {
    public:
        ReactionSubstructureQueryData(/* const */ QueryReaction& qrxn);

        virtual /*const*/ QueryObject& getQueryObject() /*const*/;

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
        virtual int currentId() = 0;
        virtual IndigoObject* currentObject() = 0;
        virtual const Index& getIndex() = 0;
        virtual float currentSimValue() = 0;
        virtual void setOptions(const char* options) = 0;
        virtual void resetThresholdLimit(float min) = 0;

        virtual int esimateRemainingResultsCount(int& delta) = 0;
        virtual float esimateRemainingTime(float& delta) = 0;
        virtual int containersCount() = 0;
        virtual int cellsCount() = 0;
        virtual int currentCell() = 0;
        virtual int minCell() = 0;
        virtual int maxCell() = 0;

        virtual ~Matcher(){};
    };

    class BaseMatcher : public Matcher
    {
    public:
        BaseMatcher(BaseIndex& index, IndigoObject*& current_obj);

        virtual int currentId();

        virtual IndigoObject* currentObject();

        virtual const Index& getIndex();

        virtual float currentSimValue();

        virtual void setOptions(const char* options);
        virtual void resetThresholdLimit(float min);

        virtual int esimateRemainingResultsCount(int& delta);
        virtual float esimateRemainingTime(float& delta);
        virtual int containersCount();
        virtual int cellsCount();
        virtual int currentCell();
        virtual int minCell();
        virtual int maxCell();

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

        ~BaseMatcher();
    };

    class BaseSubstructureMatcher : public BaseMatcher
    {
    public:
        BaseSubstructureMatcher(/*const */ BaseIndex& index, IndigoObject*& current_obj);

        virtual bool next();

        void setQueryData(SubstructureQueryData* query_data);

    protected:
        int _fp_size;
        int _cand_count;
        /*const*/ AutoPtr<SubstructureQueryData> _query_data;
        Array<byte> _query_fp;
        Array<int> _query_fp_bits_used;

        void _findPackCandidates(int pack_idx);

        void _findIncCandidates();

        virtual bool _tryCurrent() /* const */ = 0;

        virtual void _setParameters(const char* params);

        virtual void _initPartition();

    private:
        Array<int> _candidates;
        int _current_cand_id;
        int _current_pack;
        int _final_pack;
        const TranspFpStorage& _fp_storage;
    };

    class MoleculeSubMatcher : public BaseSubstructureMatcher
    {
    public:
        MoleculeSubMatcher(/*const */ BaseIndex& index);

        const Array<int>& currentMapping();

    private:
        Array<int> _mapping;

        virtual bool _tryCurrent() /*const*/;

        IndexCurrentMolecule* _current_mol;
    };

    class ReactionSubMatcher : public BaseSubstructureMatcher
    {
    public:
        ReactionSubMatcher(/*const */ BaseIndex& index);

        const ObjArray<Array<int>>& currentMapping();

    private:
        ObjArray<Array<int>> _mapping;

        virtual bool _tryCurrent() /*const*/;

        IndexCurrentReaction* _current_rxn;
    };

    class BaseSimilarityMatcher : public BaseMatcher
    {
    public:
        BaseSimilarityMatcher(BaseIndex& index, IndigoObject*& current_obj);

        virtual bool next();

        void setQueryData(SimilarityQueryData* query_data);

        void setQueryDataWithExtFP(SimilarityQueryData* query_data, IndigoObject& fp);

        ~BaseSimilarityMatcher();

        virtual int esimateRemainingResultsCount(int& delta);
        virtual float esimateRemainingTime(float& delta);
        virtual void resetThresholdLimit(float min);

        virtual int containersCount();
        virtual int cellsCount();
        virtual int currentCell();
        virtual int minCell();
        virtual int maxCell();

        virtual float currentSimValue();

    protected:
        float _current_sim_value;
        AutoPtr<SimilarityQueryData> _query_data;

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

        AutoPtr<SimCoef> _sim_coef;

        Array<byte> _current_block;
        const byte* _cur_loc;
        Array<byte> _query_fp;

        virtual void _setParameters(const char* params);

        virtual void _initPartition();
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

        virtual bool next();
        void setLimit(int limit);

        ~TopNSimMatcher();

    protected:
        void _findTopN();
        void _initModelDistribution(Array<float>& thrs, Array<int>& nhits_per_block);
        static bool _cmp_sim_res(const SimResult& res1, const SimResult& res2);

    private:
        int _idx;
        int _limit;
        std::vector<SimResult> _current_results;
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

        virtual bool next();

        void setQueryData(ExactQueryData* query_data);

        ~BaseExactMatcher();

    protected:
        int _current_cand_id;
        dword _query_hash;
        int _flags;
        Array<int> _candidates;
        /* const */ AutoPtr<ExactQueryData> _query_data;

        virtual dword _calcHash() = 0;

        virtual bool _tryCurrent() /* const */ = 0;

        virtual void _initPartition();
    };

    class MolExactMatcher : public BaseExactMatcher
    {
    public:
        MolExactMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentMolecule* _current_mol;
        float _rms_threshold;

        virtual dword _calcHash();

        virtual bool _tryCurrent() /* const */;

        virtual void _setParameters(const char* params);

        bool _tautomer;
        IndigoTautomerParams _tautomer_params;
    };

    class RxnExactMatcher : public BaseExactMatcher
    {
    public:
        RxnExactMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentReaction* _current_rxn;

        virtual dword _calcHash();

        virtual bool _tryCurrent() /* const */;

        virtual void _setParameters(const char* params);
    };

    class BaseGrossMatcher : public BaseMatcher
    {
    public:
        BaseGrossMatcher(BaseIndex& index, IndigoObject*& current_obj);

        virtual bool next();

        void setQueryData(GrossQueryData* query_data);

        ~BaseGrossMatcher();

    protected:
        int _current_cand_id;
        Array<int> _query_array;
        Array<int> _candidates;
        /* const */ AutoPtr<GrossQueryData> _query_data;

        virtual void _calcFormula() = 0;

        virtual bool _tryCurrent() /* const */ = 0;

        virtual void _initPartition();
    };

    class MolGrossMatcher : public BaseGrossMatcher
    {
    public:
        MolGrossMatcher(/*const */ BaseIndex& index);

    private:
        IndexCurrentMolecule* _current_mol;

        virtual void _calcFormula();

        virtual bool _tryCurrent() /* const */;

        virtual void _setParameters(const char* params);
    };

    class EnumeratorMatcher : public BaseMatcher
    {
    public:
        EnumeratorMatcher(BaseIndex& index);

        virtual bool next();

        ~EnumeratorMatcher()
        {
        }

    protected:
        virtual void _setParameters(const char* params){};
        virtual void _initPartition(){};

    private:
        IndigoObject* _indigoObject;
        int _id_numbers;
    };
}; // namespace bingo

#endif // __bingo_matcher__
