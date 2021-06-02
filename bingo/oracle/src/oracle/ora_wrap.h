/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __ora_wrap_h__
#define __ora_wrap_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

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

namespace indigo
{

    class OracleLogger;
    class OracleLOB;
    class OracleRowID;
    class OracleRaw;

    class OracleError : public Exception
    {
    public:
        explicit OracleError(OCIError* errhp, int oracle_rc, const char* message, int my_rc);
        explicit OracleError(int my_rc, const char* format, ...);

        void raise(OracleLogger& logger, OCIExtProcContext* ctx);
    private:
        int _code;
    };

    class OracleLogger;

    class OracleEnv
    {
    public:
        explicit OracleEnv(OCIExtProcContext* ctx, OracleLogger& logger);
        explicit OracleEnv(const char* name, const char* password, const char* base, OracleLogger& logger, bool object_mode = false);
        virtual ~OracleEnv();

        inline OCIEnv* envhp()
        {
            return _envhp;
        }
        inline OCISvcCtx* svchp()
        {
            return _svchp;
        }
        inline OCIError* errhp()
        {
            return _errhp;
        }
        inline OCIExtProcContext* ctx()
        {
            return _ctx;
        }
        inline OracleLogger& logger()
        {
            return _logger;
        }

        void callOCI(int rc);

        void dbgPrintf(const char* format, ...);
        void dbgPrintfTS(const char* format, ...);

        static int ociMajorVersion();

        int serverMajorVersion();

    private:
        OCIEnv* _envhp;
        OCISvcCtx* _svchp;
        OCIError* _errhp;
        OCIExtProcContext* _ctx;

        OracleLogger& _logger;
    };

    class OracleStatement
    {
    public:
        explicit OracleStatement(OracleEnv& env);
        virtual ~OracleStatement();

        void prepare();

        void defineIntByPos(int pos, int* value);
        void defineFloatByPos(int pos, float* value);
        void defineBlobByPos(int pos, OracleLOB& lob);
        void defineClobByPos(int pos, OracleLOB& lob);
        void defineRowidByPos(int pos, OracleRowID& rowid);
        void defineRawByPos(int pos, OracleRaw& raw);
        void defineStringByPos(int pos, char* string, int max_len);
        void bindIntByName(const char* name, int* value);
        void bindFloatByName(const char* name, float* value);
        void bindBlobByName(const char* name, OracleLOB& lob);
        void bindBlobPtrByName(const char* name, OCILobLocator** lob, short* indicators);
        void bindClobByName(const char* name, OracleLOB& lob);
        void bindRawByName(const char* name, OracleRaw& raw);
        void bindRawPtrByName(const char* name, OCIRaw* raw, int maxsize, short* indicators);
        void bindStringByName(const char* name, const char* string, int max_len);

        void execute();
        void executeMultiple(int iter);
        bool executeAllowNoData();

        bool gotNull(int pos);

        bool fetch();

        void getRowID(OracleRowID& rowid);

        void append(const char* format, ...);
        void append_v(const char* format, va_list args);

        OCIStmt* get()
        {
            return _statement;
        }

        const char* getString() const
        {
            return _query;
        }

        static void executeSingle(OracleEnv& env, const char* format, ...);
        static void executeSingle_BindString(OracleEnv& env, const char* bind, const char* value, const char* format, ...);
        static bool executeSingleInt(int& result, OracleEnv& env, const char* format, ...);
        static bool executeSingleFloat(float& result, OracleEnv& env, const char* format, ...);
        static bool executeSingleString(ArrayChar& result, OracleEnv& env, const char* format, ...);
        static bool executeSingleBlob(ArrayChar& result, OracleEnv& env, const char* format, ...);
        static bool executeSingleClob(ArrayChar& result, OracleEnv& env, const char* format, ...);

    private:
        OracleEnv& _env;
        OCIStmt* _statement;
        char _query[10240];
        int _query_len;
        short _indicators[64];
    };

    class OracleLOB
    {
    public:
        explicit OracleLOB(OracleEnv& env);
        explicit OracleLOB(OracleEnv& env, OCILobLocator* lob);
        virtual ~OracleLOB();

        void createTemporaryBLOB();
        void createTemporaryCLOB();

        inline OCILobLocator* get()
        {
            return _lob;
        }
        inline OCILobLocator** getRef()
        {
            return &_lob;
        }

        void enableBuffering();
        void disableBuffering();
        void flush();
        void openReadonly();
        void openReadWrite();

        int getLength();
        void readAll(ArrayChar& arr, bool add_zero);

        void read(int start, char* buffer, int buffer_size);
        void write(int start, const char* buffer, int bytes);
        void write(int start, const ArrayChar& data);

        void trim(int length);
        inline void doNotDelete()
        {
            _delete_desc = false;
        }

    private:
        OCILobLocator* _lob;
        bool _delete_desc;
        bool _delete_tmp;
        bool _is_buffered;
        bool _flushed;
        bool _opened;

        int _buffered_read_start_pos;

        OracleEnv& _env;
    };

    class OracleRowID
    {
    public:
        explicit OracleRowID(OracleEnv& env);
        virtual ~OracleRowID();

        inline OCIRowid* get()
        {
            return _rowid;
        }
        inline OCIRowid** getRef()
        {
            return &_rowid;
        }

    protected:
        OracleEnv& _env;

        OCIRowid* _rowid;
    };

    class OracleRaw
    {
    public:
        explicit OracleRaw(OracleEnv& env);
        virtual ~OracleRaw();

        inline OCIRaw* get()
        {
            return _raw;
        }
        inline OCIRaw** getRef()
        {
            return &_raw;
        }

        char* getDataPtr();

        int getAllocSize();
        int getSize();

        void assignBytes(const char* buffer, int length);
        void resize(int new_size);

    protected:
        OracleEnv& _env;
        int _size;

        OCIRaw* _raw;
    };

    class OraRowidText
    {
    public:
        OraRowidText();

        char* ptr()
        {
            return t;
        }
        int length();

    private:
        char t[19];
    };

    class OracleExtproc
    {
    public:
        static OCINumber* createInt(OracleEnv& env, int value);
        static OCINumber* createDouble(OracleEnv& env, double value);
    };

    class OracleUtil
    {
    public:
        static int numberToInt(OracleEnv& env, OCINumber* number);
        static float numberToFloat(OracleEnv& env, OCINumber* number);
        static double numberToDouble(OracleEnv& env, OCINumber* number);
    };

} // namespace indigo

#endif
