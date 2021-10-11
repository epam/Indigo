#include "BingoResultIterator.h"

#include <bingo-nosql.h>

using namespace indigo_cpp;

template <typename target_t> BingoResultIterator<target_t>::BingoResultIterator(int id, IndigoSessionPtr session) : id(id), session(std::move(session))
{
}

template <typename target_t> BingoResultIterator<target_t>::~BingoResultIterator()
{
    if (id >= 0)
    {
        session->_checkResult(bingoEndSearch(id));
        id = -1;
    }
}

template <typename target_t> void BingoResultIterator<target_t>::next()
{
    session->setSessionId();
    auto result_id = session->_checkResult(bingoNext(id));
    if (result_id == 0)
    {
        _current = nullptr;
    }
    else
    {
        if (_current == nullptr)
        {
            _current = std::make_shared<BingoResult<target_t>>(id, session);
        }
    }
}

template <typename target_t> bool BingoResultIterator<target_t>::valid() const
{
    return _current != nullptr;
};

template <typename target_t> BingoResultIterator<target_t>::iterator::iterator(BingoResultIterator<target_t>* obj) : _obj(obj)
{
}

template <typename target_t> BingoResult<target_t>& BingoResultIterator<target_t>::iterator::operator*()
{
    return *_obj->_current;
}

template <typename target_t> typename BingoResultIterator<target_t>::iterator& BingoResultIterator<target_t>::iterator::operator++()
{
    increment();
    return *this;
}

template <typename target_t> bool BingoResultIterator<target_t>::iterator::operator==(iterator rhs) const
{
    return _obj == rhs._obj;
}

template <typename target_t> bool BingoResultIterator<target_t>::iterator::operator!=(iterator rhs) const
{
    return !(rhs == *this);
}

template <typename target_t> void BingoResultIterator<target_t>::iterator::increment()
{
    _obj->next();
    if (!_obj->valid())
    {
        _obj = nullptr;
    }
}

template <typename target_t> typename BingoResultIterator<target_t>::iterator BingoResultIterator<target_t>::begin()
{
    return ++iterator{this};
}

template <typename target_t> typename BingoResultIterator<target_t>::iterator BingoResultIterator<target_t>::end()
{
    return iterator{nullptr};
}

template class indigo_cpp::BingoResultIterator<IndigoMolecule>;
