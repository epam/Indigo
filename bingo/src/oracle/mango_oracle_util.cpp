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

#include "oracle/bingo_oracle.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "base_cpp/profiling.h"

#include "molecule/molecule_auto_loader.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/smiles_saver.h"
#include "molecule/molfile_loader.h"
#include "molecule/smiles_loader.h"
#include "molecule/icm_loader.h"
#include "molecule/icm_saver.h"
#include "molecule/molfile_saver.h"
#include "layout/molecule_layout.h"

#include "oracle/ora_wrap.h"
#include "oracle/ora_logger.h"

#include "oracle/bingo_oracle_context.h"
#include "oracle/mango_oracle.h"
#include "molecule/elements.h"

static OCIString * _mangoSMILES (OracleEnv &env, const Array<char> &target_buf,
                                 BingoOracleContext &context, bool canonical)
{
   QS_DEF(Molecule, target);
   QS_DEF(GraphHighlighting, highlighting);

   profTimerStart(tload, "smiles.load_molecule");
   MoleculeAutoLoader loader(target_buf);

   loader.treat_x_as_pseudoatom = context.treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           context.ignore_closing_bond_direction_mismatch;
   loader.highlighting = &highlighting;
   loader.skip_3d_chirality = true;
   loader.loadMolecule(target);
   profTimerStop(tload);

   if (canonical)
      MoleculeAromatizer::aromatizeBonds(target);

   QS_DEF(Array<char>, smiles);

   ArrayOutput out(smiles);

   if (canonical)
   {
      profTimerStart(tload, "smiles.cano_saver");

      CanonicalSmilesSaver saver(out);

      saver.saveMolecule(target);
   }
   else
   {
      profTimerStart(tload, "smiles.saver");

      SmilesSaver saver(out);

      saver.highlighting = &highlighting;
      saver.saveMolecule(target);
   }
   
   OCIString *result = 0;
   env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text *)smiles.ptr(),
      smiles.size(), &result));

   return result;
}

ORAEXT OCIString * oraMangoSMILES (OCIExtProcContext *ctx,
                                   OCILobLocator *target_locator, short target_indicator,
                                   short *return_indicator)
{
   OCIString *result = NULL;

   logger.initIfClosed(log_filename);

   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      *return_indicator = OCI_IND_NULL;

      if (target_indicator != OCI_IND_NOTNULL)
         throw BingoError("null molecule given");

      BingoOracleContext &context = BingoOracleContext::get(env, 0, false, 0);
      
      OracleLOB target_lob(env, target_locator);

      QS_DEF(Array<char>, buf);

      target_lob.readAll(buf, false);

      result = _mangoSMILES(env, buf, context, false);
      
      if (result == 0) // empty SMILES?
      {
         // This is needed for Oracle 9. Returning NULL drops the extproc.
         OCIStringAssignText(env.envhp(), env.errhp(), (text *)"nil", 3, &result);
      }
      else
         *return_indicator = OCI_IND_NOTNULL;
   }
   ORABLOCK_END

   return result;
}

ORAEXT OCIString *oraMangoCanonicalSMILES (OCIExtProcContext *ctx,
                     OCILobLocator *target_locator, short target_indicator,
                     short *return_indicator)
{
   profTimersReset();
   profTimerStart(tall, "smiles.all");

   OCIString *result = NULL;

   logger.initIfClosed(log_filename);

   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      *return_indicator = OCI_IND_NULL;

      if (target_indicator != OCI_IND_NOTNULL)
         throw BingoError("null molecule given");

      BingoOracleContext &context = BingoOracleContext::get(env, 0, false, 0);
      
      OracleLOB target_lob(env, target_locator);

      QS_DEF(Array<char>, buf);

      profTimerStart(treadlob, "smiles.read_lob");
      target_lob.readAll(buf, false);
      profTimerStop(treadlob);

      result = _mangoSMILES(env, buf, context, true);

      if (result == 0) // empty SMILES?
      {
         // This is needed for Oracle 9. Returning NULL drops the extproc.
         OCIStringAssignText(env.envhp(), env.errhp(), (text *)"nil", 3, &result);
      }
      else
         *return_indicator = OCI_IND_NOTNULL;
   }
   ORABLOCK_END

   return result;
}

ORAEXT OCIString * oraMangoCheckMolecule (OCIExtProcContext *ctx,
                     OCILobLocator *target_locator, short target_indicator,
                     short *return_indicator)
{
   OCIString *result = NULL;

   logger.initIfClosed(log_filename);

   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      *return_indicator = OCI_IND_NULL;

      if (target_indicator != OCI_IND_NOTNULL)
      {
         static const char *msg = "null molecule given";

         env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(),
            (text *)msg, strlen(msg), &result));
         *return_indicator = OCI_IND_NOTNULL;
      }
      else
      {
         OracleLOB target_lob(env, target_locator);

         QS_DEF(Array<char>, buf);
         QS_DEF(Molecule, mol);

         BingoOracleContext &context = BingoOracleContext::get(env, 0, false, 0);

         target_lob.readAll(buf, false);

         TRY_READ_TARGET_MOL
         {
            MoleculeAutoLoader loader(buf);

            loader.treat_x_as_pseudoatom = context.treat_x_as_pseudoatom;
            loader.ignore_closing_bond_direction_mismatch =
                    context.ignore_closing_bond_direction_mismatch;
            loader.skip_3d_chirality = true;
            loader.loadMolecule(mol);
            Molecule::checkForConsistency(mol);
         }
         CATCH_READ_TARGET_MOL
         (
            OCIStringAssignText(env.envhp(), env.errhp(), (text *)e.message(), strlen(e.message()), &result);
            *return_indicator = OCI_IND_NOTNULL;
         )
         catch (Exception &e) 
         {
            char buf[4096];
            snprintf(buf, NELEM(buf), "INTERNAL ERROR: %s", e.message());
            OCIStringAssignText(env.envhp(), env.errhp(), (text *)buf, strlen(buf), &result);
            *return_indicator = OCI_IND_NOTNULL;
         }

         if (*return_indicator == OCI_IND_NULL)
            // This is needed for Oracle 9. Returning NULL drops the extproc.
            OCIStringAssignText(env.envhp(), env.errhp(), (text *)"nil", 3, &result);
      }
   }
   ORABLOCK_END

   return result;
}

