#ifndef __idt_alias__
#define __idt_alias__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/exception.h"
#include <string>

namespace indigo
{
    enum class IdtModification
    {
        FIVE_PRIME_END,
        INTERNAL,
        THREE_PRIME_END
    };

    class DLLEXPORT IdtAlias
    {
    public:
        DECL_ERROR;
        IdtAlias(){};

        IdtAlias(const std::string& base) : _base(base), _has_modifications(false)
        {
            if (_base.size() > 0)
            {
                _five_prime_end = "5" + base;
                _internal = "i" + base;
                _three_prime_end = "3" + base;
            }
        };

        IdtAlias(const std::string& base, const std::string& five_prime_end, const std::string& internal, const std::string& three_prime_end)
            : _base(base), _five_prime_end(five_prime_end), _internal(internal), _three_prime_end(three_prime_end), _has_modifications(true){};

        inline void setModifications(const std::string& five_prime_end, const std::string& internal, const std::string& three_prime_end)
        {
            _five_prime_end = five_prime_end;
            _internal = internal;
            _three_prime_end = three_prime_end;
            _has_modifications = true;
        };

        inline void setModification(IdtModification modification, const std::string& alias)
        {
            switch (modification)
            {
            case IdtModification::FIVE_PRIME_END:
                _five_prime_end = alias;
                break;
            case IdtModification::INTERNAL:
                _internal = alias;
                break;
            case IdtModification::THREE_PRIME_END:
                _three_prime_end = alias;
                break;
            };
            _has_modifications = true;
        };

        inline bool hasModification(IdtModification modification) const
        {
            switch (modification)
            {
            case IdtModification::FIVE_PRIME_END:
                return hasFivePrimeEnd();
            case IdtModification::INTERNAL:
                return hasInternal();
            case IdtModification::THREE_PRIME_END:
                return hasThreePrimeEnd();
            };
            return false;
        }

        inline bool hasFivePrimeEnd() const
        {
            return _five_prime_end.size() != 0;
        }

        inline bool hasInternal() const
        {
            return _internal.size() != 0;
        }

        inline bool hasThreePrimeEnd() const
        {
            return _three_prime_end.size() != 0;
        }

        const std::string& getModification(IdtModification modification) const;

        const std::string& getFivePrimeEnd() const;
        const std::string& getInternal() const;
        const std::string& getThreePrimeEnd() const;

        const std::string& getBase() const
        {
            return _base;
        };

        inline static std::string IdtModificationToString(IdtModification mod)
        {
            switch (mod)
            {
            case IdtModification::FIVE_PRIME_END:
                return "five-prime end";
            case IdtModification::INTERNAL:
                return "internal";
            case IdtModification::THREE_PRIME_END:
                return "three-prime end";
            };
            return "unknown modification";
        };

        static std::string getBaseForMod(const std::string& alias);

        inline bool hasModifications()
        {
            return _has_modifications;
        }

    private:
        std::string _base;
        std::string _five_prime_end;
        std::string _internal;
        std::string _three_prime_end;
        bool _has_modifications;
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
