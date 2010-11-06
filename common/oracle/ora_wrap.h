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

#ifndef __ora_wrap_h__
#define __ora_wrap_h__

#include "base_cpp/array.h"

// forward declaration
struct OCIStmt;
struct OCIError;
struct OCIExtProcContext;
struct OCIEnv;
struct OCISvcCtx;
struct OCILobLocator;
struct OCIRowid;
struct OCIRaw;
struct OCINumber;

namespace indigo {

class OracleLogger;
class OracleLOB;
class OracleRowID;
class OracleRaw;

class OracleError
{
public:
   explicit OracleError (OCIError *errhp, int oracle_rc, const char *message, int my_rc);
   explicit OracleError (int my_rc, const char *format, ...);
   virtual ~OracleError ();

   inline const char * getMessage () {return _message;}
   void raise (OracleLogger &logger, OCIExtProcContext *ctx);
private:
   char _message[1024];
   int _my_rc;
};

class OracleLogger;

class OracleEnv
{
public:
   explicit OracleEnv (OCIExtProcContext *ctx, OracleLogger &logger);
   explicit OracleEnv (const char *name, const char *password, const char *base, OracleLogger &logger);
   virtual ~OracleEnv ();

   inline OCIEnv    * envhp () {return _envhp;}
   inline OCISvcCtx * svchp () {return _svchp;}
   inline OCIError  * errhp () {return _errhp;}
   inline OCIExtProcContext * ctx () {return _ctx;}
   inline OracleLogger & logger () {return _logger;}

   void callOCI (int rc);

   void dbgPrintf (const char *format, ...);
   void dbgPrintfTS (const char *format, ...);

private:
   OCIEnv    *_envhp;
   OCISvcCtx *_svchp;
   OCIError  *_errhp;
   OCIExtProcContext *_ctx;

   OracleLogger &_logger;
};

class OracleStatement
{
public:
   explicit OracleStatement (OracleEnv &env);
   virtual ~OracleStatement ();

   void prepare ();

   void defineIntByPos    (int pos, int *value);
   void defineFloatByPos  (int pos, float *value);
   void defineBlobByPos   (int pos, OracleLOB &lob);
   void defineClobByPos   (int pos, OracleLOB &lob);
   void defineRowidByPos  (int pos, OracleRowID &rowid);
   void defineRawByPos    (int pos, OracleRaw &raw);
   void defineStringByPos (int pos, char *string, int max_len);
   void bindIntByName    (const char *name, int *value);
   void bindBlobByName   (const char *name, OracleLOB &lob);
   void bindClobByName   (const char *name, OracleLOB &lob);
   void bindRawByName    (const char *name, OracleRaw &raw);
   void bindStringByName (const char *name, const char *string, int max_len);

   void execute ();
   bool executeAllowNoData ();

   bool gotNull (int pos);

   bool fetch   ();
   
   void getRowID (OracleRowID &rowid);

   void append (const char *format, ...);
   void append_v (const char *format, va_list args);

   OCIStmt * get () {return _statement;}

   const char* getString () const {return _query;}

   static void executeSingle (OracleEnv &env, const char *format, ...);
   static bool executeSingleInt (int &result, OracleEnv &env, const char *format, ...);
   static bool executeSingleFloat (float &result, OracleEnv &env, const char *format, ...);
   static bool executeSingleString (Array<char> &result, OracleEnv &env, const char *format, ...);
   static bool executeSingleBlob (Array<char> &result, OracleEnv &env, const char *format, ...);
   static bool executeSingleClob (Array<char> &result, OracleEnv &env, const char *format, ...);

private:
   OracleEnv &_env;
   OCIStmt   *_statement;
   char       _query[10240];
   int        _query_len;
   short      _indicators[64];
};

class OracleLOB
{
public:
   explicit OracleLOB (OracleEnv &env);
   explicit OracleLOB (OracleEnv &env, OCILobLocator *lob);
   virtual ~OracleLOB ();

   void createTemporaryBLOB ();
   void createTemporaryCLOB ();

   inline OCILobLocator *  get    () {return _lob; }
   inline OCILobLocator ** getRef () {return &_lob; }

   void enableBuffering ();
   void disableBuffering ();
   void flush ();
   void openReadonly ();
   void openReadWrite ();
   
   int  getLength ();
   void readAll (Array<char> &arr, bool add_zero);

   void read  (int start, char *buffer, int buffer_size);
   void write (int start, const char *buffer, int bytes);
   void write (int start, const Array<char> &data);

   void trim (int length);
   inline void doNotDelete () {_delete_desc = false;}

private:
   OCILobLocator *_lob;
   bool _delete_desc;
   bool _delete_tmp;
   bool _is_buffered;
   bool _flushed;
   bool _opened;

   int  _buffered_read_start_pos;

   OracleEnv &_env;
};

class OracleRowID
{
public:
   explicit OracleRowID (OracleEnv &env);
   virtual ~OracleRowID ();

   inline OCIRowid *get() {return _rowid;}
   inline OCIRowid **getRef () {return &_rowid;}
protected:
   OracleEnv &_env;

   OCIRowid *_rowid;
};

class OracleRaw
{
public:
   explicit OracleRaw (OracleEnv &env);
   virtual ~OracleRaw ();

   inline OCIRaw  *get    () {return  _raw;}
   inline OCIRaw **getRef () {return &_raw;}

   char *getDataPtr ();

   void assignBytes (char *buffer, int length);
   void resize (int new_size);
protected:
   OracleEnv &_env;

   OCIRaw *_raw;
};

class OraRowidText
{
public:
   OraRowidText ();

   char * ptr () {return t;}
   int length ();

private:
   char t[19];

};

class OracleExtproc
{
public:
   static OCINumber * createInt (OracleEnv &env, int value);
   static OCINumber * createDouble (OracleEnv &env, double value);

};

class OracleUtil
{
public:
   static int    numberToInt    (OracleEnv &env, OCINumber *number);
   static float  numberToFloat  (OracleEnv &env, OCINumber *number);
   static double numberToDouble (OracleEnv &env, OCINumber *number);
};

}

#endif
