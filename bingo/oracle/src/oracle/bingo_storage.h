/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef __bingo_storage__
#define __bingo_storage__

#include "base_cpp/array.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/shmem.h"

using namespace indigo;

namespace indigo
{
   class OracleEnv;
   class OracleLOB;
   class SharedMemory;
}

class BingoStorage
{
public:
   explicit BingoStorage (OracleEnv &env, int context_id);
   virtual ~BingoStorage ();
   
   void validateForInsert (OracleEnv &env);
   void add (OracleEnv &env, const Array<char> &data, int &blockno, int &offset);

   void create (OracleEnv &env);
   void drop (OracleEnv &env);
   void truncate (OracleEnv &env);
   void validate (OracleEnv &env);
   
   void flush (OracleEnv &env);
   void finish (OracleEnv &env);

   void lock (OracleEnv &env);
   void markRemoved (OracleEnv &env, int blockno, int offset);
   
   int  count ();
   void get (int n, Array<char> &out);
   
   DECL_ERROR;
   
protected:
   enum
   {
      _STATE_EMPTY   = 0,
      _STATE_BULDING = 1,
      _STATE_LOADING = 2,
      _STATE_READY   = 3
   };
   
   enum
   {
      _MAX_BLOCK_SIZE = 5 * 1024 * 1024
   };
   
   struct _Block
   {
      int size;
   };
   
   struct _State
   {
      int state;  // see STATE_***
      int age;
      int age_loaded;
   };
   
   struct _Addr
   {
      short blockno;
      short length;
      int   offset;
   };
   
   SharedMemory *_shmem_state;
   Array<_Block> _blocks;
   int           _n_added;

   int           _age_loaded;

   PtrArray<SharedMemory> _shmem_array;
   
   void   * _getShared (SharedMemory * &sh_mem, char *name, int shared_size, bool allow_first);
   _State * _getState  (bool allow_first);
   void     _insertLOB (OracleEnv &env, int no);
   OracleLOB * _getLob (OracleEnv &env, int no);
   void  _finishTopLob (OracleEnv &env);
   void  _finishIndexLob (OracleEnv &env);

   Array<char> _shmem_id;
   Array<char> _table_name;
   
   Array<_Addr> _index;
   Array<char>  _top_lob_pending_data;
   Array<char>  _index_lob_pending_data;
   int          _top_lob_pending_mark;
   int          _index_lob_pending_mark;
};

#endif
