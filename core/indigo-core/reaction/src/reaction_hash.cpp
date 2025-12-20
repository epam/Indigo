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

#include "reaction/reaction_hash.h"

#include "graph/subgraph_hash.h"
#include "molecule/elements.h"
#include "molecule/molecule_hash.h"
#include "reaction/reaction.h"

namespace indigo
{
    dword ReactionHash::calculate(Reaction& rxn)
    {
        int j;
        dword reactantHash = 0;
        for (j = rxn.reactantBegin(); j != rxn.reactantEnd(); j = rxn.reactantNext(j))
        {
            reactantHash += MoleculeHash::calculate(rxn.getMolecule(j));
        }
        dword productHash = 0;
        for (j = rxn.productBegin(); j != rxn.productEnd(); j = rxn.productNext(j))
        {
            productHash += MoleculeHash::calculate(rxn.getMolecule(j));
        }
        dword catalystHash = 0;
        for (j = rxn.catalystBegin(); j != rxn.catalystEnd(); j = rxn.catalystNext(j))
        {
            catalystHash += MoleculeHash::calculate(rxn.getMolecule(j));
        }
        dword hash = 0;
        hash = static_cast<dword>((hash + (324723947 + reactantHash)) ^ 93485734985);
        hash = static_cast<dword>((hash + (324723947 + productHash)) ^ 93485734985);
        hash = static_cast<dword>((hash + (324723947 + catalystHash)) ^ 93485734985);
        return hash;
    }
}
