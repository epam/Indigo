#include "bingo_matcher.h"
#include "bingo_euclid_coef.h"
#include "bingo_tanimoto_coef.h"
#include "bingo_tversky_coef.h"

#include "molecule/molecule_substructure_matcher.h"

#include "base_c/bitarray.h"
#include "base_cpp/profiling.h"

#include <algorithm>
#include <sstream>
#include <vector>

using namespace indigo;
using namespace bingo;

static const char* _matcher_params_prop = "";
static const char* _matcher_part_prop = "part";

GrossQueryData::GrossQueryData(Array<char>& gross_str) : _obj(gross_str)
{
}

QueryObject& GrossQueryData::getQueryObject()
{
    return _obj;
}

void SimilarityQueryData::setMin(float min)
{
    throw Exception("SimilarityQueryData does not support this method");
}

MoleculeSimilarityQueryData::MoleculeSimilarityQueryData(/* const */ Molecule& qmol, float min_coef, float max_coef)
    : _obj(qmol), _min(min_coef), _max(max_coef)
{
}

/*const*/ QueryObject& MoleculeSimilarityQueryData::getQueryObject() /*const*/
{
    return _obj;
}

float MoleculeSimilarityQueryData::getMin() const
{
    return _min;
}

float MoleculeSimilarityQueryData::getMax() const
{
    return _max;
}

void MoleculeSimilarityQueryData::setMin(float min)
{
    _min = min;
}

ReactionSimilarityQueryData::ReactionSimilarityQueryData(/* const */ Reaction& qrxn, float min_coef, float max_coef)
    : _obj(qrxn), _min(min_coef), _max(max_coef)
{
}

/*const*/ QueryObject& ReactionSimilarityQueryData::getQueryObject() /*const*/
{
    return _obj;
}

float ReactionSimilarityQueryData::getMin() const
{
    return _min;
}

float ReactionSimilarityQueryData::getMax() const
{
    return _max;
}

void ReactionSimilarityQueryData::setMin(float min)
{
    _min = min;
}

MoleculeExactQueryData::MoleculeExactQueryData(/* const */ Molecule& mol) : _obj(mol)
{
}

/*const*/ QueryObject& MoleculeExactQueryData::getQueryObject() /*const*/
{
    return _obj;
}

ReactionExactQueryData::ReactionExactQueryData(/* const */ Reaction& rxn) : _obj(rxn)
{
}

/*const*/ QueryObject& ReactionExactQueryData::getQueryObject() /*const*/
{
    return _obj;
}

MoleculeSubstructureQueryData::MoleculeSubstructureQueryData(/* const */ QueryMolecule& qmol) : _obj(qmol)
{
}

/*const*/ QueryObject& MoleculeSubstructureQueryData::getQueryObject() /*const*/
{
    return _obj;
}

ReactionSubstructureQueryData::ReactionSubstructureQueryData(/* const */ QueryReaction& qrxn) : _obj(qrxn)
{
}

/*const*/ QueryObject& ReactionSubstructureQueryData::getQueryObject() /*const*/
{
    return _obj;
}

IndexCurrentMolecule::IndexCurrentMolecule(IndexCurrentMolecule*& ptr) : _ptr(ptr)
{
    matcher_exist = true;
}

IndexCurrentMolecule::~IndexCurrentMolecule()
{
    if (matcher_exist)
        _ptr = 0;
}

IndexCurrentReaction::IndexCurrentReaction(IndexCurrentReaction*& ptr) : _ptr(ptr)
{
    matcher_exist = true;
}

IndexCurrentReaction::~IndexCurrentReaction()
{
    if (matcher_exist)
        _ptr = 0;
}

BaseMatcher::BaseMatcher(BaseIndex& index, IndigoObject*& current_obj) : _index(index), _current_obj(current_obj)
{
    _current_obj_used = false;
    _current_id = -1;
    _part_id = -1;
    _part_count = -1;
}

BaseMatcher::~BaseMatcher()
{
    if (_current_obj && IndigoMolecule::is(*_current_obj))
        ((IndexCurrentMolecule*)_current_obj)->matcher_exist = false;
    else if (_current_obj && IndigoReaction::is(*_current_obj))
        ((IndexCurrentReaction*)_current_obj)->matcher_exist = false;

    if (_current_obj && !_current_obj_used)
        delete _current_obj;
}

int BaseMatcher::currentId() const
{
    MMFArray<int>& id_mapping = _index.getIdMapping();
    return id_mapping[_current_id];
}

IndigoObject* BaseMatcher::currentObject()
{
    if (_current_obj_used)
        throw Exception("BaseMatcher: Object has been already gotten");

    _current_obj_used = true;

    return _current_obj;
}

const BaseIndex& BaseMatcher::getIndex()
{
    return _index;
}

float BaseMatcher::currentSimValue() const
{
    throw Exception("BaseMatcher: Matcher does not support this method");
}

int BaseMatcher::containersCount() const
{
    throw Exception("BaseMatcher: Matcher does not support this method");
}

int BaseMatcher::cellsCount() const
{
    throw Exception("BaseMatcher: Matcher does not support this method");
}

int BaseMatcher::currentCell() const
{
    throw Exception("BaseMatcher: Matcher does not support this method");
}

int BaseMatcher::minCell() const
{
    throw Exception("BaseMatcher: Matcher does not support this method");
}

int BaseMatcher::maxCell() const
{
    throw Exception("BaseMatcher: Matcher does not support this method");
}

void BaseMatcher::resetThresholdLimit(float min)
{
    throw Exception("BaseMatcher: Matcher does not support this method");
}

