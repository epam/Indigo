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

#ifndef __output_h__
#define __output_h__

#include <cstdio>

#include "base_cpp/array.h"
#include "base_cpp/io_base.h"

namespace indigo
{

    class DLLEXPORT Output
    {
    public:
        DECL_ERROR;

        explicit Output();
        virtual ~Output();

        virtual void write(const void* data, int size) = 0;
        virtual void seek(long long offset, int from) = 0;
        virtual long long tell() = 0;
        virtual void flush() = 0;

        virtual void writeByte(byte value);

        void writeChar(char value);
        void writeBinaryInt(int value);
        void writeBinaryDword(dword value);
        void writeBinaryWord(word value);
        void writeBinaryFloat(float value);
        void writePackedShort(short value);
        void writePackedUInt(unsigned int value);
        void writeString(const char* string);
        void writeStringCR(const char* string);
        void writeCR();
        void writeArray(const std::string& data);

        void skip(int count);

        void printf(const char* format, ...);
        void vprintf(const char* format, va_list args);
        void printfCR(const char* format, ...);
    };

    class DLLEXPORT FileOutput : public Output
    {
    public:
        FileOutput(Encoding filename_encoding, const char* filename);
        explicit FileOutput(const char* name);
        // explicit FileOutput (const char *format, ...);
        explicit FileOutput(bool append, const char* format, ...);
        virtual ~FileOutput();

        virtual void write(const void* data, int size);
        virtual void seek(long long offset, int from);
        virtual long long tell();
        virtual void flush();

    protected:
        FILE* _file;
    };

    class DLLEXPORT StringOutput : public Output
    {
    public:
        explicit StringOutput(std::string& arr);
        virtual ~StringOutput();

        virtual void write(const void* data, int size);
        virtual void seek(long long offset, int from);
        virtual long long tell();
        virtual void flush();

        void clear();

    protected:
        std::string& _arr;
    };

    class DLLEXPORT StandardOutput : public Output
    {
    public:
        explicit StandardOutput();
        virtual ~StandardOutput();

        virtual void write(const void* data, int size);
        virtual void seek(long long offset, int from);
        virtual long long tell();
        virtual void flush();

    protected:
        int _count;
    };

    class DLLEXPORT NullOutput : public Output
    {
    public:
        explicit NullOutput();
        virtual ~NullOutput();

        virtual void write(const void* data, int size);
        virtual void seek(long long offset, int from);
        virtual long long tell();
        virtual void flush();
    };

    DLLEXPORT void bprintf(std::string& buf, const char* format, ...);

} // namespace indigo

#endif
