#include "IndigoSDFileIterator.h"

#include <indigo.h>

#include "IndigoSession.h"

using namespace indigo_cpp;

IndigoSDFileIterator::iterator::iterator(IndigoSDFileIterator* obj) : _obj{obj}
{
}

IndigoSDFileIterator::iterator::reference IndigoSDFileIterator::iterator::operator*()
{
    return _obj->_current;
}

IndigoSDFileIterator::iterator& IndigoSDFileIterator::iterator::operator++()
{
    increment();
    return *this;
}

bool IndigoSDFileIterator::iterator::operator==(IndigoSDFileIterator::iterator rhs) const
{
    return _obj == rhs._obj;
}

bool IndigoSDFileIterator::iterator::operator!=(IndigoSDFileIterator::iterator rhs) const
{
    return !(rhs == *this);
}

void IndigoSDFileIterator::iterator::increment()
{
    _obj->next();
    if (!_obj->valid())
    {
        _obj = nullptr;
    }
}

IndigoSDFileIterator::IndigoSDFileIterator(const int id, const IndigoSession& session) : IndigoObject(id, session)
{
}

IndigoSDFileIterator::iterator IndigoSDFileIterator::begin()
{
    return ++iterator{this};
}

IndigoSDFileIterator::iterator IndigoSDFileIterator::end()
{
    return iterator{nullptr};
}

void IndigoSDFileIterator::next()
{
    indigo.setSessionId();
    auto nextId = indigo._checkResult(indigoNext(id));
    if (nextId == 0)
    {
        _current = nullptr;
    }
    else
    {
        _current = std::make_shared<IndigoMolecule>(nextId, indigo);
    }
}

bool IndigoSDFileIterator::valid() const
{
    return _current != nullptr;
}
