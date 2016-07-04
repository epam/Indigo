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

#include "oracle/ringo_oracle.h"
#include "base_cpp/output.h"
#include "oracle/ringo_shadow_table.h"
#include "oracle/bingo_oracle_context.h"
#include "base_cpp/auto_ptr.h"

const char *bad_reaction_warning = "WARNING: bad reaction: %s\n";
const char *bad_reaction_warning_rowid = "WARNING: bad reaction %s: %s\n";

RingoOracleContext::RingoOracleContext (BingoContext &context) :
RingoContext(context),
shadow_table(context.id),
fingerprints(context.id)
{
}

RingoOracleContext::~RingoOracleContext ()
{
}

BingoOracleContext & RingoOracleContext::context ()
{
   return (BingoOracleContext &)_context;
}

RingoOracleContext & RingoOracleContext::get (OracleEnv &env, int id, bool lock)
{
   bool config_reloaded;

   BingoOracleContext &context = BingoOracleContext::get(env, id, lock, &config_reloaded);

   RingoContext *already = _get(id, context);
   RingoOracleContext *roc;
   AutoPtr<RingoOracleContext> res;

   if (already != 0)
      roc = (RingoOracleContext *)already;
   else
   {
      res.reset(new RingoOracleContext(context));
      roc = res.get();
      config_reloaded = true;
   }

   if (config_reloaded)
   {
      roc->fingerprints.init(context, context.fp_parameters.fingerprintSizeExtOrdSim() * 2);
   }

   if (already == 0)
   {
      OsLocker locker(_instances_lock);
      TL_GET(PtrArray<RingoContext>, _instances);

      _instances.add(res.release());
      return *(RingoOracleContext *)_instances.top();
   }

   return *roc;
}
