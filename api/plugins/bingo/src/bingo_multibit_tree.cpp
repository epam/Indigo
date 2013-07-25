#include "bingo_multibit_tree.h"

using namespace bingo;

int MultibitTree::_compareBitWeights(_DistrWeight &bw1, _DistrWeight &bw2, void *context)
{
   if (bw1.weight > bw2.weight)
      return 1;
   else if (bw1.weight < bw2.weight)
      return -1;
   else
      return 0;
}

BingoPtr MultibitTree::_buildNode( Array<int> &fit_fp_indices, const Array<bool> &is_parrent_mb, int level)
{
   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   BingoPtr node_ptr = bingo_allocator->allocate<_MultibitNode>();
   _MultibitNode *node = new(bingo_allocator->get(node_ptr)) _MultibitNode();
   byte *fingerprints = bingo_allocator->get(_fingerprints_ptr);


   int fp_bitsize = _fp_size * 8;

   if (fit_fp_indices.size() <= 10 || level == _max_level)
   {
      node->fp_indices_array = bingo_allocator->allocate<int>(fit_fp_indices.size());
      int *fp_indices = (int *)bingo_allocator->get(node->fp_indices_array);
      node->fp_indices_count = fit_fp_indices.size();

      for (int i = 0; i < fit_fp_indices.size(); i++)
         fp_indices[i] = fit_fp_indices[i];
      
      node->left = -1;
      node->right = -1;

      return node_ptr;
   }

   QS_DEF(Array<_DistrWeight>, bit_weights);
   bit_weights.clear_resize(fp_bitsize);
   bit_weights.zerofill();
   double bit_con = 1.0 / fit_fp_indices.size();

   for (int j = 0; j < fp_bitsize; j++)
      bit_weights[j].idx = j;
      
   for (int i = 0; i < fit_fp_indices.size(); i++)
   {
      for (int j = 0; j < fp_bitsize; j++)
      {
         bit_weights[j].weight += bitGetBit(fingerprints + fit_fp_indices[i] * _fp_size, j) * bit_con;
      }
   }

   bit_weights.qsort(_compareBitWeights, 0);

   QS_DEF(Array<int>, mb_indices);
   mb_indices.clear();
   
   double distr_coef = 1;
   for (int k = 0; k < bit_weights.size(); k++)
   {
      if (is_parrent_mb[bit_weights[k].idx])
         continue;
      distr_coef *= (1 - bit_weights[k].weight);
      mb_indices.push(bit_weights[k].idx);
      if (distr_coef < 0.5)
         break;
   }

   QS_DEF(Array<int>, left_fit);
   left_fit.clear();
   QS_DEF(Array<int>, right_fit);
   right_fit.clear();

   QS_DEF(Array<bool>, is_mb);
   is_mb.copy(is_parrent_mb);

   for (int i = 0; i < mb_indices.size(); i++)
   {
      is_mb[mb_indices[i]] = 1;
   }

   for (int i = 0; i < fit_fp_indices.size(); i++)
   {
      bool is_fit = true;
      for (int j = 0; j < mb_indices.size(); j++)
      {
         if (bitGetBit(fingerprints + fit_fp_indices[i] * _fp_size, mb_indices[j]))
         {
            is_fit = false;
            break;
         }
      }

      if (is_fit)
         right_fit.push(fit_fp_indices[i]);
      else
         left_fit.push(fit_fp_indices[i]);
   }


   if (left_fit.size() == 0 || right_fit.size() == 0)
   {
      node->fp_indices_array = bingo_allocator->allocate<int>(fit_fp_indices.size());
      int *fp_indices = (int *)bingo_allocator->get(node->fp_indices_array);
      node->fp_indices_count = fit_fp_indices.size();

      for (int i = 0; i < fit_fp_indices.size(); i++)
         fp_indices[i] = fit_fp_indices[i];
      
      node->left = -1;
      node->right = -1;

      return node_ptr;
   }

   node->match_bits_array = bingo_allocator->allocate<_MatchBit>(mb_indices.size());
   _MatchBit *match_bits = (_MatchBit *)bingo_allocator->get(node->match_bits_array);       
   node->match_bits_count = mb_indices.size();

   for (int i = 0; i < mb_indices.size(); i++)
   {
      if (mb_indices[i] < 0 || mb_indices[i] > 511)
         i = i;

      match_bits[i] = _MatchBit(mb_indices[i], 0);
   }

   node->left = _buildNode(left_fit, is_mb, level + 1);
   node->right = _buildNode(right_fit, is_mb, level + 1);

   return node_ptr;
}

