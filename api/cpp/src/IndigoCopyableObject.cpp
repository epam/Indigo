#include "IndigoCopyableObject.h"

#include <indigo.h>

using namespace indigo_cpp;

IndigoCopyableObject::IndigoCopyableObject(int id, IndigoSessionPtr session) : IndigoObject(id, std::move(session))
{
}

IndigoCopyableObject::IndigoCopyableObject(const IndigoCopyableObject& other) : IndigoObject(_cloneAndReturnId(other), other.session())
{
}

IndigoCopyableObject& IndigoCopyableObject::operator=(const IndigoCopyableObject& other)
{
    if (this != &other)
    {
        _id = std::make_unique<int>(_cloneAndReturnId(other));
        _session = other.session();
    }
    return *this;
}

int IndigoCopyableObject::_cloneAndReturnId(const IndigoObject& other)
{
    other.session()->setSessionId();
    return other.session()->_checkResult(indigoClone(other.id()));
}
