#ifndef __bingo_gross_storage__
#define __bingo_gross_storage__

#include "molecule/molecule.h"
#include "base_cpp/scanner.h"
#include "molecule/gross_formula.h"
#include "reaction/reaction.h"
#include "bingo_ptr.h"
#include "bingo_mapping.h"
#include "bingo_cf_storage.h"

using namespace indigo;

namespace bingo
{
   class GrossStorage
   {
   public:
      GrossStorage (size_t gross_block_size);

      static size_t create(BingoPtr<GrossStorage> &gross_ptr, size_t gross_block_size);

      static void load (BingoPtr<GrossStorage> &gross_ptr, size_t offset);

      void add (Array<char> &gross_formula, int id);

      void find (Array<char> &query_formula, Array<int> &indices, int part_id = -1, int part_count = -1);

      void findCandidates (Array<char> &query_formula, Array<int> &candidates, int part_id = -1, int part_count = -1);

      int findNext (Array<char> &query_formula, Array<int> &candidates, int &cur_candidate);

      bool tryCandidate (Array<int> &query_array, int id);

      static void calculateMolFormula (Molecule &mol, Array<char> &gross_formula);

      static void calculateRxnFormula (Reaction &rxn, Array<char> &gross_formula);

   private:
      BingoMapping _hashes;
      ByteBufferStorage _gross_formulas;

      static dword _calculateGrossHashForMolArray (Array<int> &gross_array);

      static dword _calculateGrossHashForMol (const char *gross_str, int len);

      static dword _calculateGrossHash (const char *gross_str, int len);
   };
}

#endif //__bingo_gross_storage__