/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include "molecule/base_molecule.h"
#include "molecule/elements.h"

using namespace indigo;

BaseMolecule::BaseMolecule ()
{
   
}

BaseMolecule::~BaseMolecule ()
{
}

Molecule& BaseMolecule::asMolecule ()
{
   throw Error("casting to molecule is invalid");
}

QueryMolecule& BaseMolecule::asQueryMolecule ()
{
   throw Error("casting to query molecule is invalid");
}

bool BaseMolecule::isQueryMolecule ()
{
   return false;
}

void BaseMolecule::clear ()
{
   have_xyz = false;
   chiral = false;
   name.clear();
   stereocenters.clear();
   cis_trans.clear();
   _xyz.clear();
   _rsite_attachment_points.clear();
   Graph::clear();
}

bool BaseMolecule::hasZCoord (BaseMolecule &mol)
{
   int i;

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      if (fabs(mol.getAtomXyz(i).z) > 0.001)
         return true;

   return false;
}

void BaseMolecule::mergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices, 
                                         const Array<int> *edges, Array<int> *mapping_out,
                                         int skip_flags)
{
   QS_DEF(Array<int>, tmp_mapping);
   int i;

   if (mapping_out == 0)
      mapping_out = &tmp_mapping;

   // vertices and edges
   _mergeWithSubgraph(mol, vertices, edges, mapping_out);

   // XYZ
   _xyz.resize(vertexEnd());
   if (!(skip_flags & SKIP_XYZ))
   {
      if (vertexCount() == 0)
         have_xyz = mol.have_xyz;
      else
         have_xyz = have_xyz || mol.have_xyz;

      for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      {
         if (mapping_out->at(i) < 0)
            continue;

         _xyz[mapping_out->at(i)] = mol.getAtomXyz(i);
      }
   }
   else
      _xyz.zerofill();

   // subclass stuff (Molecule or QueryMolecule)
   _mergeWithSubmolecule(mol, vertices, edges, *mapping_out, skip_flags);

   // stereo
   if (!(skip_flags & SKIP_STEREOCENTERS))
      stereocenters.buildOnSubmolecule(mol.stereocenters, mapping_out->ptr());
   else
      stereocenters.clear();

   if (!(skip_flags & SKIP_CIS_TRANS))
      cis_trans.buildOnSubmolecule(mol, *this, mapping_out->ptr());
   else
      cis_trans.clear(*this);

   // subclass stuff (Molecule or QueryMolecule)
   _postMergeWithSubmolecule(mol, vertices, edges, *mapping_out, skip_flags);
}

int BaseMolecule::mergeAtoms (int atom1, int atom2)
{
   const Vertex &v1 = getVertex(atom1);
   const Vertex &v2 = getVertex(atom2);
   
   int is_tetra1 = false, is_cs1 = false, cs_bond1_idx = -1;
   int is_tetra2 = false, is_cs2 = false, cs_bond2_idx = -1;
   
   if (stereocenters.exists(atom1))
      is_tetra1 = true;
   if (stereocenters.exists(atom2))
      is_tetra2 = true;

   for (int i = v1.neiBegin(); i != v1.neiEnd(); i = v1.neiNext(i))
      if (MoleculeCisTrans::isGeomStereoBond(*this, v1.neiEdge(i), NULL, false))
      {
         cs_bond1_idx = v1.neiEdge(i);
         is_cs1 = true;
         break;
      }

   for (int i = v2.neiBegin(); i != v2.neiEnd(); i = v2.neiNext(i))
      if (MoleculeCisTrans::isGeomStereoBond(*this, v2.neiEdge(i), NULL, false))
      {
         cs_bond2_idx = v2.neiEdge(i);
         is_cs2 = true;
         break;
      }

   if (((is_tetra1 || is_cs1) && (is_tetra2 || is_cs2)) || 
         (!is_tetra1 && !is_cs1 && !is_tetra2 && !is_cs2))
   {
      if (is_tetra1)
         stereocenters.remove(atom1);
      if (is_cs1)
         cis_trans.setParity(cs_bond1_idx, 0);
      if (is_tetra2)
         stereocenters.remove(atom2);
      if (is_cs2)
         cis_trans.setParity(cs_bond2_idx, 0);

      QS_DEF(Array<int>, neighbors);
      neighbors.clear();
      for (int i = v2.neiBegin(); i != v2.neiEnd(); i = v2.neiNext(i))
         neighbors.push(v2.neiVertex(i));
      for (int i = 0; i < neighbors.size(); i++)
         if (findEdgeIndex(neighbors[i], atom1) == -1)
            flipBond(neighbors[i], atom2, atom1);

      removeAtom(atom2);

      return atom1;
   }

   if (is_tetra1 || is_cs1)
   {
      if (v2.degree() > 1)
         return -1;

      if (is_tetra1 && stereocenters.getPyramid(atom1)[3] != -1)
         return -1;

      if (is_cs1 && v1.degree() != 2)
         return -1;

      flipBond(v2.neiVertex(v2.neiBegin()), atom2, atom1);

      removeAtom(atom2);

      return atom1;
   }
   else
   {
      if (v1.degree() > 1)
         return -1;
      
      if (is_tetra2 && stereocenters.getPyramid(atom2)[3] != -1)
         return -1;

      if (is_cs2 && v2.degree() != 2)
         return -1;

      flipBond(v1.neiVertex(v1.neiBegin()), atom1, atom2);

      removeAtom(atom1);

      return atom2;
   }
}

