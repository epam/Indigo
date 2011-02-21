/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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


#include "molecule/molecule_cis_trans.h"

#include "graph/filter.h"
#include "molecule/molecule.h"
#include "molecule/elements.h"

using namespace indigo;

BaseMolecule & MoleculeCisTrans::_getMolecule ()
{
   char dummy[sizeof(BaseMolecule)];

   int offset = (int)((char *)(&((BaseMolecule *)dummy)->cis_trans) - dummy);

   return *(BaseMolecule *)((char *)this - offset);
}


int MoleculeCisTrans::sameside (const Vec3f &beg, const Vec3f &end, const Vec3f &nei_beg, const Vec3f &nei_end)
{
   Vec3f norm, norm_cross;
   Vec3f diff, norm_beg, norm_end;

   diff.diff(beg, end);

   norm_beg.diff(nei_beg, beg);

   // Use double cross product for getting vector lying the same plane with (beg-end) and (nei_beg-beg)
   norm_cross.cross(diff, norm_beg);
   norm.cross(norm_cross, diff);

   if (!norm.normalize())
      return 0;
   //throw Error("cannot normalize stereo double bond");

   norm_end.diff(nei_end, end);

   if (!norm_beg.normalize())
      return 0;
   //throw Error("cannot normalize neighbor bond of stereo double bond");
   if (!norm_end.normalize())
      return 0;
   //throw Error("cannot normalize neighbor bond of stereo double bond");

   float prod_beg = Vec3f::dot(norm_beg, norm);
   float prod_end = Vec3f::dot(norm_end, norm);

   if ((float)(fabs(prod_beg)) < 1e-3 || (float)(fabs(prod_end)) < 1e-3)
      return 0;
   //throw Error("double stereo bond is collinear with its neighbor bond");

   return (prod_beg * prod_end > 0) ? 1 : -1;
}

int MoleculeCisTrans::_sameside (BaseMolecule &molecule, int i_beg, int i_end, int i_nei_beg, int i_nei_end)
{
   return sameside(molecule.getAtomXyz(i_beg), molecule.getAtomXyz(i_end),
                   molecule.getAtomXyz(i_nei_beg), molecule.getAtomXyz(i_nei_end));
}

bool MoleculeCisTrans::_pureH (BaseMolecule &mol, int idx)
{
   return mol.getAtomNumber(idx) == ELEM_H && mol.possibleAtomIsotope(idx, 0);
}

bool MoleculeCisTrans::sortSubstituents (BaseMolecule &mol, int *substituents)
{
   bool h0 = _pureH(mol, substituents[0]);
   bool h1 = substituents[1] < 0 || _pureH(mol, substituents[1]);
   bool h2 = _pureH(mol, substituents[2]);
   bool h3 = substituents[3] < 0 || _pureH(mol, substituents[3]);
   int tmp;

   if (h0 && h1)
      return false;
   if (h2 && h3)
      return false;

   if (h1)
      substituents[1] = -1;
   else if (h0)
   {
      substituents[0] = substituents[1];
      substituents[1] = -1;
   }
   else if (substituents[0] > substituents[1])
      __swap(substituents[0], substituents[1], tmp);

   if (h3)
      substituents[3] = -1;
   else if (h2)
   {
      substituents[2] = substituents[3];
      substituents[3] = -1;
   }
   else if (substituents[2] > substituents[3])
      __swap(substituents[2], substituents[3], tmp);

   return true;
}