void BaseMatcher::setOptions(const char* options)
{
    std::map<std::string, std::string> option_map;
    std::vector<std::string> allowed_props;
    allowed_props.push_back(_matcher_params_prop);
    allowed_props.push_back(_matcher_part_prop);
    Properties::parseOptions(options, option_map, &allowed_props);

    if (option_map.find(_matcher_params_prop) != option_map.end())
        _setParameters(option_map[_matcher_params_prop].c_str());

    if (option_map.find(_matcher_part_prop) != option_map.end())
    {
        std::stringstream part_str;
        part_str << option_map[_matcher_part_prop];

        int part_count, part_id;
        char sep;

        part_str >> part_id;
        part_str >> sep;
        part_str >> part_count;

        // bool ef = part_str.eof();

        if (part_str.fail() || sep != '/' || part_id <= 0 || part_count <= 0 || part_id > part_count)
            throw Exception("BaseMatcher: setOptions: incorrect partitioning parameters");

        _part_id = part_id;
        _part_count = part_count;
        _initPartition();
    }
}

bool BaseMatcher::_isCurrentObjectExist()
{
    int cf_len;
    _index.getCfStorage().get(_current_id, cf_len);

    if (cf_len == -1)
        return false;

    return true;
}

bool BaseMatcher::_loadCurrentObject()
{
    try
    {
        if (_current_obj == nullptr)
            throw Exception("BaseMatcher: Matcher's current object was destroyed");

        profTimerStart(t_get_cmf, "loadCurObj_get_cf");
        ByteBufferStorage& cf_storage = _index.getCfStorage();

        int cf_len;
        const char* cf_str = (const char*)cf_storage.get(_current_id, cf_len);

        if (cf_len == -1)
            return false;
        profTimerStop(t_get_cmf);

        profTimerStart(t_load_cmf, "loadCurObj_load_cf");
        BufferScanner buf_scn(cf_str, cf_len);

        if (IndigoMolecule::is(*_current_obj))
        {
            Molecule& mol = _current_obj->getMolecule();

            CmfLoader cmf_loader(buf_scn);

            cmf_loader.loadMolecule(mol);
        }
        else if (IndigoReaction::is(*_current_obj))
        {
            Reaction& rxn = _current_obj->getReaction();

            CrfLoader crf_loader(buf_scn);

            crf_loader.loadReaction(rxn);
        }
        else
            throw Exception("BaseMatcher::unknown current object type");

        profTimerStop(t_load_cmf);

        return true;
    }
    catch (Exception& ex)
    {
        const int db_id = _index.getIdMapping()[_current_id];
        ex.appendMessage(" on id=%d", db_id);
        throw;
    }
}

int BaseMatcher::esimateRemainingResultsCount(int& delta)
{
    _match_probability_esimate.setCount(_current_id + 1);

    float p = _match_probability_esimate.mean();
    float error = _match_probability_esimate.meanEsimationError();

    int left_obj_count = _index.getObjectsCount() - _match_time_esimate.getCount();
    delta = (int)(error * left_obj_count);

    return (int)(left_obj_count * p);
}

float BaseMatcher::esimateRemainingTime(float& delta)
{
    _match_time_esimate.setCount(_current_id + 1);

    float mean_time = _match_time_esimate.mean();
    float error = _match_time_esimate.meanEsimationError();

    int left_obj_count = _index.getObjectsCount() - _match_time_esimate.getCount();
    delta = error * left_obj_count;
    return left_obj_count * mean_time;
}

//
// BaseSubstructureMatcher
//

BaseSubstructureMatcher::BaseSubstructureMatcher(/*const */ BaseIndex& index, IndigoObject*& current_obj)
    : BaseMatcher(index, current_obj), _fp_storage(_index.getSubStorage())
{
    _fp_size = _index.getFingerprintParams().fingerprintSize();

    _current_id = -1;
    _current_cand_id = -1;
    _current_pack = -1;
    _final_pack = _fp_storage.getPackCount() + 1;

    _cand_count = 0;
}

bool BaseSubstructureMatcher::next()
{
    // int fp_size_in_bits = _fp_size * 8;
    // static int sub_cnt = 0;

    _current_cand_id++;
    while (!((_current_pack == _final_pack) && (_current_cand_id == _candidates.size())))
    {
        profTimerStart(tsingle, "sub_single");

        if (_current_cand_id == _candidates.size())
        {
            profTimerStart(tf, "sub_find_cand");
            _current_pack++;
            if (_current_pack < _final_pack)
            {
                _findPackCandidates(_current_pack);
                _cand_count += _candidates.size();
            }
            else
                break;

            _current_cand_id = 0;
            profTimerStop(tf);
        }

        if (_candidates.size() == 0)
            continue;

        _current_id = _candidates[_current_cand_id];

        profTimerStart(tt, "sub_try");
        bool status = _tryCurrent();
        profTimerStop(tt);

        if (status)
            profIncCounter("sub_found", 1);

        _match_probability_esimate.addValue((float)status);
        _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));

        if (status)
        {
            sub_cnt++;
            return true;
        }
        _current_cand_id++;
    }

    profIncCounter("sub_count_cand", _cand_count);
    return false;
}

void BaseSubstructureMatcher::setQueryData(SubstructureQueryData* query_data)
{
    _query_data.reset(query_data);

    const MoleculeFingerprintParameters& fp_params = _index.getFingerprintParams();
    _query_data->getQueryObject().buildFingerprint(fp_params, &_query_fp, 0);

    int bit_cnt = bitGetOnesCount(_query_fp.ptr(), _fp_size);

    profIncCounter("query_bit_count", bit_cnt);

    _query_fp_bits_used.clear();
    for (int i = 0; i < _fp_size * 8; i++)
    {
        if (bitGetBit(_query_fp.ptr(), i))
            _query_fp_bits_used.push(i);
    }

    // Sort bits accoring to the bits frequency
    MMFArray<int>& fp_bit_usage = _index.getSubStorage().getFpBitUsageCounts();
    std::sort(_query_fp_bits_used.ptr(), _query_fp_bits_used.ptr() + _query_fp_bits_used.size(),
              [&](int i1, int i2) { return fp_bit_usage[i1] < fp_bit_usage[i2]; });
}

