/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __mango_oracle__
#define __mango_oracle__

#include "core/mango_context.h"

#include "oracle/mango_shadow_table.h"
#include "oracle/bingo_fingerprints.h"

using namespace indigo;

namespace indigo
{
   class OracleEnv;
}

class BingoOracleContext;

class MangoOracleContext : public MangoContext
{
public:
   explicit MangoOracleContext (BingoContext &context);
   virtual ~MangoOracleContext ();

   BingoOracleContext & context ();

   MangoShadowTable shadow_table;
   BingoFingerprints fingerprints;

   static MangoOracleContext & get (OracleEnv &env, int id, bool lock);
};

extern const char *bad_molecule_warning;
extern const char *bad_molecule_warning_rowid;

#define TRY_READ_TARGET_MOL \
   try {

#define CATCH_READ_TARGET_MOL(action) \
   } \
   catch (Scanner::Error  &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (MolfileLoader::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (Element::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (Graph::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (MoleculeStereocenters::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;}  \
   catch (MoleculeCisTrans::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (SmilesLoader::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (IcmLoader::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (Molecule::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (DearomatizationException &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (MoleculeAutoLoader::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (MoleculePiSystemsMatcher::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (SkewSymmetricNetwork::Error &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} \
   catch (EmbeddingEnumerator::TimeoutException &e) { env.dbgPrintfTS(bad_molecule_warning, e.message()); action;} 
   
   

#define CATCH_READ_TARGET_MOL_ROWID(rowid, action) \
   } \
   catch (Scanner::Error  &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (MolfileLoader::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (Element::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (Graph::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (MoleculeStereocenters::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;}  \
   catch (MoleculeCisTrans::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (SmilesLoader::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (IcmLoader::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (Molecule::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (DearomatizationException &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (MoleculeAutoLoader::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (MoleculePiSystemsMatcher::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (SkewSymmetricNetwork::Error &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} \
   catch (EmbeddingEnumerator::TimeoutException &e) { env.dbgPrintfTS(bad_molecule_warning_rowid, rowid, e.message()); action;} 


#endif