bool MoleculeCisTrans::isGeomStereoBond (BaseMolecule &mol, int bond_idx,
                                         int *substituents, bool have_xyz)
{
   int substituents_local[4];

   if (substituents == 0)
      substituents = substituents_local;

   // it must be [C,N,Si,Ge]=[C,N,Si,Ge] bond
   if (!mol.possibleBondOrder(bond_idx, BOND_DOUBLE))
      return false;

   const Edge &edge = mol.getEdge(bond_idx);
   int beg_idx = edge.beg;
   int end_idx = edge.end;

   if (!mol.possibleAtomNumber(beg_idx, ELEM_C) &&
       !mol.possibleAtomNumber(beg_idx, ELEM_N) &&
       !mol.possibleAtomNumber(beg_idx, ELEM_Si) &&
       !mol.possibleAtomNumber(beg_idx, ELEM_Ge))
      return false;
   if (!mol.possibleAtomNumber(end_idx, ELEM_C) &&
       !mol.possibleAtomNumber(end_idx, ELEM_N) &&
       !mol.possibleAtomNumber(end_idx, ELEM_Si) &&
       !mol.possibleAtomNumber(end_idx, ELEM_Ge))
      return false;

   // Double bonds with R-sites are excluded because cis-trans configuration 
   // cannot be determined when R-site is substituted with R-group
   if (mol.isRSite(beg_idx) || mol.isRSite(end_idx))
      return false;

   // the atoms should have 1 or 2 single bonds
   // (apart from the double bond under consideration)
   const Vertex &beg = mol.getVertex(beg_idx);
   const Vertex &end = mol.getVertex(end_idx);

   if (beg.degree() < 2 || beg.degree() > 3 ||
       end.degree() < 2 || end.degree() > 3)
      return false;

   substituents[0] = -1;
   substituents[1] = -1;
   substituents[2] = -1;
   substituents[3] = -1;

   int i;

   for (i = beg.neiBegin(); i != beg.neiEnd(); i = beg.neiNext(i))
   {
      int nei_edge_idx = beg.neiEdge(i);

      if (nei_edge_idx == bond_idx)
         continue;
      
      if (!mol.possibleBondOrder(nei_edge_idx, BOND_SINGLE))
         return false;

      if (substituents[0] == -1)
         substituents[0] = beg.neiVertex(i);
      else // (substituents[1] == -1)
         substituents[1] = beg.neiVertex(i);
   }

   for (i = end.neiBegin(); i != end.neiEnd(); i = end.neiNext(i))
   {
      int nei_edge_idx = end.neiEdge(i);

      if (nei_edge_idx == bond_idx)
         continue;
      
      if (!mol.possibleBondOrder(nei_edge_idx, BOND_SINGLE))
         return false;

      if (substituents[2] == -1)
         substituents[2] = end.neiVertex(i);
      else // (substituents[3] == -1)
         substituents[3] = end.neiVertex(i);
   }

   if (have_xyz)
   {
      if (substituents[1] != -1 &&
          _sameside(mol, beg_idx, end_idx, substituents[0], substituents[1]) != -1)
         return false;
      if (substituents[3] != -1 &&
          _sameside(mol, beg_idx, end_idx, substituents[2], substituents[3]) != -1)
         return false;
   }

   return true;
}

void MoleculeCisTrans::restoreSubstituents (int bond_idx)
{
   BaseMolecule &mol = _getMolecule();
   int *substituents = _bonds[bond_idx].substituents;

   if (!isGeomStereoBond(mol, bond_idx, substituents, false))
      throw Error("can't restore substituents");

   if (!sortSubstituents(mol, substituents))
      throw Error("can't sort restored substituents");
}


void MoleculeCisTrans::clear ()
{
   _bonds.clear();
}

bool MoleculeCisTrans::exists () const
{
   return _bonds.size() > 0;
}

void MoleculeCisTrans::registerBond (int idx)
{
   while (_bonds.size() <= idx)
      _bonds.push().clear();
   _bonds[idx].clear();
}

void MoleculeCisTrans::build (int *exclude_bonds)
{
   BaseMolecule &mol = _getMolecule();
   int i;

   clear();
   _bonds.clear_resize(mol.edgeEnd());
   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
   {
      _bonds[i].parity = 0;
      _bonds[i].ignored = 0;

      if (exclude_bonds != 0 && exclude_bonds[i])
      {
         _bonds[i].ignored = 1;
         continue;
      }

      int beg = mol.getEdge(i).beg;
      int end = mol.getEdge(i).end;

      int *substituents = _bonds[i].substituents;

      if (!isGeomStereoBond(mol, i, substituents, true))
         continue;

      if (!sortSubstituents(mol, substituents))
         continue;

      int sign = _sameside(mol, beg, end, substituents[0], substituents[2]);

      if (sign == 1)
         setParity(i, CIS);
      else if (sign == -1)
         setParity(i, TRANS);
   }
}

