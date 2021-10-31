#include "bingo_fingerprint_table.h"

using namespace bingo;

FingerprintTable::FingerprintTable(int fp_size, const indigo::Array<int>& borders, int mt_size) : _table(100), _fp_size(fp_size), _mt_size(mt_size)
{
    _table.resize(borders.size() - 1);

    profTimerStart(tfp, "FingerprintTable constructing");

    _max_cell_count = 100;

    for (int i = 0; i < _table.size(); i++)
    {
        {
            profTimerStart(tfp, "FingerprintTable element pushing");
            _table[i].setParams(_fp_size, mt_size, borders[i], borders[i + 1] - 1);
        }
    }
}

MMFAddress FingerprintTable::create(MMFPtr<FingerprintTable>& ptr, int fp_size, int mt_size)
{
    indigo::Array<int> borders;

    borders.push(0);
    borders.push(fp_size * 8 + 1);

    ptr.allocate();
    new (ptr.ptr()) FingerprintTable(fp_size, borders, mt_size);

    return ptr.getAddress();
}

void FingerprintTable::load(MMFPtr<FingerprintTable>& ptr, MMFAddress offset)
{
    ptr = MMFPtr<FingerprintTable>(offset);
}

void FingerprintTable::add(const byte* fingerprint, int id)
{
    int fp_bit_count = bitGetOnesCount(fingerprint, _fp_size);

    for (int i = 0; i < _table.size(); i++)
    {
        if ((fp_bit_count >= _table[i].getMinBorder()) && (fp_bit_count <= _table[i].getMaxBorder()))
        {
            if (_table[i].add(fingerprint, id))
            {
                if (_table[i].getMinBorder() == _table[i].getMaxBorder() || _table[i].getContCount() > 1 || _table.size() >= _max_cell_count)
                    _table[i].buildContainer();
                else
                {
                    _table.resize(_table.size() + 1);
                    for (int j = _table.size() - 2; j >= i + 1; j--)
                        _table[j + 1] = _table[j];

                    _table[i + 1].setParams(_fp_size, _mt_size, -1, -1);
                    _table[i].splitSet(_table[i + 1]);
                }
            }

            break;
        }
    }
}

void FingerprintTable::findSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_fp_indices)
{
    sim_fp_indices.clear();

    int query_bit_number = bitGetOnesCount(query, _fp_size);

    QS_DEF(indigo::Array<SimResult>, cell_sim_indices);
    for (int i = 0; i < _table.size(); i++)
    {
        if (sim_coef.calcUpperBound(query_bit_number, _table[i].getMinBorder(), _table[i].getMaxBorder()) < min_coef)
            continue;

        cell_sim_indices.clear();
        _table[i].findSimilar(query, sim_coef, min_coef, cell_sim_indices);

        sim_fp_indices.concat(cell_sim_indices);
    }
}

void FingerprintTable::optimize()
{
    for (int i = 0; i < _table.size(); i++)
        _table[i].optimize();
}

int FingerprintTable::getCellCount() const
{
    return _table.size();
}

int FingerprintTable::getCellSize(int cell_idx) const
{
    if (cell_idx >= _table.size())
        throw indigo::Exception("FingerprintTable: Incorrect cell index");

    return _table[cell_idx].getContCount();
}

void FingerprintTable::getCellsInterval(const byte* query, SimCoef& sim_coef, double min_coef, int& min_cell, int& max_cell)
{
    min_cell = -1;
    max_cell = -1;
    int query_bit_count = bitGetOnesCount(query, _fp_size);

    for (int i = 0; i < _table.size(); i++)
    {
        if ((min_cell == -1) && (sim_coef.calcUpperBound(query_bit_count, _table[i].getMinBorder(), _table[i].getMaxBorder()) > min_coef))
            min_cell = i;

        if ((min_cell != -1) && (sim_coef.calcUpperBound(query_bit_count, _table[i].getMinBorder(), _table[i].getMaxBorder()) > min_coef))
            max_cell = i;
    }
}

int FingerprintTable::firstFitCell(int query_bit_count, int min_cell, int max_cell) const
{
    int first_cell = -1;

    for (int i = min_cell; i <= max_cell; i++)
    {
        if (query_bit_count >= _table[i].getMinBorder() && query_bit_count <= _table[i].getMaxBorder())
            first_cell = i;
    }

    return first_cell;
}

int FingerprintTable::nextFitCell(int query_bit_count, int first_fit_cell, int min_cell, int max_cell, int idx) const
{
    int next_idx;

    if (first_fit_cell == idx)
        next_idx = first_fit_cell + 1;
    else if (first_fit_cell < idx)
        next_idx = first_fit_cell - (idx - first_fit_cell);
    else
        next_idx = first_fit_cell + (first_fit_cell - idx) + 1;

    if (next_idx < min_cell || next_idx > max_cell)
    {
        if (idx < min_cell || idx > max_cell)
            return -1;
        else
            return nextFitCell(query_bit_count, first_fit_cell, min_cell, max_cell, next_idx);
    }

    return next_idx;
}

int FingerprintTable::getSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_fp_indices, int cell_idx, int cont_idx)
{
    if (cell_idx >= _table.size())
        throw indigo::Exception("FingerprintTable: Incorrect cell index");

    int query_bit_number = bitGetOnesCount(query, _fp_size);

    if (sim_coef.calcUpperBound(query_bit_number, _table[cell_idx].getMinBorder(), _table[cell_idx].getMaxBorder()) < min_coef)
        return 0;

    _table[cell_idx].getSimilar(query, sim_coef, min_coef, sim_fp_indices, cont_idx);

    return sim_fp_indices.size();
}

FingerprintTable::~FingerprintTable()
{
}
