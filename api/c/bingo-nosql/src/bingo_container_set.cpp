#include "bingo_container_set.h"
#include "bingo_multibit_tree.h"

using namespace bingo;

ContainerSet::ContainerSet()
{
    _inc_count = 0;
    _inc_total_ones_count = 0;
}

void ContainerSet::setParams(int fp_size, int container_size, int min_ones_count, int max_ones_count)
{
    profTimerStart(t, "cs_set_params");
    _fp_size = fp_size;
    _min_ones_count = min_ones_count;
    _max_ones_count = max_ones_count;
    _container_size = container_size;

    _increment.allocate(_container_size * _fp_size);
    _indices.allocate(_container_size);
}

int ContainerSet::getContCount() const
{
    return _set.size() + 1;
}

int ContainerSet::getMinBorder() const
{
    return _min_ones_count;
}

int ContainerSet::getMaxBorder() const
{
    return _max_ones_count;
}

bool ContainerSet::add(const byte* fingerprint, int id, int fp_ones_count)
{
    if (_inc_count == _container_size)
        throw indigo::Exception("ContainerSet: Increment is full");

    byte* inc = _increment.ptr();
    int* indices = _indices.ptr();

    memcpy(inc + _inc_count * _fp_size, fingerprint, _fp_size);
    indices[_inc_count] = id;
    _inc_total_ones_count += (fp_ones_count == -1 ? bitGetOnesCount(fingerprint, _fp_size) : fp_ones_count);
    _inc_count++;

    if (_inc_count == _container_size)
        return true;

    return false;
}

void ContainerSet::buildContainer()
{
    profIncCounter("trees_count", 1);

    MultibitTree& cont = _set.push<int>(_fp_size);

    cont.build(_increment, _indices, _container_size, _min_ones_count, _max_ones_count);
    _increment.allocate(_container_size * _fp_size);
    _indices.allocate(_container_size);

    _inc_count = 0;
}

void ContainerSet::splitSet(ContainerSet& new_set)
{
    if (_set.size() > 0)
        throw indigo::Exception("ContainerSet: Set with built containers can't be splited");

    int new_border = (_inc_total_ones_count / _inc_count) + 1;

    int inc_count_cur = 0;

    new_set._inc_count = 0;

    _inc_total_ones_count = 0;
    new_set._inc_total_ones_count = 0;
    for (int i = 0; i < _inc_count; i++)
    {
        int ones_count = bitGetOnesCount(_increment.ptr() + i * _fp_size, _fp_size);

        if (ones_count < new_border)
        {
            memcpy(_increment.ptr() + inc_count_cur * _fp_size, _increment.ptr() + i * _fp_size, _fp_size);
            _indices[inc_count_cur] = (int)_indices[i];
            inc_count_cur++;
            _inc_total_ones_count += ones_count;
        }
        else
        {
            memcpy(new_set._increment.ptr() + new_set._inc_count * _fp_size, _increment.ptr() + i * _fp_size, _fp_size);
            new_set._indices[new_set._inc_count] = (int)_indices[i];
            new_set._inc_count++;
            new_set._inc_total_ones_count += ones_count;
        }
    }

    new_set._min_ones_count = new_border;
    new_set._max_ones_count = _max_ones_count;

    _max_ones_count = new_border - 1;

    _inc_count = inc_count_cur;
}

void ContainerSet::findSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_indices)
{
    sim_indices.clear();

    static int idx = 0;

    // int query_bit_number = bitGetOnesCount(query, _fp_size);

    QS_DEF(indigo::Array<SimResult>, cell_sim_indices);
    for (int i = 0; i < _set.size(); i++)
    {
        MultibitTree& container = _set[i];
        cell_sim_indices.clear();
        container.findSimilar(query, sim_coef, min_coef, cell_sim_indices);

        sim_indices.concat(cell_sim_indices);
    }

    cell_sim_indices.clear();
    _findSimilarInc(query, sim_coef, min_coef, cell_sim_indices);
    sim_indices.concat(cell_sim_indices);

    idx++;
}

void ContainerSet::optimize()
{
    if (_inc_count < _container_size / 10)
        return;

    profIncCounter("trees_count", 1);

    MultibitTree& cont = _set.push<int>(_fp_size);
    cont.build(_increment, _indices, _inc_count, _min_ones_count, _max_ones_count);
    _increment.allocate(_container_size * _fp_size);
    _indices.allocate(_container_size);
    _inc_count = 0;
}

int ContainerSet::getSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_fp_indices, int cont_idx)
{
    profTimerStart(cs_s, "getSimilar");

    if (cont_idx >= getContCount())
        throw indigo::Exception("ContainerSet: Incorrect container index");

    if (cont_idx == _set.size())
    {
        {
            profTimerStart(cs_s, "inc_findSimilar");
            _findSimilarInc(query, sim_coef, min_coef, sim_fp_indices);
            profIncCounter("inc_findSimilar_count", sim_fp_indices.size());
        }

        return sim_fp_indices.size();
    }

    MultibitTree& container = _set[cont_idx];

    {
        profTimerStart(cs_s, "set_findSimilar");
        container.findSimilar(query, sim_coef, min_coef, sim_fp_indices);
        profIncCounter("set_findSimilar_count", sim_fp_indices.size());
    }

    return sim_fp_indices.size();
}

int ContainerSet::_findSimilarInc(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_indices)
{
    byte* inc = _increment.ptr();
    int* indices = _indices.ptr();

    sim_indices.clear();

    int query_bit_number = bitGetOnesCount(query, _fp_size);

    for (int i = 0; i < _inc_count; i++)
    {
        byte* fp = inc + i * _fp_size;
        int fp_bit_number = bitGetOnesCount(fp, _fp_size);

        double coef = sim_coef.calcCoef(fp, query, query_bit_number, fp_bit_number);
        if (coef < min_coef)
            continue;

        sim_indices.push(SimResult(indices[i], (float)coef));
    }

    return sim_indices.size();
}
