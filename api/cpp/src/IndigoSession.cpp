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

#include "IndigoSession.h"

#include <indigo.h>

#include "IndigoException.h"
#include "IndigoIterator.h"
#include "IndigoMolecule.h"
#include "IndigoQueryMolecule.h"
#include "IndigoReaction.h"
#include "IndigoSubstructureMatcher.h"
#include "IndigoWriteBuffer.h"

using namespace indigo_cpp;

//#define INDIGO_CPP_DEBUG

#ifdef INDIGO_CPP_DEBUG
#include <iostream>
#include <sstream>
#include <thread>
#endif

IndigoSession::IndigoSession() : id(indigoAllocSessionId())
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": IndigoSession(" << id << ")\n";
    std::cout << ss.str();
#endif
}

IndigoSession::~IndigoSession()
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": ~IndigoSession(" << id << ")\n";
    std::cout << ss.str();
#endif
    indigoReleaseSessionId(id);
}

unsigned long long IndigoSession::getSessionId() const
{
    return id;
}

void IndigoSession::setSessionId() const
{
    //#ifdef INDIGO_CPP_DEBUG
    //    std::stringstream ss;
    //    ss << "T_" << std::this_thread::get_id() << ": IndigoSession::_setSessionId(" << id << ")\n";
    //    std:: cout << ss.str();
    //#endif
    indigoSetSessionId(id);
}

int IndigoSession::_checkResult(int result) const
{
    if (result < 0)
    {
        setSessionId();
        throw(IndigoException(indigoGetLastError()));
    }
    return result;
}

int64_t IndigoSession::_checkResult(int64_t result) const
{
    if (result < 0)
    {
        setSessionId();
        throw(IndigoException(indigoGetLastError()));
    }
    return result;
}

double IndigoSession::_checkResultFloat(double result) const
{
    if (result < -0.5)
    {
        setSessionId();
        throw(IndigoException(indigoGetLastError()));
    }
    return result;
}

std::string IndigoSession::_checkResultString(const char* result) const
{
    if (result == nullptr)
    {
        setSessionId();
        throw(IndigoException(indigoGetLastError()));
    }
    return result;
}

void IndigoSession::setOption(const std::string& key, const std::string& value) const
{
    setSessionId();
    _checkResult(indigoSetOption(key.c_str(), value.c_str()));
}

void IndigoSession::setOption(const std::string& key, const int value) const
{
    setSessionId();
    _checkResult(indigoSetOptionInt(key.c_str(), value));
}

void IndigoSession::setOption(const std::string& key, const float value) const
{
    setSessionId();
    _checkResult(indigoSetOptionFloat(key.c_str(), value));
}

void IndigoSession::setOption(const std::string& key, const bool value) const
{
    setSessionId();
    _checkResult(indigoSetOptionBool(key.c_str(), static_cast<int>(value)));
}

std::string IndigoSession::version() const
{
    setSessionId();
    return _checkResultString(indigoVersion());
}

IndigoMolecule IndigoSession::loadMolecule(const std::string& data)
{
    setSessionId();
    return {_checkResult(indigoLoadMoleculeFromString(data.c_str())), shared_from_this()};
}

IndigoMolecule IndigoSession::loadMoleculeFromFile(const std::string& path)
{
    setSessionId();
    return {_checkResult(indigoLoadMoleculeFromFile(path.c_str())), shared_from_this()};
}

IndigoQueryMolecule IndigoSession::loadQueryMolecule(const std::string& data)
{
    setSessionId();
    return {_checkResult(indigoLoadQueryMoleculeFromString(data.c_str())), shared_from_this()};
}

IndigoWriteBuffer IndigoSession::writeBuffer()
{
    setSessionId();
    return {_checkResult(indigoWriteBuffer()), shared_from_this()};
}

IndigoIterator<IndigoMolecule> IndigoSession::iterateSDFile(const std::string& path)
{
    setSessionId();
    return {_checkResult(indigoIterateSDFile(path.c_str())), shared_from_this()};
}

IndigoIterator<IndigoMolecule> IndigoSession::iterateSmilesFile(const std::string& path)
{
    setSessionId();
    return {_checkResult(indigoIterateSmilesFile(path.c_str())), shared_from_this()};
}

IndigoIterator<IndigoReaction> IndigoSession::iterateRDFile(const std::string& path)
{
    setSessionId();
    return {_checkResult(indigoIterateRDFile(path.c_str())), shared_from_this()};
}

IndigoSessionPtr IndigoSession::create()
{
    return IndigoSessionPtr(new IndigoSession());
}

IndigoSubstructureMatcher IndigoSession::substructureMatcher(const IndigoMolecule& molecule, const std::string& mode)
{
    setSessionId();
    return {_checkResult(indigoSubstructureMatcher(molecule.id(), mode.c_str())), shared_from_this()};
}

IndigoReaction IndigoSession::loadReaction(const std::string& data)
{
    setSessionId();
    return {_checkResult(indigoLoadReactionFromString(data.c_str())), shared_from_this()};
}
