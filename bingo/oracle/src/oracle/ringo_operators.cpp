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

#include <oci.h>

#include "oracle/bingo_oracle.h"
#include "oracle/ora_wrap.h"
#include "oracle/ora_logger.h"
#include "base_cpp/tlscont.h"
#include "core/ringo_matchers.h"
#include "oracle/ringo_oracle.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molfile_loader.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/reaction_auto_loader.h"
#include "molecule/cmf_saver.h"
#include "reaction/crf_saver.h"
#include "molecule/elements.h"
#include "molecule/molecule_pi_systems_matcher.h"
#include "molecule/molecule_auto_loader.h"
#include "graph/embedding_enumerator.h"

static OCINumber * _ringoSub (OracleEnv &env, RingoOracleContext &context,
                              const Array<char> &query_buf,
                              const Array<char> &target_buf, const char *params)
{
   context.substructure.loadQuery(query_buf);

   TRY_READ_TARGET_RXN
   {
      context.substructure.loadTarget(target_buf);
   }
   CATCH_READ_TARGET_RXN(return 0)

   int result = context.substructure.matchLoadedTarget() ? 1 : 0;

   return OracleExtproc::createInt(env, result);
}

ORAEXT OCINumber * oraRingoSub (OCIExtProcContext *ctx, int context_id,
    OCILobLocator *target_loc, short target_ind,
    OCILobLocator *query_loc,  short query_ind,
    const char    *params,     short params_ind,
    short *return_ind)
{
   OCINumber *result = NULL;

   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      *return_ind = OCI_IND_NULL;

      if (query_ind  != OCI_IND_NOTNULL)
         throw BingoError("Null query given");
      if (target_ind != OCI_IND_NOTNULL)
         throw BingoError("Null target given");
      if (params_ind != OCI_IND_NOTNULL)
         params = 0;

      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      QS_DEF(Array<char>, query_buf);
      QS_DEF(Array<char>, target_buf);

      OracleLOB target_lob(env, target_loc);
      OracleLOB query_lob(env, query_loc);

      target_lob.readAll(target_buf, false);
      query_lob.readAll(query_buf, false);

      if (!context.substructure.parse(params))
         throw BingoError("can not parse parameters: %s", params);
      
      result = _ringoSub(env, context, query_buf, target_buf, params);

      if (result == 0)
         // This is needed for Oracle 9. Returning NULL drops the extproc.
         result = OracleExtproc::createInt(env, 0);
      else
         *return_ind = OCI_IND_NOTNULL;
   }
   ORABLOCK_END

   return result;
}

ORAEXT OCILobLocator * oraRingoSubHi (OCIExtProcContext *ctx, int context_id,
    OCILobLocator *target_loc, short target_ind,
    OCILobLocator *query_loc,  short query_ind,
    const char    *params,     short params_ind,
    short *return_ind)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      *return_ind = OCI_IND_NULL;

      if (query_ind  != OCI_IND_NOTNULL)
         throw BingoError("Null query given");
      if (target_ind != OCI_IND_NOTNULL)
         throw BingoError("Null target given");
      if (params_ind != OCI_IND_NOTNULL)
         params = 0;

      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      QS_DEF(Array<char>, query_buf);
      QS_DEF(Array<char>, target_buf);

      OracleLOB target_lob(env, target_loc);
      OracleLOB query_lob(env, query_loc);

      target_lob.readAll(target_buf, false);
      query_lob.readAll(query_buf, false);

      if (!context.substructure.parse(params))
         throw BingoError("can not parse parameters: %s", params);

      context.substructure.preserve_bonds_on_highlighting = true;
      context.substructure.loadQuery(query_buf);
      context.substructure.loadTarget(target_buf);

      if (!context.substructure.matchLoadedTarget())
         throw BingoError("SubHi: match not found");

      context.substructure.getHighlightedTarget(target_buf);

      OracleLOB lob(env);

      lob.createTemporaryCLOB();
      lob.write(0, target_buf); 
      lob.doNotDelete();
      *return_ind= OCI_IND_NOTNULL;
      return lob.get();
   }
   ORABLOCK_END

   return 0;
}

ORAEXT OCINumber * oraRingoRSmarts (OCIExtProcContext *ctx, int context_id,
    OCILobLocator *target_loc, short target_ind,
    const char *query,  short query_ind,
    short *return_ind)
{
   OCINumber *result = NULL;

   ORABLOCK_BEGIN
   {
      *return_ind = OCI_IND_NULL;

      OracleEnv env(ctx, logger);

      if (query_ind  != OCI_IND_NOTNULL)
         throw BingoError("Null query given");
      if (target_ind != OCI_IND_NOTNULL)
         throw BingoError("Null target given");

      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      QS_DEF(Array<char>, query_buf);
      QS_DEF(Array<char>, target_buf);

      OracleLOB target_lob(env, target_loc);

      target_lob.readAll(target_buf, false);
      query_buf.readString(query, false);

      context.substructure.loadSMARTS(query_buf);

      TRY_READ_TARGET_RXN
      {
         context.substructure.loadTarget(target_buf);
      }
      CATCH_READ_TARGET_RXN(return OracleExtproc::createInt(env, 0))

      int match = context.substructure.matchLoadedTarget() ? 1 : 0;

      result = OracleExtproc::createInt(env, match);
      *return_ind = OCI_IND_NOTNULL;
   }
   ORABLOCK_END

   return result;
}