void BaseSubstructureMatcher::_findPackCandidates(int pack_idx)
{
    if (pack_idx == _fp_storage.getPackCount())
    {
        _findIncCandidates();
        return;
    }

    profTimerStart(t, "sub_find_cand_pack");

    _candidates.clear();

    TranspFpStorage& fp_storage = _index.getSubStorage();

    // const byte *query_fp = _query_fp.ptr();

    const byte* block;

    int fp_size_in_bits = _fp_size * 8;

    Array<byte> fit_bits;
    fit_bits.clear_resize(fp_storage.getBlockSize());
    fit_bits.fill(255);

    profTimerStart(tgs, "sub_find_cand_pack_get_search");
    int left = 0, right = fp_storage.getBlockSize() - 1;

    // Filter only based on the first 10 bits
    // TODO: collect time infromation about the reading and matching measurements and
    // and balance between reading new block or check filtered items without reading new block
    for (int i = 0; i < _query_fp_bits_used.size() && i < 15; i++)
    {
        int j = _query_fp_bits_used[i];

        profTimerStart(tgb, "sub_find_cand_pack_get_block");
        block = fp_storage.getBlock(pack_idx * fp_size_in_bits + j);
        profTimerStop(tgb);

        profTimerStart(tgu, "sub_find_cand_pack_fit_update");
        bitAnd(fit_bits.ptr() + left, &block[0] + left, right - left + 1);

        while (left <= right && fit_bits[left] == 0)
            left++;
        while (left <= right && fit_bits[right] == 0)
            right--;

        if (left > right)
            // Not more results
            break;

        profTimerStop(tgu);
    }
    profTimerStop(tgs);

    for (int k = 0; k < 8 * fp_storage.getBlockSize(); k++)
        if (bitGetBit(fit_bits.ptr(), k))
            _candidates.push(k + pack_idx * fp_storage.getBlockSize() * 8);
}

void BaseSubstructureMatcher::_findIncCandidates()
{
    profTimerStart(t, "sub_find_cand_inc");
    _candidates.clear();

    const TranspFpStorage& fp_storage = _index.getSubStorage();

    int inc_block_id_offset = fp_storage.getPackCount() * fp_storage.getBlockSize() * 8;
    const byte* inc = fp_storage.getIncrement();
    for (int i = 0; i < fp_storage.getIncrementSize(); i++)
    {
        const byte* fp = inc + i * _fp_size;
        if (bitTestOnes(_query_fp.ptr(), fp, _fp_size))
            _candidates.push(i + inc_block_id_offset);
    }
}

void BaseSubstructureMatcher::_setParameters(const char* params)
{
}

void BaseSubstructureMatcher::_initPartition()
{
    int pack_count_with_inc = _fp_storage.getPackCount() + 1;

    if (_part_count > pack_count_with_inc)
    {
        if (_part_id > pack_count_with_inc)
        {
            _current_pack = -1;
            _final_pack = -1;
        }
        else
        {
            _current_pack = (_part_id - 1) - 1;
            _final_pack = _part_id;
        }
    }
    else
    {
        _current_pack = (_part_id - 1) * pack_count_with_inc / _part_count - 1;
        _final_pack = _part_id * pack_count_with_inc / _part_count;
    }
}

MoleculeSubMatcher::MoleculeSubMatcher(/*const */ BaseIndex& index)
    : BaseSubstructureMatcher(index, (IndigoObject*&)_current_mol), _current_mol(new IndexCurrentMolecule(_current_mol))
{
    _mapping.clear();
}

const Array<int>& MoleculeSubMatcher::currentMapping()
{
    return _mapping;
}

bool MoleculeSubMatcher::_tryCurrent() // const
{
    SubstructureMoleculeQuery& query = (SubstructureMoleculeQuery&)(_query_data->getQueryObject());
    QueryMolecule& query_mol = (QueryMolecule&)(query.getMolecule());

    if (!_loadCurrentObject())
        return false;

    if (_current_obj == 0)
        throw Exception("MoleculeSubMatcher: Matcher's current object was destroyed");

    Molecule& target_mol = _current_obj->getMolecule();

    profTimerStart(tr_m, "sub_try_matching");
    MoleculeSubstructureMatcher msm(target_mol);

    msm.setQuery(query_mol);

    bool find_res = msm.find();

    profTimerStop(tr_m);

    if (find_res)
    {
        _mapping.copy(msm.getTargetMapping(), target_mol.vertexCount());
        return true;
    }

    // return true;
    return false;
}

ReactionSubMatcher::ReactionSubMatcher(/*const */ BaseIndex& index)
    : BaseSubstructureMatcher(index, (IndigoObject*&)_current_rxn), _current_rxn(new IndexCurrentReaction(_current_rxn))
{
    _mapping.clear();
}

const ObjArray<Array<int>>& ReactionSubMatcher::currentMapping()
{
    return _mapping;
}

bool ReactionSubMatcher::_tryCurrent() // const
{
    SubstructureReactionQuery& query = (SubstructureReactionQuery&)_query_data->getQueryObject();
    QueryReaction& query_rxn = (QueryReaction&)(query.getReaction());

    if (!_loadCurrentObject())
        return false;

    if (_current_obj == 0)
        throw Exception("ReactionSubMatcher: Matcher's current object was destroyed");

    Reaction& target_rxn = _current_obj->getReaction();

    ReactionSubstructureMatcher rsm(target_rxn);

    rsm.setQuery(query_rxn);

    if (rsm.find())
    {
        _mapping.resize(target_rxn.end());
        for (int i = target_rxn.begin(); i != target_rxn.end(); i = target_rxn.next(i))
            _mapping[i].clear();

        for (int i = query_rxn.begin(); i != query_rxn.end(); i = query_rxn.next(i))
        {
            int target_mol_idx = rsm.getTargetMoleculeIndex(i);

            _mapping[target_mol_idx].copy(rsm.getQueryMoleculeMapping(i), query_rxn.getQueryMolecule(i).vertexCount());
        }

        return true;
    }

    return false;
}

