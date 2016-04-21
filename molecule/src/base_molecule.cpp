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

#include "molecule/base_molecule.h"

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/query_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_exact_substructure_matcher.h"

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
   sgroups.clear();
   Graph::clear();
   _hl_atoms.clear();
   _hl_bonds.clear();
   _bond_directions.clear();
   custom_collections.clear();

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

   for (i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
   {
      SGroup &supersg = mol.sgroups.getSGroup(i);
      int idx = sgroups.addSGroup(supersg.sgroup_type);
      SGroup &sg = sgroups.getSGroup(idx);

      if (_mergeSGroupWithSubmolecule(sg, supersg, mol, mapping, edge_mapping)) {
         if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
         {
            DataSGroup &dg = (DataSGroup &)sg;
            DataSGroup &superdg = (DataSGroup &)supersg;

            dg.detached = superdg.detached;
            dg.display_pos = superdg.display_pos;
            dg.data.copy(superdg.data);
            dg.dasp_pos = superdg.dasp_pos;
            dg.relative = superdg.relative;
            dg.display_units = superdg.display_units;
            dg.description.copy(superdg.description);
            dg.name.copy(superdg.name);
            dg.type.copy(superdg.type);
            dg.querycode.copy(superdg.querycode);
            dg.queryoper.copy(superdg.queryoper);
            dg.num_chars = superdg.num_chars;
            dg.tag = superdg.tag;
         }
         else if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
         {
            Superatom &sa = (Superatom &)sg;
            Superatom &supersa = (Superatom &)supersg;

            if (supersa.bond_connections.size() > 0)
            {
               for (int j = 0; j < supersa.bond_connections.size(); j++)
               {
                Superatom::_BondConnection &bond = sa.bond_connections.push();
                bond.bond_dir = supersa.bond_connections[j].bond_dir;
                bond.bond_idx = edge_mapping[supersa.bond_connections[j].bond_idx];
               }
            }
            sa.subscript.copy(supersa.subscript);
            sa.sa_class.copy(supersa.sa_class);
            sa.contracted = supersa.contracted;         
            if (supersa.attachment_points.size() > 0)
            {
               for (int j = supersa.attachment_points.begin(); j < supersa.attachment_points.end(); j = supersa.attachment_points.next(j))
               {
                int ap_idx =  sa.attachment_points.add();
                Superatom::_AttachmentPoint &ap = sa.attachment_points.at(ap_idx);
                ap.aidx = mapping[supersa.attachment_points[j].aidx];
                int leave_idx = supersa.attachment_points[j].lvidx;
                if (leave_idx > -1)
                   ap.lvidx = mapping[supersa.attachment_points[j].lvidx];
                else
                   ap.lvidx = leave_idx;
   
                ap.apid.copy(supersa.attachment_points[j].apid);
               }
            }
         }
         else if (sg.sgroup_type == SGroup::SG_TYPE_SRU)
         {
            RepeatingUnit &ru = (RepeatingUnit &)sg;
            RepeatingUnit &superru = (RepeatingUnit &)supersg;

            ru.connectivity = superru.connectivity;
            ru.subscript.copy(superru.subscript);
         }
         else if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
         {
            MultipleGroup &mg = (MultipleGroup &)sg;
            MultipleGroup &supermg = (MultipleGroup &)supersg;

            mg.multiplier = supermg.multiplier;
            for (int j = 0; j != supermg.parent_atoms.size(); j++)
               if (mapping[supermg.parent_atoms[j]] >= 0)
                  mg.parent_atoms.push(mapping[supermg.parent_atoms[j]]);
         }
      } else {
         sgroups.remove(idx);
      }
   }
}

void BaseMolecule::clearSGroups()
{
   sgroups.clear();
}

