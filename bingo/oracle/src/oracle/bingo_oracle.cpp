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

#include "oracle/bingo_oracle.h"
#include "oracle/ora_wrap.h"
#include "oracle/ora_logger.h"
#include "base_cpp/output.h"

OracleLogger logger;

const char *log_filename = "bingo.log";

int bingoPopRowidsToArray (OracleEnv &env, List<OraRowidText> &matched, int maxrows, OCIArray *array)
{
   OCIString *rid_string = 0;
   int count = 0;

   while (matched.size() > 0 && maxrows > 0)
   {
      const char *rid_text = matched.at(matched.begin()).ptr();

      env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (OraText *)rid_text, (ub4)strlen(rid_text), &rid_string));
      env.callOCI(OCICollAppend(env.envhp(), env.errhp(), rid_string, 0, array));

      maxrows--;
      count++;
      matched.remove(matched.begin());
   }
   return count;
}

int bingoGetExactRightPart (OracleEnv &env, OCINumber *p_strt, OCINumber *p_stop, int flags)
{
   if ((flags & 64) == 0)
      throw BingoError("only exact match allowed");

   if (p_strt == 0 || p_stop == 0)
      throw BingoError("only exact match allowed");

   int strt = OracleUtil::numberToInt(env, p_strt);
   int stop = OracleUtil::numberToInt(env, p_stop);

   if (strt != stop)
      throw BingoError("only exact match allowed");

   if (strt != 0 && strt != 1)
      throw BingoError("only =0 and =1 allowed");

   return strt;
}

void bingoBuildQueryID (OracleEnv &env,
                        const char *oper, const Array<char> &query_buf,
                        OCINumber *p_strt, OCINumber *p_stop, int flags,
                        const char *params, Array<char> &id)
{
   ArrayOutput output(id);

   output.printf("%s ", oper);

   if (params == 0)
      output.printf("<null> ");
   else
      output.printf("%s ", params);

   if (p_strt == 0)
      output.printf("<null> ");
   else
      output.printf("%.2f ", OracleUtil::numberToFloat(env, p_strt));

   if (p_stop == 0)
      output.printf("<null> ");
   else
      output.printf("%.2f ", OracleUtil::numberToFloat(env, p_stop));

   output.printf("%d ", flags);

   output.writeArray(query_buf);
}
