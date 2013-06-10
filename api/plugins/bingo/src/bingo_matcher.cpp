#include "bingo_matcher.h"

#include "molecule/molecule_substructure_matcher.h"

#include "base_c/nano.h"
#include "base_c/bitarray.h"
#include "base_cpp/profiling.h"

#include <algorithm>

using namespace indigo;

using namespace bingo;


MoleculeSimilarityQueryData::MoleculeSimilarityQueryData (/* const */ Molecule &qmol, float min_coef, float max_coef) : 
   _obj(qmol), _min(min_coef), _max(max_coef)
{
}

/*const*/ QueryObject & MoleculeSimilarityQueryData::getQueryObject () /*const*/
{
   return _obj;
}

float MoleculeSimilarityQueryData::getMin () const
{
   return _min;
}

float MoleculeSimilarityQueryData::getMax () const
{
   return _max;
}

ReactionSimilarityQueryData::ReactionSimilarityQueryData (/* const */ Reaction &qrxn, float min_coef, float max_coef) : 
   _obj(qrxn), _min(min_coef), _max(max_coef)
{
}

/*const*/ QueryObject & ReactionSimilarityQueryData::getQueryObject() /*const*/
{
   return _obj;
}

float ReactionSimilarityQueryData::getMin () const
{
   return _min;
}

float ReactionSimilarityQueryData::getMax () const
{
   return _max;
}
   
MoleculeSubstructureQueryData::MoleculeSubstructureQueryData (/* const */ QueryMolecule &qmol) : _obj(qmol)
{
}

/*const*/ QueryObject & MoleculeSubstructureQueryData::getQueryObject () /*const*/
{
   return _obj;
}

ReactionSubstructureQueryData::ReactionSubstructureQueryData (/* const */ QueryReaction &qrxn) : _obj(qrxn)
{
}

/*const*/ QueryObject & ReactionSubstructureQueryData::getQueryObject() /*const*/
{
   return _obj;
}




IndexCurrentMolecule::IndexCurrentMolecule ( IndexCurrentMolecule *& ptr ) : _ptr(ptr)
{
   matcher_exist = true;
}

IndexCurrentMolecule::~IndexCurrentMolecule ()
{
   if (matcher_exist)
      _ptr = 0;
}

IndexCurrentReaction::IndexCurrentReaction ( IndexCurrentReaction *& ptr ) : _ptr(ptr)
{
   matcher_exist = true;
}

IndexCurrentReaction::~IndexCurrentReaction ()
{
   if (matcher_exist)
      _ptr = 0;
}


BaseMatcher::BaseMatcher(BaseIndex &index, IndigoObject *& current_obj) : _index(index), _current_obj(current_obj)
{
   _current_obj_used = false;
   _current_id = 0;
}

BaseMatcher::~BaseMatcher ()
{
   if (_current_obj && IndigoMolecule::is(*_current_obj))
      ((IndexCurrentMolecule *)_current_obj)->matcher_exist = false;
   else if (_current_obj && IndigoReaction::is(*_current_obj))
      ((IndexCurrentReaction *)_current_obj)->matcher_exist = false;

   if (!_current_obj_used)
      delete _current_obj;
}

int BaseMatcher::currentId ()
{
   const Array<int> &id_mapping = _index.getIdMapping();
   return id_mapping[_current_id];
}

IndigoObject * BaseMatcher::currentObject ()
{
   if (_current_obj_used)
      throw Exception("BaseMatcher: Object has been already gotten");

   _current_obj_used = true;

   return _current_obj;
}

const Index & BaseMatcher::getIndex ()
{
   return _index;
}

bool BaseMatcher::_loadCurrentObject()
{
   if (_current_obj == 0)
      throw Exception("BaseMatcher: Matcher's current object was destroyed");

   FlatStorage &cf_storage = _index.getCfStorage();

   int cf_len;
   const char *cf_str = (const char *)cf_storage.get(_current_id, cf_len);

   if (cf_len == -1)
      return false;

   BufferScanner buf_scn(cf_str, cf_len);
      
   if (IndigoMolecule::is(*_current_obj))
   {
      Molecule &mol = _current_obj->getMolecule();
   
      CmfLoader cmf_loader(buf_scn);

      cmf_loader.loadMolecule(mol);
   }
   else if (IndigoReaction::is(*_current_obj))
   {
      Reaction &rxn = _current_obj->getReaction();
   
      CrfLoader crf_loader(buf_scn);

      crf_loader.loadReaction(rxn);
   }
   else
      throw Exception("BaseMatcher::unknown current object type");

   return true;
}

int BaseMatcher::esimateRemainingResultsCount (int &delta)
{
   _match_probability_esimate.setCount(_current_id + 1);

   float p = _match_probability_esimate.mean();
   float error = _match_probability_esimate.meanEsimationError();

   int left_obj_count = _index.getObjectsCount() - _match_time_esimate.getCount();
   delta = (int)(error * left_obj_count);
   return (int)(left_obj_count * p);
}