void BaseMolecule::_mergeWithSubmolecule_Sub (BaseMolecule &mol, const Array<int> &vertices,
                                              const Array<int> *edges, Array<int> &mapping,
                                              Array<int> &edge_mapping, int skip_flags)
{
   QS_DEF(Array<char>, apid);
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
            if (ap[j] >= 0 && ap[j] < mapping.size() && mapping[ap[j]] >= 0)
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

   if (!(skip_flags & SKIP_TGROUPS))
   {
      tgroups.copyTGroupsFromMolecule(mol.tgroups);
   }

   if (!(skip_flags & SKIP_TEMPLATE_ATTACHMENT_POINTS))
   {
      for (i = 0; i < vertices.size(); i++)
      {
         if (mol.isTemplateAtom(vertices[i]))
         {
            for (int j = 0; j < mol.getTemplateAtomAttachmentPointsCount(vertices[i]); j++)
            {
               if ( (mol.getTemplateAtomAttachmentPoint(vertices[i], j) != -1) &&
                    (mapping[mol.getTemplateAtomAttachmentPoint(vertices[i], j)] != -1) )
               {  
                  mol.getTemplateAtomAttachmentPointId(vertices[i], j, apid);
                  setTemplateAtomAttachmentOrder(mapping[vertices[i]],
                                                 mapping[mol.getTemplateAtomAttachmentPoint(vertices[i], j)],
                                                 apid.ptr());
               }
            }
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

void BaseMolecule::_flipSuperatomBond(Superatom &sa, int src_bond_idx, int new_bond_idx)
{
   if (sa.bond_connections.size() > 0)
   {
      for (int j = 0; j < sa.bond_connections.size(); j++)
      {
         Superatom::_BondConnection &bond = sa.bond_connections[j];
         if (bond.bond_idx == src_bond_idx)
            bond.bond_idx = new_bond_idx;
      }
   }
}

void BaseMolecule::_flipTemplateAtomAttachmentPoint(int idx, int atom_from, int atom_to)
{
   for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
   {
      BaseMolecule::TemplateAttPoint &ap = template_attachment_points.at(j);
      if ( (ap.ap_occur_idx == idx) && (ap.ap_aidx == atom_from) )
      {
         ap.ap_aidx = atom_to;
      }
   }
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

   for (j = sgroups.begin(); j != sgroups.end(); j = sgroups.next(j))
   {
      SGroup &sg = sgroups.getSGroup(j);
      _flipSGroupBond(sg, src_bond_idx, new_bond_idx);
      if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
         _flipSuperatomBond((Superatom &)sg, src_bond_idx, new_bond_idx);
   }

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
   for (j = sgroups.begin(); j != sgroups.end(); j = sgroups.next(j))
   {
      SGroup &sg = sgroups.getSGroup(j);
      _removeAtomsFromSGroup(sg, mapping);
      if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
         _removeAtomsFromSuperatom((Superatom &)sg, mapping);
      else if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
         _removeAtomsFromMultipleGroup((MultipleGroup &)sg, mapping);
      if (sg.atoms.size() < 1)
         removeSGroup(j);
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
   QS_DEF(Array<int>, mapping);
   int i, j;

   mapping.clear_resize(edgeEnd());

   for (i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
      mapping[i] = i;

   // Mark removed vertices
   for (i = 0; i < indices.size(); i++)
      mapping[indices[i]] = -1;

   // sgroups
   for (j = sgroups.begin(); j != sgroups.end(); j = sgroups.next(j))
   {
      SGroup &sg = sgroups.getSGroup(j);
      _removeBondsFromSGroup(sg, mapping);
      if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
         _removeBondsFromSuperatom((Superatom &)sg, mapping);
   }

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

void BaseMolecule::removeSGroup (int idx)
{
   SGroup &sg = sgroups.getSGroup(idx);
   _checkSgroupHierarchy(sg.parent_group, sg.original_group);
   sgroups.remove(idx);
}

void BaseMolecule::removeSGroupWithBasis (int idx)
{
   QS_DEF(Array<int>, sg_atoms);
   SGroup &sg = sgroups.getSGroup(idx);
   _checkSgroupHierarchy(sg.parent_group, sg.original_group);
   sg_atoms.copy(sg.atoms);
   removeAtoms(sg_atoms);
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

void BaseMolecule::setTemplateAtomAttachmentOrder (int atom_idx, int att_atom_idx, const char *att_id)
{
   int att_idx = template_attachment_points.add();
   TemplateAttPoint &ap = template_attachment_points.at(att_idx);
   ap.ap_occur_idx = atom_idx;
   ap.ap_aidx = att_atom_idx;
   ap.ap_id.readString(att_id, false);
   ap.ap_id.push(0);
   updateEditRevision();
}

int BaseMolecule::getTemplateAtomAttachmentPoint (int atom_idx, int order)
{
   int ap_count = 0;
   for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
   {
      BaseMolecule::TemplateAttPoint &ap = template_attachment_points.at(j);
      if (ap.ap_occur_idx == atom_idx)
      {
         if (ap_count == order)
            return ap.ap_aidx;

         ap_count++;    
      }
   }
   return -1;
}

void BaseMolecule::getTemplateAtomAttachmentPointId (int atom_idx, int order, Array<char> &apid)
{
   int ap_count = 0;
   for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
   {
      BaseMolecule::TemplateAttPoint &ap = template_attachment_points.at(j);
      if (ap.ap_occur_idx == atom_idx)
      {
         if (ap_count == order)
         {
            apid.copy(ap.ap_id);
            return;
         }
         ap_count++;    
      }
   }
   throw Error("attachment point order %d is out of range (%d)", order, ap_count);
}

int BaseMolecule::getTemplateAtomAttachmentPointById (int atom_idx, Array<char> &att_id)
{
   QS_DEF(Array<char>, tmp);
   int aidx = -1;
   for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
   {
      BaseMolecule::TemplateAttPoint &ap = template_attachment_points.at(j);
      if ( (ap.ap_occur_idx == atom_idx) && (ap.ap_id.memcmp(att_id) == 0) )
      {
         return ap.ap_aidx;
      }
   }
   return aidx;
}

int BaseMolecule::getTemplateAtomAttachmentPointsCount (int atom_idx)
{
   int count = 0;
   for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
   {
      BaseMolecule::TemplateAttPoint &ap = template_attachment_points.at(j);
      if (ap.ap_occur_idx == atom_idx)
      {
         count++;
      }
   }
   return count;
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

void BaseMolecule::_checkSgroupHierarchy(int pidx, int oidx)
{
   for (int i = sgroups.begin(); i != sgroups.end(); i = sgroups.next(i))
   {
      SGroup &sg = sgroups.getSGroup(i);
      if (sg.parent_group == oidx)
         sg.parent_group = pidx;
   }
}

int copyBaseBond (BaseMolecule& bm, int beg, int end, int srcId)
{
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

void BaseMolecule::collapse (BaseMolecule& bm)
{
   for (int i = bm.sgroups.begin(); i != bm.sgroups.end(); i = bm.sgroups.next(i))
   {
      SGroup &sg = bm.sgroups.getSGroup(i);
      if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
         collapse(bm, i);
   }
}

void BaseMolecule::collapse (BaseMolecule& bm, int id) {
   QS_DEF(Mapping, mapAtom);
   mapAtom.clear();
   QS_DEF(Mapping, mapBondInv);
   mapBondInv.clear();
   collapse(bm, id, mapAtom, mapBondInv);
}

void BaseMolecule::collapse (BaseMolecule& bm, int id, Mapping& mapAtom, Mapping& mapBondInv)
{
   SGroup &sg = bm.sgroups.getSGroup(id);
  
   if (sg.sgroup_type != SGroup::SG_TYPE_MUL)
      throw Error("The group is wrong type");

   const MultipleGroup& group = (MultipleGroup &)sg;

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

int BaseMolecule::transformSCSRtoFullCTAB ()
{
   int result = 0;
   QS_DEF(Array<int>, tinds);
   tinds.clear();

   for (auto i : vertices())
   {
      if (isTemplateAtom(i))
         tinds.push(i);
   }

   for (auto i = 0; i < tinds.size(); i++)
   {
      _transformTGroupToSGroup(tinds[i]);
   }

   if (tinds.size() > 0)
   {
      removeAtoms(tinds);
      tgroups.clear();
      template_attachment_points.clear();
   }

   return result;
}

int BaseMolecule::transformFullCTABtoSCSR (ObjArray<TGroup> &templates)
{
   int result = 0;
   QS_DEF(Molecule, fragment);
   QS_DEF(Array<int>, added_templates);
   QS_DEF(Array<int>, mapping);
   QS_DEF(Array<int>, remove_atoms);
   QS_DEF(Array<int>, base_sgs);
   QS_DEF(Array<int>, sgs);
   QS_DEF(Array<int>, ap_points_atoms);
   QS_DEF(StringPool, ap_points_ids);
   QS_DEF(Array<int>, ap_ids);
   QS_DEF(Array<int>, ignore_atoms);

   added_templates.clear();
   ignore_atoms.clear();

   int seq_id = 1;

   templates.qsort(TGroup::cmp, 0);

   for (auto i = 0; i < templates.size(); i++)
   {
      const TGroup &tg = templates.at(i);
      fragment.clear();
      fragment.clone_KeepIndices(*tg.fragment);

      sgs.clear();
      base_sgs.clear();
      fragment.sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", sgs);
      for (int j = fragment.sgroups.begin(); j != fragment.sgroups.end(); j = fragment.sgroups.next(j))
      {
         if (sgs.find(j) == -1)
            base_sgs.push(j);
      }

      ap_points_atoms.clear();
      ap_points_ids.clear();
      ap_ids.clear();
      for (int j = 0; j < base_sgs.size(); j++)
      {
         SGroup &sg = fragment.sgroups.getSGroup(base_sgs[j]);
         if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
         {
            Superatom &su = (Superatom &)sg;
         
            if (su.attachment_points.size() > 0)
            {
               for (int k = su.attachment_points.begin(); k < su.attachment_points.end(); k = su.attachment_points.next(k))
               {
                  Superatom::_AttachmentPoint &ap = su.attachment_points.at(k);
                  ap_points_atoms.push(ap.aidx);
                  ap_ids.push(ap_points_ids.add(ap.apid));
               }
            }
         }
         else
            throw Error("Wrong template structure was found (base SGroup is not Superatom type)");
      }

      for (int j = 0; j < sgs.size(); j++)
      {
         fragment.removeSGroupWithBasis(sgs[j]);
      }

      int count_occur = 0;
      ignore_atoms.clear();
      for (;;)
      {
         MoleculeExactSubstructureMatcher matcher(fragment, this->asMolecule());

         for (int j = 0; j < ignore_atoms.size(); j++)
            matcher.ignoreTargetAtom(ignore_atoms[j]);

         if (!matcher.find())
            break;

         mapping.clear();
         remove_atoms.clear();
         mapping.copy(matcher.getQueryMapping(), fragment.vertexEnd());
         for (int j = 0; j < mapping.size(); j++)
         {
            if (mapping[j] > -1)
               remove_atoms.push(mapping[j]);
              
         }

         int out_bonds = 0;
         for (int j = 0; j < remove_atoms.size(); j++)
         {
            const Vertex &v = getVertex(remove_atoms[j]);
            for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
            {
               if (remove_atoms.find(v.neiVertex(k)) == -1)
               {
                  out_bonds++;
               }
            }
         }

         if (out_bonds > ap_points_atoms.size())
         {
            ignore_atoms.concat(remove_atoms);
            continue;
         }

         int idx = this->asMolecule().addAtom(-1);
         this->asMolecule().setTemplateAtom(idx, tg.tgroup_name.ptr());
         this->asMolecule().setTemplateAtomClass(idx, tg.tgroup_class.ptr());
//         this->asMolecule().setTemplateAtomSeqid(idx, seq_id);
         seq_id++;
         count_occur++;

         for (int j = 0; j < ap_points_atoms.size(); j++)
         {
            int att_point_idx = mapping[ap_points_atoms[j]];
            if (remove_atoms.find(att_point_idx) != -1)
            {
               const Vertex &v = getVertex(att_point_idx);
               QS_DEF(Array<int>, neighbors);
               neighbors.clear();
               for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
               {
                  if (remove_atoms.find(v.neiVertex(k)) == -1)
                  {
                     neighbors.push(v.neiVertex(k));
                  }
               }
               for (int k = 0; k < neighbors.size(); k++)
               {
                  if (findEdgeIndex(neighbors[k], att_point_idx) != -1)
                  {
                     flipBond(neighbors[k], att_point_idx, idx);
                     this->asMolecule().setTemplateAtomAttachmentOrder(idx, neighbors[k], ap_points_ids.at(ap_ids[j]));
                     if (isTemplateAtom(neighbors[k]))
                        _flipTemplateAtomAttachmentPoint(neighbors[k], att_point_idx, idx);
                  }
               }
            }
         }

         QS_DEF(Vec2f, cp);
         QS_DEF(Vec3f, p);
         p.set(0, 0, 0);
         getAtomsCenterPoint(remove_atoms, cp);
         p.x = cp.x;
         p.y = cp.y;
         setAtomXyz(idx, p);

         removeAtoms(remove_atoms);
      }
    
      if (count_occur > 0)
         added_templates.push(i);
   }

   for (auto i = 0; i < added_templates.size(); i++)
   {
      _addTemplate(templates.at(added_templates[i]));
   }

   return result;
}

int BaseMolecule::_addTemplate (TGroup &tgroup)
{
   int result = 0;
   int idx = tgroups.addTGroup();
   (tgroups.getTGroup(idx)).copy(tgroup);
   return result;
}

int BaseMolecule::_transformTGroupToSGroup (int idx)
{
   int result = 0;
   QS_DEF(Molecule, fragment);
   QS_DEF(Array<int>, sgs);
   QS_DEF(Array<int>, mapping);
   QS_DEF(Array<int>, att_atoms);
   QS_DEF(Array<int>, tg_atoms);
   QS_DEF(Array<int>, lvgroups);

   int tg_idx = tgroups.findTGroup(getTemplateAtom(idx));
   TGroup &tgroup = tgroups.getTGroup(tg_idx);
   fragment.clear();
   fragment.clone_KeepIndices(*tgroup.fragment);

   sgs.clear();
   att_atoms.clear();
   tg_atoms.clear();
   lvgroups.clear();
   fragment.sgroups.findSGroups(SGroup::SG_LABEL, getTemplateAtom(idx), sgs);
   if (sgs.size() > 1)
      throw Error("transformTGroupToSGroup(): wrong template structure found (more then one base SGroup detected)");
   
   SGroup &sg = fragment.sgroups.getSGroup(sgs[0]);
   if (sg.sgroup_type != SGroup::SG_TYPE_SUP)
      throw Error("transformTGroupToSGroup(): wrong template structure found (base SGroup is not Superatom type)");

   Superatom &su = (Superatom &)sg;

   if (su.attachment_points.size() > 0)
   {
      for (int j = su.attachment_points.begin(); j < su.attachment_points.end(); j = su.attachment_points.next(j))
      {
         Superatom::_AttachmentPoint &ap = su.attachment_points.at(j);

         int att_atom_idx = getTemplateAtomAttachmentPointById(idx, ap.apid);
         if (att_atom_idx > -1)
         {
            att_atoms.push(att_atom_idx);
            tg_atoms.push(ap.aidx);
            lvgroups.push(ap.lvidx);
         }
      }
   }

   sgs.clear();
   fragment.sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", sgs);
   for (int i = 0; i < sgs.size(); i++)
   {
      SGroup &lvg = fragment.sgroups.getSGroup(sgs[i]);
      for (int j = 0; j < lvgroups.size(); j++)
      {
         if (lvg.atoms.find(lvgroups[j]) > -1)
            fragment.removeSGroupWithBasis(sgs[i]);
      }
   }

   mergeWithMolecule(fragment, &mapping);

   for (auto i : fragment.vertices())
   {
      int aidx = mapping[i];
      setAtomXyz(aidx, getAtomXyz(idx));
   }

   for (int i = 0; i < att_atoms.size(); i++)
   {
      flipBond(att_atoms[i], idx, mapping[tg_atoms[i]]);
      if (isTemplateAtom(att_atoms[i]))
         _flipTemplateAtomAttachmentPoint(att_atoms[i], idx, mapping[tg_atoms[i]]);
   }

   return result;
}

void BaseMolecule::_removeAtomsFromSGroup (SGroup &sgroup, Array<int> &mapping)
{
   int i;

   for (i = sgroup.atoms.size() - 1; i >= 0; i--)
   {
      if (mapping[sgroup.atoms[i]] == -1)
         sgroup.atoms.remove(i);
   }
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
   {
      if (mapping[mg.parent_atoms[i]] == -1)
         mg.parent_atoms.remove(i);
   }
   updateEditRevision();
}

void BaseMolecule::_removeAtomsFromSuperatom (Superatom &sa, Array<int> &mapping)
{

   if (sa.bond_connections.size() > 0)
   {
      for (int j = sa.bond_connections.size() - 1; j >= 0; j--)
      {
         Superatom::_BondConnection &bond = sa.bond_connections[j];
         const Edge &edge = getEdge(bond.bond_idx);
         if (mapping[edge.beg] == -1 || mapping[edge.end] == -1)
            sa.bond_connections.remove(j);
      }
   }
   if (sa.attachment_points.size() > 0)
   {
      for (int j = sa.attachment_points.begin(); j < sa.attachment_points.end(); j = sa.attachment_points.next(j))
      {
         Superatom::_AttachmentPoint &ap = sa.attachment_points.at(j);
         if (ap.aidx >= 0 && mapping[ap.aidx] == -1) 
            sa.attachment_points.remove(j);
         else if (ap.lvidx >= 0 && mapping[ap.lvidx] == -1)
            ap.lvidx = -1;
      }
   }
   updateEditRevision();
}

void BaseMolecule::_removeBondsFromSGroup (SGroup &sgroup, Array<int> &mapping)
{
   int i;

   for (i = sgroup.bonds.size() - 1; i >= 0; i--)
   {
      if (mapping[sgroup.bonds[i]] == -1)
         sgroup.bonds.remove(i);
   }
   updateEditRevision();
}

void BaseMolecule::_removeBondsFromSuperatom (Superatom &sa, Array<int> &mapping)
{
   if (sa.bond_connections.size() > 0)
   {
      for (int j = sa.bond_connections.size() - 1; j >= 0; j--)
      {
         Superatom::_BondConnection &bond = sa.bond_connections[j];
         const Edge &edge = getEdge(bond.bond_idx);
         if (mapping[bond.bond_idx] == -1)
            sa.bond_connections.remove(j);
      }
   }
   updateEditRevision();
}

bool BaseMolecule::_mergeSGroupWithSubmolecule (SGroup &sgroup, SGroup &super, BaseMolecule &supermol,
        Array<int> &mapping, Array<int> &edge_mapping)
{
   int i;
   bool merged = false;

   sgroup.parent_group = super.parent_group;

   sgroup.sgroup_subtype = super.sgroup_subtype;

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
   return sgroups.getSGroupCount();
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
   // Molecule is Chiral if it has at least one Abs stereocenter and all the stereocenters are Abs or Any
   return stereocenters.size() != 0 && stereocenters.haveAllAbsAny() && stereocenters.haveAbs();
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
   getAtomsCenterPoint(sgroup.atoms, res);
}

void BaseMolecule::getAtomsCenterPoint (Array<int> &atoms, Vec2f &res)
{
   res.set(0, 0);
   for (int j = 0; j < atoms.size(); j++)
   {
      int ai = atoms[j];
      Vec3f &p = getAtomXyz(ai);
      res.x += p.x;
      res.y += p.y;
   }
   if (atoms.size() != 0)
      res.scale(1.0f / atoms.size());
}

void BaseMolecule::getAtomSymbol (int v, Array<char> &result)
{
   if (isPseudoAtom(v))
   {
      result.readString(getPseudoAtom(v), true);
   }
   else if (isTemplateAtom(v))
   {
      result.readString(getTemplateAtom(v), true);
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