BaseSimilarityMatcher::BaseSimilarityMatcher(/*const */ BaseIndex& index, IndigoObject*& current_obj) : BaseMatcher(index, current_obj)
{
    _min_cell = -1;
    _max_cell = -1;
    _first_cell = -1;
    _containers_count = 0;
    _current_id = -1;
    _current_cell = 0;
    _current_container = -1;
    _current_portion_id = 0;
    _current_portion.clear();
    _current_sim_value = -1;
    _fp_size = _index.getFingerprintParams().fingerprintSizeSim();
    _sim_coef = std::make_unique<TanimotoCoef>(_fp_size);
}

bool BaseSimilarityMatcher::next()
{
    profTimerStart(tsimnext, "sim_next");

    SimStorage& sim_storage = _index.getSimStorage();
    int query_bit_count = bitGetOnesCount(_query_fp.ptr(), _fp_size);

    if (_current_cell == -1)
        return false;

    while (true)
    {
        profTimerStart(tsingle, "sim_single");

        if (_current_portion_id >= _current_portion.size())
        {
            _current_portion_id = 0;
            _current_container++;

            if (!sim_storage.isSmallBase())
            {
                if (_current_container == sim_storage.getCellSize(_current_cell))
                {
                    _current_cell = sim_storage.nextFitCell(query_bit_count, _first_cell, _min_cell, _max_cell, _current_cell);

                    if (_part_count != -1 && _part_id != -1)
                        while ((_current_cell % _part_count != _part_id - 1) && (_current_cell != -1))
                            _current_cell = sim_storage.nextFitCell(query_bit_count, _first_cell, _min_cell, _max_cell, _current_cell);

                    if (_current_cell == -1)
                        return false;

                    _current_container = 0;
                }

                _current_portion.clear();
                sim_storage.getSimilar(_query_fp.ptr(), *_sim_coef, _query_data->getMin(), _current_portion, _current_cell, _current_container);
            }
            else
            {
                if (_current_container > 0)
                    return false;

                _current_portion.clear();
                sim_storage.getIncSimilar(_query_fp.ptr(), *_sim_coef, _query_data->getMin(), _current_portion);
            }

            _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));
            _match_probability_esimate.addValue((float)_current_portion.size());

            continue;
        }

        _current_id = _current_portion[_current_portion_id].id;
        _current_sim_value = _current_portion[_current_portion_id].sim_value;

        _current_portion_id++;

        bool is_obj_exist = _isCurrentObjectExist();

        if (!is_obj_exist)
        {
            _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));
            continue;
        }

        _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));
        _loadCurrentObject();
        return true;
    }
}

void BaseSimilarityMatcher::setQueryData(SimilarityQueryData* query_data)
{
    _query_data.reset(query_data);

    const MoleculeFingerprintParameters& fp_params = _index.getFingerprintParams();
    _query_data->getQueryObject().buildFingerprint(fp_params, 0, &_query_fp);

    SimStorage& sim_storage = _index.getSimStorage();

    int query_bit_count = bitGetOnesCount(_query_fp.ptr(), _fp_size);

    if (sim_storage.isSmallBase())
        return;

    sim_storage.getCellsInterval(_query_fp.ptr(), *_sim_coef.get(), _query_data->getMin(), _min_cell, _max_cell);

    _first_cell = sim_storage.firstFitCell(query_bit_count, _min_cell, _max_cell);
    _current_cell = _first_cell;

    if (_part_count != -1 && _part_id != -1)
    {
        while (((_current_cell % _part_count) != _part_id - 1) && (_current_cell != -1))
            _current_cell = sim_storage.nextFitCell(query_bit_count, _first_cell, _min_cell, _max_cell, _current_cell);
    }
    _containers_count = 0;
    for (int i = _min_cell; i <= _max_cell; i++)
        _containers_count += sim_storage.getCellSize(i);
}

void BaseSimilarityMatcher::setQueryDataWithExtFP(SimilarityQueryData* query_data, IndigoObject& fp)
{
    _query_data.reset(query_data);

    const MoleculeFingerprintParameters& fp_params = _index.getFingerprintParams();
    IndigoFingerprint& ext_fp = IndigoFingerprint::cast(fp);
    if (_index.getFingerprintParams().fingerprintSizeSim() == ext_fp.bytes.size())
        _query_fp.copy(ext_fp.bytes);
    else
        throw Exception("BaseSimilarityMatcher: external fingerprint is incompatible with current database");

    SimStorage& sim_storage = _index.getSimStorage();

    int query_bit_count = bitGetOnesCount(_query_fp.ptr(), _fp_size);

    if (sim_storage.isSmallBase())
        return;

    sim_storage.getCellsInterval(_query_fp.ptr(), *_sim_coef.get(), _query_data->getMin(), _min_cell, _max_cell);

    _first_cell = sim_storage.firstFitCell(query_bit_count, _min_cell, _max_cell);
    _current_cell = _first_cell;

    if (_part_count != -1 && _part_id != -1)
    {
        while (((_current_cell % _part_count) != _part_id - 1) && (_current_cell != -1))
            _current_cell = sim_storage.nextFitCell(query_bit_count, _first_cell, _min_cell, _max_cell, _current_cell);
    }
    _containers_count = 0;
    for (int i = _min_cell; i <= _max_cell; i++)
        _containers_count += sim_storage.getCellSize(i);
}

