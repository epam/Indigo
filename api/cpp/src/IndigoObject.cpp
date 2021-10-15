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

#include "IndigoObject.h"

#include <indigo.h>

#include "IndigoSession.h"

//#define INDIGO_CPP_DEBUG

#ifdef INDIGO_CPP_DEBUG
#include <iostream>
#include <sstream>
#include <thread>
#endif

using namespace indigo_cpp;

IndigoObject::IndigoObject(int id, IndigoSessionPtr session) : _id(std::make_unique<int>(id)), _session(std::move(session))
{
#ifdef INDIGO_CPP_DEBUG
    if (id != 0)
    {
        std::stringstream ss;
        ss << "T_" << std::this_thread::get_id() << ": IndigoObject(" << this->session()->getSessionId() << ", " << this->id() << ")\n";
        std::cout << ss.str();
    }
#endif
    this->session()->_checkResult(id);
}

IndigoObject::~IndigoObject()
{
    if (_id != nullptr)
    {
#ifdef INDIGO_CPP_DEBUG
        std::stringstream ss;
        ss << "T_" << std::this_thread::get_id() << ": ~IndigoObject(" << session()->getSessionId() << ", " << id() << ")\n";
        std::cout << ss.str();
#endif
        session()->setSessionId();
        indigoFree(id());
    }
}

int IndigoObject::id() const
{
    return *_id;
}

const IndigoSessionPtr& IndigoObject::session() const
{
    return _session;
}
