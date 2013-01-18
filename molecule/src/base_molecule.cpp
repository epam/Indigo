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

#include "molecule/base_molecule.h"

#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/query_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom_match.h"

using namespace indigo;

IMPL_ERROR(BaseMolecule, "molecule");

BaseMolecule::BaseMolecule ()
{
   _edit_revision = 0;
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
   name.clear();
   stereocenters.clear();
   cis_trans.clear();
   allene_stereo.clear();
   rgroups.clear();
   _xyz.clear();
   _rsite_attachment_points.clear();
   _attachment_index.clear();
   data_sgroups.clear();
   superatoms.clear();
   repeating_units.clear();
   multiple_groups.clear();
   generic_sgroups.clear();
   Graph::clear();
   _hl_atoms.clear();
   _hl_bonds.clear();
   _bond_directions.clear();

   updateEditRevision();
}

bool BaseMolecule::hasCoord (BaseMolecule &mol)
{
   int i;

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
   {
      Vec3f &xyz = mol.getAtomXyz(i);
      if (fabs(xyz.x) > 0.001 || fabs(xyz.y) > 0.001 || fabs(xyz.z) > 0.001)
         return true;
   }

   return false;
}


bool BaseMolecule::hasZCoord (BaseMolecule &mol)
{
   int i;

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      if (fabs(mol.getAtomXyz(i).z) > 0.001)
         return true;

   return false;
}

void BaseMolecule::mergeSGroupsWithSubmolecule (BaseMolecule &mol, Array<int> &mapping)
{
   QS_DEF(Array<int>, edge_mapping);
   edge_mapping.clear_resize(mol.edgeEnd());
   edge_mapping.fffill();

   buildEdgeMapping(mol, &mapping, &edge_mapping);

   mergeSGroupsWithSubmolecule(mol, mapping, edge_mapping);
}

void BaseMolecule::mergeSGroupsWithSubmolecule (BaseMolecule &mol, Array<int> &mapping,
                                              Array<int> &edge_mapping)
{
   int i;

   // data sgroups
   for (i = mol.data_sgroups.begin(); i != mol.data_sgroups.end(); i = mol.data_sgroups.next(i))
   {
      DataSGroup &supersg = mol.data_sgroups[i];
      int idx = data_sgroups.add();
      DataSGroup &sg = data_sgroups[idx];
      if (_mergeSGroupWithSubmolecule(sg, supersg, mol, mapping, edge_mapping))
      {
         sg.detached = supersg.detached;
         sg.display_pos = supersg.display_pos;
         sg.data.copy(supersg.data);
         sg.dasp_pos = supersg.dasp_pos;
         sg.relative = supersg.relative;
         sg.display_units = supersg.display_units;
         sg.description.copy(supersg.description);
      }
      else
         data_sgroups.remove(idx);
   }

   // superatoms
   for (i = mol.superatoms.begin(); i != mol.superatoms.end(); i = mol.superatoms.next(i))
   {
      Superatom &supersa = mol.superatoms[i];
      int idx = superatoms.add();
      Superatom &sa = superatoms[idx];

      if (_mergeSGroupWithSubmolecule(sa, supersa, mol, mapping, edge_mapping))
      {
         sa.bond_dir = supersa.bond_dir;
         if (supersa.bond_idx >= 0)
            sa.bond_idx = edge_mapping[supersa.bond_idx];
         else
            sa.bond_idx = -1;
         sa.subscript.copy(supersa.subscript);
      }
      else
         superatoms.remove(idx);
   }

   // repeating units
   for (i = mol.repeating_units.begin(); i != mol.repeating_units.end(); i = mol.repeating_units.next(i))
   {
      RepeatingUnit &superru = mol.repeating_units[i];
      int idx = repeating_units.add();
      RepeatingUnit &ru = repeating_units[idx];
      if (_mergeSGroupWithSubmolecule(ru, superru, mol, mapping, edge_mapping))
         ru.connectivity = superru.connectivity;
      else
         repeating_units.remove(idx);
      ru.subscript.copy(superru.subscript);
   }

   // multiple groups
   for (i = mol.multiple_groups.begin(); i != mol.multiple_groups.end(); i = mol.multiple_groups.next(i))
   {
      MultipleGroup &supermg = mol.multiple_groups[i];
      int idx = multiple_groups.add();
      MultipleGroup &mg = multiple_groups[idx];
      if (_mergeSGroupWithSubmolecule(mg, supermg, mol, mapping, edge_mapping))
      {
         mg.multiplier = supermg.multiplier;
         for (int j = 0; j != supermg.parent_atoms.size(); j++)
            if (mapping[supermg.parent_atoms[j]] >= 0)
               mg.parent_atoms.push(mapping[supermg.parent_atoms[j]]);
      }
      else
         multiple_groups.remove(idx);
   }

   // generic sgroups
   for (i = mol.generic_sgroups.begin(); i != mol.generic_sgroups.end(); i = mol.generic_sgroups.next(i))
   {
      SGroup &supergg = mol.generic_sgroups[i];
      int idx = generic_sgroups.add();
      SGroup &gg = generic_sgroups[idx];

      if (_mergeSGroupWithSubmolecule(gg, supergg, mol, mapping, edge_mapping))
         ;
      else
         generic_sgroups.remove(idx);
   }
}