void BaseSimilarityMatcher::resetThresholdLimit(float min)
{
    SimStorage& sim_storage = _index.getSimStorage();

    int query_bit_count = bitGetOnesCount(_query_fp.ptr(), _fp_size);

    _query_data->setMin(min);

    _min_cell = -1;
    _max_cell = -1;
    _first_cell = -1;
    _containers_count = 0;
    _current_id = -1;
    _current_cell = 0;
    _current_container = -1;
    _current_portion_id = 0;
    _current_portion.clear();
    _current_sim_value = -1;

    if (sim_storage.isSmallBase())
        return;

    sim_storage.getCellsInterval(_query_fp.ptr(), *_sim_coef.get(), min, _min_cell, _max_cell);

    _first_cell = sim_storage.firstFitCell(query_bit_count, _min_cell, _max_cell);
    _current_cell = _first_cell;

    if (_part_count != -1 && _part_id != -1)
    {
        while (((_current_cell % _part_count) != _part_id - 1) && (_current_cell != -1))
            _current_cell = sim_storage.nextFitCell(query_bit_count, _first_cell, _min_cell, _max_cell, _current_cell);
    }
    _containers_count = 0;
    for (int i = _min_cell; i <= _max_cell; i++)
        _containers_count += sim_storage.getCellSize(i);
}

void BaseSimilarityMatcher::_setParameters(const char* parameters)
{
    if (_query_data.get() != 0)
        throw Exception("BaseSimilarityMatcher: setParameters: query data have been already set");

    std::stringstream param_str;
    param_str << parameters;

    std::string type;

    param_str >> type;

    if (param_str.fail())
        throw Exception("BaseSimilarityMatcher: setParameters: incorrect similarity parameters");

    if (type.compare("tanimoto") == 0)
    {
        if (!param_str.eof())
            throw Exception("BaseSimilarityMatcher: setParameters: tanimoto metric has no parameters");

        _sim_coef = std::make_unique<TanimotoCoef>(_fp_size);
    }
    else if (type.compare("euclid-sub") == 0)
    {
        if (!param_str.eof())
            throw Exception("BaseSimilarityMatcher: setParameters: euclid-sub metric has no parameters");

        _sim_coef = std::make_unique<EuclidCoef>(_fp_size);
    }
    else if (type.compare("tversky") == 0)
    {
        double alpha, beta;

        if (!param_str.eof())
        {
            param_str >> alpha;

            if (param_str.fail())
                throw Exception("BaseSimilarityMatcher: setParameters: incorrect similarity parameters. Allowed 'tversky <alpha> <beta>'");

            param_str >> beta;

            if (param_str.fail())
                throw Exception("BaseSimilarityMatcher: setParameters: incorrect similarity parameters. Allowed 'tversky <alpha> <beta>'");
        }
        else
        {
            alpha = beta = 0.5;
        }

        if (fabs(alpha + beta - 1) > EPSILON)
            throw Exception("BaseSimilarityMatcher: setParameters: Tversky parameters have to satisfy the condition: alpha + beta = 1 ");

        _sim_coef = std::make_unique<TverskyCoef>(_fp_size, alpha, beta);
    }
    else
        throw Exception("BaseSimilarityMatcher: setParameters: incorrect similarity parameters. Allowed types: tanimoto, euclid-sub, tversky [<alpha> <beta>]");
}

void BaseSimilarityMatcher::_initPartition()
{
}

int BaseSimilarityMatcher::esimateRemainingResultsCount(int& delta)
{
    int left_cont_count = _containers_count - _match_probability_esimate.getCount();

    float error = _match_probability_esimate.meanEsimationError();
    delta = (int)(error * left_cont_count);

    return (int)(_match_probability_esimate.mean() * left_cont_count);
}

float BaseSimilarityMatcher::esimateRemainingTime(float& delta)
{
    _match_time_esimate.setCount(_match_probability_esimate.getCount());

    int left_cont_count = _containers_count - _match_probability_esimate.getCount();

    float error = _match_time_esimate.meanEsimationError();
    delta = error * left_cont_count;

    return _match_time_esimate.mean() * left_cont_count;
}

int BaseSimilarityMatcher::containersCount() const
{
    return _containers_count;
}

int BaseSimilarityMatcher::cellsCount() const
{
    return _max_cell - _min_cell + 1;
}

int BaseSimilarityMatcher::currentCell() const
{
    return _current_cell;
}

int BaseSimilarityMatcher::minCell() const
{
    return _min_cell;
}

int BaseSimilarityMatcher::maxCell() const
{
    return _max_cell;
}

float BaseSimilarityMatcher::currentSimValue() const
{
    return _current_sim_value;
}

BaseSimilarityMatcher::~BaseSimilarityMatcher()
{
    _current_block.clear();
}

MoleculeSimMatcher::MoleculeSimMatcher(/*const */ BaseIndex& index)
    : BaseSimilarityMatcher(index, (IndigoObject*&)_current_mol), _current_mol(new IndexCurrentMolecule(_current_mol))
{
}

ReactionSimMatcher::ReactionSimMatcher(/*const */ BaseIndex& index)
    : BaseSimilarityMatcher(index, (IndigoObject*&)_current_rxn), _current_rxn(new IndexCurrentReaction(_current_rxn))
{
}

TopNSimMatcher::TopNSimMatcher(BaseIndex& index, IndigoObject*& current_obj) : BaseSimilarityMatcher(index, current_obj)
{
    _idx = -1;
    _limit = 0;
    _result_ids.clear();
    _result_sims.clear();
}

bool TopNSimMatcher::next()
{
    if (_idx < 0)
    {
        _findTopN();
        if (_result_ids.size() > 0)
        {
            _idx = 0;
        }
    }

    if (_idx >= 0 && _idx < _result_ids.size())
    {
        _current_id = _result_ids[_idx];
        _current_sim_value = _result_sims[_idx];
        _idx++;

        bool is_obj_exist = _isCurrentObjectExist();

        if (!is_obj_exist)
        {
            return false;
        }

        _loadCurrentObject();

        return true;
    }

    return false;
}

