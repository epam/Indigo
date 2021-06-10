#ifndef __bingo_gross_storage__
#define __bingo_gross_storage__

#include "base_cpp/scanner.h"
#include "bingo_cf_storage.h"
#include "bingo_mapping.h"
#include "bingo_ptr.h"
#include "molecule/molecule.h"
#include "molecule/molecule_gross_formula.h"
#include "reaction/reaction.h"

using namespace indigo;

namespace bingo
{
    class GrossStorage
    {
    public:
        GrossStorage(size_t gross_block_size);

        static BingoAddr create(BingoPtr<GrossStorage>& gross_ptr, size_t gross_block_size);

        static void load(BingoPtr<GrossStorage>& gross_ptr, BingoAddr offset);

        void add(ArrayChar& gross_formula, int id);

        void find(ArrayChar& query_formula, ArrayInt& indices, int part_id = -1, int part_count = -1);

        void findCandidates(ArrayChar& query_formula, ArrayInt& candidates, int part_id = -1, int part_count = -1);

        int findNext(ArrayChar& query_formula, ArrayInt& candidates, int& cur_candidate);

        bool tryCandidate(ArrayInt& query_array, int id);

        static void calculateMolFormula(Molecule& mol, ArrayChar& gross_formula);

        static void calculateRxnFormula(Reaction& rxn, ArrayChar& gross_formula);

    private:
        BingoMapping _hashes;
        ByteBufferStorage _gross_formulas;

        static dword _calculateGrossHashForMolArray(ArrayInt& gross_array);

        static dword _calculateGrossHashForMol(const char* gross_str, int len);

        static dword _calculateGrossHash(const char* gross_str, int len);
    };
} // namespace bingo

#endif //__bingo_gross_storage__