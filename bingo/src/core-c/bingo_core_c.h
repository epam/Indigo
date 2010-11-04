#ifndef __bingo_core_c_h___
#define __bingo_core_c_h___

#include "base_c/defs.h"
#include "core/bingo_context.h"
#include "core/mango_context.h"
#include "core/ringo_context.h"

#include "core/mango_matchers.h"
#include "core/bingo_error.h"

#include "base_cpp/scanner.h"
#include "molecule/smiles_loader.h"
#include "molecule/elements.h"
#include "molecule/sdf_loader.h"
#include "molecule/rdf_loader.h"

#include "bingo_core_c.h"

class BingoCore
{
public:
   BingoCore ();
   void reset ();

   static BingoCore& getInstance ();

   Array<char> error;
   Array<char> warning;

   BingoContext * bingo_context;
   MangoContext * mango_context;
   RingoContext * ringo_context;

   Obj<FileScanner> file_scanner;
   Obj<SdfLoader> sdf_loader;
   Obj<RdfLoader> rdf_loader;
   Array<char> buffer;

   Obj<MangoIndex> mango_index;
   Obj<RingoIndex> ringo_index;
   Obj<Array<char> > mango_index_bindata;
   Obj<Array<char> > ringo_index_bindata;

   enum 
   {
      _UNDEF,
      _SUBSTRUCTRE, _TAUTOMER, _EXACT, _SIMILARITY, _GROSS
   } mango_search_type, ringo_search_type;
   
   bool mango_search_type_non;

   int sub_screening_max_bits, sim_screening_pass_mark;

   byte *test_ptr;
};

#define BINGO_BEGIN { BingoCore &self = BingoCore::getInstance(); try { self.error.clear();

#define BINGO_END(success, fail) } catch (Exception &ex) \
      { self.error.readString(ex.message(), true); return fail; } \
      return success; }

#define TRY_READ_TARGET_MOL \
   try {

#define CATCH_READ_TARGET_MOL(action) \
   } \
   catch (Scanner::Error  &e) { action;} \
   catch (MolfileLoader::Error &e) { action;} \
   catch (Element::Error &e) { action;} \
   catch (Graph::Error &e) { action;} \
   catch (MoleculeStereocenters::Error &e) { action;}  \
   catch (MoleculeCisTrans::Error &e) { action;} \
   catch (SmilesLoader::Error &e) { action;} \
   catch (Molecule::Error &e) { action;} \
   catch (MoleculeAutoLoader::Error &e) { action;}

   //catch (IcmLoader::Error &e) { action;} \

#define TRY_READ_TARGET_RXN \
   try {

#define CATCH_READ_TARGET_RXN(action) \
   } \
   catch (Scanner::Error  &e) { action;} \
   catch (MolfileLoader::Error &e) { action;} \
   catch (RxnfileLoader::Error &e) { action;} \
   catch (Element::Error &e) { action;} \
   catch (Graph::Error &e) { action;} \
   catch (MoleculeStereocenters::Error &e) { action;}  \
   catch (MoleculeCisTrans::Error &e) { action;} \
   catch (SmilesLoader::Error &e) { action;} \
   catch (RSmilesLoader::Error &e) { action;} \
   catch (Molecule::Error &e) { action;} \
   catch (Reaction::Error &e) { action;} \
   catch (ReactionAutoLoader::Error &e) { action;} 


#endif // __bingo_core_c_h___
