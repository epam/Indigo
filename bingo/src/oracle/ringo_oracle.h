/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __ringo_oracle__
#define __ringo_oracle__

#include "core/ringo_context.h"

#include "oracle/ringo_shadow_table.h"
#include "bingo_fingerprints.h"

using namespace indigo;

namespace indigo
{
   class OracleEnv;
}

class BingoOracleContext;

class RingoOracleContext : public RingoContext
{
public:
   explicit RingoOracleContext (BingoContext &context);
   virtual ~RingoOracleContext ();

   BingoOracleContext & context ();

   RingoShadowTable shadow_table;
   BingoFingerprints fingerprints;

   static RingoOracleContext & get (OracleEnv &env, int id, bool lock);
};

extern const char *bad_reaction_warning;
extern const char *bad_reaction_warning_rowid;;

#define TRY_READ_TARGET_RXN \
   try {

#define CATCH_READ_TARGET_RXN(action) \
   } \
   catch (Scanner::Error  &e) { env.dbgPrintf(bad_reaction_warning, e.message()); action;} \
   catch (MolfileLoader::Error &e) { env.dbgPrintf(bad_reaction_warning, e.message()); action;} \
   catch (Element::Error &e) { env.dbgPrintf(bad_reaction_warning, e.message()); action;} \
   catch (Graph::Error &e) { env.dbgPrintf(bad_reaction_warning, e.message()); action;} \
   catch (MoleculeStereocenters::Error &e) { env.dbgPrintf(bad_reaction_warning, e.message()); action;}  \
   catch (MoleculeCisTrans::Error &e) { env.dbgPrintf(bad_reaction_warning, e.message()); action;} \
   catch (RxnfileLoader::Error &e) { env.dbgPrintf(bad_reaction_warning, e.message()); action;} \
   catch (Molecule::Error &e) { env.dbgPrintf(bad_reaction_warning, e.message()); action;} \

#define CATCH_READ_TARGET_RXN_ROWID(rowid, action) \
   } \
   catch (Scanner::Error  &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); action;} \
   catch (MolfileLoader::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); action;} \
   catch (Element::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); action;} \
   catch (Graph::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); action;} \
   catch (MoleculeStereocenters::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); action;}  \
   catch (MoleculeCisTrans::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); action;} \
   catch (RxnfileLoader::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); action;} \
   catch (Molecule::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); action;} \


#endif
