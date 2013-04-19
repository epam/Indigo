/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef __bingo_context__
#define __bingo_context__

#include "base_cpp/nullable.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_tautomer.h"
#include "lzw/lzw_dictionary.h"
#include "bingo_version.h"

using namespace indigo;

// extern const char *bingo_version_string;

class BingoContext
{
public:

   enum {
      /*
       * Default operation timeout = 60000 ms
       */
      DEFAULT_TIMEOUT = 60000
   };

   explicit BingoContext (int id_);
   virtual ~BingoContext ();

   bool tautomer_rules_ready;
   bool fp_parameters_ready;
   bool atomic_mass_map_ready;

   int     id;
   LzwDict cmf_dict;
   LzwDict rid_dict;
   int     fp_chunk_qwords;

   int     nthreads;
   int     timeout;

   Nullable<bool> treat_x_as_pseudoatom;
   Nullable<bool> ignore_closing_bond_direction_mismatch;

   MoleculeFingerprintParameters fp_parameters;

   PtrArray<TautomerRule> tautomer_rules;

   RedBlackMap<int, float> relative_atomic_mass_map;

   bool getTreatXAsPseudoatom ();
   void setTreatXAsPseudoatom (bool state);

   bool getIgnoreClosingBondDirectionMismatch ();
   void setIgnoreClosingBondDirectionMismatch (bool state);

   static void remove (int id);

   void reset ();

   DECL_ERROR;

   static BingoContext * get (int id);
   static BingoContext * existing (int id);

protected:
   TL_DECL(PtrArray<BingoContext>, _instances);

   static BingoContext * _get (int id);
   static OsLock _instances_lock;

   Array<char> _relative_atomic_mass;
private:
   BingoContext (const BingoContext &); // no implicit copy
};

void bingoGetTauCondition (const char *list_ptr, int &aromaticity, Array<int> &label_list);

void bingoGetName (Scanner &scanner, Array<char> &result);

#endif
