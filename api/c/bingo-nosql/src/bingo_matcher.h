#ifndef __bingo_matcher__
#define __bingo_matcher__

#include <atomic>

#include "bingo_base_index.h"
#include "bingo_object.h"

#include "indigo_fingerprints.h"
#include "indigo_match.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"

#include "base_cpp/os_thread_wrapper.h"
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
    public:
        int db_id;
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

        static bool _loadCurrentObject(BaseIndex& index, int current_id, IndigoObject*& current_obj);

        virtual void _setParameters(const char* params) = 0;
        virtual void _initPartition() = 0;

        ~BaseMatcher() override;
    };

    struct BaseSubstructureMatcherResult : public OsCommandResult
    {
        bool match = false;
        int db_object_idx = -1;
        std::unique_ptr<IndigoObject> indigo_obj;
        void clear() override
        {
            match = false;
            db_object_idx = -1;
            indigo_obj.reset();
        }
    };

    class BaseSubstructureMatcher;

    struct BaseSubstructureMatcherCommand : public OsCommand
    {
    public:
        int db_object_idx;

        BaseSubstructureMatcherCommand(BaseSubstructureMatcher* matcher);
        virtual ~BaseSubstructureMatcherCommand();

        void clear() override
        {
            db_object_idx = -1;
        }

        void execute(OsCommandResult& res) override;

    private:
        BaseSubstructureMatcher* _matcher;
    };

    class BaseSubstructureMatcherDispatcher : public OsCommandDispatcher
    {
    public:
        BaseSubstructureMatcherDispatcher(BaseSubstructureMatcher* matcher, std::deque<int>& src, std::mutex& i_mtx, std::condition_variable& cv_input,
                                          std::atomic_bool& eod, std::deque<std::pair<int, std::unique_ptr<IndigoObject>>>& results, std::mutex& r_mtx,
                                          std::condition_variable& cv_results)
            : OsCommandDispatcher(HANDLING_ORDER_SERIAL, true), _matcher(matcher), _input_data(src), _input_mtx(i_mtx), _cv_input(cv_input),
              _all_data_in_queue(eod), _results(results), _results_mtx(r_mtx), _cv_results(cv_results)
        {
        }

        void operator()()
        {
            run();
        }

    protected:
        OsCommand* _allocateCommand() override
        {
            return new BaseSubstructureMatcherCommand(_matcher);
        }
        OsCommandResult* _allocateResult() override
        {
            return new BaseSubstructureMatcherResult();
        }

        bool _setupCommand(OsCommand& command) override;

        void _handleResult(OsCommandResult& result) override;

    private:
        std::deque<int>& _input_data;
        std::mutex& _input_mtx;
        std::condition_variable& _cv_input;
        std::atomic_bool& _all_data_in_queue;
        std::deque<std::pair<int, std::unique_ptr<IndigoObject>>>& _results;
        std::mutex& _results_mtx;
        std::condition_variable& _cv_results;
        BaseSubstructureMatcher* _matcher;
    };

    class BaseSubstructureMatcher : public BaseMatcher
    {
    public:
        BaseSubstructureMatcher(/*const */ BaseIndex& index, IndigoObject*& current_obj);
        virtual ~BaseSubstructureMatcher();

        bool next() override;

        void setQueryData(SubstructureQueryData* query_data);

        virtual bool tryCurrent(int current_id, IndigoObject* current_obj) /* const */ = 0;

        virtual IndigoObject* allocateObject() = 0;

        int getDbId() const;

        void run_dispatcher();

        AromaticityOptions _arom_options;
        PtrArray<TautomerRule>* _tautomer_rules;

    protected:
        int _fp_size;
        int _cand_count;
        /*const*/ std::unique_ptr<SubstructureQueryData> _query_data;
        Array<byte> _query_fp;
        Array<int> _query_fp_bits_used;

        void _findPackCandidates(int pack_idx);

        void _findIncCandidates();

        void _setParameters(const char* params) override;

        void _initPartition() override;

        bool _tautomer;
        IndigoTautomerParams _tautomer_params;

        bool _multithread = false;
        int _thread_count = -1;

    private:
        Array<int> _candidates;
        int _current_cand_id;
        int _current_pack;
        int _final_pack;
        const TranspFpStorage& _fp_storage;
        int sub_cnt;

        std::unique_ptr<BaseSubstructureMatcherDispatcher> _disp;
        std::optional<std::thread> _t;
        std::deque<int> _input_data;
        std::mutex _input_mtx;
        std::condition_variable _cv_input;
        std::atomic_bool _all_data_in_queue = false;
        std::deque<std::pair<int, std::unique_ptr<IndigoObject>>> results;
        std::mutex _results_mtx;
        std::atomic_bool _finished_processing = false;
        std::condition_variable _cv_results;
    };

    class MoleculeSubMatcher : public BaseSubstructureMatcher
    {
    public:
        MoleculeSubMatcher(/*const */ BaseIndex& index);

        const Array<int>& currentMapping();

        virtual bool tryCurrent(int current_id, IndigoObject* current_obj) /*const*/ override;

        virtual IndigoObject* allocateObject() override;

    private:
        Array<int> _mapping;

        IndexCurrentMolecule* _current_mol;
    };

    class ReactionSubMatcher : public BaseSubstructureMatcher
    {
    public:
        ReactionSubMatcher(/*const */ BaseIndex& index);

        const ObjArray<Array<int>>& currentMapping();

        virtual bool tryCurrent(int current_id, IndigoObject* current_obj) /*const*/ override;

        virtual IndigoObject* allocateObject() override;

    private:
        ObjArray<Array<int>> _mapping;

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