float BaseMatcher::esimateRemainingTime (float &delta)
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

BaseSubstructureMatcher::BaseSubstructureMatcher (/*const */ BaseIndex &index, IndigoObject *& current_obj) : BaseMatcher(index, current_obj), _fp_storage(_index.getSubStorage())
{
   _fp_size = _index.getFingerprintParams().fingerprintSize();
   
   _current_id = -1;
   _current_cand_id = -1;
   _current_pack = -1;

   _cand_count = 0;
}

bool BaseSubstructureMatcher::next ()
{
   int fp_size_in_bits = _fp_size * 8;
   
   _current_cand_id++;
   while (!((_current_pack == _fp_storage.getPackCount()) && (_current_cand_id == _candidates.size())))
   {
      profTimerStart(tsingle, "sub_single");

      if (_current_cand_id == _candidates.size())
      {
         profTimerStart(tf, "sub_find_cand");
         _current_pack++;
         if (_current_pack < _fp_storage.getPackCount())
         {
            _findPackCandidates(_current_pack);
            _cand_count += _candidates.size();
         }
         else
         {
            _findIncCandidates();
            _cand_count += _candidates.size();
         }

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
         return true;

      _current_cand_id++;
   }

   profIncCounter("sub_count_cand", _cand_count);
   return false;
}

void BaseSubstructureMatcher::setQueryData (SubstructureQueryData *query_data)
{
   _query_data.reset(query_data);

   const MoleculeFingerprintParameters & fp_params = _index.getFingerprintParams();
   _query_data->getQueryObject().buildFingerprint(fp_params, &_query_fp, 0);

   int bit_cnt = bitGetOnesCount(_query_fp.ptr(), _fp_size);

   profIncCounter("query_bit_count", bit_cnt);

   _query_fp_bits_used.clear();
   for (int i = 0; i < _fp_size * 8; i++)
      if (bitGetBit(_query_fp.ptr(), i))
         _query_fp_bits_used.push(i);

   // Sort bits accoring to the bits frequency
   const Array<int> &fp_bit_usage = _index.getSubStorage().getFpBitUsageCounts();
   std::sort(_query_fp_bits_used.ptr(), _query_fp_bits_used.ptr() + _query_fp_bits_used.size(), 
      [&](int i1, int i2) 
      { 
         return fp_bit_usage[i1] < fp_bit_usage[i2];
      });
}

void BaseSubstructureMatcher::_findPackCandidates (int pack_idx)
{
   profTimerStart(t, "sub_find_cand_pack");

   _candidates.clear();

   const TranspFpStorage &fp_storage = _index.getSubStorage();

   const byte *query_fp = _query_fp.ptr();

   byte *block = new byte[fp_storage.getBlockSize()];

   int fp_size_in_bits = _fp_size * 8;

   Array<byte> fit_bits;
   fit_bits.clear_resize(fp_storage.getBlockSize());
   fit_bits.fill(255);

   profTimerStart(tgs, "sub_find_cand_pack_get_search");
   int left = 0, right = fp_storage.getBlockSize() - 1;

   // Filter only based on the first 10 bits
   // TODO: collect time infromation about the reading and matching measurements and
   // and balance between reading new block or check filtered items without reading new block
   for (int i = 0; i < _query_fp_bits_used.size() && i < 10; i++)
   {
      int j = _query_fp_bits_used[i];
      
      profTimerStart(tgb, "sub_find_cand_pack_get_block");
      fp_storage.getBlock(pack_idx * fp_size_in_bits + j, block);
      profTimerStop(tgb);

      profTimerStart(tgu, "sub_find_cand_pack_fit_update");
      bitAnd(fit_bits.ptr() + left, block + left, right - left + 1);

      while(left <= right && fit_bits[left] == 0)
         left++;
      while(left <= right && fit_bits[right] == 0)
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

   delete block;
}

void BaseSubstructureMatcher::_findIncCandidates ()
{
   profTimerStart(t, "sub_find_cand_inc");
   _candidates.clear();

   const TranspFpStorage &fp_storage = _index.getSubStorage();

   int inc_block_id_offset = fp_storage.getPackCount() * fp_storage.getBlockSize() * 8;
   const byte *inc = fp_storage.getIncrement();
   for (int i = 0; i < fp_storage.getIncrementSize(); i++)
   {
      const byte *fp = inc + i * _fp_size;
      if (bitTestOnes(_query_fp.ptr(), fp, _fp_size))
         _candidates.push(i + inc_block_id_offset);
   }
}


MoleculeSubMatcher::MoleculeSubMatcher (/*const */ BaseIndex &index) : _current_mol(new IndexCurrentMolecule(_current_mol)), BaseSubstructureMatcher(index, (IndigoObject *&)_current_mol)
{
   _mapping.clear();
}

const Array<int> & MoleculeSubMatcher::currentMapping ()
{
   return _mapping;
}

bool MoleculeSubMatcher::_tryCurrent ()// const
{
   SubstructureMoleculeQuery &query = (SubstructureMoleculeQuery &)(_query_data->getQueryObject());
   QueryMolecule &query_mol = (QueryMolecule &)(query.getMolecule());

   if (!_loadCurrentObject())
      return false;

   if (_current_obj == 0)
      throw Exception("MoleculeSubMatcher: Matcher's current object was destroyed");

   Molecule &target_mol = _current_obj->getMolecule();

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

   return false;
}
   
ReactionSubMatcher::ReactionSubMatcher (/*const */ BaseIndex &index) : _current_rxn(new IndexCurrentReaction(_current_rxn)), BaseSubstructureMatcher(index, (IndigoObject *&)_current_rxn)
{
   _mapping.clear();
}

const ObjArray<Array<int> > & ReactionSubMatcher::currentMapping ()
{
   return _mapping;
}

bool ReactionSubMatcher::_tryCurrent ()// const
{
   SubstructureReactionQuery &query = (SubstructureReactionQuery &)_query_data->getQueryObject();
   QueryReaction &query_rxn = (QueryReaction &)(query.getReaction());

   if (_current_obj == 0)
      throw Exception("ReactionSubMatcher: Matcher's current object was destroyed");

   Reaction &target_rxn = _current_obj->getReaction();

   if (!_loadCurrentObject())
      return false;

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

BaseSimilarityMatcher::BaseSimilarityMatcher (/*const */ BaseIndex &index, IndigoObject *& current_obj ) : BaseMatcher(index, current_obj)
{
   _current_id = -1;
   _current_block = new byte[_index.getSimStorage().getBlockSize()];
   _fp_size = _index.getFingerprintParams().fingerprintSizeSim();
}

bool BaseSimilarityMatcher::next ()
{
   _current_id++;
   const RowFpStorage &fp_storage = _index.getSimStorage();
   int stor_fp_count = fp_storage.getBlockCount() * fp_storage.getFpPerBlockCount() + fp_storage.getIncrementSize();
   FlatStorage &cf_storage = _index.getCfStorage();

   while (_current_id < stor_fp_count)
   {
      profTimerStart(tsingle, "sim_single");

      if (!_loadCurrentObject())
      {
         _current_id++;
         _match_probability_esimate.addValue(0);
         _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));
         continue;
      }

      int fp_per_block = fp_storage.getBlockSize() / _fp_size;
      int block_idx = _current_id / fp_per_block;

      if (_current_id % fp_per_block == 0)
      {
         if (block_idx < fp_storage.getBlockCount())
         {
            fp_storage.getBlock(block_idx, _current_block);
            _cur_loc = _current_block;
         }
         else
            _cur_loc = fp_storage.getIncrement();
      }
      int fp_idx_in_block = _current_id - block_idx * fp_per_block;
      float tan_coef = _calcTanimoto(_cur_loc + fp_idx_in_block * _fp_size);

      if (tan_coef < _query_data->getMin() || tan_coef > _query_data->getMax())
      {
         _current_id++;
         _match_probability_esimate.addValue(0);
         _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));
         continue;
      }

      _match_probability_esimate.addValue(1);
      _match_time_esimate.addValue(profTimerGetTimeSec(tsingle));
      return true;
   }

   return false;
}

void BaseSimilarityMatcher::setQueryData (SimilarityQueryData *query_data)
{
   _query_data.reset(query_data);

   const MoleculeFingerprintParameters & fp_params = _index.getFingerprintParams();
   _query_data->getQueryObject().buildFingerprint(fp_params, 0, &_query_fp);
}

BaseSimilarityMatcher::~BaseSimilarityMatcher ()
{
   delete _current_block;
}

float BaseSimilarityMatcher::_calcTanimoto (const byte *fp)
{
   int common_bits = bitCommonOnes(fp, _query_fp.ptr(), _fp_size);
   int unique_bits = bitUniqueOnes(fp, _query_fp.ptr(), _fp_size);
      
   return (float)common_bits / (common_bits + unique_bits);
}

MoleculeSimMatcher::MoleculeSimMatcher (/*const */ BaseIndex &index) : _current_mol(new IndexCurrentMolecule(_current_mol)), BaseSimilarityMatcher(index, (IndigoObject *&)_current_mol)
{
}

ReactionSimMatcher::ReactionSimMatcher (/*const */ BaseIndex &index) : _current_rxn(new IndexCurrentReaction(_current_rxn)), BaseSimilarityMatcher(index, (IndigoObject *&)_current_rxn)
{
}
