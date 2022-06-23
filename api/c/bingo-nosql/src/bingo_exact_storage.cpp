#include "bingo_exact_storage.h"

#include "base_cpp/profiling.h"
#include "graph/subgraph_hash.h"
#include "molecule/elements.h"
#include "molecule/molecule_hash.h"
#include "reaction/reaction_hash.h"

using namespace bingo;
using namespace indigo;

ExactStorage::ExactStorage()
{
}

MMFAddress ExactStorage::create(MMFPtr<ExactStorage>& exact_ptr)
{
    exact_ptr.allocate();
    new (exact_ptr.ptr()) ExactStorage();

    return exact_ptr.getAddress();
}

void ExactStorage::load(MMFPtr<ExactStorage>& exact_ptr, MMFAddress offset)
{
    exact_ptr = MMFPtr<ExactStorage>(offset);
}

void ExactStorage::add(dword hash, int id)
{
    _molecule_hashes.add(hash, id);
}

void ExactStorage::findCandidates(dword query_hash, Array<int>& candidates, int part_id, int part_count)
{
    profTimerStart(tsingle, "exact_filter");

    dword first_hash = 0;
    dword last_hash = (dword)(-1);

    if (part_id != -1 && part_count != -1)
    {
        const dword part_multiplier = last_hash / part_count;
        first_hash = (part_id - 1) * part_multiplier;
        last_hash = part_id * part_multiplier;
    }

    if (query_hash < first_hash || query_hash > last_hash)
        return;

    Array<size_t> indices;
    _molecule_hashes.getAll(query_hash, indices);

    for (int i = 0; i < indices.size(); i++)
        candidates.push(indices[i]);
}

dword ExactStorage::calculateMolHash(Molecule& mol)
{
    return MoleculeHash::calculate(mol);
}

dword ExactStorage::calculateRxnHash(Reaction& rxn)
{
    return ReactionHash::calculate(rxn);
}
