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
                     const char *rowid, int blockno, int offset, bool append);
   bool getMoleculeLocation (OracleEnv &env, const char *rowid, 
                             int &blockno, int &offset);
   void deleteMolecule (OracleEnv &env, const char *rowid);
   void addMolecule (OracleEnv &env, const char *rowid, int blockno, int offset,
                     const char *data_cmf, int len_cmf,
                     const char *data_xyz, int len_xyz,
                     const MangoExact::Hash &hash, const char *gross, 
                     const Array<int> &counters, float molecular_mass, const char *fp_ord,
                     bool append);
   void flush (OracleEnv &env);

   void analyze (OracleEnv &env);
   
   const char * getName ();
   const char * getComponentsName ();

   DECL_ERROR;

protected:
   Array<char> _table_name, _components_table_name;

   void _flushMain (OracleEnv &env);
   void _flushComponents (OracleEnv &env);

   Obj<OracleStatement> _main_table_statement;
   Obj<OracleStatement> _components_table_statement;
   
   int _main_table_statement_count;
   int _components_table_statement_count;

   Array<char[19]>        _pending_rid;
   Array<int>             _pending_blockno;
   Array<int>             _pending_offset;
   Array<char[512]>       _pending_gross;
   ObjArray<OracleRaw>    _pending_cmf;
   ObjArray<OracleRaw>    _pending_xyz;
   Array<float>           _pending_mass;
   Array<int>             _pending_fragcount;
   ObjArray< Array<int> > _pending_counters;

   Array<char[19]>        _pending_comp_rid;
   Array<char[9]>         _pending_comp_hash;
   Array<int>             _pending_comp_count;

   bool _commit_main;
   bool _commit_comp;

private:
   MangoShadowTable (MangoShadowTable &); // no implicit copy
};

#endif
