#include "IndigoWriteBuffer.h"

#include <indigo.h>

using namespace indigo_cpp;

IndigoWriteBuffer::IndigoWriteBuffer(const int id, const IndigoSession& session) : IndigoObject(id, session)
{
}

std::vector<char> IndigoWriteBuffer::toBuffer() const
{
    indigo.setSessionId();
    char* buffer = nullptr;
    int size = 0;
    indigo._checkResult(indigoToBuffer(id, &buffer, &size));
    return std::vector<char>(buffer, buffer + size);
}

std::string IndigoWriteBuffer::toString() const
{
    const auto& buffer = toBuffer();
    return {buffer.begin(), buffer.end()};
}
