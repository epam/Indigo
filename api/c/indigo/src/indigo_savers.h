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

#ifndef __indigo_savers__
#define __indigo_savers__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "indigo_internal.h"

class DLLEXPORT IndigoSaver : public IndigoObject
{
public:
    IndigoSaver(Output& output);
    ~IndigoSaver();

    void acquireOutput(Output* output);
    void close();

    void appendObject(IndigoObject& object);

    static IndigoSaver* create(Output& output, const char* type);

protected:
    virtual void _appendHeader(){};
    virtual void _appendFooter(){};
    virtual void _append(IndigoObject& object) = 0;

    Output& _output;

private:
    bool _closed;
    Output* _own_output;
};

class IndigoSdfSaver : public IndigoSaver
{
public:
    IndigoSdfSaver(Output& output) : IndigoSaver(output)
    {
    }
    const char* debugInfo() const override;
    static void append(Output& output, IndigoObject& object);
    static void appendMolfile(Output& output, IndigoObject& object);

protected:
    virtual void _append(IndigoObject& object);
};

class IndigoSmilesSaver : public IndigoSaver
{
public:
    IndigoSmilesSaver(Output& output) : IndigoSaver(output)
    {
    }
    const char* debugInfo() const override;

    static void generateSmiles(IndigoObject& obj, Array<char>& out_buffer, int smiles_format = 0);

    static void generateSmarts(IndigoObject& obj, Array<char>& out_buffer);

    static void append(Output& output, IndigoObject& object);

protected:
    void _append(IndigoObject& object) override;
};

class IndigoCanonicalSmilesSaver : public IndigoSaver
{
public:
    IndigoCanonicalSmilesSaver(Output& output) : IndigoSaver(output)
    {
    }
    const char* debugInfo() const override;

    static void generateSmiles(IndigoObject& obj, Array<char>& out_buffer);

    static void generateSmarts(IndigoObject& obj, Array<char>& out_buffer);

protected:
};

class IndigoCmlSaver : public IndigoSaver
{
public:
    IndigoCmlSaver(Output& output) : IndigoSaver(output)
    {
    }
    const char* debugInfo() const override;
    static void append(Output& output, IndigoObject& object);
    static void appendHeader(Output& output);
    static void appendFooter(Output& output);

protected:
    void _append(IndigoObject& object) override;
    void _appendHeader() override;
    void _appendFooter() override;
};

class IndigoRdfSaver : public IndigoSaver
{
public:
    IndigoRdfSaver(Output& output) : IndigoSaver(output)
    {
    }
    const char* debugInfo() const override;
    static void append(Output& output, IndigoObject& object);
    static void appendRXN(Output& output, IndigoObject& object);
    static void appendHeader(Output& output);

protected:
    void _append(IndigoObject& object) override;
    void _appendHeader() override;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __indigo_savers__
