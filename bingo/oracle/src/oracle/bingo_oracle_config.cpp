/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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
#include "base_cpp/tlscont.h"
#include "oracle/ora_wrap.h"
#include "oracle/ora_logger.h"
#include "oracle/bingo_oracle_context.h"

//
// Oracle wrappers for bingo config
//

ORAEXT void oraConfigResetAll (OCIExtProcContext *ctx, int context_id)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      BingoOracleContext &context = BingoOracleContext::get(env, context_id, false, 0);
      context.configResetAll(env);
   }
   ORABLOCK_END
}


ORAEXT void oraConfigSetInt (OCIExtProcContext *ctx, int context_id, char *key_name, short key_name_indicator, 
                             OCINumber *value, short value_indicator)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      if (key_name_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null key is given");
      if (value_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null value is given");

      ub4 value_int;

      env.callOCI(OCINumberToInt(env.errhp(), value, sizeof(value_int),
         OCI_NUMBER_UNSIGNED, &value_int));

      BingoOracleContext &context = BingoOracleContext::get(env, context_id, false, 0);
      context.configSetInt(env, key_name, value_int);
   }
   ORABLOCK_END
}


ORAEXT void oraConfigSetFloat (OCIExtProcContext *ctx, int context_id, char *key_name, short key_name_indicator, 
                             OCINumber *value, short value_indicator)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      if (key_name_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null key is given");
      if (value_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null value is given");

      double value_float;

      env.callOCI(OCINumberToReal(env.errhp(), value, sizeof(value_float), &value_float));

      BingoOracleContext &context = BingoOracleContext::get(env, context_id, false, 0);
      context.configSetFloat(env, key_name, (float)value_float);
   }
   ORABLOCK_END
}


ORAEXT void oraConfigSetString (OCIExtProcContext *ctx, int context_id, char *key_name, short key_name_indicator, 
                               char *value, short value_indicator)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      if (key_name_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null key is given");
      if (value_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null value is given");

      BingoOracleContext &context = BingoOracleContext::get(env, context_id, false, 0);
      context.configSetString(env, key_name, value);
   }
   ORABLOCK_END
}


ORAEXT OCINumber * oraConfigGetInt (OCIExtProcContext *ctx, int context_id, char *key_name, short key_name_indicator, 
                             short *return_indicator)
{
   OCINumber *result = NULL;

   ORABLOCK_BEGIN
   {
      *return_indicator = OCI_IND_NULL;

      OracleEnv env(ctx, logger);

      if (key_name_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null key is given");

      int value;
      BingoOracleContext &context = BingoOracleContext::get(env, context_id, false, 0);
      if (!context.configGetInt(env, key_name, value))
         throw BingoError("Key wasn't found");

      result = (OCINumber *)OCIExtProcAllocCallMemory(ctx, sizeof(OCINumber));

      if (result == NULL)
         throw BingoError("can't allocate memory for number");

      env.callOCI(OCINumberFromInt(env.errhp(), &value, sizeof(int), OCI_NUMBER_SIGNED, result));

      *return_indicator = OCI_IND_NOTNULL;
   }
   ORABLOCK_END

   return result;
}

ORAEXT OCINumber * oraConfigGetFloat (OCIExtProcContext *ctx, int context_id, 
                                      char *key_name, short key_name_indicator, 
                                      short *return_indicator)
{
   OCINumber *result = NULL;

   ORABLOCK_BEGIN
   {
      *return_indicator = OCI_IND_NULL;

      OracleEnv env(ctx, logger);

      if (key_name_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null key is given");

      float value;
      BingoOracleContext &context = BingoOracleContext::get(env, context_id, false, 0);
      if (!context.configGetFloat(env, key_name, value))
         throw BingoError("Key wasn't found");

      result = (OCINumber *)OCIExtProcAllocCallMemory(ctx, sizeof(OCINumber));

      if (result == NULL)
         throw BingoError("can't allocate memory for number");

      double value_double = value;
      env.callOCI(OCINumberFromReal(env.errhp(), &value_double, sizeof(value_double), result));

      *return_indicator = OCI_IND_NOTNULL;
   }
   ORABLOCK_END

   return result;
}

ORAEXT OCIString * oraConfigGetString (OCIExtProcContext *ctx, int context_id, 
                                       char *key_name, short key_name_indicator, 
                                       short *return_indicator)
{
   OCIString *result = NULL;

   ORABLOCK_BEGIN
   {
      *return_indicator = OCI_IND_NULL;

      OracleEnv env(ctx, logger);

      if (key_name_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null key is given");

      QS_DEF(Array<char>, value);
      BingoOracleContext &context = BingoOracleContext::get(env, context_id, false, 0);
      if (!context.configGetString(env, key_name, value))
         throw BingoError("Key wasn't found");

      env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text *)value.ptr(), value.size() - 1, &result));

      *return_indicator = OCI_IND_NOTNULL;
   }
   ORABLOCK_END

   return result;
}

ORAEXT void oraConfigReset (OCIExtProcContext *ctx, int context_id, 
                            char *key_name, short key_name_indicator)
{
   ORABLOCK_BEGIN
   {
      OracleEnv env(ctx, logger);

      if (key_name_indicator != OCI_IND_NOTNULL)
         throw BingoError("Null key is given");

      BingoOracleContext &context = BingoOracleContext::get(env, context_id, false, 0);
      context.configReset(env, key_name);
   }
   ORABLOCK_END
}
