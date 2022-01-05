#include <bingo-nosql.h>

#include "BingoResult.h"
#include "IndigoMolecule.h"

using namespace indigo_cpp;

template <typename target_t>
BingoResult<target_t>::BingoResult(int id, IndigoSessionPtr session) : id(id), session(std::move(session))
{
}

template <typename target_t>
int BingoResult<target_t>::getId() const
{
    session->setSessionId();
    return session->_checkResult(bingoGetCurrentId(id));
}

template <typename target_t>
double BingoResult<target_t>::getSimilarityValue() const
{
    session->setSessionId();
    return session->_checkResultFloat(bingoGetCurrentSimilarityValue(id));
}

template <typename target_t>
target_t BingoResult<target_t>::getTarget()
{
    session->setSessionId();
    return target_t(session->_checkResult(bingoGetObject(id)), session);
}

template class indigo_cpp::BingoResult<IndigoMolecule>;
