#include "bingo_multibit_tree.h"

using namespace bingo;
using namespace indigo;

int MultibitTree::_compareBitWeights(_DistrWeight& bw1, _DistrWeight& bw2, void* context)
{
    if (bw1.weight > bw2.weight)
        return 1;
    else if (bw1.weight < bw2.weight)
        return -1;
    else
        return 0;
}

MMFPtr<MultibitTree::_MultibitNode> MultibitTree::_buildNode(Array<int>& fit_fp_indices, const Array<bool>& is_parrent_mb, int level)
{
    MMFPtr<_MultibitNode> node_ptr;
    node_ptr.allocate();
    _MultibitNode* node = new (node_ptr.ptr()) _MultibitNode();
    byte* fingerprints = _fingerprints_ptr.ptr();

    int fp_bitsize = _fp_size * 8;

    if (fit_fp_indices.size() <= 10 || level == _max_level)
    {
        node->fp_indices_array.allocate(fit_fp_indices.size());
        int* fp_indices = node->fp_indices_array.ptr();
        node->fp_indices_count = fit_fp_indices.size();

        for (int i = 0; i < fit_fp_indices.size(); i++)
            fp_indices[i] = fit_fp_indices[i];

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
        node->fp_indices_array.allocate(fit_fp_indices.size());
        int* fp_indices = (int*)node->fp_indices_array.ptr();
        node->fp_indices_count = fit_fp_indices.size();

        for (int i = 0; i < fit_fp_indices.size(); i++)
            fp_indices[i] = fit_fp_indices[i];

        return node_ptr;
    }

    node->match_bits_array.allocate(mb_indices.size());
    _MatchBit* match_bits = node->match_bits_array.ptr();
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

void MultibitTree::_build()
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

void MultibitTree::_findLinear(_MultibitNode* node, const byte* query, int query_bit_number, SimCoef& sim_coef, double min_coef, Array<SimResult>& sim_indices,
                               int fp_bit_number)
{
    profTimerStart(tmsl, "multibit_tree_search_linear");
    byte* fingerprints = _fingerprints_ptr.ptr();
    int* indices = _indices_ptr.ptr();

    int* fp_indices = node->fp_indices_array.ptr();

    for (int i = 0; i < node->fp_indices_count; i++)
    {
        const byte* fp = fingerprints + fp_indices[i] * _fp_size;
        int f_bit_number = bitGetOnesCount(fp, _fp_size);

        double coef = sim_coef.calcCoef(query, fp, query_bit_number, f_bit_number);
        if (coef < min_coef)
            continue;

        sim_indices.push(SimResult(indices[fp_indices[i]], (float)coef));
    }
}

void MultibitTree::_findSimilarInNode(MMFPtr<_MultibitNode> node_ptr, const byte* query, int query_bit_number, SimCoef& sim_coef, double min_coef,
                                      Array<SimResult>& sim_indices, int m01, int m10)
{
    if (node_ptr.isNull())
        return;

    _MultibitNode* node = node_ptr.ptr();

    if (node->fp_indices_count != 0)
    {
        if (_min_fp_bit_number == _max_fp_bit_number) // if fingerpint bits_count is fixed
            _findLinear(node, query, query_bit_number, sim_coef, min_coef, sim_indices, _min_fp_bit_number);
        else
            _findLinear(node, query, query_bit_number, sim_coef, min_coef, sim_indices);

        return;
    }

    _MatchBit* match_bits = node->match_bits_array.ptr();

    QS_DEF(Array<SimResult>, left_indices);
    left_indices.clear();
    QS_DEF(Array<SimResult>, right_indices);
    right_indices.clear();

    int right_m01 = m01, right_m10 = m10;
    for (int i = 0; i < node->match_bits_count; i++)
        if (match_bits[i].val == 0)
        {
            if (bitGetBit(query, match_bits[i].idx))
                right_m01++;
        }
        else if (!bitGetBit(query, match_bits[i].idx))
            right_m10++;

    double right_upper_bound = sim_coef.calcUpperBound(query_bit_number, _min_fp_bit_number, _max_fp_bit_number, right_m10, right_m01);

    if (!node->left.isNull())
        _findSimilarInNode(node->left, query, query_bit_number, sim_coef, min_coef, left_indices, m01, m10);
    if ((!node->left.isNull()) && right_upper_bound + EPSILON > min_coef)
        _findSimilarInNode(node->right, query, query_bit_number, sim_coef, min_coef, right_indices, right_m01, right_m10);

    for (int i = 0; i < left_indices.size(); i++)
        sim_indices.push(left_indices[i]);
    for (int i = 0; i < right_indices.size(); i++)
        sim_indices.push(right_indices[i]);
}

MultibitTree::MultibitTree(int fp_size) : _fp_size(fp_size)
{
    _tree_ptr.allocate();
    new (_tree_ptr.ptr()) _MultibitNode();
    _query_bit_number = -1;
    _max_level = 6;
}

void MultibitTree::build(MMFPtr<byte> fingerprints, MMFPtr<int> indices, int fp_count, int min_fp_bit_number, int max_fp_bit_number)
{
    _fingerprints_ptr = fingerprints;
    _indices_ptr = indices;

    _min_fp_bit_number = min_fp_bit_number;
    _max_fp_bit_number = max_fp_bit_number;

    _fp_count = fp_count;

    _build();
}

int MultibitTree::findSimilar(const byte* query, SimCoef& sim_coef, double min_coef, Array<SimResult>& sim_fp_indices)
{
    profTimerStart(tms, "multibit_tree_search");
    int query_bit_number = bitGetOnesCount(query, _fp_size);
    sim_fp_indices.clear();

    _findSimilarInNode(_tree_ptr, query, query_bit_number, sim_coef, min_coef, sim_fp_indices, 0, 0);

    return sim_fp_indices.size();
}
