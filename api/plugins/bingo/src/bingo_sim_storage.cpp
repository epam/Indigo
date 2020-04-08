#include "bingo_sim_storge.h"

#include <iostream>

using namespace bingo;

SimStorage::SimStorage(int fp_size, int mt_size, int inc_size)
    : _fingerprint_table(BingoAddr::bingo_null), _inc_size(inc_size), _mt_size(mt_size), _fp_size(fp_size)
{
    _inc_buffer.allocate(_inc_size * _fp_size);
    _inc_id_buffer.allocate(_inc_size * _fp_size);
}

BingoAddr SimStorage::create(BingoPtr<SimStorage>& ptr, int fp_size, int mt_size, int inc_size)
{
    ptr.allocate();
    new (ptr.ptr()) SimStorage(fp_size, mt_size, inc_size);

    return (BingoAddr)ptr;
}

void SimStorage::load(BingoPtr<SimStorage>& ptr, BingoAddr offset)
{
    ptr = BingoPtr<SimStorage>(offset);
}

void SimStorage::add(const byte* fingerprint, int id)
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
    {
        memcpy(_inc_buffer.ptr() + (_inc_fp_count * _fp_size), fingerprint, _fp_size);
        _inc_id_buffer[_inc_fp_count] = id;

        _inc_fp_count++;

        if (_inc_fp_count == _inc_size)
        {
            FingerprintTable::create(_fingerprint_table, _fp_size, _mt_size);
            for (int i = 0; i < _inc_fp_count; i++)
                _fingerprint_table->add(_inc_buffer.ptr() + (i * _fp_size), _inc_id_buffer[i]);

            _inc_fp_count = 0;
        }
    }
    else
    {
        _fingerprint_table->add(fingerprint, id);
    }
}

void SimStorage::optimize()
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
        return;

    _fingerprint_table->optimize();
}

int SimStorage::getCellCount() const
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
        throw Exception("SimStorage: fingerptint table wasn't built");

    return _fingerprint_table->getCellCount();
}

int SimStorage::getCellSize(int cell_idx) const
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
        throw Exception("SimStorage: fingerptint table wasn't built");

    return _fingerprint_table->getCellSize(cell_idx);
}

void SimStorage::getCellsInterval(const byte* query, SimCoef& sim_coef, double min_coef, int& min_cell, int& max_cell)
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
        throw Exception("SimStorage: fingerptint table wasn't built");

    _fingerprint_table->getCellsInterval(query, sim_coef, min_coef, min_cell, max_cell);
}

int SimStorage::firstFitCell(int query_bit_count, int min_cell, int max_cell) const
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
        throw Exception("SimStorage: fingerptint table wasn't built");

    return _fingerprint_table->firstFitCell(query_bit_count, min_cell, max_cell);
}

int SimStorage::nextFitCell(int query_bit_count, int first_fit_cell, int min_cell, int max_cell, int idx) const
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
        throw Exception("SimStorage: fingerptint table wasn't built");

    return _fingerprint_table->nextFitCell(query_bit_count, first_fit_cell, min_cell, max_cell, idx);
}

int SimStorage::getSimilar(const byte* query, SimCoef& sim_coef, double min_coef, Array<SimResult>& sim_fp_indices, int cell_idx, int cont_idx)
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
        throw Exception("SimStorage: fingerptint table wasn't built");

    return _fingerprint_table->getSimilar(query, sim_coef, min_coef, sim_fp_indices, cell_idx, cont_idx);
}

bool SimStorage::isSmallBase()
{
    if ((BingoAddr)_fingerprint_table == BingoAddr::bingo_null)
        return true;
    return false;
}

int SimStorage::getIncSimilar(const byte* query, SimCoef& sim_coef, double min_coef, Array<SimResult>& sim_fp_indices)
{
    for (int i = 0; i < _inc_fp_count; i++)
    {
        double coef = sim_coef.calcCoef(_inc_buffer.ptr() + (i * _fp_size), query, -1, -1);
        if (coef < min_coef)
            continue;
        size_t id = _inc_id_buffer[i];

        sim_fp_indices.push(SimResult(id, coef));
    }

    return sim_fp_indices.size();
}

SimStorage::~SimStorage()
{
}