void TopNSimMatcher::_findTopN()
{
    QS_DEF(Array<float>, thrs);
    QS_DEF(Array<int>, nhits_per_block);
    QS_DEF(Array<int>, blocks);
    QS_DEF(Array<int>, cells);

    thrs.clear();
    nhits_per_block.clear();
    blocks.clear();
    cells.clear();

    SimStorage& sim_storage = _index.getSimStorage();

    float thr_low_limit = _query_data->getMin();
    int hits_limit = _limit;

    _initModelDistribution(thrs, nhits_per_block);

    blocks.clear_resize(thrs.size());
    cells.clear_resize(thrs.size());
    _current_results.clear();

    bool already_found = false;
    bool too_many = false;
    float thr_proc = 0.0f;
    float cur_thr = 0.0f;
    float thr_too_many = 0.0f;
    int nhits = 0;
    int start_iter = thrs.size();

    int cont_count = 0;
    int cells_count = 0;
    int min_cell = 0;
    int max_cell = 0;
    int cur_cell = 0;

    int i;
    int cnt = 0;

    for (i = 0; i < thrs.size(); i++)
    {
        if (too_many)
        {
            if (thr_proc == 0.0f)
                cur_thr = thr_too_many + (1.0f - thr_too_many) / 2;
            else
                cur_thr = thr_too_many + (thr_proc - thr_too_many) / 2;
        }
        else
            cur_thr = thrs[i];

        resetThresholdLimit(cur_thr);

        cont_count = containersCount();
        blocks[i] = cont_count;
        cells_count = cellsCount();
        cells[i] = cells_count;
        min_cell = minCell();
        max_cell = maxCell();

        if ((cont_count > 0) && (nhits == 0))
        {
            cnt = 0;
            _current_results.clear();

            SimResult* res = 0;

            while (BaseSimilarityMatcher::next())
            {
                cnt++;
                res = &_current_results.push();
                res->id = _current_id;
                res->sim_value = _current_sim_value;
                cur_cell = currentCell();
                if ((cnt > hits_limit * 2) && ((max_cell - cur_cell) > cells_count / 2))
                {
                    thr_too_many = cur_thr;
                    too_many = true;
                    break;
                }
                else
                    too_many = false;
            }

            if (too_many)
                continue;

            if ((cnt >= hits_limit) || (cur_thr <= thr_low_limit))
            {
                already_found = true;
                break;
            }

            nhits = (cnt / cont_count);

            if ((nhits == 0) && (cnt > 0))
                nhits = 1;

            thr_proc = cur_thr;
            nhits_per_block[i] = nhits;
            if (i < thrs.size() - 2)
                start_iter = i + 1;
            else
            {
                already_found = true;
                break;
            }

            if (thr_too_many > 0.0)
            {
                thrs[i] = thr_proc;
                thrs[start_iter] = thr_proc - (thr_proc - thr_too_many) / 2;
                if (start_iter < thrs.size() - 2)
                    thrs[start_iter + 1] = thr_too_many;
            }
        }
        else if ((cont_count == 0) && sim_storage.isSmallBase())
        {
            cnt = 0;
            _current_results.clear();

            SimResult* res = 0;

            while (BaseSimilarityMatcher::next())
            {
                cnt++;
                res = &_current_results.push();
                res->id = _current_id;
                res->sim_value = _current_sim_value;

                if (cnt > hits_limit * 2)
                {
                    thr_too_many = cur_thr;
                    too_many = true;
                    break;
                }
                else
                    too_many = false;
            }

            if (too_many)
                continue;

            if ((cnt >= hits_limit) || (cur_thr <= thr_low_limit))
            {
                already_found = true;
                break;
            }
        }
        else if (cont_count == 0)
            continue;
        else
        {
            nhits_per_block[i] = nhits_per_block[i - 1] * 2;
        }
    }

    if (!already_found)
    {
        bool adjusted = false;
        float next_thr = 0.0f;
        float prev_thr = 0.0f;

        i = start_iter;

        while (i < thrs.size())
        {
            if ((blocks[i] * nhits_per_block[i] < hits_limit) && (i < (thrs.size() - 1)) && (!adjusted))
            {
                i++;
                continue;
            }

            cur_thr = thrs[i];

            if ((blocks[i] * nhits_per_block[i] > 2 * hits_limit) && (!adjusted))
            {
                next_thr = cur_thr + (thrs[i - 1] - cur_thr) / 2;
                prev_thr = cur_thr;
                cur_thr = next_thr;
                adjusted = true;
            }

            too_many = false;

            resetThresholdLimit(cur_thr);

            cont_count = containersCount();
            cells_count = cellsCount();
            cells[i] = cells_count;
            min_cell = minCell();
            max_cell = maxCell();

            if (cont_count > 0)
            {
                cnt = 0;
                _current_results.clear();

                SimResult* res = 0;

                while (BaseSimilarityMatcher::next())
                {
                    cnt++;
                    res = &_current_results.push();
                    res->id = _current_id;
                    res->sim_value = _current_sim_value;
                    cur_cell = currentCell();
                    if ((cnt > hits_limit * 2) && (max_cell - cur_cell) > cells_count / 2)
                    {
                        next_thr = cur_thr + (thr_proc - cur_thr) / 2;
                        thr_too_many = cur_thr;
                        thrs[i] = next_thr;
                        too_many = true;
                        adjusted = true;
                        break;
                    }
                }

                if (((cnt >= hits_limit) || (cur_thr <= thr_low_limit)) && (!too_many))
                    break;
            }
            else
                i++;

            if (!too_many)
            {
                thr_proc = cur_thr;
                nhits_per_block[i] = cnt / cont_count;
                if (i < (thrs.size() - 2))
                    nhits_per_block[i + 1] = nhits_per_block[i] * 2;

                if (thr_too_many > 0.0)
                {
                    next_thr = cur_thr - (cur_thr - thr_too_many) / 2;
                    thrs[i] = next_thr;
                }
                else if (i < (thrs.size() - 2))
                {
                    if ((hits_limit - cnt) < (nhits_per_block[i + 1] * (blocks[i + 1] - cont_count)))
                    {
                        next_thr = cur_thr - (cur_thr - thrs[i + 1]) / 2;
                        thrs[i] = next_thr;
                        adjusted = true;
                    }
                    else
                    {
                        i++;
                        next_thr = thrs[i];
                    }
                }
            }

            cur_thr = next_thr;
        }
    }

    if (_current_results.size() > 0)
    {
        _current_results.qsort(_cmp_sim_res, 0);

        for (i = 0; i < _current_results.size(); i++)
        {
            _result_ids.push(_current_results[i].id);
            _result_sims.push(_current_results[i].sim_value);

            if (i == (hits_limit - 1))
                break;
        }
    }
}

