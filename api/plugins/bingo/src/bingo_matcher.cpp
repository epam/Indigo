#include "bingo_matcher.h"

#include "molecule/molecule_substructure_matcher.h"

#include "base_c/bitarray.h"
#include "base_cpp/profiling.h"

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
   
   
BaseSubstructureMatcher::BaseSubstructureMatcher (/*const */ BaseIndex &index) : 
   _index(index), _fp_storage(_index.getSubStorage())
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
      if (_tryCurrent())
         return true;
      profTimerStop(tt);

      _current_cand_id++;
   }

   profIncCounter("sub_count_cand", _cand_count);
   return false;
}

int BaseSubstructureMatcher::currentId ()
{
   return _current_id;
}

void BaseSubstructureMatcher::setQueryData (SubstructureQueryData *query_data)
{
   _query_data.reset(query_data);

   const MoleculeFingerprintParameters & fp_params = _index.getFingerprintParams();
   _query_data->getQueryObject().buildFingerprint(fp_params, &_query_fp, 0);

   int bit_cnt = bitGetOnesCount(_query_fp.ptr(), _fp_size);

   profIncCounter("query_bit_count", bit_cnt);
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
   for (int j = 0; j < fp_size_in_bits; j++)
   {
      byte query_bit = query_fp[j / 8] & (0x80 >> (j % 8));

      if (!query_bit)
         continue;
      
      profTimerStart(tgb, "sub_find_cand_pack_get_block");
      fp_storage.getBlock(pack_idx * fp_size_in_bits + j, block);
      profTimerStop(tgb);

      profTimerStart(tgu, "sub_find_cand_pack_fit_update");
      bitAnd(fit_bits.ptr() + left, block + left, right - left);

      while(fit_bits[left] == 0 && (left != right))
         left++;
      while(fit_bits[right] == 0 && (left != right))
         right--;

      profTimerStop(tgu);
   }
   profTimerStop(tgs);
   

   for (int k = 0; k < fp_storage.getBlockSize(); k++)
   {
      for (int bit_cnt  = 0; bit_cnt < 8; bit_cnt++)
      {
         byte fp_flag = fit_bits[k] & (0x80 >> bit_cnt);
            
         if (fp_flag)
            _candidates.push(k * 8 + bit_cnt + pack_idx * fp_storage.getBlockSize() * 8);
      }
   }

   delete block;
}

void BaseSubstructureMatcher::_findIncCandidates ()
{
   profTimerStart(t, "sub_find_cand_inc");
   _candidates.clear();
   Array<bool> is_candidate;

   const TranspFpStorage &fp_storage = _index.getSubStorage();

   const byte *query_fp = _query_fp.ptr();

   is_candidate.clear_resize(fp_storage.getIncrementSize());
   is_candidate.fill(true);

   const byte *inc = fp_storage.getIncrement();
   for (int i = 0; i < fp_storage.getIncrementSize(); i++)
   {
      const byte *fp = inc + i * _fp_size;
      bool cand_flag = true;

      for (int j = 0; j < _fp_size; j++)
      {
         if ((fp[j] & query_fp[j]) != query_fp[j])
         {
            cand_flag = false;
            break;
         }
      }

      is_candidate[i] = cand_flag;
   }

   for (int i = 0; i < is_candidate.size(); i++)
      if (is_candidate[i])
         _candidates.push(i + fp_storage.getPackCount() * fp_storage.getBlockSize() * 8);
}


MoleculeSubMatcher::MoleculeSubMatcher (/*const */ BaseIndex &index) : BaseSubstructureMatcher(index)
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
   Molecule target_mol;
   CfStorage &cf_storage = _index.getCfStorage();

   int cf_len;
   profTimerStart(tr_gr, "sub_try_get_cmf");
   const char *cf_str = cf_storage.get(_current_id, cf_len);
   profTimerStop(tr_gr);

   profTimerStart(tr_g, "sub_try_load_from_cmf");
   BufferScanner buf_scn(cf_str, cf_len);
   CmfLoader cmf_loader(buf_scn);

   if (cf_len == -1)
      return false;

   cmf_loader.loadMolecule(target_mol);
   profTimerStop(tr_g);

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
   
   
ReactionSubMatcher::ReactionSubMatcher (/*const */ BaseIndex &index) : BaseSubstructureMatcher(index)
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
   Reaction target_rxn;
   CfStorage &cf_storage = _index.getCfStorage();

   int cf_len;
   const char *cf_str = cf_storage.get(_current_id, cf_len);

   if (cf_len == -1)
      return false;

   BufferScanner buf_scn(cf_str, cf_len);
   CrfLoader crf_loader(buf_scn);

   crf_loader.loadReaction(target_rxn);

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


SimMatcher::SimMatcher (/*const */ BaseIndex &index ) : _index(index)
{
   _current_id = -1;
   _current_block = new byte[_index.getSimStorage().getBlockSize()];
   _fp_size = _index.getFingerprintParams().fingerprintSizeSim();
}

bool SimMatcher::next ()
{
   _current_id++;
   const RowFpStorage &fp_storage = _index.getSimStorage();
   int stor_fp_count = fp_storage.getBlockCount() * fp_storage.getFpPerBlockCount() + fp_storage.getIncrementSize();

   while (_current_id < stor_fp_count)
   {
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
         continue;
      }

      return true;
   }

   return false;
}
    
int SimMatcher::currentId ()
{
   return _current_id;
}


void SimMatcher::setQueryData (SimilarityQueryData *query_data)
{
   _query_data.reset(query_data);

   const MoleculeFingerprintParameters & fp_params = _index.getFingerprintParams();
   _query_data->getQueryObject().buildFingerprint(fp_params, 0, &_query_fp);
}

SimMatcher::~SimMatcher ()
{
   delete _current_block;
}

float SimMatcher::_calcTanimoto (const byte *fp)
{
   int common_bits = bitCommonOnes(fp, _query_fp.ptr(), _fp_size);
   int unique_bits = bitUniqueOnes(fp, _query_fp.ptr(), _fp_size);
      
   return (float)common_bits / (common_bits + unique_bits);
}