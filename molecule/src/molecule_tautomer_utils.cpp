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

#include "molecule/molecule.h"
#include "molecule/molecule_tautomer_utils.h"
#include "molecule/elements.h"

using namespace indigo;

bool MoleculeTautomerUtils::_isRepMetal (int elem)
{
   static const int list[] =
   {ELEM_Li, ELEM_Na, ELEM_K, ELEM_Rb, ELEM_Cs, ELEM_Be, ELEM_Mg, ELEM_Ca, ELEM_Sr, ELEM_Ba};

   for (int i = 0; i < (int)NELEM(list); i++)
      if (elem == list[i])
         return true;

   return false;
}

// Count potential hydrogens (+, - charges or metal bonds)
void MoleculeTautomerUtils::countHReplacements (BaseMolecule &g, Array<int> &h_rep_count)
{

   h_rep_count.clear_resize(g.vertexEnd());

   for (int i = g.vertexBegin(); i < g.vertexEnd(); i = g.vertexNext(i))
   {
      const Vertex &vertex = g.getVertex(i);

      h_rep_count[i] = 0;

      for (int bond_idx = vertex.neiBegin(); bond_idx != vertex.neiEnd(); bond_idx = vertex.neiNext(bond_idx))
         if (_isRepMetal(g.getAtomNumber(vertex.neiVertex(bond_idx))))
         {
            int bond_type = g.getBondOrder(vertex.neiEdge(bond_idx));

            if (bond_type != BOND_AROMATIC)
               h_rep_count[i] += bond_type;
         }

         // + or - charge also count as potential hydrogens
         int charge = g.getAtomCharge(i);

         if (charge != CHARGE_UNKNOWN)
            h_rep_count[i] += abs(charge);
   }
}

// If core_2 != 0 highlights g1 too
void MoleculeTautomerUtils::highlightChains (BaseMolecule &g1, BaseMolecule &g2, const Array<int> &chains_2, const int *core_2)
{
   int i;

   for (i = g2.vertexBegin(); i < g2.vertexEnd(); i = g2.vertexNext(i))
   {
      if (chains_2[i] > 0 || (core_2 != 0 && core_2[i] >= 0))
         g2.highlightAtom(i);
   }

   for (i = g2.edgeBegin(); i < g2.edgeEnd(); i = g2.edgeNext(i))
   {
      const Edge &edge = g2.getEdge(i);

      // zeroed bond?
      if (g2.getBondOrder(i) == -1 && g2.possibleBondOrder(i, BOND_SINGLE))
         continue;

      if (chains_2[edge.beg] > 0 && chains_2[edge.end] > 0 && abs(chains_2[edge.beg] - chains_2[edge.end]) == 1)
         g2.highlightBond(i);
      else if (core_2 != 0 && core_2[edge.beg] >= 0 && core_2[edge.end] >= 0)
      {
         if (g1.findEdgeIndex(core_2[edge.beg], core_2[edge.end]) != -1)
            g2.highlightBond(i);
      }
   }
}