int TopNSimMatcher::_cmp_sim_res(SimResult& res1, SimResult& res2, void* context)
{
    if ((res1.sim_value - res2.sim_value) > 0.0)
        return -1;
    else if ((res1.sim_value - res2.sim_value) < 0.0)
        return 1;

    return 0;
}

void TopNSimMatcher::_initModelDistribution(Array<float>& model_thrs, Array<int>& model_nhits_per_block)
{
    for (int i = 0; i < 9; i++)
    {
        model_thrs.push(_2FLOAT(1.0 - 0.1 * (i + 1)));
        model_nhits_per_block.push(5 * 2 ^ (i));
    }
}

void TopNSimMatcher::setLimit(int limit)
{
    _limit = limit;
}

TopNSimMatcher::~TopNSimMatcher()
{
}

MoleculeTopNSimMatcher::MoleculeTopNSimMatcher(BaseIndex& index)
    : TopNSimMatcher(index, (IndigoObject*&)_current_mol), _current_mol(new IndexCurrentMolecule(_current_mol))
{
}

ReactionTopNSimMatcher::ReactionTopNSimMatcher(BaseIndex& index)
    : TopNSimMatcher(index, (IndigoObject*&)_current_rxn), _current_rxn(new IndexCurrentReaction(_current_rxn))
{
}

BaseExactMatcher::BaseExactMatcher(BaseIndex& index, IndigoObject*& current_obj) : BaseMatcher(index, current_obj)
{
    _candidates.clear();
    _current_cand_id = 0;
    _flags = 0;
}

bool BaseExactMatcher::next()
{
    ExactStorage& exact_storage = _index.getExactStorage();

    if (_candidates.size() == 0)
        exact_storage.findCandidates(_query_hash, _candidates, _part_id, _part_count);

    while (_current_cand_id < _candidates.size())
    {
        profTimerStart(tsingle, "exact_single");

        _current_id = _candidates[_current_cand_id];
        _current_cand_id++;

        bool status = _tryCurrent();
        if (status)
            profIncCounter("exact_found", 1);

        _match_probability_esimate.addValue((float)status);
        _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));

        if (status)
            return true;
    }

    return false;
}

void BaseExactMatcher::setQueryData(ExactQueryData* query_data)
{
    _query_data.reset(query_data);

    _query_hash = _calcHash();
}

void BaseExactMatcher::_initPartition()
{
}

BaseExactMatcher::~BaseExactMatcher()
{
}

MolExactMatcher::MolExactMatcher(/*const */ BaseIndex& index)
    : BaseExactMatcher(index, (IndigoObject*&)_current_mol), _current_mol(new IndexCurrentMolecule(_current_mol))
{
    _tautomer = false;
}

void MolExactMatcher::_setParameters(const char* parameters)
{
    if (_indigoParseTautomerFlags(parameters, _tautomer_params))
    {
        _tautomer = true;
    }
    else
    {
        _tautomer = false;
        MoleculeExactMatcher::parseConditions(parameters, _flags, _rms_threshold);
    }
}

dword MolExactMatcher::_calcHash()
{
    SimilarityMoleculeQuery& query = (SimilarityMoleculeQuery&)(_query_data->getQueryObject());
    Molecule& query_mol = (Molecule&)(query.getMolecule());

    return ExactStorage::calculateMolHash(query_mol);
}

bool MolExactMatcher::_tryCurrent() /* const */
{
    SimilarityMoleculeQuery& query = (SimilarityMoleculeQuery&)(_query_data->getQueryObject());
    Molecule& query_mol = (Molecule&)(query.getMolecule());

    if (!_loadCurrentObject())
        return false;

    if (_current_obj == 0)
        throw Exception("MoleculeExactMatcher: Matcher's current object was destroyed");

    Molecule& target_mol = _current_obj->getMolecule();

    if (_tautomer)
    {
        MoleculeTautomerMatcher matcher(target_mol, false);

        Indigo& indigo = indigoGetInstance();

        matcher.arom_options = indigo.arom_options;
        matcher.setRulesList(&indigo.tautomer_rules);
        matcher.setRules(_tautomer_params.conditions, _tautomer_params.force_hydrogens, _tautomer_params.ring_chain, _tautomer_params.method);
        matcher.setQuery(query_mol);
        return matcher.find();
    }
    else
    {
        MoleculeExactMatcher mem(query_mol, target_mol);
        mem.flags = _flags;
        mem.rms_threshold = _rms_threshold;

        return mem.find();
    }
}

RxnExactMatcher::RxnExactMatcher(/*const */ BaseIndex& index)
    : BaseExactMatcher(index, (IndigoObject*&)_current_rxn), _current_rxn(new IndexCurrentReaction(_current_rxn))
{
}

