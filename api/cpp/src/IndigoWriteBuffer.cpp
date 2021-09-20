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