void MultibitTree::_build( void )
{
   QS_DEF(Array<int>, indices);
   indices.clear_resize(_fp_count);

   for (int i = 0; i < indices.size(); i++)
      indices[i] = i;

   QS_DEF(Array<bool>, is_mb);
   is_mb.clear_resize(_fp_size * 8);
   is_mb.zerofill();

   _tree_ptr = _buildNode(indices, is_mb, 0);
}

void MultibitTree::_findLinear( _MultibitNode *node, const byte *query, SimCoef &sim_coef, double min_coef, Array<int> &sim_indices, int fp_bit_number )
{
   profTimerStart(tmsl, "multibit_tree_search_linear");
   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   byte *fingerprints = bingo_allocator->get(_fingerprints_ptr);
   int *indices = (int *)bingo_allocator->get(_indices_ptr);
   
   int *fp_indices = (int *)bingo_allocator->get(node->fp_indices_array);
   
   //std::cout << "find linear! " << node->fp_indices.size() << std::endl;
      
   for (int i = 0; i < node->fp_indices_count; i++)
   {
      const byte *fp = fingerprints + fp_indices[i] * _fp_size;
      int f_bit_number = bitGetOnesCount(fp, _fp_size);

      if (sim_coef.calcCoef(query, fp, _query_bit_number, f_bit_number) < min_coef)
         continue;

      //std::cout << "one!!" << std::endl;
      sim_indices.push(indices[fp_indices[i]]);
   }
}

void MultibitTree::_findSimilarInNode( BingoPtr node_ptr, const byte *query, SimCoef &sim_coef, double min_coef, 
                          Array<int> &sim_indices, int m01, int m10 )
{
   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   
   if (node_ptr == -1)
      return;

   _MultibitNode *node = (_MultibitNode *)bingo_allocator->get(node_ptr);
   
   if (node->fp_indices_count != 0)
   {
      if (_min_fp_bit_number == _max_fp_bit_number)//if fingerpint bits_count is fixed
         _findLinear(node, query, sim_coef, min_coef, sim_indices, _min_fp_bit_number);
      else
         _findLinear(node, query, sim_coef, min_coef, sim_indices);
      
      return;
   }

   _MatchBit *match_bits = (_MatchBit *)bingo_allocator->get(node->match_bits_array);
   
   
   QS_DEF(Array<int>, left_indices);
   left_indices.clear();
   QS_DEF(Array<int>, right_indices);
   right_indices.clear();
   
   int right_m01 = m01, right_m10 = m10;
   for (int i = 0; i < node->match_bits_count; i++)
      if (match_bits[i].val == 0)
      {
         if (bitGetBit(query, match_bits[i].idx))
            right_m10++;
      }
      else if (!bitGetBit(query, match_bits[i].idx))
            right_m01++;

   int a = _query_bit_number - right_m10;
   int min_b = _min_fp_bit_number - right_m01;
   int max_b = _max_fp_bit_number - right_m01;

   int min = (a < max_b ? a : max_b);
   int max = (a > min_b ? a : min_b);

   double right_upper_bound = (double)min / (right_m01 + right_m10 + max);

   if (node->left != 0)
      _findSimilarInNode(node->left, query, sim_coef, min_coef, left_indices, m01, m10);  
   if (node->right != 0 && right_upper_bound + EPSILON > min_coef)
      _findSimilarInNode(node->right, query, sim_coef, min_coef, right_indices, right_m01, right_m10);

   for (int i = 0; i < left_indices.size(); i++)
      sim_indices.push(left_indices[i]);
   for (int i = 0; i < right_indices.size(); i++)
      sim_indices.push(right_indices[i]);
}

MultibitTree::MultibitTree( int fp_size ) : _fp_size(fp_size)
{
   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   
   _fingerprints_ptr = -1;
   _tree_ptr = bingo_allocator->allocate<_MultibitNode>();
   new(bingo_allocator->get(_tree_ptr)) _MultibitNode();
   _query_bit_number = -1;
   _max_level = 6;
}

void MultibitTree::build( BingoPtr fingerprints, BingoPtr indices, int fp_count, int min_fp_bit_number, int max_fp_bit_number )
{
   _fingerprints_ptr = fingerprints;
   _indices_ptr = indices;

   _min_fp_bit_number = min_fp_bit_number;
   _max_fp_bit_number = max_fp_bit_number;

   _fp_count = fp_count;
   
   _build();
}

int MultibitTree::findSimilar( const byte *query, SimCoef &sim_coef, double min_coef, Array<int> &sim_fp_indices)
{
   profTimerStart(tms, "multibit_tree_search");
   _query_bit_number = bitGetOnesCount(query, _fp_size);
   sim_fp_indices.clear();

   _findSimilarInNode(_tree_ptr, query, sim_coef, min_coef, sim_fp_indices, 0, 0);

   return sim_fp_indices.size();
}