void BaseMolecule::flipBond (int atom_parent, int atom_from, int atom_to)
{
   stereocenters.flipBond(atom_parent, atom_from, atom_to);
   cis_trans.flipBond(*this, atom_parent, atom_from, atom_to);

   // subclass (Molecule or QueryMolecule) adds the new bond
   _flipBond(atom_parent, atom_from, atom_to);

   int src_bond_idx = findEdgeIndex(atom_parent, atom_from);
   removeEdge(src_bond_idx);
}

void BaseMolecule::makeSubmolecule (BaseMolecule &mol, const Array<int> &vertices, 
                                    Array<int> *mapping_out, int skip_flags)
{
   clear();
   mergeWithSubmolecule(mol, vertices, 0, mapping_out, skip_flags);
}

void BaseMolecule::makeSubmolecule (BaseMolecule &other, const Filter &filter,
                                Array<int> *mapping_out, Array<int> *inv_mapping,
                                int skip_flags)
{
   QS_DEF(Array<int>, vertices);

   if (mapping_out == 0)
      mapping_out = &vertices;

   filter.collectGraphVertices(other, *mapping_out);

   makeSubmolecule(other, *mapping_out, inv_mapping, skip_flags);
}

void BaseMolecule::makeEdgeSubmolecule (BaseMolecule &mol, const Array<int> &vertices, 
                                        const Array<int> &edges, Array<int> *v_mapping,
                                        int skip_flags)
{
   clear();
   mergeWithSubmolecule(mol, vertices, &edges, v_mapping, skip_flags);
}

void BaseMolecule::clone (BaseMolecule &other, Array<int> *mapping, 
                          Array<int> *inv_mapping, int skip_flags)
{
   QS_DEF(Array<int>, tmp_mapping);

   if (mapping == 0)
      mapping = &tmp_mapping;

   mapping->clear();

   for (int i = other.vertexBegin(); i < other.vertexEnd(); i = other.vertexNext(i))
      mapping->push(i);

   makeSubmolecule(other, *mapping, inv_mapping, skip_flags);

   name.copy(other.name);
}

void BaseMolecule::mergeWithMolecule (BaseMolecule &other, Array<int> *mapping, int skip_flags)
{
   QS_DEF(Array<int>, vertices);
   int i;

   vertices.clear();

   for (i = other.vertexBegin(); i != other.vertexEnd(); i = other.vertexNext(i))
      vertices.push(i);

   mergeWithSubmolecule(other, vertices, 0, mapping, skip_flags);
}

void BaseMolecule::removeAtoms (const Array<int> &indices)
{
   QS_DEF(Array<int>, mapping);
   int i, j;
   
   mapping.clear_resize(vertexEnd());

   for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
      mapping[i] = i;

   // Mark removed vertices
   for (i = 0; i < indices.size(); i++)
      mapping[indices[i]] = -1;

   // subclass (Molecule or QueryMolecule) removes its data
   _removeAtoms(indices, mapping.ptr());

   stereocenters.removeAtoms(indices);
   cis_trans.buildOnSubmolecule(*this, *this, mapping.ptr());

   // sgroups
   for (j = data_sgroups.size() - 1; j >= 0; j--)
   {
      _removeAtomsFromSGroup(data_sgroups[j], mapping);
      if (data_sgroups[j].atoms.size() < 1)
         data_sgroups.remove(j);
   }
   for (j = superatoms.size() - 1; j >= 0; j--)
   {
      _removeAtomsFromSGroup(superatoms[j], mapping);
      if (superatoms[j].atoms.size() < 1)
         superatoms.remove(j);
   }

   // Remove vertices from graph
   for (i = 0; i < indices.size(); i++)
      removeVertex(indices[i]);

}

void BaseMolecule::removeAtom (int idx)
{
   QS_DEF(Array<int>, vertices);

   vertices.clear();
   vertices.push(idx);
   removeAtoms(vertices);
}

void BaseMolecule::removeAtoms (const Filter &filter)
{
   QS_DEF(Array<int>, vertices);

   filter.collectGraphVertices(*this, vertices);
   removeAtoms(vertices);
}

void BaseMolecule::removeBonds (const Array<int> &indices)
{
   // subclass (Molecule or QueryMolecule) removes its data
   _removeBonds(indices);

   stereocenters.removeBonds(indices);

   for (int i = 0; i < indices.size(); i++)
      removeEdge(indices[i]);
}

