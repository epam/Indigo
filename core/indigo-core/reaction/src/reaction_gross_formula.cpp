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

#include "reaction/reaction_gross_formula.h"
#include "molecule/molecule_gross_formula.h"
#include "reaction/base_reaction.h"

#include "base_cpp/output.h"

using namespace indigo;

std::unique_ptr<std::pair<PtrArray<GROSS_UNITS>, PtrArray<GROSS_UNITS>>> ReactionGrossFormula::collect(BaseReaction& rxn, bool add_isotopes)
{

    std::unique_ptr<std::pair<PtrArray<GROSS_UNITS>, PtrArray<GROSS_UNITS>>> result =
        std::make_unique<std::pair<PtrArray<GROSS_UNITS>, PtrArray<GROSS_UNITS>>>();
    auto& gross = *result;

    if (rxn.reactantsCount() > 0)
    {
        for (int i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
        {
            auto mgross = MoleculeGrossFormula::collect(rxn.getBaseMolecule(i), add_isotopes);
            gross.first.add(mgross.release());
        }
    }
    if (rxn.productsCount() > 0)
    {
        for (int i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
        {
            auto mgross = MoleculeGrossFormula::collect(rxn.getBaseMolecule(i), add_isotopes);
            gross.second.add(mgross.release());
        }
    }
    return result;
}

void ReactionGrossFormula::toString_Hill(std::pair<PtrArray<GROSS_UNITS>, PtrArray<GROSS_UNITS>>& gross, Array<char>& str, bool add_rsites)
{
    ArrayOutput output(str);
    Array<char> temp_str;

    bool first_written = false;
    for (int i = 0; i < gross.first.size(); i++)
    {
        if (first_written)
        {
            output.printf(" + ");
        }
        MoleculeGrossFormula::toString_Hill(*gross.first[i], temp_str, add_rsites);
        output.printf("%s", temp_str.ptr());
        first_written = true;
    }
    output.printf(" > ");
    first_written = false;
    for (int i = 0; i < gross.second.size(); i++)
    {
        if (first_written)
        {
            output.printf(" + ");
        }
        MoleculeGrossFormula::toString_Hill(*gross.second[i], temp_str, add_rsites);
        output.printf("%s", temp_str.ptr());
        first_written = true;
    }
}
