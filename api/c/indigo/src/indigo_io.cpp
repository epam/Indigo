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

#include "indigo_io.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "indigo_savers.h"
#include "molecule/molecule_gross_formula.h"
#include <memory>

IndigoScanner::IndigoScanner(Scanner* scanner) : IndigoObject(SCANNER), ptr(scanner)
{
}

IndigoScanner::IndigoScanner(const char* str) : IndigoObject(SCANNER)
{
    _buf.readString(str, false);
    ptr = std::make_unique<BufferScanner>(_buf);
}

IndigoScanner::IndigoScanner(const char* buf, int size) : IndigoObject(SCANNER)
{
    _buf.copy(buf, size);
    ptr = std::make_unique<BufferScanner>(_buf);
}

IndigoScanner::~IndigoScanner()
{
}

Scanner& IndigoScanner::get(IndigoObject& obj)
{
    if (obj.type == SCANNER)
        return *((IndigoScanner&)obj).ptr;
    throw IndigoError("%s is not a scanner", obj.debugInfo());
}

IndigoOutput::IndigoOutput(Output* output) : IndigoObject(OUTPUT), ptr(output)
{
    _own_buf = false;
}

IndigoOutput::IndigoOutput() : IndigoObject(OUTPUT)
{
    ptr = std::make_unique<ArrayOutput>(_buf);
    _own_buf = true;
}

void IndigoOutput::toString(Array<char>& str)
{
    if (_own_buf)
        str.copy(_buf);
    else
        throw IndigoError("can not convert %s to string", debugInfo());
}

IndigoOutput::~IndigoOutput()
{
}

Output& IndigoOutput::get(IndigoObject& obj)
{
    if (obj.type == OUTPUT)
    {
        auto& ptr = ((IndigoOutput&)obj).ptr;

        if (ptr.get() == nullptr)
            throw IndigoError("output stream has been closed");
        return *ptr;
    }
    throw IndigoError("%s is not an output", obj.debugInfo());
}

CEXPORT int indigoReadFile(const char* filename)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoScanner(new FileScanner(self.filename_encoding, filename)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoReadString(const char* str)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoScanner(new BufferScanner(str)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoReadBuffer(const char* buffer, int size)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoScanner(new BufferScanner(buffer, size)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadString(const char* str)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoScanner(str));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoLoadBuffer(const char* buffer, int size)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoScanner(buffer, size));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoWriteFile(const char* filename)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoOutput(new FileOutput(filename)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClose(int output)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(output);
        if (obj.type == IndigoObject::OUTPUT)
        {
            IndigoOutput& out = ((IndigoOutput&)obj);
            out.ptr.reset(nullptr);
            return 1;
        }
        else if (obj.type == IndigoObject::SAVER)
        {
            IndigoSaver& saver = ((IndigoSaver&)obj);
            saver.close();
            return 1;
        }
        else
            throw IndigoError("indigoClose(): does not accept %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoWriteBuffer(void)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoOutput());
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoToString(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        auto& tmp = self.getThreadTmpData();
        obj.toString(tmp.string);
        tmp.string.push(0);

        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoToBase64String(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        auto& tmp = self.getThreadTmpData();
        obj.toBase64String(tmp.string);
        tmp.string.push(0);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoToBuffer(int handle, char** buf, int* size)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        auto& tmp = self.getThreadTmpData();
        obj.toBuffer(tmp.string);

        *buf = tmp.string.ptr();
        *size = tmp.string.size();
        return 1;
    }
    INDIGO_END(-1);
}
