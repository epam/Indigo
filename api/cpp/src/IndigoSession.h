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

#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace indigo_cpp
{
    class IndigoMolecule;
    class IndigoReaction;
    class IndigoQueryMolecule;
    class IndigoWriteBuffer;
    template <typename T>
    class IndigoIterator;
    class IndigoSession;
    class IndigoSubstructureMatcher;
    using IndigoSessionPtr = std::shared_ptr<IndigoSession>;

    class IndigoSession : public std::enable_shared_from_this<IndigoSession>
    {
    public:
        IndigoSession(IndigoSession&&) = default;
        IndigoSession& operator=(IndigoSession&&) = delete;
        IndigoSession(IndigoSession const&) = delete;
        void operator=(IndigoSession const&) = delete;
        ~IndigoSession();

        unsigned long long getSessionId() const;
        void setSessionId() const;

        int _checkResult(int result) const;
        int64_t _checkResult(int64_t result) const;
        double _checkResultFloat(double result) const;
        std::string _checkResultString(const char* result) const;

        void setOption(const std::string& key, const std::string& value) const;
        void setOption(const std::string& key, int value) const;
        void setOption(const std::string& key, float value) const;
        void setOption(const std::string& key, bool value) const;

        std::string version() const;
        std::string versionInfo() const;

        static IndigoSessionPtr create();

        IndigoMolecule loadMolecule(const std::string& data);
        IndigoMolecule loadMoleculeFromFile(const std::string& path);

        IndigoQueryMolecule loadQueryMolecule(const std::string& data);

        IndigoReaction loadReaction(const std::string& data);

        IndigoReaction loadQueryReaction(const std::string& data);

        IndigoWriteBuffer writeBuffer();
        IndigoIterator<IndigoReaction> iterateRDFile(const std::string& path);
        IndigoIterator<IndigoMolecule> iterateSDFile(const std::string& path);
        IndigoIterator<IndigoMolecule> iterateSmilesFile(const std::string& path);
        IndigoSubstructureMatcher substructureMatcher(const IndigoMolecule& molecule, const std::string& mode = "");

    private:
        IndigoSession();

        const unsigned long long id;
    };
}