void RxnExactMatcher::_setParameters(const char* flags)
{
    // TODO: merge Indigo code into Bingo and stop this endless copy-paste

    if (flags == 0)
        flags = "";

    static struct
    {
        const char* token;
        int value;
    } token_list[] = {{"ELE", MoleculeExactMatcher::CONDITION_ELECTRONS},
                      {"MAS", MoleculeExactMatcher::CONDITION_ISOTOPE},
                      {"STE", MoleculeExactMatcher::CONDITION_STEREO},
                      {"AAM", ReactionExactMatcher::CONDITION_AAM},
                      {"RCT", ReactionExactMatcher::CONDITION_REACTING_CENTERS}};

    int i, res = 0, count = 0;
    bool had_none = false;
    bool had_all = false;

    BufferScanner scanner(flags);

    QS_DEF(Array<char>, word);
    while (1)
    {
        scanner.skipSpace();
        if (scanner.isEOF())
            break;
        scanner.readWord(word, 0);

        if (strcasecmp(word.ptr(), "NONE") == 0)
        {
            if (had_all)
                throw Exception("RxnExactMatcher: setParameters: NONE conflicts with ALL");
            had_none = true;
            count++;
            continue;
        }
        if (strcasecmp(word.ptr(), "ALL") == 0)
        {
            if (had_none)
                throw Exception("RxnExactMatcher: setParameters: ALL conflicts with NONE");
            had_all = true;
            res = MoleculeExactMatcher::CONDITION_ALL | ReactionExactMatcher::CONDITION_ALL;
            count++;
            continue;
        }

        for (i = 0; i < NELEM(token_list); i++)
        {
            if (strcasecmp(token_list[i].token, word.ptr()) == 0)
            {
                if (had_all)
                    throw Exception("RxnExactMatcher: setParameters: only negative flags are allowed together with ALL");
                res |= token_list[i].value;
                break;
            }
            if (word[0] == '-' && strcasecmp(token_list[i].token, word.ptr() + 1) == 0)
            {
                res &= ~token_list[i].value;
                break;
            }
        }
        if (i == NELEM(token_list))
            throw Exception("RxnExactMatcher: setParameters: unknown token %s", word.ptr());
        count++;
    }

    if (had_none && count > 1)
        throw Exception("RxnExactMatcher: setParameters: no flags are allowed together with NONE");

    if (count == 0)
        res = MoleculeExactMatcher::CONDITION_ALL | ReactionExactMatcher::CONDITION_ALL;

    _flags = res;
}

dword RxnExactMatcher::_calcHash()
{
    SimilarityReactionQuery& query = (SimilarityReactionQuery&)_query_data->getQueryObject();
    Reaction& query_rxn = (Reaction&)(query.getReaction());

    return ExactStorage::calculateRxnHash(query_rxn);
}

bool RxnExactMatcher::_tryCurrent() /* const */
{
    SimilarityReactionQuery& query = (SimilarityReactionQuery&)_query_data->getQueryObject();
    Reaction& query_rxn = (Reaction&)(query.getReaction());

    if (!_loadCurrentObject())
        return false;

    if (_current_obj == 0)
        throw Exception("ReactionExactMatcher: Matcher's current object was destroyed");

    Reaction& target_rxn = _current_obj->getReaction();

    ReactionExactMatcher rem(query_rxn, target_rxn);
    rem.flags = _flags;

    if (rem.find())
        return true;

    return false;
}

BaseGrossMatcher::BaseGrossMatcher(BaseIndex& index, IndigoObject*& current_obj) : BaseMatcher(index, current_obj)
{
    _candidates.clear();
    _current_cand_id = 0;
}

bool BaseGrossMatcher::next()
{
    GrossStorage& gross_storage = _index.getGrossStorage();
    GrossQuery& gross_qobj = (GrossQuery&)_query_data->getQueryObject();

    if (_candidates.size() == 0)
        gross_storage.findCandidates(gross_qobj.getGrossString(), _candidates, _part_id, _part_count);

    while (_current_cand_id < _candidates.size())
    {
        profTimerStart(tsingle, "exact_single");

        _current_id = _candidates[_current_cand_id];
        _current_cand_id++;

        bool status = _tryCurrent();
        if (status)
            profIncCounter("exact_found", 1);

        _match_probability_esimate.addValue((float)status);
        _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));

        if (status)
            return true;
    }

    return false;
}

void BaseGrossMatcher::setQueryData(GrossQueryData* query_data)
{
    _query_data.reset(query_data);
    GrossQuery& gross_qobj = (GrossQuery&)_query_data->getQueryObject();
    MoleculeGrossFormula::fromString(gross_qobj.getGrossString().ptr(), _query_array);

    _calcFormula();
}

void BaseGrossMatcher::_initPartition()
{
}

BaseGrossMatcher::~BaseGrossMatcher()
{
}

MolGrossMatcher::MolGrossMatcher(/*const */ BaseIndex& index)
    : BaseGrossMatcher(index, (IndigoObject*&)_current_mol), _current_mol(new IndexCurrentMolecule(_current_mol))
{
}

void MolGrossMatcher::_setParameters(const char* parameters)
{
}

void MolGrossMatcher::_calcFormula()
{
    GrossQuery& query = (GrossQuery&)(_query_data->getQueryObject());

    MoleculeGrossFormula::fromString(query.getGrossString().ptr(), _query_array);
}

bool MolGrossMatcher::_tryCurrent() /* const */
{
    GrossQuery& query = (GrossQuery&)(_query_data->getQueryObject());

    if (!_loadCurrentObject())
        return false;

    if (_current_obj == 0)
        throw Exception("MolGrossMatcher: Matcher's current object was destroyed");

    GrossStorage& gross_storage = _index.getGrossStorage();

    return gross_storage.tryCandidate(_query_array, _current_id);
}

EnumeratorMatcher::EnumeratorMatcher(BaseIndex& index) : BaseMatcher(index, (IndigoObject*&)_indigoObject)
{
    _id_numbers = index.getIdMapping().size();
    _indigoObject = nullptr;
}

bool EnumeratorMatcher::next()
{
    if (_current_id + 1 < _id_numbers)
    {
        _current_id++;
        return true;
    }
    return false;
}

void EnumeratorMatcher::_setParameters(const char* params)
{
}

void EnumeratorMatcher::_initPartition()
{
}