ORAEXT OCILobLocator * oraRingoRSmartsHi (OCIExtProcContext *ctx, int context_id,
    OCILobLocator *target_loc, short target_ind,
    const char *query,  short query_ind,
    short *return_ind)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      *return_ind = OCI_IND_NULL;

      if (query_ind  != OCI_IND_NOTNULL)
         throw BingoError("Null query given");
      if (target_ind != OCI_IND_NOTNULL)
         throw BingoError("Null target given");

      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      QS_DEF(Array<char>, query_buf);
      QS_DEF(Array<char>, target_buf);

      OracleLOB target_lob(env, target_loc);

      target_lob.readAll(target_buf, false);
      query_buf.readString(query, false);

      context.substructure.preserve_bonds_on_highlighting = true;
      context.substructure.loadSMARTS(query_buf);
      context.substructure.loadTarget(target_buf);

      if (!context.substructure.matchLoadedTarget())
         throw BingoError("SubHi: match not found");

      context.substructure.getHighlightedTarget(target_buf);

      OracleLOB lob(env);

      lob.createTemporaryCLOB();
      lob.write(0, target_buf); 
      lob.doNotDelete();
      *return_ind= OCI_IND_NOTNULL;
      return lob.get();
   }
   ORABLOCK_END

   return 0;
}

ORAEXT OCILobLocator * oraRingoAAM (OCIExtProcContext *ctx,
                   OCILobLocator *target_loc, short target_ind,
                   const char    *params,     short params_ind,
                   short *return_ind)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      *return_ind = OCI_IND_NULL;

      if (target_ind != OCI_IND_NOTNULL)
         throw BingoError("Null target given");
      if (params_ind != OCI_IND_NOTNULL)
         params = 0;

      RingoOracleContext &context = RingoOracleContext::get(env, 0, false);

      QS_DEF(Array<char>, target_buf);

      OracleLOB target_lob(env, target_loc);

      target_lob.readAll(target_buf, false);

      context.ringoAAM.loadReaction(target_buf);
      context.ringoAAM.parse(params);
      context.ringoAAM.getResult(target_buf);

      OracleLOB lob(env);

      lob.createTemporaryCLOB();
      lob.write(0, target_buf); 
      lob.doNotDelete();
      *return_ind = OCI_IND_NOTNULL;
      return lob.get();
   }
   ORABLOCK_END

   return 0;
}

static OCINumber * _ringoExact (OracleEnv &env, RingoOracleContext &context,
                                const Array<char> &query_buf,
                                const Array<char> &target_buf, const char *params)
{
   context.exact.setParameters(params);
   context.exact.loadQuery(query_buf);

   TRY_READ_TARGET_RXN
   {
      context.exact.loadTarget(target_buf);
   }
   CATCH_READ_TARGET_RXN(return 0)

   int result = context.exact.matchLoadedTarget() ? 1 : 0;

   return OracleExtproc::createInt(env, result);
}

ORAEXT OCINumber * oraRingoExact (OCIExtProcContext *ctx, int context_id,
    OCILobLocator *target_loc, short target_ind,
    OCILobLocator *query_loc,  short query_ind,
    const char    *params,     short params_ind,
    short *return_ind)
{
   OCINumber *result = NULL;

   ORABLOCK_BEGIN
   {
      *return_ind = OCI_IND_NULL;

      OracleEnv env(ctx, logger);

      if (query_ind  != OCI_IND_NOTNULL)
         throw BingoError("Null query given");
      if (target_ind != OCI_IND_NOTNULL)
         throw BingoError("Null target given");
      if (params_ind != OCI_IND_NOTNULL)
         params = 0;

      RingoOracleContext &context = RingoOracleContext::get(env, context_id, false);

      QS_DEF(Array<char>, query_buf);
      QS_DEF(Array<char>, target_buf);

      OracleLOB target_lob(env, target_loc);
      OracleLOB query_lob(env, query_loc);

      target_lob.readAll(target_buf, false);
      query_lob.readAll(query_buf, false);

      result = _ringoExact(env, context, query_buf, target_buf, params);

      if (result == 0)
         // This is needed for Oracle 9. Returning NULL drops the extproc.
         result = OracleExtproc::createInt(env, 0);
      else
         *return_ind = OCI_IND_NOTNULL;
   }
   ORABLOCK_END

   return result;
}
