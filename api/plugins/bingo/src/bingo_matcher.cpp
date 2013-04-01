#include "bingo_matcher.h"

#include "molecule\molecule_substructure_matcher.h"

using namespace indigo;

using namespace bingo;

MoleculeSimilarityQueryData::MoleculeSimilarityQueryData( /* const */ Molecule &qmol, float min_coef, float max_coef ) : 
   _obj(qmol), _min(min_coef), _max(max_coef)
{
}

/*const*/ QueryObject & MoleculeSimilarityQueryData::getQueryObject() /*const*/
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

ReactionSimilarityQueryData::ReactionSimilarityQueryData( /* const */ Reaction &qrxn, float min_coef, float max_coef ) : 
   _obj(qrxn), _min(min_coef), _max(max_coef)
{
}

/*const*/ QueryObject & ReactionSimilarityQueryData::getQueryObject() /*const*/
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


   
MoleculeSubstructureQueryData::MoleculeSubstructureQueryData( /* const */ QueryMolecule &qmol ) : _obj(qmol)
{
}

/*const*/ QueryObject & MoleculeSubstructureQueryData::getQueryObject() /*const*/
{
   return _obj;
}
   

   
ReactionSubstructureQueryData::ReactionSubstructureQueryData( /* const */ QueryReaction &qrxn ) : _obj(qrxn)
{
}

/*const*/ QueryObject & ReactionSubstructureQueryData::getQueryObject() /*const*/
{
   return _obj;
}

   
   
BaseSubstructureMatcher::BaseSubstructureMatcher(/*const */ BaseIndex &index) : 
   _index(index), _fp_storage(_index.getSubStorage())
{
   _fp_size = _index.getFingerprintParams().fingerprintSize();
   
   _current_id = -1;

   _current_cand_id = -1;
   _current_pack = -1;
}

bool BaseSubstructureMatcher::next ()
{
   int fp_size_in_bits = _fp_size * 8;

   _current_cand_id++;
   while (!((_current_pack == _fp_storage.getPackCount()) && (_current_cand_id == _candidates.size())))
   {
      if (_current_cand_id == _candidates.size())
      {
         _current_pack++;
         if (_current_pack < _fp_storage.getPackCount())
            _findPackCandidates(_current_pack);
         else
            _findIncCandidates();

         _current_cand_id = 0;
      }

      _current_id = _candidates[_current_cand_id];

      if (_tryCurrent())
         return true;

      _current_cand_id++;
   }

   return false;
}

int BaseSubstructureMatcher::currentId ()
{
   return _current_id;
}

void BaseSubstructureMatcher::setQueryData(SubstructureQueryData *query_data)
{
   _query_data.reset(query_data);

   const MoleculeFingerprintParameters & fp_params = _index.getFingerprintParams();
   _query_data->getQueryObject().buildFingerprint(fp_params, &_query_fp, 0);
}

BaseSubstructureMatcher::~BaseSubstructureMatcher()
{
}

void BaseSubstructureMatcher::_findPackCandidates( int pack_idx )
{
   _candidates.clear();
   Array<bool> is_candidate;

   const TranspFpStorage &fp_storage = _index.getSubStorage();

   const byte *query_fp = _query_fp.ptr();

   is_candidate.clear_resize( fp_storage.getBlockSize() * 8);
   is_candidate.fill(true);

   byte *block = new byte[fp_storage.getBlockSize()];

   int fp_size_in_bits = _fp_size * 8;

   for (int j = 0; j < fp_size_in_bits; j++)
   {
      fp_storage.getBlock(pack_idx * fp_size_in_bits + j, block);

      byte query_bit = query_fp[j / 8] & (0x80 >> (j % 8));
      
      for (int k = 0; k < fp_storage.getBlockSize(); k++)
      {
         for (int bit_cnt  = 0; bit_cnt < 8; bit_cnt++)
         {
            byte block_bit = block[k] & (0x80 >> bit_cnt);
            
            if (query_bit & !block_bit)
               is_candidate[k * 8 + bit_cnt] = false;
         }
      }
   }

   for (int i = 0; i < is_candidate.size(); i++)
      if (is_candidate[i])
         _candidates.push(i + pack_idx * fp_storage.getBlockSize());

   delete block;
}

void BaseSubstructureMatcher::_findIncCandidates()
{
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


MoleculeSubMatcher::MoleculeSubMatcher(/*const */ BaseIndex &index) : BaseSubstructureMatcher(index)
{
   _mapping.clear();
}

const Array<int> & MoleculeSubMatcher::currentMapping()
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
   const char *cf_str = cf_storage.get(_current_id, cf_len);

   BufferScanner buf_scn(cf_str, cf_len);
   CmfLoader cmf_loader(buf_scn);

   cmf_loader.loadMolecule(target_mol);

   MoleculeSubstructureMatcher msm(target_mol);

   msm.setQuery(query_mol);

   if (msm.find())
   {
      _mapping.copy(msm.getTargetMapping(), target_mol.vertexCount());
      return true;
   }

   return false;
}
   
   
   
ReactionSubMatcher::ReactionSubMatcher(/*const */ BaseIndex &index) : BaseSubstructureMatcher(index)
{
   _mapping.clear();
}

const ObjArray<Array<int>> & ReactionSubMatcher::currentMapping()
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


SimMatcher::SimMatcher(/*const */ BaseIndex &index ) : _index(index)
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


void SimMatcher::setQueryData(SimilarityQueryData *query_data)
{
   _query_data = query_data;

   const MoleculeFingerprintParameters & fp_params = _index.getFingerprintParams();
   _query_data->getQueryObject().buildFingerprint(fp_params, 0, &_query_fp);
}

SimMatcher::~SimMatcher()
{
   delete _current_block;
}

float SimMatcher::_calcTanimoto( const byte *fp )
{
   static int _bit_count[] = {0,1,1,2,1,2,2,3,1,
                        2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,
                        2,3,3,4,3,4,4,5,1,2,2,3,2,3,3,4,2,3,
                        3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,
                        4,5,5,6,1,2,2,3,2,3,3,4,2,3,3,4,3,4,
                        4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,
                        4,5,4,5,5,6,4,5,5,6,5,6,6,7,1,2,2,3,
                        2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,
                        4,5,3,4,4,5,4,5,5,6,2,3,3,4,3,4,4,5,
                        3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,
                        5,6,5,6,6,7,2,3,3,4,3,4,4,5,3,4,4,5,
                        4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,
                        6,7,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                        4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8};

   int common_bits = 0;
   int or_bits = 0;
      
   for (int k = 0; k < _fp_size; k++)
   {
      common_bits += _bit_count[fp[k] & _query_fp[k]];
      or_bits +=_bit_count[fp[k] | _query_fp[k]];
   }

   return (float)common_bits / or_bits;
}