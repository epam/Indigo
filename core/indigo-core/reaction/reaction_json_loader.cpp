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

#include "reaction/reaction_json_loader.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(ReactionJsonLoader, "reaction KET loader");

ReactionJsonLoader::ReactionJsonLoader(rapidjson::Value& molecule, rapidjson::Value& rgroups, rapidjson::Value& pluses, rapidjson::Value& arrows )
: _molecule(molecule), _rgroups( rgroups ), _pluses( pluses ), _arrows( arrows )
{
    ignore_bad_valence = false;
}

ReactionJsonLoader::~ReactionJsonLoader()
{
}

void ReactionJsonLoader::loadReaction( BaseReaction& rxn )
{
    _prxn = dynamic_cast<Reaction*>(&rxn);
    _pqrxn = dynamic_cast<QueryReaction*>(&rxn);
    if( _prxn == NULL && _pqrxn == NULL )
        throw Error("unknown reaction type: %s", typeid(rxn).name());
   
}
