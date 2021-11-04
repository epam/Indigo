#ifndef __bingo_exact_storage__
#define __bingo_exact_storage__

#include "molecule/molecule.h"
#include "reaction/reaction.h"

#include "mmf/mmf_ptr.h"
#include "src/mmf/mmf_mapping.h"

namespace bingo
{
    class ExactStorage
    {
    public:
        ExactStorage();

        static MMFAddress create(MMFPtr<ExactStorage>& exact_ptr);

        static void load(MMFPtr<ExactStorage>& exact_ptr, MMFAddress offset);

        size_t getOffset();

        void add(dword hash, int id);

        void findCandidates(dword query_hash, indigo::Array<int>& candidates, int part_id = -1, int part_count = -1);

        static dword calculateMolHash(indigo::Molecule& mol);

        static dword calculateRxnHash(indigo::Reaction& rxn);

    private:
        MMFMapping _molecule_hashes;
    };
} // namespace bingo

#endif //__bingo_exact_storage__