void _ICM (BingoOracleContext &context, OracleLOB &target_lob, int save_xyz, Array<char> &icm)
{
   QS_DEF(Array<char>, target);
   QS_DEF(Molecule, mol);

   target_lob.readAll(target, false);

   MoleculeAutoLoader loader(target);

   loader.treat_x_as_pseudoatom = context.treat_x_as_pseudoatom;
   loader.ignore_closing_bond_direction_mismatch =
           context.ignore_closing_bond_direction_mismatch;
   loader.skip_3d_chirality = true;
   loader.loadMolecule(mol);

   if ((save_xyz != 0) && !mol.have_xyz)
      throw BingoError("molecule has no XYZ");

   ArrayOutput output(icm);
   IcmSaver saver(output);

   saver.save_xyz = (save_xyz != 0);
   saver.saveMolecule(mol);
}

ORAEXT OCILobLocator *oraMangoICM (OCIExtProcContext *ctx,
                                   OCILobLocator *target_locator, short target_indicator,
                                   int save_xyz, short *return_indicator)
{
   OCILobLocator *result = 0;

   ORABLOCK_BEGIN
   {
      *return_indicator = OCI_IND_NULL;

      OracleEnv env(ctx, logger);

      if (target_indicator == OCI_IND_NULL)
         throw BingoError("null molecule given");

      OracleLOB target_lob(env, target_locator);
      BingoOracleContext &context = BingoOracleContext::get(env, 0, false, 0);

      QS_DEF(Array<char>, icm);

      _ICM(context, target_lob, save_xyz, icm);
      
      OracleLOB lob(env);
      
      lob.createTemporaryBLOB();
      lob.write(0, icm);
      *return_indicator = OCI_IND_NOTNULL;
      lob.doNotDelete();
      result = lob.get();
   }
   ORABLOCK_END

   return result;
}

ORAEXT void oraMangoICM2 (OCIExtProcContext *ctx,
                          OCILobLocator *target_locator, short target_indicator,
                          OCILobLocator *result_locator, short result_indicator,
                          int save_xyz)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      if (target_indicator == OCI_IND_NULL)
         throw BingoError("null molecule given");
      if (result_indicator == OCI_IND_NULL)
         throw BingoError("null LOB given");

      OracleLOB target_lob(env, target_locator);
      BingoOracleContext &context = BingoOracleContext::get(env, 0, false, 0);
      QS_DEF(Array<char>, icm);

      _ICM(context, target_lob, save_xyz, icm);

      OracleLOB result_lob(env, result_locator);

      result_lob.write(0, icm);
      result_lob.trim(icm.size());
   }
   ORABLOCK_END
}

ORAEXT OCILobLocator *oraMangoMolfile (OCIExtProcContext *ctx,
                                       OCILobLocator *target_locator, short target_indicator,
                                       short *return_indicator)
{
   OCILobLocator *result = 0;

   ORABLOCK_BEGIN
   {
      *return_indicator = OCI_IND_NULL;

      OracleEnv env(ctx, logger);

      if (target_indicator == OCI_IND_NULL)
         throw BingoError("null molecule given");

      BingoOracleContext &context = BingoOracleContext::get(env, 0, false, 0);
      OracleLOB target_lob(env, target_locator);

      QS_DEF(Array<char>, target);
      QS_DEF(Array<char>, icm);
      QS_DEF(Molecule, mol);

      target_lob.readAll(target, false);

      MoleculeAutoLoader loader(target);

      loader.treat_x_as_pseudoatom = context.treat_x_as_pseudoatom;
      loader.ignore_closing_bond_direction_mismatch =
              context.ignore_closing_bond_direction_mismatch;
      loader.skip_3d_chirality = true;
      loader.loadMolecule(mol);

      if (!mol.have_xyz)
      {
         MoleculeLayout layout(mol);

         layout.make();
         mol.stereocenters.markBonds();
      }

      ArrayOutput output(icm);
      MolfileSaver saver(output);

      saver.saveMolecule(mol);

      OracleLOB lob(env);

      lob.createTemporaryCLOB();
      lob.write(0, icm);
      *return_indicator = OCI_IND_NOTNULL;
      lob.doNotDelete();
      result = lob.get();
   }
   ORABLOCK_END

   return result;
}
