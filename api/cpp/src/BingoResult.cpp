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
