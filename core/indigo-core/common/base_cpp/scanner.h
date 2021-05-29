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

#ifndef __scanner_h__
#define __scanner_h__

#include "base_cpp/array.h"
#include "base_cpp/io_base.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/reusable_obj_array.h"
#include <stdio.h>

namespace indigo
{

    class DLLEXPORT Scanner
    {
    public:
        DECL_ERROR;

        virtual ~Scanner();

        virtual void read(int length, void* res) = 0;
        virtual void skip(int n) = 0;
        virtual bool isEOF() = 0;
        virtual int lookNext() = 0;
        virtual void seek(long long pos, int from) = 0;
        virtual long long length() = 0;
        virtual long long tell() = 0;

        virtual byte readByte();
        virtual void readAll(std::string& arr);

        void read(int length, std::string& buf);

        void readLine(std::string& out, bool append_zero);
        void appendLine(std::string& out, bool append_zero);
        bool skipLine();

        virtual char readChar();
        word readBinaryWord();
        int readBinaryInt();
        dword readBinaryDword();
        float readBinaryFloat();
        short readPackedShort();
        unsigned int readPackedUInt();

        void readCharsFix(int n, char* chars_out);
        int readCharsFlexible(int n, char* chars_out);
        float readFloatFix(int digits);
        int readIntFix(int digits);
        void skipSpace();

        void skipUntil(const char* delimiters);

        float readFloat(void);
        bool tryReadFloat(float& value);
        int readInt(void);
        int readInt1(void);
        int readUnsigned();

        // when delimiters = 0, any isspace() character is considered delimiter
        void readWord(std::string& word, const char* delimiters);

        bool findWord(const char* word);
        int findWord(ReusableObjArray<std::string>& words);
        bool findWordIgnoreCase(const char* word);
        int findWordIgnoreCase(ReusableObjArray<std::string>& words);

        static bool isSingleLine(Scanner& scanner);

    protected:
        bool _readDouble(double& res, int max);
        void _prefixFunction(std::string& str, Array<int>& prefix);
    };

    class DLLEXPORT FileScanner : public Scanner
    {
    public:
        FileScanner(Encoding filename_encoding, const char* filename);
        explicit FileScanner(const char* format, ...);
        virtual ~FileScanner();

        virtual void read(int length, void* res);
        virtual bool isEOF();
        virtual void skip(int n);
        virtual int lookNext();
        virtual void seek(long long pos, int from);
        virtual long long length();
        virtual long long tell();

        virtual char readChar();

    private:
        FILE* _file;
        long long _file_len;

        unsigned char _cache[1024];
        int _cache_pos, _max_cache;

        void _validateCache();
        void _invalidateCache();
        void _init(Encoding filename_encoding, const char* filename);

        // no implicit copy
        FileScanner(const FileScanner&);
    };

    class DLLEXPORT BufferScanner : public Scanner
    {
    public:
        explicit BufferScanner(const char* buffer, int buffer_size);
        explicit BufferScanner(const byte* buffer, int buffer_size);
        explicit BufferScanner(const char* str);
        explicit BufferScanner(const std::string& arr);
        virtual ~BufferScanner();

        virtual bool isEOF();
        virtual void read(int length, void* res);
        virtual void skip(int n);
        virtual int lookNext();
        virtual void seek(long long pos, int from);
        virtual long long length();
        virtual long long tell();
        virtual byte readByte();

        const void* curptr();

    private:
        const char* _buffer;
        int _size;
        int _offset;

        void _init(const char* buffer, int length);

        // no implicit copy
        BufferScanner(const BufferScanner&);
    };

} // namespace indigo

#endif