void BaseMolecule::removeBond (int idx)
{
   QS_DEF(Array<int>, edges);

   edges.clear();
   edges.push(idx);
   removeBonds(edges);
}

int BaseMolecule::getVacantPiOrbitals (int group, int charge, int radical, 
                                       int conn, int *lonepairs_out)
{
   int orbitals;

   if (conn < 0)
      throw Error("invalid connectivity given: %d", conn);

   switch (group)
   {
      case 1: orbitals = 1; break;
      case 2: orbitals = 2; break;
      default: orbitals = 4;
   }

   int free_electrons = group - conn - charge - radical;
   if (free_electrons < 0)
      return -1;

   int lonepair = free_electrons / 2;
   int implicit_radical = free_electrons % 2;

   int vacant = orbitals - conn - lonepair - radical - implicit_radical;
   if (vacant < 0)
      return -1;

   if (lonepairs_out != 0)
      *lonepairs_out = lonepair;
   return vacant;
}


void BaseMolecule::_mergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
           const Array<int> *edges, const Array<int> &mapping, int skip_flags)
{
}

void BaseMolecule::_postMergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
        const Array<int> *edges, const Array<int> &mapping, int skip_flags)
{
}


void BaseMolecule::_flipBond (int atom_parent, int atom_from, int atom_to)
{
}

void BaseMolecule::_removeAtoms (const Array<int> &indices, const int *mapping)
{
}

void BaseMolecule::_removeBonds (const Array<int> &indices)
{
}

Vec3f & BaseMolecule::getAtomXyz (int idx)
{
   return _xyz[idx];
}

void BaseMolecule::setAtomXyz (int idx, float x, float y, float z)
{
   _xyz[idx].set(x, y, z);
}

int BaseMolecule::_addBaseAtom ()
{
   int idx = addVertex();

   _xyz.expand(idx + 1);
   _xyz[idx].zero();
   return idx;
}

int BaseMolecule::_addBaseBond (int beg, int end)
{
   int idx = addEdge(beg, end);

   cis_trans.registerBond(idx);
   return idx;
}

int BaseMolecule::possibleAtomTotalH (int idx, int hcount)
{
   int minh = getAtomMinH(idx);

   if (minh > hcount)
      return false;

   int maxh = getAtomMaxH(idx);

   if (maxh == -1)
      return true;

   if (maxh < hcount)
      return false;

   return true;
}

void BaseMolecule::getAllowedRGroups (int atom_idx, Array<int> &rgroup_list)
{
   rgroup_list.clear();

   int bits = getRSiteBits(atom_idx);
   int rg_idx = 1;

   while (bits != 0)
   {
      if (bits & 1)
         rgroup_list.push(rg_idx);

      rg_idx++;
      bits >>= 1;
   }
}

int BaseMolecule::getSingleAllowedRGroup (int atom_idx)
{
   int bits = getRSiteBits(atom_idx);
   int rg_idx = 1;

   while (bits != 0)
   {
      if (bits & 1)
      {
         bits >>= 1;
         if (bits != 0)
            throw Error("getSingleAllowedRGroup(): multiple r-groups defined on atom #%d", atom_idx);
         return rg_idx;
      }

      rg_idx++;
      bits >>= 1;
   }

   throw Error("getSingleAllowedRGroup(): no r-groups defined on atom #%d", atom_idx);
}

int BaseMolecule::countRSites ()
{
   int i, sum = 0;

   for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
      if (isRSite(i))
         sum++;

   return sum;
}

int BaseMolecule::getRSiteAttachmentPointByOrder (int idx, int order) const
{
   if (idx >= _rsite_attachment_points.size())
      return -1;

   if (order >= _rsite_attachment_points[idx].size())
      return -1;

   return _rsite_attachment_points[idx][order];
}

void BaseMolecule::setRSiteAttachmentOrder (int atom_idx, int att_atom_idx, int order)
{
   _rsite_attachment_points.expand(atom_idx + 1);
   _rsite_attachment_points[atom_idx].expandFill(order + 1, -1);
   _rsite_attachment_points[atom_idx][order] = att_atom_idx;
}

BaseMolecule::SGroup::~SGroup ()
{
}

BaseMolecule::DataSGroup::DataSGroup ()
{
   attached = false;
   relative = false;
   display_units = false;
   dasp_pos = 1;
}

BaseMolecule::Superatom::Superatom ()
{
   bond_idx = -1;
}

void BaseMolecule::_removeAtomsFromSGroup (SGroup &sgroup, Array<int> &mapping)
{
   int i;

   for (i = sgroup.atoms.size() - 1; i >= 0; i--)
      if (mapping[i] == -1)
         sgroup.atoms.remove(i);

   for (i = sgroup.bonds.size() - 1; i >= 0; i--)
   {
      const Edge &edge = getEdge(i);
      if (mapping[edge.beg] == -1 || mapping[edge.end] == -1)
         sgroup.bonds.remove(i);
   }
}
