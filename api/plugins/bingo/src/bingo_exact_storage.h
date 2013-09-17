#ifndef __bingo_exact_storage__
#define __bingo_exact_storage__

#include "molecule/molecule.h"
#include "reaction/reaction.h"
#include "bingo_ptr.h"

using namespace indigo;

namespace bingo
{
   class ExactStorage
   {
   public:
      ExactStorage ();

      size_t create();

      void load( size_t offset );

      size_t getOffset ();

      void add( dword hash, int id );

      void findCandidates( dword query_hash, Array<int> &candidates );

      static dword calculateMolHash (Molecule &mol);

      static dword calculateRxnHash (Reaction &rxn);

   private:
      BingoPtr< BingoArray<dword> > _hashes_ptr;

      static int _vertexCode (Molecule &mol, int vertex_idx);
   };
}

#endif //__bingo_exact_storage__