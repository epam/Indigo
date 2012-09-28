/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include "molecule/max_common_submolecule.h"
#include "molecule/molecule.h"
#include "molecule/molecule_exact_matcher.h"

using namespace indigo;

MaxCommonSubmolecule::MaxCommonSubmolecule(BaseMolecule &submol, BaseMolecule &supermol):
MaxCommonSubgraph(submol,supermol)
{
   conditionEdgeWeight = matchBonds;
   conditionVerticesColor = matchAtoms;
}


bool MaxCommonSubmolecule::matchBonds (Graph &g1, Graph &g2, int i, int j, void* userdata){
   BaseMolecule &mol1 = (BaseMolecule &)g1;
   BaseMolecule &mol2 = (BaseMolecule &)g2;

   int flags = MoleculeExactMatcher::CONDITION_ELECTRONS;

   if(userdata)
      flags = *((int*)userdata);

   if(flags > MoleculeExactMatcher::CONDITION_ALL || flags < 0)
      throw Error("Wrong userdata...need correct flag");

   return MoleculeExactMatcher::matchBonds(mol1, mol2, i, j, flags);
}

bool MaxCommonSubmolecule::matchAtoms (Graph &g1, Graph &g2, const int *core_sub, int i, int j, void* userdata){
   BaseMolecule &mol1 = (BaseMolecule &)g1;
   BaseMolecule &mol2 = (BaseMolecule &)g2;

   int flags = MoleculeExactMatcher::CONDITION_ELECTRONS;

   if(userdata)
      flags = *((int*)userdata);

   if(flags > MoleculeExactMatcher::CONDITION_ALL || flags < 0)
      throw Error("Wrong userdata...need correct flag");

   return MoleculeExactMatcher::matchAtoms(mol1, mol2, i, j, flags);
}