void BaseMolecule::clearSGroups()
{
   data_sgroups.clear();
   superatoms.clear();
   repeating_units.clear();
   multiple_groups.clear();
   generic_sgroups.clear();
}

void BaseMolecule::_mergeWithSubmolecule_Sub (BaseMolecule &mol, const Array<int> &vertices,
                                              const Array<int> *edges, Array<int> &mapping,
                                              Array<int> &edge_mapping, int skip_flags)
{
   int i;
   
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
         if (mapping[i] < 0)
            continue;

         _xyz[mapping[i]] = mol.getAtomXyz(i);
      }
   }
   else
      _xyz.zerofill();


   _bond_directions.expandFill(mol.edgeEnd(), 0);

   // trick for molecules with incorrect stereochemistry, of which we do permutations
   if (vertexCount() == mol.vertexCount() && edgeCount() == mol.edgeCount())
   {
      for (int j = mol.edgeBegin(); j != mol.edgeEnd(); j = mol.edgeNext(j))
      {
         const Edge &edge = mol.getEdge(j);

         if (mol.getBondDirection(j) != 0)
            _bond_directions[findEdgeIndex(mapping[edge.beg], mapping[edge.end])] =
                    mol.getBondDirection(j);
      }
   }

   // RGroups
   if (!(skip_flags & SKIP_RGROUPS))
   {
      rgroups.copyRGroupsFromMolecule(mol.rgroups);

      for (i = 0; i < vertices.size(); i++)
      {
         if (!mol.isRSite(vertices[i]))
            continue;

         int atom_idx = mapping[vertices[i]];

         if (atom_idx == -1)
            continue;
         if (mol._rsite_attachment_points.size() <= vertices[i])
            continue;
         Array<int> &ap = mol._rsite_attachment_points[vertices[i]];
         int j;

         for (j = 0; j < ap.size(); j++)
            if (mapping[ap[j]] >= 0)
               setRSiteAttachmentOrder(atom_idx, mapping[ap[j]], j);
      }
   }

   if (!(skip_flags & SKIP_ATTACHMENT_POINTS))
   {
      if (mol.attachmentPointCount() > 0)
      {
         for (i = 1; i <= mol.attachmentPointCount(); i++)
         {
            int att_idx;
            int j;

            for (j = 0; (att_idx = mol.getAttachmentPoint(i, j)) != -1; j++)
               if (mapping[att_idx] != -1)
                  this->addAttachmentPoint(i, mapping[att_idx]);
         }
      }
   }

   // SGroups merging
   mergeSGroupsWithSubmolecule(mol, mapping, edge_mapping);

   // highlighting
   highlightSubmolecule(mol, mapping.ptr(), false);

   // subclass stuff (Molecule or QueryMolecule)
   _mergeWithSubmolecule(mol, vertices, edges, mapping, skip_flags);

   // stereo
   if (!(skip_flags & SKIP_STEREOCENTERS))
      stereocenters.buildOnSubmolecule(mol.stereocenters, mapping.ptr());
   else
      stereocenters.clear();

   if (!(skip_flags & SKIP_CIS_TRANS))
      cis_trans.buildOnSubmolecule(mol, mapping.ptr());
   else
      cis_trans.clear();

   allene_stereo.buildOnSubmolecule(mol.allene_stereo, mapping.ptr());

   // subclass stuff (Molecule or QueryMolecule)
   _postMergeWithSubmolecule(mol, vertices, edges, mapping, skip_flags);

   updateEditRevision();
}

