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

#include "oracle/ora_wrap.h"
#include "oracle/ora_logger.h"
#include "oracle/ringo_oracle.h"
#include "oracle/bingo_fingerprints.h"
#include "oracle/bingo_oracle_context.h"
#include "base_cpp/output.h"
#include "molecule/cmf_saver.h"
#include "reaction/crf_saver.h"
#include "molecule/molfile_loader.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/reaction_auto_loader.h"
#include "oracle/rowid_saver.h"
#include "molecule/elements.h"
#include "base_cpp/auto_ptr.h"

bool _ringoRegisterReaction (OracleEnv &env, const char *rowid,
                             const Array<char> &reaction_buf,
                             RingoOracleContext &context,
                             RingoIndex &index,
                             BingoFingerprints &fingerprints)
{
   QS_DEF(Array<char>, data);
   QS_DEF(Array<char>, compressed_rowid);
   ArrayOutput output(data);

   output.writeChar(0); // 0 -- present, 1 -- removed from index
   
   ArrayOutput rid_output(compressed_rowid);
   RowIDSaver rid_saver(context.context().rid_dict, rid_output);

   rid_saver.saveRowID(rowid);

   output.writeByte((byte)compressed_rowid.size());
   output.writeArray(compressed_rowid);

   TRY_READ_TARGET_RXN
   {
      BufferScanner scanner(reaction_buf);

      try
      {
         index.prepare(scanner, output);
      }
      catch (CmfSaver::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); return false;}
      catch (CrfSaver::Error &e) { env.dbgPrintf(bad_reaction_warning_rowid, rowid, e.message()); return false;}
   }
   CATCH_READ_TARGET_RXN_ROWID(rowid, return false);

   // some magic: round it up to avoid ora-22282
   if (data.size() % 2 == 1)
      output.writeChar(0);
   
   int blockno, offset;
   
   context.context().storage.add(env, data, blockno, offset);
   
   fingerprints.addFingerprint(env, index.getFingerprint());

   context.shadow_table.addReaction(env, index, rowid, blockno + 1, offset);
   return true;
}


void ringoRegisterTable (OracleEnv &env, RingoOracleContext &context)
{
   QS_DEF(Array<char>, source_table);
   QS_DEF(Array<char>, source_column);
   QS_DEF(Array<char>, target_datatype);
   QS_DEF(Array<char>, reaction_buf);
   OracleStatement statement(env);
   AutoPtr<OracleLOB> reaction_lob;
   OraRowidText rowid;
   char varchar2_text[4001];

   context.context().getSourceTable(env, source_table);
   context.context().getSourceColumn(env, source_column);
   context.context().getTargetDatatype(env, target_datatype);

   bool blob = (strcmp(target_datatype.ptr(), "BLOB") == 0);
   bool clob = (strcmp(target_datatype.ptr(), "CLOB") == 0);
   
   int total_count = 0;

   OracleStatement::executeSingleInt(total_count, env, "SELECT COUNT(*) FROM %s", source_table.ptr());

   context.context().longOpInit(env, total_count, "Building reaction index",
      source_table.ptr(), "reactions");

   if (blob || clob)
      statement.append("SELECT %s, RowidToChar(rowid) FROM %s ", source_column.ptr(), source_table.ptr());
   else
      statement.append("SELECT COALESCE(%s, ' '), RowidToChar(rowid) FROM %s ", source_column.ptr(), source_table.ptr());

   statement.prepare();

   if (blob)
   {
      reaction_lob.reset(new OracleLOB(env));
      statement.defineBlobByPos(1, reaction_lob.ref());
   }
   else if (clob)
   {
      reaction_lob.reset(new OracleLOB(env));
      statement.defineClobByPos(1, reaction_lob.ref());
   }
   else
      statement.defineStringByPos(1, varchar2_text, sizeof(varchar2_text));

   statement.defineStringByPos(2, rowid.ptr(), sizeof(rowid));

   BingoFingerprints &fingerprints = context.fingerprints;
   int nthreads = 0;

   fingerprints.validateForUpdate(env);
   context.context().configGetInt(env, "NTHREADS", nthreads);
   
   nthreads = 1;

   //if (nthreads == 1)
   {
      int n = 0;

      RingoIndex index(context.context());

      if (statement.executeAllowNoData()) do
      {
         env.dbgPrintf("inserting reaction #%d with rowid %s\n", n, rowid.ptr());

         if (blob || clob)
            reaction_lob->readAll(reaction_buf, false);
         else
            reaction_buf.readString(varchar2_text, false);

         try
         {
            if (_ringoRegisterReaction(env, rowid.ptr(), reaction_buf, context, index, fingerprints))
               n++;
         }
         catch (Exception &ex)
         {
            char buf[4096];
            snprintf(buf, NELEM(buf), "Failed on record with rowid=%s. Error message is '%s'",
               rowid.ptr(), ex.message());

            throw Exception(buf);
         }

         if ((n % 50) == 0)
            context.context().longOpUpdate(env, n);
         
         if ((n % 1000) == 0)
         {
            env.dbgPrintfTS("done %d reactions ; flushing\n", n);
            context.context().storage.flush(env);
            
         }
      } while (statement.fetch());
      
      fingerprints.flush(env);
   }
}

