/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __bingo_core_c_internal_h___
#define __bingo_core_c_internal_h___

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
#include "molecule/cmf_saver.h"
#include "molecule/molfile_loader.h"
#include "molecule/molecule_auto_loader.h"

#include "bingo_core_c.h"
#include "mango_core_c_parallel.h"

namespace indigo {
namespace bingo_core {

class BingoCore
{
public:
   BingoCore ();
   void reset ();

   static BingoCore& getInstance ();

   Array<char> error;
   Array<char> warning;

   BINGO_ERROR_HANDLER error_handler;
   void  *error_handler_context;

   BingoContext * bingo_context;
   MangoContext * mango_context;
   RingoContext * ringo_context;

   Obj<FileScanner> file_scanner;
   Obj<SdfLoader> sdf_loader;
   Obj<RdfLoader> rdf_loader;
   Array<char> buffer;

   MangoIndex *mango_index;
   RingoIndex *ringo_index;

   Obj< Array<char> > index_record_data;
   int index_record_data_id;

   Obj<MangoIndexingDispatcher> mango_indexing_dispatcher;

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
      { self.error.readString(ex.message(), true); \
         if (self.error_handler != 0)                       \
            self.error_handler(ex.message(),                \
            self.error_handler_context); \
         return fail; } \
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

}
}

#endif // __bingo_core_c_h___
