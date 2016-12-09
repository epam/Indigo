/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "reaction/reaction_gross_formula.h"
#include "molecule/molecule_gross_formula.h"

#include "base_cpp/output.h"

using namespace indigo;

void ReactionGrossFormula::collect (BaseReaction &rxn, std::pair<ObjArray<std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > > , ObjArray<std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > > > &gross)
{
    gross.first.clear();
    gross.first.resize(rxn.reactantsCount());
    gross.second.clear();
    gross.second.resize(rxn.productsCount());
    int array_index = 0;

    if (rxn.reactantsCount() > 0)
    {
        for (int i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
        {
            MoleculeGrossFormula::collect(rxn.getBaseMolecule(i), gross.first[array_index++]);
        }
    }
    if (rxn.productsCount() > 0)
    {
        array_index = 0;
        for (int i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
        {
            MoleculeGrossFormula::collect(rxn.getBaseMolecule(i), gross.second[array_index++]);
        }
    }
}

void ReactionGrossFormula::toString_Hill (const std::pair<ObjArray<std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > > , ObjArray<std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > > > &gross, Array<char> &str, bool add_rsites)
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
        MoleculeGrossFormula::toString_Hill(gross.first[i], temp_str, add_rsites);
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
        MoleculeGrossFormula::toString_Hill(gross.second[i], temp_str, add_rsites);
        output.printf("%s", temp_str.ptr());
        first_written = true;
    }
}
