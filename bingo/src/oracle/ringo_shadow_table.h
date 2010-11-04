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

#ifndef __ringo_shadow_table__
#define __ringo_shadow_table__

#include "base_cpp/tlscont.h"
#include "oracle/ora_wrap.h"
#include "oracle/bingo_fetch_engine.h"

class RingoIndex;

class RingoShadowTable
{
public:

   explicit RingoShadowTable (int context_id);

   void drop (OracleEnv &env);
   void truncate (OracleEnv &env);
   void create (OracleEnv &env);
   void addReaction (OracleEnv &env, RingoIndex &index, const char *rowid, int blockno, int offset);
   bool getReactionLocation (OracleEnv &env, const char *rowid, int &blockno, int &offset);
   void deleteReaction (OracleEnv &env, const char *rowid);

   void analyze (OracleEnv &env);
   int  countOracleBlocks (OracleEnv &env);

   const char * getName ();

   DEF_ERROR("ringo shadow table");

protected:
   Array<char>              _table_name;

private:
   RingoShadowTable (RingoShadowTable &); // no implicit copy
};

#endif
