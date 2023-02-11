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

#pragma once

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/io_base.h"

namespace indigo
{
    class DLLEXPORT Output
    {
    public:
        DECL_ERROR;

        Output() = default;
        Output(Output&&) = delete;
        Output& operator=(Output&&) = delete;
        Output(const Output&) = delete;
        Output& operator=(const Output&) = delete;
        virtual ~Output() = default;

        virtual void write(const void* data, int size) = 0;
        virtual void flush() = 0;

        virtual void writeByte(byte value);

        void writeChar(char value);
        void writeBinaryInt(int value);
        void writeBinaryWord(word value);
        void writeBinaryUInt16(uint16_t value);

        void writeBinaryFloat(float value);
        void writePackedShort(short value);
        void writePackedUInt(unsigned int value);
        void writeString(const char* string);

        void writeStringCR(const char* string);
        void writeCR();
        void writeArray(const Array<char>& data);

        void printf(const char* format, ...);
        void vprintf(const char* format, va_list args);
        void printfCR(const char* format, ...);
    };

    class DLLEXPORT OutputTell
    {
        virtual long long tell() const noexcept = 0;
    };

    class DLLEXPORT OutputSeek
    {
        virtual void seek(long long offset, int from) = 0;
        void skip(int count);
    };

    class DLLEXPORT FileOutput : public Output, public OutputSeek, public OutputTell
    {
    public:
        FileOutput(Encoding filename_encoding, const char* filename);
        explicit FileOutput(const char* name);
        // explicit FileOutput (const char *format, ...);
        explicit FileOutput(bool append, const char* format, ...);
        FileOutput(FileOutput&&) = delete;
        FileOutput& operator=(FileOutput&&) = delete;
        FileOutput(const FileOutput&) = delete;
        FileOutput& operator=(const FileOutput&) = delete;
        ~FileOutput() override;

        void write(const void* data, int size) override;
        void seek(long long offset, int from) override;
        long long tell() const noexcept override;
        void flush() override;

    protected:
        FILE* _file;
    };

    class DLLEXPORT ArrayOutput : public Output, public OutputTell
    {
    public:
        explicit ArrayOutput(Array<char>& arr);

        void write(const void* data, int size) override;
        long long tell() const noexcept override;
        void flush() override;

        void clear();

    protected:
        Array<char>& _arr;
    };

    class DLLEXPORT StringOutput : public Output, public OutputTell
    {
    public:
        StringOutput() = delete;
        explicit StringOutput(std::string& str);

        void write(const void* data, int size) override;
        long long tell() const noexcept override;
        void flush() override;

        void clear() noexcept;

    protected:
        std::string& _str;
    };

    class DLLEXPORT StandardOutput : public Output, public OutputTell
    {
    public:
        explicit StandardOutput();

        void write(const void* data, int size) override;
        long long tell() const noexcept override;
        void flush() override;

    protected:
        int _count;
    };

    class DLLEXPORT NullOutput : public Output
    {
    public:
        void write(const void* data, int size) override;
        void flush() override;
    };

    DLLEXPORT void bprintf(Array<char>& buf, const char* format, ...);
}
