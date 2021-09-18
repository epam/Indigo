#include "IndigoRenderer.h"

#ifdef INDIGO_CPP_DEBUG
#include <iostream>
#include <thread>
#include <sstream>
#endif

#include <indigo-renderer.h>

#include "IndigoChemicalEntity.h"
#include "IndigoWriteBuffer.h"

using namespace indigo_cpp;

IndigoRenderer::IndigoRenderer(const IndigoSession& session) : _session(session)
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": IndigoRenderer(" << _session.getSessionId() << ")\n";
    std::cout << ss.str();
#endif
    _session.setSessionId();
    indigoRendererInit();
}

IndigoRenderer::~IndigoRenderer()
{
#ifdef INDIGO_CPP_DEBUG
    std::stringstream ss;
    ss << "T_" << std::this_thread::get_id() << ": ~IndigoRenderer(" << _session.getSessionId() << ")\n";
    std::cout << ss.str();
#endif
    _session.setSessionId();
    indigoRendererDispose();
}

std::string IndigoRenderer::svg(const IndigoChemicalEntity& data) const
{
    _session.setSessionId();
    _session.setOption("render-output-format", std::string("svg"));
    const auto buffer = _session.writeBuffer();
    _session._checkResult(indigoRender(data.id, buffer.id));
    return buffer.toString();
}

std::vector<char> IndigoRenderer::png(const IndigoChemicalEntity& data) const
{
    _session.setSessionId();
    _session.setOption("render-output-format", std::string("png"));
    const auto buffer = _session.writeBuffer();
    _session._checkResult(indigoRender(data.id, buffer.id));
    return buffer.toBuffer();
}