void BaseMolecule::_flipSGroupBond(SGroup &sgroup, int src_bond_idx, int new_bond_idx)
{
   int idx = sgroup.bonds.find(src_bond_idx);
   if (idx != -1)
      sgroup.bonds[idx] = new_bond_idx;
}

void BaseMolecule::mergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
                                         const Array<int> *edges, Array<int> *mapping_out,
                                         int skip_flags)
{
   QS_DEF(Array<int>, tmp_mapping);
   QS_DEF(Array<int>, edge_mapping);

   if (mapping_out == 0)
      mapping_out = &tmp_mapping;

   // vertices and edges
   _mergeWithSubgraph(mol, vertices, edges, mapping_out, &edge_mapping);
   
   // all the chemical stuff
   _mergeWithSubmolecule_Sub(mol, vertices, edges, *mapping_out, edge_mapping, skip_flags);
}

int BaseMolecule::mergeAtoms (int atom1, int atom2)
{
   updateEditRevision();

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
   cis_trans.flipBond(atom_parent, atom_from, atom_to);

   // subclass (Molecule or QueryMolecule) adds the new bond
   _flipBond(atom_parent, atom_from, atom_to);

   int src_bond_idx = findEdgeIndex(atom_parent, atom_from);
   removeEdge(src_bond_idx);

   int new_bond_idx = findEdgeIndex(atom_parent, atom_to);

   // Clear bond direction because sterecenters 
   // should mark bond directions properly
   setBondDirection(new_bond_idx, 0);
   
   // sgroups
   int j;

   for (j = data_sgroups.begin(); j != data_sgroups.end(); j = data_sgroups.next(j))
      _flipSGroupBond(data_sgroups[j], src_bond_idx, new_bond_idx);

   for (j = superatoms.begin(); j != superatoms.end(); j = superatoms.next(j))
      _flipSGroupBond(superatoms[j], src_bond_idx, new_bond_idx);

   for (j = repeating_units.begin(); j != repeating_units.end(); j = repeating_units.next(j))
      _flipSGroupBond(repeating_units[j], src_bond_idx, new_bond_idx);

   for (j = multiple_groups.begin(); j != multiple_groups.end(); j = multiple_groups.next(j))
      _flipSGroupBond(multiple_groups[j], src_bond_idx, new_bond_idx);

   updateEditRevision();
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

void BaseMolecule::clone_KeepIndices (BaseMolecule &other, int skip_flags)
{
   QS_DEF(Array<int>, mapping);
   QS_DEF(Array<int>, edge_mapping);
   QS_DEF(Array<int>, vertices);
   int i;

   mapping.clear_resize(other.vertexEnd());
   mapping.fffill();

   vertices.clear();

   for (i = other.vertexBegin(); i < other.vertexEnd(); i = other.vertexNext(i))
   {
      vertices.push(i);
      mapping[i] = i;
   }

   edge_mapping.clear_resize(other.edgeEnd());
   edge_mapping.fffill();

   for (i = other.edgeBegin(); i < other.edgeEnd(); i = other.edgeNext(i))
      edge_mapping[i] = i;

   _cloneGraph_KeepIndices(other);

   _mergeWithSubmolecule_Sub(other, vertices, 0, mapping, edge_mapping, skip_flags);

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

   // sgroups
   for (j = data_sgroups.begin(); j != data_sgroups.end(); j = data_sgroups.next(j))
   {
      _removeAtomsFromSGroup(data_sgroups[j], mapping);
      if (data_sgroups[j].atoms.size() < 1)
         data_sgroups.remove(j);
   }
   for (j = superatoms.begin(); j != superatoms.end(); j = superatoms.next(j))
   {
      _removeAtomsFromSGroup(superatoms[j], mapping);
      if (superatoms[j].atoms.size() < 1)
         superatoms.remove(j);
   }
   for (j = repeating_units.begin(); j != repeating_units.end(); j = repeating_units.next(j))
   {
      _removeAtomsFromSGroup(repeating_units[j], mapping);
      if (repeating_units[j].atoms.size() < 1)
         repeating_units.remove(j);
   }
   for (j = multiple_groups.begin(); j != multiple_groups.end(); j = multiple_groups.next(j))
   {
      _removeAtomsFromSGroup(multiple_groups[j], mapping);
      _removeAtomsFromMultipleGroup(multiple_groups[j], mapping);
      if (multiple_groups[j].atoms.size() < 1)
         multiple_groups.remove(j);
   }

   // stereo
   stereocenters.removeAtoms(indices);
   cis_trans.buildOnSubmolecule(*this, mapping.ptr());
   allene_stereo.removeAtoms(indices);

   // highlighting
   for (i = 0; i < indices.size(); i++)
   {
      const Vertex &vertex = getVertex(indices[i]);
      unhighlightAtom(indices[i]);
      for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
         unhighlightBond(vertex.neiEdge(j));
   }

   // subclass (Molecule or QueryMolecule) removes its data
   _removeAtoms(indices, mapping.ptr());

   // Remove vertices from graph
   for (i = 0; i < indices.size(); i++)
      removeVertex(indices[i]);

   updateEditRevision();
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
   allene_stereo.removeBonds(indices);

   for (int i = 0; i < indices.size(); i++)
   {
      unhighlightBond(indices[i]);
      removeEdge(indices[i]);
   }
   updateEditRevision();
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
   updateEditRevision();
}

void BaseMolecule::setAtomXyz (int idx, const Vec3f& v)
{
   _xyz[idx].copy(v);
   updateEditRevision();
}

void BaseMolecule::clearXyz ()
{
   for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
      setAtomXyz(i, 0, 0, 0);
   have_xyz = 0;
}

int BaseMolecule::_addBaseAtom ()
{
   int idx = addVertex();

   _xyz.expand(idx + 1);
   _xyz[idx].zero();

   updateEditRevision();

   return idx;
}

int BaseMolecule::_addBaseBond (int beg, int end)
{
   int idx = addEdge(beg, end);

   cis_trans.registerBond(idx);
   updateEditRevision();
   return idx;
}

int BaseMolecule::getAtomRadical_NoThrow (int idx, int fallback)
{
   try
   {
      return getAtomRadical(idx);
   }
   catch (Element::Error &)
   {
      return fallback;
   }
}

int BaseMolecule::getAtomValence_NoThrow (int idx, int fallback)
{
   try
   {
      return getAtomValence(idx);
   }
   catch (Element::Error &)
   {
      return fallback;
   }
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

   dword bits = getRSiteBits(atom_idx);
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
   dword bits = getRSiteBits(atom_idx);
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
   updateEditRevision();
}

int BaseMolecule::attachmentPointCount () const
{
   return _attachment_index.size();
}

void BaseMolecule::addAttachmentPoint (int order, int atom_index)
{
   if (order < 1)
      throw Error("attachment point order %d no allowed (should start from 1)", order);

   if (_attachment_index.size() < order)
      _attachment_index.resize(order);

   _attachment_index[order - 1].push(atom_index);
   updateEditRevision();
}

void BaseMolecule::removeAttachmentPoints ()
{
   _attachment_index.clear();
   updateEditRevision();
}

void BaseMolecule::removeAttachmentPointsFromAtom (int atom_index)
{
   int i, j;

   for (i = 0; i < _attachment_index.size(); i++)
      if ((j = _attachment_index[i].find(atom_index)) != -1)
      {
         if (j == _attachment_index[i].size() - 1)
            _attachment_index[i].pop();
         else
            _attachment_index[i][j] = _attachment_index[i].pop();
      }
   updateEditRevision();
}

int BaseMolecule::getAttachmentPoint (int order, int index) const
{
   if (order < 1)
      throw Error("attachment point order %d no allowed (should start from 1)", order);

   return index < _attachment_index[order - 1].size() ? _attachment_index[order - 1][index] : -1;
}

BaseMolecule::SGroup::~SGroup ()
{
}

BaseMolecule::DataSGroup::DataSGroup ()
{
   detached = false;
   relative = false;
   display_units = false;
   dasp_pos = 1;
}

BaseMolecule::DataSGroup::~DataSGroup ()
{
}

BaseMolecule::Superatom::Superatom ()
{
   bond_idx = -1;
}

BaseMolecule::Superatom::~Superatom ()
{
}

BaseMolecule::RepeatingUnit::RepeatingUnit ()
{
   connectivity = 0;
}

BaseMolecule::RepeatingUnit::~RepeatingUnit ()
{
}

BaseMolecule::MultipleGroup::MultipleGroup ()
{
   multiplier = 1;
}

BaseMolecule::MultipleGroup::~MultipleGroup ()
{
}

int copyBaseBond (BaseMolecule& bm, int beg, int end, int srcId) {
   int bid = -1;
   if (bm.isQueryMolecule()) {
      QueryMolecule& qm = bm.asQueryMolecule();
      bid = qm.addBond(beg, end, qm.getBond(srcId).clone());
   } else {
      Molecule& mol = bm.asMolecule();
      bid = mol.addBond(beg, end, mol.getBondOrder(srcId));
      mol.setEdgeTopology(bid, mol.getBondTopology(srcId));
   }
   return bid;
}

void BaseMolecule::MultipleGroup::collapse (BaseMolecule& bm) {
   for (int i = bm.multiple_groups.begin(); i < bm.multiple_groups.end(); i = bm.multiple_groups.next(i)) {
      collapse(bm, i);
   }
}

void BaseMolecule::MultipleGroup::collapse (BaseMolecule& bm, int id) {
   QS_DEF(Mapping, mapAtom);
   mapAtom.clear();
   QS_DEF(Mapping, mapBondInv);
   mapBondInv.clear();
   collapse(bm, id, mapAtom, mapBondInv);
}

void BaseMolecule::MultipleGroup::collapse (BaseMolecule& bm, int id, Mapping& mapAtom, Mapping& mapBondInv) {
   const BaseMolecule::MultipleGroup& group = bm.multiple_groups[id];

   if (group.atoms.size() != group.multiplier * group.parent_atoms.size())
      throw Error("The group is already collapsed or invalid");

   QS_DEF(Array<int>, toRemove);
   toRemove.clear();
   for (int j = 0; j < group.atoms.size(); ++j) {
      int k = j % group.parent_atoms.size();
      int *value = mapAtom.at2(group.atoms[j]);
      if (value == 0)
         mapAtom.insert(group.atoms[j], group.atoms[k]);
      else if (*value != group.atoms[k])
         throw Error("Invalid mapping in MultipleGroup::collapse");

      if (k != j)
         toRemove.push(group.atoms[j]);
   }
   for (int j = bm.edgeBegin(); j < bm.edgeEnd(); j = bm.edgeNext(j)) {
      const Edge& edge = bm.getEdge(j);
      bool in1 = mapAtom.find(edge.beg),
         in2 = mapAtom.find(edge.end),
         p1 = in1 && mapAtom.at(edge.beg) == edge.beg,
         p2 = in2 && mapAtom.at(edge.end) == edge.end;
      if ((in1 && !p1 && !in2) || (!in1 && !p2 && in2)) {
         int beg = in1 ? mapAtom.at(edge.beg) : edge.beg;
         int end = in2 ? mapAtom.at(edge.end) : edge.end;
         int bid = copyBaseBond(bm, beg, end, j);
         if (!mapBondInv.find(bid))
            mapBondInv.insert(bid, j);
      }
   }

   for (int j = 0; j < toRemove.size(); ++j) {
      int aid = toRemove[j];
      bm.removeAtom(aid);
   }
}

void BaseMolecule::_removeAtomsFromSGroup (SGroup &sgroup, Array<int> &mapping)
{
   int i;

   for (i = sgroup.atoms.size() - 1; i >= 0; i--)
      if (mapping[sgroup.atoms[i]] == -1)
         sgroup.atoms.remove(i);

   for (i = sgroup.bonds.size() - 1; i >= 0; i--)
   {
      const Edge &edge = getEdge(sgroup.bonds[i]);
      if (mapping[edge.beg] == -1 || mapping[edge.end] == -1)
         sgroup.bonds.remove(i);
   }
   updateEditRevision();
}

void BaseMolecule::_removeAtomsFromMultipleGroup (MultipleGroup &mg, Array<int> &mapping)
{
   int i;

   for (i = mg.parent_atoms.size() - 1; i >= 0; i--)
      if (mapping[mg.parent_atoms[i]] == -1)
         mg.parent_atoms.remove(i);
   updateEditRevision();
}

bool BaseMolecule::_mergeSGroupWithSubmolecule (SGroup &sgroup, SGroup &super, BaseMolecule &supermol,
        Array<int> &mapping, Array<int> &edge_mapping)
{
   int i;
   bool merged = false;

   sgroup.brackets.copy(super.brackets);

   for (i = 0; i < super.atoms.size(); i++)
   {
      if (mapping[super.atoms[i]] >= 0)
      {
         sgroup.atoms.push(mapping[super.atoms[i]]);
         merged = true;
      }
   }
   for (i = 0; i < super.bonds.size(); i++)
   {
      const Edge &edge = supermol.getEdge(super.bonds[i]);

      if (edge_mapping[super.bonds[i]] < 0)
         continue;

      if (mapping[edge.beg] < 0 || mapping[edge.end] < 0)
         throw Error("internal: edge is not mapped");

      sgroup.bonds.push(edge_mapping[super.bonds[i]]);
      merged = true;
   }
   
   if (merged)
      updateEditRevision();
   return merged;
}

void BaseMolecule::unhighlightAll ()
{
   _hl_atoms.clear();
   _hl_bonds.clear();
   updateEditRevision();
}

void BaseMolecule::highlightAtom (int idx)
{
   _hl_atoms.expandFill(idx + 1, 0);
   _hl_atoms[idx] = 1;
   updateEditRevision();
}

void BaseMolecule::highlightBond (int idx)
{
   _hl_bonds.expandFill(idx + 1, 0);
   _hl_bonds[idx] = 1;
   updateEditRevision();
}

void BaseMolecule::highlightAtoms (const Filter &filter)
{
   int i;

   for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
      if (filter.valid(i))
         highlightAtom(i);
   updateEditRevision();
}

void BaseMolecule::highlightBonds (const Filter &filter)
{
   int i;

   for (i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
      if (filter.valid(i))
         highlightBond(i);
   updateEditRevision();
}

void BaseMolecule::unhighlightAtom (int idx)
{
   if (_hl_atoms.size() > idx)
   {
      _hl_atoms[idx] = 0;
      updateEditRevision();
   }
}

void BaseMolecule::unhighlightBond (int idx)
{
   if (_hl_bonds.size() > idx)
   {
      _hl_bonds[idx] = 0;
      updateEditRevision();
   }
}

int BaseMolecule::countHighlightedAtoms ()
{
   int i, res = 0;

   for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
   {
      if (i >= _hl_atoms.size())
         break;
      res += _hl_atoms[i];
   }

   return res;
}

int BaseMolecule::countHighlightedBonds ()
{
   int i, res = 0;

   for (i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
   {
      if (i >= _hl_bonds.size())
         break;
      res += _hl_bonds[i];
   }

   return res;
}

bool BaseMolecule::hasHighlighting ()
{
   return countHighlightedAtoms() > 0 || countHighlightedBonds() > 0;
}

bool BaseMolecule::isAtomHighlighted (int idx)
{
   return _hl_atoms.size() > idx && _hl_atoms[idx] == 1;
}

bool BaseMolecule::isBondHighlighted (int idx)
{
   return _hl_bonds.size() > idx && _hl_bonds[idx] == 1;
}

void BaseMolecule::highlightSubmolecule (BaseMolecule &subgraph, const int *mapping, bool entire)
{
   int i;

   for (i = subgraph.vertexBegin(); i != subgraph.vertexEnd(); i = subgraph.vertexNext(i))
      if (mapping[i] >= 0 && (entire || subgraph.isAtomHighlighted(i)))
         highlightAtom(mapping[i]);

   for (i = subgraph.edgeBegin(); i != subgraph.edgeEnd(); i = subgraph.edgeNext(i))
   {
      if (!entire && !subgraph.isBondHighlighted(i))
         continue;

      const Edge &edge = subgraph.getEdge(i);

      int beg = mapping[edge.beg];
      int end = mapping[edge.end];

      if (beg >= 0 && end >= 0)
      {
         int edge_idx = findEdgeIndex(beg, end);
         if (edge_idx >= 0)
            highlightBond(edge_idx);
      }
   }
}

int BaseMolecule::countSGroups ()
{
   return generic_sgroups.size() + data_sgroups.size() + multiple_groups.size() +
          repeating_units.size() + superatoms.size();
}

void BaseMolecule::getAttachmentIndicesForAtom (int atom_idx, Array<int> &res)
{
   res.clear();

   for (int i = 1; i <= attachmentPointCount(); i++)
   {
      int idx = 0, aidx;

      for (idx = 0; (aidx = getAttachmentPoint(i, idx)) != -1; idx++)
      {
         if (aidx == atom_idx)
            res.push(i);
      }
   }
}

int BaseMolecule::getEditRevision ()
{
   return _edit_revision;
}

void BaseMolecule::updateEditRevision ()
{
   _edit_revision++;
}

int BaseMolecule::getBondDirection (int idx) const
{
   if (idx > _bond_directions.size() - 1)
      return 0;

   return _bond_directions[idx];
}

int BaseMolecule::getBondDirection2 (int center_idx, int nei_idx)
{
   int idx = findEdgeIndex(center_idx, nei_idx);

   if (idx == -1)
      throw Error("getBondDirection2(): can not find bond");

   if (center_idx != getEdge(idx).beg)
      return 0;

   return getBondDirection(idx);
}

void BaseMolecule::setBondDirection (int idx, int dir)
{
   _bond_directions.expandFill(idx + 1, 0);
   _bond_directions[idx] = dir;
}

void BaseMolecule::clearBondDirections ()
{
   _bond_directions.clear();
}

bool BaseMolecule::isChrial ()
{
   return stereocenters.size() != 0 && stereocenters.haveAllAbsAny();
}

void BaseMolecule::invalidateAtom (int index, int mask)
{
   if (mask & CHANGED_ATOM_NUMBER)
   {
      // Cis-trans and stereocenters can be removed
      if (stereocenters.exists(index))
      {
         if (!stereocenters.isPossibleStereocenter(index))
            stereocenters.remove(index);
      }

      const Vertex &v = getVertex(index);
      for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
      {
         int edge_idx = v.neiEdge(nei);
         if (cis_trans.getParity(edge_idx) != 0)
         {
            if (!cis_trans.isGeomStereoBond(*this, edge_idx, 0, false))
               cis_trans.setParity(edge_idx, 0);
         }
      }
   }
}

void BaseMolecule::getSGroupAtomsCenterPoint (SGroup &sgroup, Vec2f &res)
{
   res.set(0, 0);
   for (int j = 0; j < sgroup.atoms.size(); j++)
   {
      int ai = sgroup.atoms[j];
      Vec3f &p = getAtomXyz(ai);
      res.x += p.x;
      res.y += p.y;
   }
   if (sgroup.atoms.size() != 0)
      res.scale(1.0f / sgroup.atoms.size());
}

void BaseMolecule::getAtomSymbol (int v, Array<char> &result)
{
   if (isPseudoAtom(v))
   {
      result.readString(getPseudoAtom(v), true);
   }
   else if (isRSite(v))
   {
      QS_DEF(Array<int>, rgroups);
      int i;
      getAllowedRGroups(v, rgroups);

      if (rgroups.size() == 0)
      {
         result.readString("R", true);
         return;
      }

      ArrayOutput output(result);
      for (i = 0; i < rgroups.size(); i++)
      {
         if (i > 0)
            output.writeChar(',');
         output.printf("R%d", rgroups[i]);
      }
      output.writeChar(0);
   }
   else 
   {
      int number = getAtomNumber(v);
      QS_DEF(Array<int>, list);

      if (number != -1)
      {
         result.readString(Element::toString(number), true);
         return;
      }

      int query_atom_type;

      if (isQueryMolecule() &&
            (query_atom_type = QueryMolecule::parseQueryAtom(asQueryMolecule(), v, list)) != -1)
      {
         if (query_atom_type == QueryMolecule::QUERY_ATOM_A)
         {
            result.readString("A", true);
            return;
         }
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_Q)
         {
            result.readString("Q", true);
            return;
         }
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_X)
         {
            result.readString("X", true);
            return;
         }
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST ||
                  query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
         {
            int k;
            ArrayOutput output(result);

            if (query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
               output.writeString("NOT");

            output.writeChar('[');
            for (k = 0; k < list.size(); k++)
            {
               if (k > 0)
                  output.writeChar(',');
               output.writeString(Element::toString(list[k]));
            }
            output.writeChar(']');
            output.writeChar(0);
         }
      }
   }
   if (result.size() == 0)
      result.readString("*", true);
}
