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

#include "IndigoInChI.h"

#ifdef INDIGO_CPP_DEBUG
#include <iostream>
#include <sstream>
#include <thread>
#endif

#include <indigo-inchi.h>

#include "IndigoChemicalStructure.h"
#include "IndigoSession.h"

using namespace indigo_cpp;

IndigoInChI::IndigoInChI(IndigoSessionPtr session) : session(std::move(session))
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": IndigoInChI(" << session->getSessionId() << ")\n";
    std::cout << ss.str();
#endif
    this->session->setSessionId();
    indigoInchiInit(this->session->getSessionId());
}

IndigoInChI::~IndigoInChI()
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": ~IndigoInChI(" << session->getSessionId() << ")\n";
    std::cout << ss.str();
#endif
    session->setSessionId();
    indigoInchiDispose(session->getSessionId());
}

std::string IndigoInChI::getInChI(const IndigoChemicalStructure& data) const
{
    session->setSessionId();
    return {session->_checkResultString(indigoInchiGetInchi(data.id()))};
}
