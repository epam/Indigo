#include "bingo_gross_storage.h"
#include <sstream>

using namespace indigo;
using namespace bingo;

GrossStorage::GrossStorage(size_t gross_block_size) : _gross_formulas(gross_block_size)
{
}

MMFAddress GrossStorage::create(MMFPtr<GrossStorage>& gross_ptr, size_t gross_block_size)
{
    gross_ptr.allocate();
    new (gross_ptr.ptr()) GrossStorage(gross_block_size);

    return gross_ptr.getAddress();
}

void GrossStorage::load(MMFPtr<GrossStorage>& gross_ptr, MMFAddress offset)
{
    gross_ptr = MMFPtr<GrossStorage>(offset);
}

void GrossStorage::add(const Array<char>& gross_formula, int id)
{
    _gross_formulas.add((byte*)gross_formula.ptr(), gross_formula.size(), id);
    dword hash = _calculateGrossHash(gross_formula.ptr(), gross_formula.size());
    _hashes.add(hash, id);
}

void GrossStorage::find(Array<char>& query_formula, Array<int>& indices, int part_id, int part_count)
{
    Array<int> candidates;

    findCandidates(query_formula, candidates, part_id, part_count);

    int cur_candidate = 0;
    int match_id;
    while ((match_id = findNext(query_formula, candidates, cur_candidate)) != -1)
    {
        indices.push(match_id);
    }
}

void GrossStorage::findCandidates(Array<char>& query_formula, Array<int>& candidates, int part_id, int part_count)
{
    dword query_hash = _calculateGrossHash(query_formula.ptr(), query_formula.size());

    dword first_hash = 0;
    dword last_hash = (dword)(-1);

    if (part_id != -1 && part_count != -1)
    {
        first_hash = (part_id - 1) * last_hash / part_count;
        last_hash = part_id * last_hash / part_count;
    }

    if (query_hash < first_hash || query_hash > last_hash)
        return;

    Array<size_t> indices;
    _hashes.getAll(query_hash, indices);

    for (int i = 0; i < indices.size(); i++)
        candidates.push(indices[i]);
}

int GrossStorage::findNext(Array<char>& query_formula, Array<int>& candidates, int& cur_candidate)
{
    Array<int> query_array;
    MoleculeGrossFormula::fromString(query_formula.ptr(), query_array);

    while (++cur_candidate)
    {
        if (cur_candidate >= candidates.size())
            return -1;

        if (tryCandidate(query_array, candidates[cur_candidate]))
            return candidates[cur_candidate];
    }

    return -1;
}

bool GrossStorage::tryCandidate(Array<int>& query_array, int id)
{
    Array<int> cand_array;
    const char* cand_formula;
    int len;

    cand_formula = (const char*)_gross_formulas.get(id, len);
    Array<char> cand_fstr;
    cand_fstr.copy(cand_formula, len);

    MoleculeGrossFormula::fromString(cand_fstr.ptr(), cand_array);

    if (MoleculeGrossFormula::equal(query_array, cand_array))
        return true;

    return false;
}

void GrossStorage::calculateMolFormula(Molecule& mol, Array<char>& gross_formula)
{
    auto gross_array = MoleculeGrossFormula::collect(mol);

    MoleculeGrossFormula::toString(*gross_array, gross_formula, false);
}

void GrossStorage::calculateRxnFormula(Reaction& rxn, Array<char>& gross_formula)
{
    gross_formula.clear();

    for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
    {
        Array<char> mol_formula;

        auto gross_array = MoleculeGrossFormula::collect(rxn.getBaseMolecule(i));
        MoleculeGrossFormula::toString(*gross_array, mol_formula, false);

        gross_formula.concat(mol_formula);
        if (rxn.next(i) != rxn.end())
        {
            if (rxn.getSideType(i) != rxn.getSideType(rxn.next(i)))
                gross_formula.concat(">>", 2);
            else
                gross_formula.concat("+", 1);
        }
    }
}

dword GrossStorage::_calculateGrossHashForMolArray(Array<int>& gross_array)
{
    dword hash = 0;

    for (int i = 0; i < gross_array.size(); i++)
        hash += gross_array[i] * (i + 1);

    return hash;
}

dword GrossStorage::_calculateGrossHashForMol(const char* gross_str, int len)
{
    Array<int> gross_array;
    MoleculeGrossFormula::fromString(gross_str, gross_array);

    return _calculateGrossHashForMolArray(gross_array);
}

dword GrossStorage::_calculateGrossHash(const char* gross_str, int len)
{
    dword hash = 0;

    std::string rxn_str(gross_str, len);
    int delim_pos = rxn_str.find('>');
    if (delim_pos == -1)
        hash = _calculateGrossHashForMol(gross_str, len);
    else
    {
        rxn_str[delim_pos] = '+';
        rxn_str.erase(rxn_str.begin() + delim_pos + 1);
        std::stringstream rxn_stream;
        rxn_stream << rxn_str;
        std::string mol_str;
        while (rxn_stream.good())
        {
            std::getline(rxn_stream, mol_str, '+');
            hash += _calculateGrossHashForMol(mol_str.c_str(), len);
        }
    }

    return hash;
}