void MoleculeCisTrans::buildFromSmiles (int *dirs)
{
   QS_DEF(Array<int>, subst_used);
   int i, j;

   BaseMolecule &mol = _getMolecule();
   
   clear();
   subst_used.clear_resize(mol.vertexEnd());
   subst_used.zerofill();

   _bonds.clear_resize(mol.edgeEnd());

   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
   {
      _bonds[i].parity = 0;

      int beg = mol.getEdge(i).beg;
      int end = mol.getEdge(i).end;

      if (mol.getEdgeTopology(i) == TOPOLOGY_RING)
         continue;

      if (!isGeomStereoBond(mol, i, _bonds[i].substituents, false))
         continue;

      if (!sortSubstituents(mol, _bonds[i].substituents))
         continue;

      int substituents[4];

      memcpy(substituents, _bonds[i].substituents, 4 * sizeof(int));

      if (substituents[1] == -1)
      {
         const Vertex &vbeg = mol.getVertex(beg);

         for (int j = vbeg.neiBegin(); j != vbeg.neiEnd(); j = vbeg.neiNext(j))
            if (_pureH(mol, vbeg.neiVertex(j)))
               substituents[1] = vbeg.neiVertex(j);
      }

      if (substituents[3] == -1)
      {
         const Vertex &vend = mol.getVertex(end);

         for (int j = vend.neiBegin(); j != vend.neiEnd(); j = vend.neiNext(j))
            if (_pureH(mol, vend.neiVertex(j)))
               substituents[3] = vend.neiVertex(j);
      }

      int subst_dirs[4] = {0, 0, 0, 0};
      int nei_edge;

      nei_edge = mol.findEdgeIndex(beg, substituents[0]);

      if (dirs[nei_edge] == 1)
         subst_dirs[0] = mol.getEdge(nei_edge).beg == beg ? 1 : 2;
      if (dirs[nei_edge] == 2)
         subst_dirs[0] = mol.getEdge(nei_edge).beg == beg ? 2 : 1;

      if (substituents[1] != -1)
      {
         nei_edge = mol.findEdgeIndex(beg, substituents[1]);

         if (dirs[nei_edge] == 1)
            subst_dirs[1] = mol.getEdge(nei_edge).beg == beg ? 1 : 2;
         if (dirs[nei_edge] == 2)
            subst_dirs[1] = mol.getEdge(nei_edge).beg == beg ? 2 : 1;
      }

      nei_edge = mol.findEdgeIndex(end, substituents[2]);

      if (dirs[nei_edge] == 1)
         subst_dirs[2] = mol.getEdge(nei_edge).beg == end ? 1 : 2;
      if (dirs[nei_edge] == 2)
         subst_dirs[2] = mol.getEdge(nei_edge).beg == end ? 2 : 1;

      if (substituents[3] != -1)
      {
         nei_edge = mol.findEdgeIndex(end, substituents[3]);

         if (dirs[nei_edge] == 1)
            subst_dirs[3] = mol.getEdge(nei_edge).beg == end ? 1 : 2;
         if (dirs[nei_edge] == 2)
            subst_dirs[3] = mol.getEdge(nei_edge).beg == end ? 2 : 1;
      }

      if ((subst_dirs[0] != 0 && subst_dirs[0] == subst_dirs[1]) ||
          (subst_dirs[2] != 0 && subst_dirs[2] == subst_dirs[3]))
         //throw Error("cis-trans bonds %d have co-directed subsituents", i);
         // can happen on fragments such as CC=C(C=CN)C=CO
         continue;
      

      if (subst_dirs[0] == 0 && subst_dirs[1] == 0)
         continue;
      if (subst_dirs[2] == 0 && subst_dirs[3] == 0)
         continue;

      if (subst_dirs[1] == 1)
         subst_dirs[0] = 2;
      else if (subst_dirs[1] == 2)
         subst_dirs[0] = 1;

      if (subst_dirs[3] == 1)
         subst_dirs[2] = 2;
      else if (subst_dirs[3] == 2)
         subst_dirs[2] = 1;

      if (subst_dirs[0] == subst_dirs[2])
         setParity(i, CIS);
      else
         setParity(i, TRANS);

      for (j = 0; j < 4; j++)
         if (substituents[j] != -1)
            subst_used[substituents[j]] = 1;

   }

   /*for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
   {
      if (dirs[i] == 0)
         continue;

      const Edge &edge = mol.getEdge(i);

      if (!subst_used[edge.beg] && !subst_used[edge.end])
         throw Error("direction of bond %d makes no sense", i);
   }*/
}

int MoleculeCisTrans::getParity (int bond_idx) const
{
   if (bond_idx >= _bonds.size())
      return 0;
   return _bonds[bond_idx].parity;
}

bool MoleculeCisTrans::isIgnored (int bond_idx) const
{
   if (bond_idx >= _bonds.size())
      return false;
   return _bonds[bond_idx].ignored == 1;
}

void MoleculeCisTrans::ignore (int bond_idx)
{
   while (bond_idx >= _bonds.size())
      _bonds.push().clear();
   _bonds[bond_idx].parity = 0;
   _bonds[bond_idx].ignored = 1;
}

