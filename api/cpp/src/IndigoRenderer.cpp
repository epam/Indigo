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

#include "IndigoRenderer.h"

#ifdef INDIGO_CPP_DEBUG
#include <iostream>
#include <sstream>
#include <thread>
#endif

#include <indigo-renderer.h>

#include "IndigoChemicalStructure.h"
#include "IndigoSession.h"
#include "IndigoWriteBuffer.h"

using namespace indigo_cpp;

IndigoRenderer::IndigoRenderer(IndigoSessionPtr session) : session(std::move(session))
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": IndigoRenderer(" << _session.getSessionId() << ")\n";
    std::cout << ss.str();
#endif
    this->session->setSessionId();
    indigoRendererInit(this->session->getSessionId());
}

IndigoRenderer::~IndigoRenderer()
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": ~IndigoRenderer(" << _session.getSessionId() << ")\n";
    std::cout << ss.str();
#endif
    session->setSessionId();
    indigoRendererDispose(session->getSessionId());
}

std::string IndigoRenderer::svg(const IndigoChemicalStructure& data) const
{
    session->setSessionId();
    session->setOption("render-output-format", std::string("svg"));
    const auto& buffer = session->writeBuffer();
    session->_checkResult(indigoRender(data.id(), buffer.id()));
    return buffer.toString();
}

std::vector<char> IndigoRenderer::png(const IndigoChemicalStructure& data) const
{
    session->setSessionId();
    session->setOption("render-output-format", std::string("png"));
    const auto& buffer = session->writeBuffer();
    session->_checkResult(indigoRender(data.id(), buffer.id()));
    return buffer.toBuffer();
}
