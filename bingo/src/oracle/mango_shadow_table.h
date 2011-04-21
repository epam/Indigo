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

#ifndef __mango_shadow_table__
#define __mango_shadow_table__

#include "base_cpp/tlscont.h"
#include "oracle/ora_wrap.h"
#include "oracle/bingo_fetch_engine.h"
#include "core/mango_matchers.h"
#include "base_cpp/queue.h"

using namespace indigo;

class MangoIndex;

class MangoShadowTable
{
public:

   MangoShadowTable (int context_id);
   virtual ~MangoShadowTable ();

   bool getXyz (OracleEnv &env, const char *rowid, Array<char> &xyz);

   void drop (OracleEnv &env);
   void truncate (OracleEnv &env);
   void create (OracleEnv &env);
   void createIndices (OracleEnv &env);
   void addMolecule (OracleEnv &env, const MangoIndex &index, 
                     const char *rowid, int blockno, int offset);
   bool getMoleculeLocation (OracleEnv &env, const char *rowid, 
                             int &blockno, int &offset);
   void deleteMolecule (OracleEnv &env, const char *rowid);
   void addMolecule (OracleEnv &env, const char *rowid, int blockno, int offset,
                     const char *data_cmf, int len_cmf,
                     const char *data_xyz, int len_xyz,
                     const MangoExact::Hash &hash, const char *gross, 
                     const char *counters, float molecular_mass, const char *fp_ord);
   void flush (OracleEnv &env);

   void analyze (OracleEnv &env);
   
   const char * getName ();
   const char * getComponentsName ();

   DEF_ERROR("shadow table");

protected:
   Array<char> _table_name, _components_table_name;

private:
   MangoShadowTable (MangoShadowTable &); // no implicit copy

   void _flushMain (OracleEnv &env);
   void _flushComponents (OracleEnv &env);

   Obj<OracleStatement> _components_table_statement;
   int _components_table_statement_count;

   class _PendingValue
   {
   public:
      _PendingValue (const char *basename, int number);

      char name[20];

   private:
      _PendingValue (const _PendingValue &);
   };

   class _PendingLOB : public _PendingValue
   {
   public:
      _PendingLOB (OracleEnv &env, const char *basename, int number);

      OracleLOB lob;
   };

   class _PendingInt : public _PendingValue
   {
   public:
      _PendingInt (int val, const char *basename, int number);

      int value;
   };

   class _PendingFloat : public _PendingValue
   {
   public:
      _PendingFloat (float val, const char *basename, int number);

      float value;
   };

   class _PendingString : public _PendingValue
   {
   public:
      _PendingString (const char *val, const char *basename, int number);

      Array<char> value;
   };

   Obj<OracleStatement> _main_table_statement;
   int _main_table_statement_count;
   ObjArray<_PendingLOB> _pending_lobs;
   ObjArray<_PendingInt> _pending_ints;
   ObjArray<_PendingFloat> _pending_floats;
   ObjArray<_PendingString> _pending_strings;
};

#endif
