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
        virtual void readAll(Array<char>& arr);
        virtual void readAll(std::string& str);

        void read(int length, Array<char>& buf);

        void readLine(Array<char>& out, bool append_zero);
        void appendLine(Array<char>& out, bool append_zero);
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
        void skipBom();

        void skipUntil(const char* delimiters);

        float readFloat(void);
        bool tryReadFloat(float& value);
        int readInt(void);
        int readInt1(void);
        int readUnsigned();
        int tryReadUnsigned();

        // when delimiters = 0, any isspace() character is considered delimiter
        void readWord(Array<char>& word, const char* delimiters);

        bool findWord(const char* word);
        int findWord(ReusableObjArray<Array<char>>& words);
        bool findWordIgnoreCase(const char* word);
        int findWordIgnoreCase(ReusableObjArray<Array<char>>& words);

        static bool isSingleLine(Scanner& scanner);

    protected:
        bool _readDouble(double& res, int max);
        void _prefixFunction(Array<char>& str, Array<int>& prefix);
    };

    class DLLEXPORT FileScanner : public Scanner
    {
    public:
        FileScanner(Encoding filename_encoding, const char* filename);
        explicit FileScanner(const char* format, ...);
        ~FileScanner() override;

        void read(int length, void* res) override;
        bool isEOF() override;
        void skip(int n) override;
        int lookNext() override;
        void seek(long long pos, int from) override;
        long long length() override;
        long long tell() override;

        char readChar() override;

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
        explicit BufferScanner(const char* buffer, int buffer_size, bool is_base64 = false);
        explicit BufferScanner(const byte* buffer, int buffer_size, bool is_base64 = false);
        explicit BufferScanner(const char* str, bool is_base64 = false);
        explicit BufferScanner(const Array<char>& arr, bool is_base64 = false);
        ~BufferScanner() override;

        bool isEOF() override;
        void read(int length, void* res) override;
        void skip(int n) override;
        int lookNext() override;
        void seek(long long pos, int from) override;
        long long length() override;
        long long tell() override;
        byte readByte() override;

        const void* curptr();

    private:
        const char* _buffer;
        int _size;
        int _offset;
        bool _is_base64;
        Array<char> _base64_buffer;
        void _init(const char* buffer, int length);

        // no implicit copy
        BufferScanner(const BufferScanner&);
    };

} // namespace indigo

#endif
