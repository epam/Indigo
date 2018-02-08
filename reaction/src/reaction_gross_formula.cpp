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
#include "reaction/base_reaction.h"

#include "base_cpp/output.h"

using namespace indigo;

std::unique_ptr<std::pair<PtrArray<GROSS_UNITS> , PtrArray<GROSS_UNITS > > > ReactionGrossFormula::collect (BaseReaction &rxn, bool add_isotopes)
{
   
   std::unique_ptr<std::pair<PtrArray<GROSS_UNITS> , PtrArray<GROSS_UNITS > > > result (new std::pair<PtrArray<GROSS_UNITS> , PtrArray<GROSS_UNITS > >);
   auto& gross = *result;

   if (rxn.reactantsCount() > 0) {
      for (int i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i)) {
         auto mgross = MoleculeGrossFormula::collect(rxn.getBaseMolecule(i), add_isotopes);
         gross.first.add(mgross.release());
      }
   }
   if (rxn.productsCount() > 0) {
      for (int i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i)) {
         auto mgross = MoleculeGrossFormula::collect(rxn.getBaseMolecule(i), add_isotopes);
         gross.second.add(mgross.release());
      }
   }
   return result;
}

void ReactionGrossFormula::toString_Hill (std::pair<PtrArray<GROSS_UNITS> , PtrArray<GROSS_UNITS> > &gross, Array<char> &str, bool add_rsites)
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
