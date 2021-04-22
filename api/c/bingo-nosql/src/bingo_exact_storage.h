#ifndef __bingo_exact_storage__
#define __bingo_exact_storage__

#include "bingo_mapping.h"
#include "bingo_ptr.h"
#include "molecule/molecule.h"
#include "reaction/reaction.h"

using namespace indigo;

namespace bingo
{
    class ExactStorage
    {
    public:
        ExactStorage();

        static BingoAddr create(BingoPtr<ExactStorage>& exact_ptr);

        static void load(BingoPtr<ExactStorage>& exact_ptr, BingoAddr offset);

        size_t getOffset();

        void add(dword hash, int id);

        void findCandidates(dword query_hash, Array<int>& candidates, int part_id = -1, int part_count = -1);

        static dword calculateMolHash(Molecule& mol);

        static dword calculateRxnHash(Reaction& rxn);

    private:
        BingoMapping _molecule_hashes;
    };
} // namespace bingo

#endif //__bingo_exact_storage__