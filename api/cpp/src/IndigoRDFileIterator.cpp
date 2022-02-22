#include "IndigoRDFileIterator.h"

#include <indigo.h>

#include "IndigoSession.h"

using namespace indigo_cpp;

IndigoRDFileIterator::iterator::iterator(IndigoRDFileIterator* obj) : _obj{obj}
{
}

IndigoRDFileIterator::iterator::reference IndigoRDFileIterator::iterator::operator*()
{
    return _obj->_current;
}

IndigoRDFileIterator::iterator& IndigoRDFileIterator::iterator::operator++()
{
    increment();
    return *this;
}

bool IndigoRDFileIterator::iterator::operator==(IndigoRDFileIterator::iterator rhs) const
{
    return _obj == rhs._obj;
}

bool IndigoRDFileIterator::iterator::operator!=(IndigoRDFileIterator::iterator rhs) const
{
    return !(rhs == *this);
}

void IndigoRDFileIterator::iterator::increment()
{
    _obj->next();
    if (!_obj->valid())
    {
        _obj = nullptr;
    }
}

IndigoRDFileIterator::IndigoRDFileIterator(const int id, IndigoSessionPtr session) : IndigoObject(id, std::move(session))
{
}

IndigoRDFileIterator::iterator IndigoRDFileIterator::begin()
{
    return ++iterator{this};
}

IndigoRDFileIterator::iterator IndigoRDFileIterator::end()
{
    return iterator{nullptr};
}

void IndigoRDFileIterator::next()
{
    session()->setSessionId();
    auto nextId = session()->_checkResult(indigoNext(id()));
    if (nextId == 0)
    {
        _current = nullptr;
    }
    else
    {
        _current = std::make_shared<IndigoReaction>(nextId, session());
    }
}

bool IndigoRDFileIterator::valid() const
{
    return _current != nullptr;
}
