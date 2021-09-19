#pragma once

#include <string>
#include <map>

namespace indigo_cpp
{
    class IndigoMolecule;
    class IndigoQueryMolecule;
    class IndigoWriteBuffer;

    class IndigoSession
    {
    public:
        IndigoSession();
        ~IndigoSession();

        IndigoSession(IndigoSession&&) =default;
        IndigoSession(IndigoSession const&) = delete;
        IndigoSession& operator=(IndigoSession&&) = delete;
        void operator=(IndigoSession const&) = delete;

        unsigned long long getSessionId() const;
        void setSessionId() const;

        int _checkResult(int result) const;
        double _checkResultFloat(double result) const;
        std::string _checkResultString(const char* result) const;

        void setOption(const std::string& key, const std::string& value) const;
        void setOption(const std::string& key, int value) const;
        void setOption(const std::string& key, float value) const;
        void setOption(const std::string& key, bool value) const;

        std::string version() const;

        IndigoMolecule loadMolecule(const std::string& data) const;
        IndigoQueryMolecule loadQueryMolecule(const std::string& data) const;
        IndigoWriteBuffer writeBuffer() const;

    private:
        const unsigned long long id;
    };
} // namespace indigo_cpp
