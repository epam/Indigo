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

#include "molecule/molecule_stereocenter_iterator.h"

using namespace indigo;

StereocenterIterator::StereocenterIterator(MoleculeStereocenters::Container::const_iterator it) : _iterator {it}
{
}

bool StereocenterIterator::operator==(const StereocenterIterator& rhs) const
{
    return _iterator == rhs._iterator;
}

bool StereocenterIterator::operator!=(const StereocenterIterator& rhs) const
{
    return !(*this == rhs);
}