ORAEXT void oraRingoCreateIndex (OCIExtProcContext *ctx,
                                 int context_id,
                                 const char *params, short params_ind)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      env.dbgPrintfTS("Creating index\n");

      BingoOracleContext &bcontext = BingoOracleContext::get(env, context_id, false, 0);

      // parse parameters before creating RingoOracleContext because
      // it creates the BingoSreening according to fingerprintSizeNoTau()
      if (params_ind == OCI_IND_NOTNULL)
         bcontext.parseParameters(env, params);

      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      BingoStorage &storage = context.context().storage;
      BingoFingerprints &fingerprints = context.fingerprints;
      
      context.shadow_table.drop(env);
      context.shadow_table.create(env);

      fingerprints.drop(env);
      fingerprints.create(env);
      storage.drop(env);
      storage.create(env);
      storage.validateForInsert(env);
      
      ringoRegisterTable(env, context);

      storage.finish(env);
      context.context().saveCmfDict(env);
      context.context().saveRidDict(env);
      OracleStatement::executeSingle(env, "COMMIT");
      RingoContext::remove(context_id);
      BingoContext::remove(context_id);
   }
   ORABLOCK_END
}

ORAEXT void oraRingoDropIndex (OCIExtProcContext *ctx, int context_id)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      env.dbgPrintfTS("Dropping index #%d\n", context_id);
      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      context.shadow_table.drop(env);
      context.context().storage.drop(env);
      context.fingerprints.drop(env);

      RingoContext::remove(context_id);
      BingoContext::remove(context_id);

      // TEMP: remove CMF dictionary
      OracleStatement::executeSingle(env, "DELETE FROM CONFIG_BLOB WHERE n=%d", context_id);
   }
   ORABLOCK_END
}

ORAEXT void oraRingoTruncateIndex (OCIExtProcContext *ctx, int context_id)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      env.dbgPrintfTS("Truncating index #%d\n", context_id);
      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      context.shadow_table.truncate(env);
      context.context().storage.truncate(env);
      context.fingerprints.truncate(env);

      RingoContext::remove(context_id);
      BingoContext::remove(context_id);
   }
   ORABLOCK_END
}

ORAEXT void oraRingoIndexInsert (OCIExtProcContext *ctx, int context_id,
                                 const char    *rowid,      short rowid_ind,
                                 OCILobLocator *target_loc, short target_ind)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      if (rowid_ind != OCI_IND_NOTNULL)
         throw BingoError("null rowid given");

      RingoOracleContext &context = RingoOracleContext::get(env, context_id, true);

      env.dbgPrintf("inserting reaction with rowid %s\n", rowid);
      
      RingoIndex index(context.context());
      BingoFingerprints &fingerprints = context.fingerprints;
      BingoStorage &storage = context.context().storage;
      
      storage.lock(env);
      storage.validateForInsert(env);

      fingerprints.validateForUpdate(env);

      QS_DEF(Array<char>, target_buf);

      OracleLOB target_lob(env, target_loc);

      target_lob.readAll(target_buf, false);

      _ringoRegisterReaction(env, rowid, target_buf, context, index, fingerprints);

      storage.finish(env);
      //fingerprints.flush(env);

      if (context.context().cmf_dict.isModified())
         context.context().saveCmfDict(env);

      if (context.context().rid_dict.isModified())
         context.context().saveRidDict(env);
   }
   ORABLOCK_END
}

ORAEXT void oraRingoIndexDelete (OCIExtProcContext *ctx, int context_id,
                                 const char *rowid, short rowid_ind)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      if (rowid_ind != OCI_IND_NOTNULL)
         throw BingoError("null rowid given");

      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      int blockno, offset;

      if (context.shadow_table.getReactionLocation(env, rowid, blockno, offset))
      {
         env.dbgPrintf("deleting reaction that has rowid %s\n", rowid);
         
         BingoStorage &storage = context.context().storage;

         storage.lock(env);
         storage.markRemoved(env, blockno, offset);

         context.shadow_table.deleteReaction(env, rowid);
      }
      else
         env.dbgPrintf("reaction with rowid %s not found\n", rowid);
   }
   ORABLOCK_END
}

ORAEXT void oraRingoFlushInserts (OCIExtProcContext *ctx, int commit)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);
      int i;

      for (i = RingoContext::begin(); i != RingoContext::end(); i = RingoContext::next(i))
      {
         env.dbgPrintfTS("flushing inserts of context #%d\n", i);

         RingoOracleContext &context = RingoOracleContext::get(env, i, false);

         context.fingerprints.flush(env);
         if (commit)
            OracleStatement::executeSingle(env, "COMMIT");
         context.context().unlock(env);
      }
   }
   ORABLOCK_END
}