void MoleculeCisTrans::setParity (int bond_idx, int parity)
{
   while (_bonds.size() <= bond_idx)
      _bonds.push().clear();
   _bonds[bond_idx].parity = parity;
}

const int * MoleculeCisTrans::getSubstituents (int bond_idx) const
{
   return _bonds[bond_idx].substituents;
}

void MoleculeCisTrans::getSubstituents_All (int bond_idx, int subst[4])
{
   int i;
   BaseMolecule &mol = _getMolecule();

   memcpy(subst, _bonds[bond_idx].substituents, 4 * sizeof(int));

   if (subst[1] == -1)
   {
      const Vertex &vertex = mol.getVertex(mol.getEdge(bond_idx).beg);

      for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
      {
         if (_pureH(mol, vertex.neiVertex(i)))
         {
            subst[1] = vertex.neiVertex(i);
            break;
         }
      }
   }

   if (subst[3] == -1)
   {
      const Vertex &vertex = mol.getVertex(mol.getEdge(bond_idx).end);

      for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
      {
         if (_pureH(mol, vertex.neiVertex(i)))
         {
            subst[3] = vertex.neiVertex(i);
            break;
         }
      }
   }

}

void MoleculeCisTrans::add (int bond_idx, int substituents[4], int parity)
{
   registerBond(bond_idx);
   setParity(bond_idx, parity);
   for (int i = 0; i < 4; i++)
      _bonds[bond_idx].substituents[i] = substituents[i];
}

int MoleculeCisTrans::applyMapping (int parity, const int *substituents, const int *mapping)
{
   int sum = 0;

   if (substituents[1] >= 0 && mapping[substituents[1]] < mapping[substituents[0]])
      sum++;
   if (substituents[3] >= 0 && mapping[substituents[3]] < mapping[substituents[2]])
      sum++;

   if ((sum % 2) == 0)
      return parity;

   return (parity == CIS) ? TRANS : CIS;
}

int MoleculeCisTrans::getMappingParitySign (BaseMolecule &query, BaseMolecule &target,
                                     int bond_idx, const int *mapping)
{
   int query_parity = query.cis_trans.getParity(bond_idx);

   int target_edge_idx = Graph::findMappedEdge(query, target, bond_idx, mapping);
   int target_parity = target.cis_trans.getParity(target_edge_idx);

   if (target_parity == 0)
      return 0;

   int query_parity_mapped = applyMapping(query_parity,
                                 query.cis_trans.getSubstituents(bond_idx), mapping);

   if (query_parity_mapped != target_parity)
      return -1;

   return 1;
}

bool MoleculeCisTrans::checkSub (BaseMolecule &query, BaseMolecule &target, const int *mapping)
{
   int i;

   for (i = query.edgeBegin(); i != query.edgeEnd(); i = query.edgeNext(i))
   {
      if (!query.bondStereoCare(i))
         continue;

      int query_parity = query.cis_trans.getParity(i);

      if (query_parity == 0)
         throw Error("bond #%d has stereo-care flag, but is not cis-trans bond", i); 

      if (getMappingParitySign(query, target, i, mapping) <= 0)
         return false;
   }   
   return true;
}

void MoleculeCisTrans::buildOnSubmolecule (BaseMolecule &super, int *mapping)
{
   BaseMolecule &sub = _getMolecule();

   if (!super.cis_trans.exists())
      return;

   while (_bonds.size() < sub.edgeEnd())
   {
      _Bond &bond = _bonds.push();
      
      memset(&bond, 0, sizeof(_Bond));
   }

   int i, j;

   for (i = super.edgeBegin(); i != super.edgeEnd(); i = super.edgeNext(i))
   {
      int parity = super.cis_trans.getParity(i);
      int sub_edge_idx = Graph::findMappedEdge(super, sub, i, mapping);

      if (sub_edge_idx < 0)
         continue;

      _Bond &bond = _bonds[sub_edge_idx];
      bond.ignored = super.cis_trans.isIgnored(i);
      
      if (parity == 0)
      {
         bond.parity = 0;
         continue;
      }

      const int *substituents = super.cis_trans.getSubstituents(i);

      for (j = 0; j < 4; j++)
      {
         if (substituents[j] < 0 || mapping[substituents[j]] < 0)
            bond.substituents[j] = -1;
         else
            bond.substituents[j] = mapping[substituents[j]];
      }

      int sum = 0;
      int tmp;
      if (bond.substituents[0] == -1)
      {
         __swap(bond.substituents[0], bond.substituents[1], tmp);
         sum++;
      }
      if (bond.substituents[2] == -1)
      {
         __swap(bond.substituents[2], bond.substituents[3], tmp);
         sum++;
      }

      if (bond.substituents[0] == -1 || bond.substituents[2] == -1)
      {
         bond.parity = 0;
         continue;
      }

      bond.parity = applyMapping(parity, substituents, mapping);
      if (sum % 2 == 1)
      {
         if (bond.parity == CIS)
            bond.parity = TRANS;
         else
            bond.parity = CIS;
      }

      if (!sortSubstituents(sub, bond.substituents))
         throw Error("buildOnSubmolecule() internal error");
   }
}

