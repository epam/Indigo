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

#ifndef __indigo_io__
#define __indigo_io__

#include "indigo_internal.h"
#include <memory>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

class DLLEXPORT IndigoScanner : public IndigoObject
{
public:
    IndigoScanner(Scanner* scanner);
    IndigoScanner(const char* str);
    IndigoScanner(const char* buf, int size);

    static Scanner& get(IndigoObject& obj);

    virtual ~IndigoScanner();

    std::unique_ptr<Scanner> ptr;

protected:
    std::string _buf;
};

class DLLEXPORT IndigoOutput : public IndigoObject
{
public:
    IndigoOutput();
    IndigoOutput(Output* output);
    virtual ~IndigoOutput();

    virtual void toString(std::string& str);

    static Output& get(IndigoObject& obj);

    std::unique_ptr<Output> ptr;

protected:
    bool _own_buf;
    std::string _buf;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
