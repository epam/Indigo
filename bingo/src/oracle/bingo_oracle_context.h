/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#ifndef __bingo_oracle_context__
#define __bingo_oracle_context__

#include "base_cpp/exception.h"
#include "core/bingo_context.h"
#include "oracle/bingo_storage.h"

using namespace indigo;

namespace ingido
{
   class BingoContext;
   class OracleEnv;
   class SharedMemory;
}

class BingoOracleContext : public BingoContext
{
public:
   
   explicit BingoOracleContext (OracleEnv &env, int id);
   virtual ~BingoOracleContext ();
   
   BingoStorage storage;

   int sim_screening_pass_mark;
   int sub_screening_pass_mark;
   int sub_screening_max_bits;

   static BingoOracleContext & get (OracleEnv &env, int id, bool lock, bool *config_reloaded);

   bool configGetInt    (OracleEnv &env, const char *name, int &value);
   void configSetInt    (OracleEnv &env, const char *name, int value);
   bool configGetFloat  (OracleEnv &env, const char *name, float &value);
   void configSetFloat  (OracleEnv &env, const char *name, float value);
   bool configGetString (OracleEnv &env, const char *name, Array<char> &value);
   void configSetString (OracleEnv &env, const char *name, const char *value);
   bool configGetBlob   (OracleEnv &env, const char *name, Array<char> &value);
   void configSetBlob   (OracleEnv &env, const char *name, const Array<char> &value);
   bool configGetClob   (OracleEnv &env, const char *name, Array<char> &value);
   void configSetClob   (OracleEnv &env, const char *name, const Array<char> &value);

   void configResetAll    (OracleEnv &env);
   void configReset       (OracleEnv &env, const char *name);

   void tautomerLoadRules         (OracleEnv &env);
   void fingerprintLoadParameters (OracleEnv &env);
   
   void saveCmfDict (OracleEnv &env);
   void saveRidDict (OracleEnv &env);

   void longOpInit   (OracleEnv &env, int total, const char *operation, const char *target, const char *units);
   void longOpUpdate (OracleEnv &env, int sofar);

   void parseParameters (OracleEnv &env, const char *str);

   void atomicMassLoad (OracleEnv &env);
   void atomicMassSave (OracleEnv &env);

   void lock (OracleEnv &env);
   void unlock (OracleEnv &env);

protected:
   bool        _config_changed;

   int         _longop_slno;
   int         _longop_rindex;
   Array<char> _longop_operation;
   Array<char> _longop_units;
   Array<char> _longop_target;    // actually, it is the source table
   int         _longop_total;

   Array<char> _id;
   SharedMemory *_shmem;

   void _loadConfigParameters (OracleEnv &env);
};


#endif