bool MoleculeCisTrans::isAutomorphism (BaseMolecule &mol, const Array<int> &mapping, const Filter *edge_filter)
{
   for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
   {
      if (edge_filter && !edge_filter->valid(i))
         continue;

      const Edge &edge = mol.getEdge(i);
      int parity = mol.cis_trans.getParity(i);
      int subst[4];

      memcpy(subst, mol.cis_trans.getSubstituents(i), sizeof(int) * 4);

      if (mapping[subst[0]] < 0 || mapping[subst[2]] < 0)
         continue;

      if (subst[1] >= 0 && mapping[subst[1]] < 0)
         subst[1] = -1;
      if (subst[3] >= 0 && mapping[subst[3]] < 0)
         subst[3] = -1;

      int parity2 = MoleculeCisTrans::applyMapping(parity, subst, mapping.ptr());

      int i2 = mol.findEdgeIndex(mapping[edge.beg], mapping[edge.end]);
      if (mol.cis_trans.getParity(i2) != parity2)
         return false;
   }

   return true;
}

int MoleculeCisTrans::applyMapping (int idx, const int *mapping) const
{
   return applyMapping(getParity(idx), getSubstituents(idx), mapping);
}

void MoleculeCisTrans::flipBond (int atom_parent, int atom_from, int atom_to)
{
   BaseMolecule &mol = _getMolecule();
   int parent_edge_index = mol.findEdgeIndex(atom_parent, atom_from);
   if (parent_edge_index == -1 || getParity(parent_edge_index) != 0)
      // Such call wasn't expected and wasn't implemented
      throw Error("bond flipping with may cause stereobond destruction. "
         "Such functionality isn't implemented yet.");

   const Vertex &parent_vertex = mol.getVertex(atom_parent);
   for (int i = parent_vertex.neiBegin();
            i != parent_vertex.neiEnd();
            i = parent_vertex.neiNext(i))
   {
      int edge = parent_vertex.neiEdge(i);
      if (getParity(edge) == 0)
         continue;

      _Bond &bond = _bonds[edge];
      
      for (int i = 0; i < 4; i++)
         if (bond.substituents[i] == atom_from)
         {
            bond.substituents[i] = atom_to;
            break;
         }
   }

   const Vertex &from_vertex = mol.getVertex(atom_from);
   for (int i = from_vertex.neiBegin();
            i != from_vertex.neiEnd();
            i = from_vertex.neiNext(i))
   {
      int edge = parent_vertex.neiEdge(i);
      if (getParity(edge) == 0)
         continue;

      _Bond &bond = _bonds[edge];
      
      for (int i = 0; i < 4; i++)
         if (bond.substituents[i] == atom_parent)
         {
            bond.substituents[i] = atom_to;
            break;
         }
   }

   const Vertex &to_vertex = mol.getVertex(atom_to);
   for (int i = to_vertex.neiBegin();
            i != to_vertex.neiEnd();
            i = to_vertex.neiNext(i))
   {
      int edge = parent_vertex.neiEdge(i);
      if (getParity(edge) == 0)
         continue;

      _Bond &bond = _bonds[edge];

      int edge_beg = mol.getEdge(edge).beg;
      if (atom_to == edge_beg)
      {
         if (bond.substituents[1] != -1)
            throw Error("Cannot flip bond if all substituents are present");
         bond.substituents[1] = atom_parent;
      }
      else
      {
         if (bond.substituents[3] != -1)
            throw Error("Cannot flip bond if all substituents are present");
         bond.substituents[3] = atom_parent;
      }
   }
}

int MoleculeCisTrans::count ()
{
   int i, res = 0;

   for (i = 0; i < _bonds.size(); i++)
      if (_bonds[i].parity != 0)
         res++;

   return res;
}
