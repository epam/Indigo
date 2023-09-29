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

#include "molecule/base_molecule.h"
#include "base_cpp/crc32.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "graph/dfs_walk.h"
#include "molecule/elements.h"
#include "molecule/inchi_wrapper.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_exact_substructure_matcher.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"

using namespace indigo;

IMPL_ERROR(BaseMolecule, "molecule");

BaseMolecule::BaseMolecule()
{
    _edit_revision = 0;
}

BaseMolecule::~BaseMolecule()
{
}

Molecule& BaseMolecule::asMolecule()
{
    throw Error("casting to molecule is invalid");
}

QueryMolecule& BaseMolecule::asQueryMolecule()
{
    throw Error("casting to query molecule is invalid");
}

bool BaseMolecule::isQueryMolecule()
{
    return false;
}

void BaseMolecule::changed()
{
    if (have_cip)
        clearCIP();
}

void BaseMolecule::clear()
{
    have_xyz = false;
    name.clear();
    _chiral_flag = -1;
    stereocenters.clear();
    cis_trans.clear();
    allene_stereo.clear();
    rgroups.clear();
    _xyz.clear();
    _rsite_attachment_points.clear();
    _attachment_index.clear();
    sgroups.clear();
    tgroups.clear();
    template_attachment_points.clear();
    Graph::clear();
    _hl_atoms.clear();
    _hl_bonds.clear();
    _bond_directions.clear();
    custom_collections.clear();

    reaction_atom_mapping.clear();
    reaction_atom_inversion.clear();
    reaction_atom_exact_change.clear();
    reaction_bond_reacting_center.clear();

    use_scsr_sgroups_only = false;
    remove_scsr_lgrp = false;
    use_scsr_name = false;
    expand_mod_templates = false;
    ignore_chem_templates = false;
    updateEditRevision();
    _meta.resetMetaData();
    clearCIP();
    aliases.clear();
}

bool BaseMolecule::hasCoord(BaseMolecule& mol)
{
    int i;

    for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        Vec3f& xyz = mol.getAtomXyz(i);
        if (fabs(xyz.x) > 0.001 || fabs(xyz.y) > 0.001 || fabs(xyz.z) > 0.001)
            return true;
    }

    return false;
}

bool BaseMolecule::hasZCoord(BaseMolecule& mol)
{
    int i;

    for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        if (fabs(mol.getAtomXyz(i).z) > 0.001)
            return true;

    return false;
}

void BaseMolecule::mergeSGroupsWithSubmolecule(BaseMolecule& mol, Array<int>& mapping)
{
    QS_DEF(Array<int>, edge_mapping);
    edge_mapping.clear_resize(mol.edgeEnd());
    edge_mapping.fffill();

    buildEdgeMapping(mol, &mapping, &edge_mapping);

    mergeSGroupsWithSubmolecule(mol, mapping, edge_mapping);
}

void BaseMolecule::mergeSGroupsWithSubmolecule(BaseMolecule& mol, Array<int>& mapping, Array<int>& edge_mapping)
{
    int i;

    for (i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& supersg = mol.sgroups.getSGroup(i);
        int idx = sgroups.addSGroup(supersg.sgroup_type);
        SGroup& sg = sgroups.getSGroup(idx);
        sg.parent_idx = supersg.parent_idx;

        if (_mergeSGroupWithSubmolecule(sg, supersg, mol, mapping, edge_mapping))
        {
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                DataSGroup& superdg = (DataSGroup&)supersg;

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
                Superatom& sa = (Superatom&)sg;
                Superatom& supersa = (Superatom&)supersg;

                if (supersa.bond_connections.size() > 0)
                {
                    for (int j = 0; j < supersa.bond_connections.size(); j++)
                    {
                        if (supersa.bond_connections[j].bond_idx > -1 && edge_mapping[supersa.bond_connections[j].bond_idx] > -1)
                        {
                            Superatom::_BondConnection& bond = sa.bond_connections.push();
                            bond.bond_dir = supersa.bond_connections[j].bond_dir;
                            bond.bond_idx = edge_mapping[supersa.bond_connections[j].bond_idx];
                        }
                    }
                }
                sa.subscript.copy(supersa.subscript);
                sa.sa_class.copy(supersa.sa_class);
                sa.sa_natreplace.copy(supersa.sa_natreplace);
                sa.contracted = supersa.contracted;
                if (supersa.attachment_points.size() > 0)
                {
                    for (int j = supersa.attachment_points.begin(); j < supersa.attachment_points.end(); j = supersa.attachment_points.next(j))
                    {
                        int ap_idx = sa.attachment_points.add();
                        Superatom::_AttachmentPoint& ap = sa.attachment_points.at(ap_idx);
                        int a_idx = supersa.attachment_points[j].aidx;
                        if (a_idx > -1)
                            ap.aidx = mapping[a_idx];
                        else
                            ap.aidx = a_idx;
                        int leave_idx = supersa.attachment_points[j].lvidx;
                        if (leave_idx > -1)
                            ap.lvidx = mapping[leave_idx];
                        else
                            ap.lvidx = leave_idx;

                        ap.apid.copy(supersa.attachment_points[j].apid);
                    }
                }
            }
            else if (sg.sgroup_type == SGroup::SG_TYPE_SRU)
            {
                RepeatingUnit& ru = (RepeatingUnit&)sg;
                RepeatingUnit& superru = (RepeatingUnit&)supersg;

                ru.connectivity = superru.connectivity;
                ru.subscript.copy(superru.subscript);
            }
            else if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
            {
                MultipleGroup& mg = (MultipleGroup&)sg;
                MultipleGroup& supermg = (MultipleGroup&)supersg;

                mg.multiplier = supermg.multiplier;
                for (int j = 0; j != supermg.parent_atoms.size(); j++)
                    if (mapping[supermg.parent_atoms[j]] >= 0)
                        mg.parent_atoms.push(mapping[supermg.parent_atoms[j]]);
            }
        }
        else
        {
            sgroups.remove(idx);
        }
    }
}

void BaseMolecule::clearSGroups()
{
    sgroups.clear();
}

void BaseMolecule::_mergeWithSubmolecule_Sub(BaseMolecule& mol, const Array<int>& vertices, const Array<int>* edges, Array<int>& mapping,
                                             Array<int>& edge_mapping, int skip_flags)
{
    QS_DEF(Array<char>, apid);

    // XYZ
    _xyz.expandFill(vertexEnd(), Vec3f(0, 0, 0));
    if (!(skip_flags & SKIP_XYZ))
    {
        if (vertexCount() == 0)
            have_xyz = mol.have_xyz;
        else
            have_xyz = have_xyz || mol.have_xyz;

        for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        {
            if (mapping[i] < 0)
                continue;

            _xyz[mapping[i]] = mol.getAtomXyz(i);
        }
    }
    else
        _xyz.zerofill();

    // copy cip values
    for (auto i = mol._cip_atoms.begin(); i != mol._cip_atoms.end(); i = mol._cip_atoms.next(i))
    {
        try
        {
            _cip_atoms.insert(mapping[mol._cip_atoms.key(i)], mol._cip_atoms.value(i));
        }
        catch (Exception& e)
        {
        }
    }

    for (auto i = mol._cip_bonds.begin(); i != mol._cip_bonds.end(); i = mol._cip_bonds.next(i))
    {
        try
        {
            _cip_bonds.insert(mapping[mol._cip_bonds.key(i)], mol._cip_bonds.value(i));
        }
        catch (Exception& e)
        {
        }
    }

    reaction_atom_mapping.expandFill(vertexEnd(), 0);
    reaction_atom_inversion.expandFill(vertexEnd(), 0);
    reaction_atom_exact_change.expandFill(vertexEnd(), 0);
    reaction_bond_reacting_center.expandFill(edgeEnd(), 0);
    _bond_directions.expandFill(edgeEnd(), -1);

    for (auto i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (mapping[i] < 0)
            continue;

        reaction_atom_mapping[mapping[i]] = mol.reaction_atom_mapping[i];
        reaction_atom_inversion[mapping[i]] = mol.reaction_atom_inversion[i];
        reaction_atom_exact_change[mapping[i]] = mol.reaction_atom_exact_change[i];
    }

    for (int j = mol.edgeBegin(); j != mol.edgeEnd(); j = mol.edgeNext(j))
    {
        int edge_idx = edge_mapping[j];
        if (edge_idx < 0)
            continue;
        reaction_bond_reacting_center[edge_idx] = mol.reaction_bond_reacting_center[j];
        _bond_directions[edge_idx] = mol.getBondDirection(j);
    }

    // RGroups
    if (!(skip_flags & SKIP_RGROUPS))
    {
        rgroups.copyRGroupsFromMolecule(mol.rgroups);

        for (auto i = 0; i < vertices.size(); i++)
        {
            if (!mol.isRSite(vertices[i]))
                continue;

            int atom_idx = mapping[vertices[i]];

            if (atom_idx == -1)
                continue;
            if (mol._rsite_attachment_points.size() <= vertices[i])
                continue;
            Array<int>& ap = mol._rsite_attachment_points[vertices[i]];
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
            for (auto i = 1; i <= mol.attachmentPointCount(); i++)
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
        for (auto i = 0; i < vertices.size(); i++)
        {
            if (mol.isTemplateAtom(vertices[i]))
            {
                for (int j = 0; j < mol.getTemplateAtomAttachmentPointsCount(vertices[i]); j++)
                {
                    if ((mol.getTemplateAtomAttachmentPoint(vertices[i], j) != -1) && (mapping[mol.getTemplateAtomAttachmentPoint(vertices[i], j)] != -1))
                    {
                        mol.getTemplateAtomAttachmentPointId(vertices[i], j, apid);
                        setTemplateAtomAttachmentOrder(mapping[vertices[i]], mapping[mol.getTemplateAtomAttachmentPoint(vertices[i], j)], apid.ptr());
                    }
                }
            }
        }
    }

    // SGroups merging
    mergeSGroupsWithSubmolecule(mol, mapping, edge_mapping);

    // highlighting
    highlightSubmolecule(mol, mapping.ptr(), false);

    // aliases
    for (int i = 0; i < vertices.size(); i++)
    {
        if (mol.isAlias(vertices[i]))
        {
            setAlias(mapping[vertices[i]], mol.getAlias(vertices[i]));
        }
    }

    // subclass stuff (Molecule or QueryMolecule)
    _mergeWithSubmolecule(mol, vertices, edges, mapping, skip_flags);

    // stereo
    if (!(skip_flags & SKIP_STEREOCENTERS))
        buildOnSubmoleculeStereocenters(mol, mapping.ptr());
    else
        stereocenters.clear();

    if (!(skip_flags & SKIP_CIS_TRANS))
        buildOnSubmoleculeCisTrans(mol, mapping.ptr());
    else
        cis_trans.clear();

    buildOnSubmoleculeAlleneStereo(mol, mapping.ptr());

    // subclass stuff (Molecule or QueryMolecule)
    _postMergeWithSubmolecule(mol, vertices, edges, mapping, skip_flags);

    updateEditRevision();
}

void BaseMolecule::_flipSGroupBond(SGroup& sgroup, int src_bond_idx, int new_bond_idx)
{
    int idx = sgroup.bonds.find(src_bond_idx);
    if (idx != -1)
        sgroup.bonds[idx] = new_bond_idx;
}

void BaseMolecule::_flipSuperatomBond(Superatom& sa, int src_bond_idx, int new_bond_idx)
{
    if (sa.bond_connections.size() > 0)
    {
        for (int j = 0; j < sa.bond_connections.size(); j++)
        {
            Superatom::_BondConnection& bond = sa.bond_connections[j];
            if (bond.bond_idx == src_bond_idx)
                bond.bond_idx = new_bond_idx;
        }
    }
    if (sa.attachment_points.size() > 0)
    {
        for (int j = sa.attachment_points.begin(); j != sa.attachment_points.end(); j = sa.attachment_points.next(j))
        {
            Superatom::_AttachmentPoint& ap = sa.attachment_points.at(j);
            const Edge& edge = getEdge(new_bond_idx);
            if ((edge.beg == ap.aidx) || (edge.end == ap.aidx))

            {
                int ap_aidx = -1;
                int ap_lvidx = -1;
                if (sa.atoms.find(edge.beg) != -1)
                {
                    ap_aidx = edge.beg;
                    ap_lvidx = edge.end;
                }
                else if (sa.atoms.find(edge.end) != -1)
                {
                    ap_aidx = edge.end;
                    ap_lvidx = edge.beg;
                }
                ap.aidx = ap_aidx;
                ap.lvidx = ap_lvidx;
            }
        }
    }
}

void BaseMolecule::_flipTemplateAtomAttachmentPoint(int idx, int atom_from, Array<char>& ap_id, int atom_to)
{
    for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
    {
        BaseMolecule::TemplateAttPoint& ap = template_attachment_points.at(j);
        if ((ap.ap_occur_idx == idx) && (ap.ap_aidx == atom_from) && (ap.ap_id.memcmp(ap_id) == 0))
        {
            ap.ap_aidx = atom_to;
        }
    }
}

void BaseMolecule::mergeWithSubmolecule(BaseMolecule& mol, const Array<int>& vertices, const Array<int>* edges, Array<int>* mapping_out, int skip_flags)
{
    QS_DEF(Array<int>, tmp_mapping);
    QS_DEF(Array<int>, edge_mapping);

    if (mapping_out == 0)
        mapping_out = &tmp_mapping;

    // vertices and edges
    _mergeWithSubgraph(mol, vertices, edges, mapping_out, &edge_mapping);

    // all the chemical stuff
    _mergeWithSubmolecule_Sub(mol, vertices, edges, *mapping_out, edge_mapping, skip_flags);
    copyProperties(mol, *mapping_out);
}

int BaseMolecule::mergeAtoms(int atom1, int atom2)
{
    updateEditRevision();

    const Vertex& v1 = getVertex(atom1);
    const Vertex& v2 = getVertex(atom2);

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

    if (((is_tetra1 || is_cs1) && (is_tetra2 || is_cs2)) || (!is_tetra1 && !is_cs1 && !is_tetra2 && !is_cs2))
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

void BaseMolecule::flipBond(int atom_parent, int atom_from, int atom_to)
{
    stereocenters.flipBond(atom_parent, atom_from, atom_to);
    cis_trans.flipBond(*this, atom_parent, atom_from, atom_to);

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
        SGroup& sg = sgroups.getSGroup(j);
        _flipSGroupBond(sg, src_bond_idx, new_bond_idx);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            _flipSuperatomBond((Superatom&)sg, src_bond_idx, new_bond_idx);
    }

    updateEditRevision();
}

void BaseMolecule::makeSubmolecule(BaseMolecule& mol, const Array<int>& vertices, Array<int>* mapping_out, int skip_flags)
{
    clear();
    mergeWithSubmolecule(mol, vertices, 0, mapping_out, skip_flags);
}

void BaseMolecule::makeSubmolecule(BaseMolecule& other, const Filter& filter, Array<int>* mapping_out, Array<int>* inv_mapping, int skip_flags)
{
    QS_DEF(Array<int>, vertices);

    if (mapping_out == 0)
        mapping_out = &vertices;

    filter.collectGraphVertices(other, *mapping_out);

    makeSubmolecule(other, *mapping_out, inv_mapping, skip_flags);
}

void BaseMolecule::makeEdgeSubmolecule(BaseMolecule& mol, const Array<int>& vertices, const Array<int>& edges, Array<int>* v_mapping, int skip_flags)
{
    clear();
    mergeWithSubmolecule(mol, vertices, &edges, v_mapping, skip_flags);
}

void BaseMolecule::copyProperties(BaseMolecule& other, const Array<int>& mapping)
{
    for (auto it = other._properties.begin(); it != other._properties.end(); ++it)
    {
        auto ref_atom = mapping[other._properties.key(it)];
        if (ref_atom >= 0)
            _properties.findOrInsert(ref_atom).copy(other._properties.value(it));
    }
}

void BaseMolecule::clone(BaseMolecule& other, Array<int>* mapping, Array<int>* inv_mapping, int skip_flags)
{
    QS_DEF(Array<int>, tmp_mapping);

    if (mapping == 0)
        mapping = &tmp_mapping;

    mapping->clear();

    for (int i = other.vertexBegin(); i < other.vertexEnd(); i = other.vertexNext(i))
        mapping->push(i);

    makeSubmolecule(other, *mapping, inv_mapping, skip_flags);
    _meta.clone(other._meta);
    name.copy(other.name);
    copyProperties(other, *mapping);
}

void BaseMolecule::clone_KeepIndices(BaseMolecule& other, int skip_flags)
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

    _meta.clone(other._meta);
    _mergeWithSubmolecule_Sub(other, vertices, 0, mapping, edge_mapping, skip_flags);
    name.copy(other.name);
    copyProperties(other, mapping);
}

void BaseMolecule::mergeWithMolecule(BaseMolecule& other, Array<int>* mapping, int skip_flags)
{
    QS_DEF(Array<int>, vertices);
    int i;

    vertices.clear();

    for (i = other.vertexBegin(); i != other.vertexEnd(); i = other.vertexNext(i))
        vertices.push(i);

    mergeWithSubmolecule(other, vertices, 0, mapping, skip_flags);
}

void BaseMolecule::removeAtoms(const Array<int>& indices)
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
        SGroup& sg = sgroups.getSGroup(j);
        _removeAtomsFromSGroup(sg, mapping);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            _removeAtomsFromSuperatom((Superatom&)sg, mapping);
        else if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
            _removeAtomsFromMultipleGroup((MultipleGroup&)sg, mapping);
        if (sg.atoms.size() < 1)
            removeSGroup(j);
    }

    // stereo
    removeAtomsStereocenters(indices);
    buildOnSubmoleculeCisTrans(*this, mapping.ptr());
    removeAtomsAlleneStereo(indices);

    // highlighting and stereo
    int b_idx;
    for (i = 0; i < indices.size(); i++)
    {
        const Vertex& vertex = getVertex(indices[i]);
        unhighlightAtom(indices[i]);
        for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
        {
            b_idx = vertex.neiEdge(j);
            unhighlightBond(b_idx);
            if (getBondDirection(b_idx) > 0)
                setBondDirection(b_idx, 0);
        }
    }

    // aliases
    for (i = 0; i < indices.size(); i++)
    {
        if (isAlias(indices[i]))
        {
            removeAlias(indices[i]);
        }
    }

    // subclass (Molecule or QueryMolecule) removes its data
    _removeAtoms(indices, mapping.ptr());

    // Remove vertices from graph
    for (i = 0; i < indices.size(); i++)
        removeVertex(indices[i]);

    updateEditRevision();
}

void BaseMolecule::removeAtom(int idx)
{
    QS_DEF(Array<int>, vertices);

    vertices.clear();
    vertices.push(idx);
    removeAtoms(vertices);
}

void BaseMolecule::removeAtoms(const Filter& filter)
{
    QS_DEF(Array<int>, vertices);

    filter.collectGraphVertices(*this, vertices);
    removeAtoms(vertices);
}

void BaseMolecule::removeBonds(const Array<int>& indices)
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
        SGroup& sg = sgroups.getSGroup(j);
        _removeBondsFromSGroup(sg, mapping);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            _removeBondsFromSuperatom((Superatom&)sg, mapping);
    }

    // subclass (Molecule or QueryMolecule) removes its data
    _removeBonds(indices);

    removeBondsStereocenters(indices);
    removeBondsAlleneStereo(indices);

    for (int i = 0; i < indices.size(); i++)
    {
        unhighlightBond(indices[i]);
        if (getBondDirection(indices[i]) > 0)
            setBondDirection(indices[i], 0);
        removeEdge(indices[i]);
    }
    updateEditRevision();
}

void BaseMolecule::removeBond(int idx)
{
    QS_DEF(Array<int>, edges);

    edges.clear();
    edges.push(idx);
    removeBonds(edges);
}

void BaseMolecule::removeSGroup(int idx)
{
    SGroup& sg = sgroups.getSGroup(idx);
    _checkSgroupHierarchy(sg.parent_group, sg.original_group);
    sgroups.remove(idx);
}

void BaseMolecule::removeSGroupWithBasis(int idx)
{
    QS_DEF(Array<int>, sg_atoms);
    SGroup& sg = sgroups.getSGroup(idx);
    _checkSgroupHierarchy(sg.parent_group, sg.original_group);
    sg_atoms.copy(sg.atoms);
    removeAtoms(sg_atoms);
}

int BaseMolecule::getVacantPiOrbitals(int group, int charge, int radical, int conn, int* lonepairs_out)
{
    int orbitals;

    if (conn < 0)
        throw Error("invalid connectivity given: %d", conn);

    switch (group)
    {
    case 1:
        orbitals = 1;
        break;
    case 2:
        orbitals = 2;
        break;
    default:
        orbitals = 4;
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

void BaseMolecule::_postMergeWithSubmolecule(BaseMolecule& mol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping, int skip_flags)
{
}

void BaseMolecule::_flipBond(int atom_parent, int atom_from, int atom_to)
{
}

void BaseMolecule::_removeAtoms(const Array<int>& indices, const int* mapping)
{
}

void BaseMolecule::_removeBonds(const Array<int>& indices)
{
}

Vec3f& BaseMolecule::getAtomXyz(int idx)
{
    return _xyz[idx];
}

void BaseMolecule::setAtomXyz(int idx, float x, float y, float z)
{
    _xyz[idx].set(x, y, z);
    updateEditRevision();
}

void BaseMolecule::setAtomXyz(int idx, const Vec3f& v)
{
    _xyz[idx].copy(v);
    updateEditRevision();
}

void BaseMolecule::clearXyz()
{
    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        setAtomXyz(i, 0, 0, 0);
    have_xyz = 0;
}

int BaseMolecule::_addBaseAtom()
{
    int idx = addVertex();

    _xyz.expand(idx + 1);
    _xyz[idx].zero();

    reaction_atom_mapping.expand(idx + 1);
    reaction_atom_mapping[idx] = 0;
    reaction_atom_inversion.expand(idx + 1);
    reaction_atom_inversion[idx] = 0;
    reaction_atom_exact_change.expand(idx + 1);
    reaction_atom_exact_change[idx] = 0;

    updateEditRevision();

    return idx;
}

int BaseMolecule::_addBaseBond(int beg, int end)
{
    int idx = addEdge(beg, end);

    reaction_bond_reacting_center.expand(idx + 1);
    reaction_bond_reacting_center[idx] = 0;

    cis_trans.registerBond(idx);
    updateEditRevision();
    return idx;
}

int BaseMolecule::getAtomRadical_NoThrow(int idx, int fallback)
{
    try
    {
        return getAtomRadical(idx);
    }
    catch (Exception& ex)
    {
        return fallback;
    }
}

int BaseMolecule::getAtomValence_NoThrow(int idx, int fallback)
{
    try
    {
        return getAtomValence(idx);
    }
    catch (Exception& ex)
    {
        return fallback;
    }
}

int BaseMolecule::possibleAtomTotalH(int idx, int hcount)
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

void BaseMolecule::getAllowedRGroups(int atom_idx, Array<int>& rgroup_list)
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

int BaseMolecule::getSingleAllowedRGroup(int atom_idx)
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

int BaseMolecule::countRSites()
{
    int i, sum = 0;

    for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        if (isRSite(i))
            sum++;

    return sum;
}

int BaseMolecule::getRSiteAttachmentPointByOrder(int idx, int order) const
{
    if (idx >= _rsite_attachment_points.size())
        return -1;

    if (order >= _rsite_attachment_points[idx].size())
        return -1;

    return _rsite_attachment_points[idx][order];
}

void BaseMolecule::setRSiteAttachmentOrder(int atom_idx, int att_atom_idx, int order)
{
    _rsite_attachment_points.expand(atom_idx + 1);
    _rsite_attachment_points[atom_idx].expandFill(order + 1, -1);
    _rsite_attachment_points[atom_idx][order] = att_atom_idx;
    updateEditRevision();
}

void BaseMolecule::setTemplateAtomAttachmentOrder(int atom_idx, int att_atom_idx, const char* att_id)
{
    int att_idx = template_attachment_points.add();
    TemplateAttPoint& ap = template_attachment_points.at(att_idx);
    ap.ap_occur_idx = atom_idx;
    ap.ap_aidx = att_atom_idx;
    ap.ap_id.readString(att_id, false);
    ap.ap_id.push(0);
    updateEditRevision();
}

int BaseMolecule::getTemplateAtomAttachmentPoint(int atom_idx, int order)
{
    int ap_count = 0;
    for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
    {
        BaseMolecule::TemplateAttPoint& ap = template_attachment_points.at(j);
        if (ap.ap_occur_idx == atom_idx)
        {
            if (ap_count == order)
                return ap.ap_aidx;

            ap_count++;
        }
    }
    return -1;
}

void BaseMolecule::getTemplateAtomAttachmentPointId(int atom_idx, int order, Array<char>& apid)
{
    int ap_count = 0;
    for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
    {
        BaseMolecule::TemplateAttPoint& ap = template_attachment_points.at(j);
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

int BaseMolecule::getTemplateAtomAttachmentPointById(int atom_idx, Array<char>& att_id)
{
    QS_DEF(Array<char>, tmp);
    int aidx = -1;
    for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
    {
        BaseMolecule::TemplateAttPoint& ap = template_attachment_points.at(j);
        if ((ap.ap_occur_idx == atom_idx) && (ap.ap_id.memcmp(att_id) == 0))
        {
            return ap.ap_aidx;
        }
    }
    return aidx;
}

int BaseMolecule::getTemplateAtomAttachmentPointsCount(int atom_idx)
{
    int count = 0;
    for (int j = template_attachment_points.begin(); j != template_attachment_points.end(); j = template_attachment_points.next(j))
    {
        BaseMolecule::TemplateAttPoint& ap = template_attachment_points.at(j);
        if (ap.ap_occur_idx == atom_idx)
        {
            count++;
        }
    }
    return count;
}

int BaseMolecule::attachmentPointCount() const
{
    return _attachment_index.size();
}

void BaseMolecule::addAttachmentPoint(int order, int atom_index)
{
    if (order < 1)
        throw Error("attachment point order %d no allowed (should start from 1)", order);

    if (_attachment_index.size() < order)
        _attachment_index.resize(order);

    _attachment_index[order - 1].push(atom_index);
    updateEditRevision();
}

void BaseMolecule::removeAttachmentPoints()
{
    _attachment_index.clear();
    updateEditRevision();
}

void BaseMolecule::removeAttachmentPointsFromAtom(int atom_index)
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

int BaseMolecule::getAttachmentPoint(int order, int index) const
{
    if (order < 1)
        throw Error("attachment point order %d no allowed (should start from 1)", order);

    return index < _attachment_index[order - 1].size() ? _attachment_index[order - 1][index] : -1;
}

void BaseMolecule::_checkSgroupHierarchy(int pidx, int oidx)
{
    for (int i = sgroups.begin(); i != sgroups.end(); i = sgroups.next(i))
    {
        SGroup& sg = sgroups.getSGroup(i);
        if (sg.parent_group == oidx)
            sg.parent_group = pidx;
    }
}

int copyBaseBond(BaseMolecule& bm, int beg, int end, int srcId)
{
    int bid = -1;
    if (bm.isQueryMolecule())
    {
        QueryMolecule& qm = bm.asQueryMolecule();
        bid = qm.addBond(beg, end, qm.getBond(srcId).clone());
    }
    else
    {
        Molecule& mol = bm.asMolecule();
        bid = mol.addBond(beg, end, mol.getBondOrder(srcId));
        mol.setEdgeTopology(bid, mol.getBondTopology(srcId));
    }
    return bid;
}

void BaseMolecule::collapse(BaseMolecule& bm)
{
    for (int i = bm.sgroups.begin(); i != bm.sgroups.end(); i = bm.sgroups.next(i))
    {
        SGroup& sg = bm.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
            collapse(bm, i);
    }
}

void BaseMolecule::collapse(BaseMolecule& bm, int id)
{
    QS_DEF(Mapping, mapAtom);
    mapAtom.clear();
    QS_DEF(Mapping, mapBondInv);
    mapBondInv.clear();
    collapse(bm, id, mapAtom, mapBondInv);
}

void BaseMolecule::collapse(BaseMolecule& bm, int id, Mapping& mapAtom, Mapping& mapBondInv)
{
    SGroup& sg = bm.sgroups.getSGroup(id);

    if (sg.sgroup_type != SGroup::SG_TYPE_MUL)
        throw Error("The group is wrong type");

    const MultipleGroup& group = (MultipleGroup&)sg;

    if (group.atoms.size() != group.multiplier * group.parent_atoms.size())
        throw Error("The group is already collapsed or invalid");

    QS_DEF(Array<int>, toRemove);
    toRemove.clear();

    for (int j = 0; j < group.atoms.size(); ++j)
    {
        int k = j % group.parent_atoms.size();
        int from = group.atoms[j];
        int to = group.atoms[k];

        auto it = mapAtom.find(from);
        if (it == mapAtom.end())
            mapAtom.emplace(from, to);
        else if (it->second != to)
            throw Error("Invalid mapping in MultipleGroup::collapse");

        if (k != j)
            toRemove.push(from);
    }

    for (int j = bm.edgeBegin(); j < bm.edgeEnd(); j = bm.edgeNext(j))
    {
        const Edge& edge = bm.getEdge(j);
        bool in1 = mapAtom.find(edge.beg) != mapAtom.end(), in2 = mapAtom.find(edge.end) != mapAtom.end();
        bool p1 = in1 && mapAtom.at(edge.beg) == edge.beg, p2 = in2 && mapAtom.at(edge.end) == edge.end;
        if ((in1 && !p1 && !in2) || (!in1 && !p2 && in2))
        {
            int beg = in1 ? mapAtom.at(edge.beg) : edge.beg;
            int end = in2 ? mapAtom.at(edge.end) : edge.end;
            int bid = copyBaseBond(bm, beg, end, j);
            if (mapBondInv.find(bid) == mapBondInv.end())
                mapBondInv.emplace(bid, j);
        }
    }

    for (int j = 0; j < toRemove.size(); ++j)
    {
        int aid = toRemove[j];
        bm.removeAtom(aid);
    }
}

int BaseMolecule::transformSCSRtoFullCTAB()
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
        _transformTGroupToSGroup(tinds[i], -1);
    }

    if (tinds.size() > 0)
    {
        tgroups.clear();
        template_attachment_points.clear();
    }

    /*
       StereocentersOptions stereochemistry_options;
       stereocenters.buildFromBonds(stereochemistry_options, 0);
       allene_stereo.buildFromBonds(stereochemistry_options.ignore_errors, 0);
       cis_trans.build(0);
    */

    return result;
}

int BaseMolecule::transformFullCTABtoSCSR(ObjArray<TGroup>& templates)
{
    int result = 0;
    QS_DEF(Molecule, fragment);
    QS_DEF(Molecule, su_fragment);
    QS_DEF(Molecule, rep);
    QS_DEF(Array<int>, added_templates);
    QS_DEF(Array<int>, added_template_occurs);
    QS_DEF(Array<int>, new_templates);
    QS_DEF(Array<int>, new_template_occurs);
    QS_DEF(Array<int>, mapping);
    QS_DEF(Array<int>, su_mapping);
    QS_DEF(Array<int>, tm_mapping);
    QS_DEF(Array<int>, remove_atoms);
    QS_DEF(Array<int>, base_sgs);
    QS_DEF(Array<int>, sgs);
    QS_DEF(Array<int>, sgs_to_remove);
    QS_DEF(Array<int>, atoms_to_remove);
    QS_DEF(Array<int>, ap_points_atoms);
    QS_DEF(Array<int>, ap_lgrp_atoms);
    QS_DEF(StringPool, ap_points_ids);
    QS_DEF(Array<int>, ap_ids);
    QS_DEF(Array<int>, ignore_atoms);
    QS_DEF(Array<int>, query_atoms);
    QS_DEF(Array<int>, ignore_query_atoms);
    QS_DEF(Array<char>, tg_alias);
    QS_DEF(Array<char>, tg_name);

    QS_DEF(Array<int>, att_atoms);
    QS_DEF(Array<int>, ap_neibs);

    added_templates.clear();
    new_templates.clear();
    new_template_occurs.clear();
    ignore_atoms.clear();

    AromaticityOptions arom_opt;

    QS_DEF(Molecule, target);
    QS_DEF(Molecule, query);

    InchiWrapper indigo_inchi;
    QS_DEF(Array<char>, inchi_target);
    QS_DEF(Array<char>, inchi_query);
    QS_DEF(Array<int>, mapping_out);

    indigo_inchi.setOptions("/SNon");

    bool arom = this->asMolecule().aromatize(arom_opt);

    templates.qsort(TGroup::cmp, 0);

    for (auto i = 0; i < templates.size(); i++)
    {
        const TGroup& tg = templates.at(i);

        fragment.clear();
        fragment.clone_KeepIndices(*tg.fragment.get());

        if (ignore_chem_templates)
        {
            if (((tg.tgroup_class.size() > 3) && strncmp(tg.tgroup_class.ptr(), "CHEM", 4) == 0) ||
                ((tg.tgroup_class.size() > 5) && strncmp(tg.tgroup_class.ptr(), "LINKER", 6) == 0) || fragment.vertexCount() < 6)
                continue;
        }

        sgs.clear();
        base_sgs.clear();
        fragment.sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", sgs);
        for (int j = fragment.sgroups.begin(); j != fragment.sgroups.end(); j = fragment.sgroups.next(j))
        {
            if (sgs.find(j) == -1)
                base_sgs.push(j);
        }

        ap_points_atoms.clear();
        ap_lgrp_atoms.clear();
        ap_points_ids.clear();
        ap_ids.clear();
        for (int j = 0; j < base_sgs.size(); j++)
        {
            SGroup& sg = fragment.sgroups.getSGroup(base_sgs[j]);
            if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                Superatom& su = (Superatom&)sg;

                if (su.attachment_points.size() > 0)
                {
                    for (int k = su.attachment_points.begin(); k < su.attachment_points.end(); k = su.attachment_points.next(k))
                    {
                        Superatom::_AttachmentPoint& ap = su.attachment_points.at(k);
                        ap_points_atoms.push(ap.aidx);
                        ap_lgrp_atoms.push(ap.lvidx);
                        ap_ids.push(ap_points_ids.add(ap.apid));
                    }
                }
            }
            else
                throw Error("Wrong template structure was found (base SGroup is not Superatom type)");
        }

        ignore_query_atoms.clear();
        for (int j = 0; j < sgs.size(); j++)
        {
            SGroup& sg = fragment.sgroups.getSGroup(sgs[j]);
            ignore_query_atoms.concat(sg.atoms);
        }

        int count_occur = 0;
        ignore_atoms.clear();

        int flags = MoleculeExactMatcher::CONDITION_ELECTRONS | // bond types, atom charges, valences, radicals must match
                    MoleculeExactMatcher::CONDITION_ISOTOPE |   // atom isotopes must match
                    MoleculeExactMatcher::CONDITION_STEREO;     // tetrahedral and cis-trans configurations must match

        bool unrestricted_search = false;

        /*
            TautomerEnumerator _enumerator(fragment, RSMARTS);
            int tau_index = 0;
            for (;;)
            {
              if (_enumerator.isValid(tau_index))
              {
                 _enumerator.constructMolecule(fragment, tau_index);
                 printf("Template index = %d, tautomer index = %d\n", tg.tgroup_id, tau_index);

                 printSmile(fragment);

                 tau_index = _enumerator.next(tau_index);
              }
              else if (tau_index > 1)
                 break;
              else
              {
                 printf("Template index = %d, original template\n", tg.tgroup_id);
                 printSmile(fragment);
                 tau_index = 1;
              }
        */

        if (arom)
            fragment.aromatize(arom_opt);

        for (;;)
        {
            MoleculeExactSubstructureMatcher matcher(fragment, this->asMolecule());

            for (int j = 0; j < ignore_atoms.size(); j++)
                matcher.ignoreTargetAtom(ignore_atoms[j]);

            for (int j = 0; j < ignore_query_atoms.size(); j++)
                matcher.ignoreQueryAtom(ignore_query_atoms[j]);

            matcher.flags = flags;

            if (!matcher.find())
            {
                break;
                /*
                            if (flags == 0)
                               break;
                            else
                            {
                               flags = 0;
                               unrestricted_search = true;
                               continue;
                            }
                */
            }
            bool templ_found = false;
            do
            {
                mapping.clear();
                remove_atoms.clear();
                query_atoms.clear();
                mapping.copy(matcher.getQueryMapping(), fragment.vertexEnd());
                for (int j = 0; j < mapping.size(); j++)
                {
                    if (mapping[j] > -1)
                    {
                        query_atoms.push(j);
                        remove_atoms.push(mapping[j]);
                    }
                }

                att_atoms.clear();
                for (int l = 0; l < ap_points_atoms.size(); l++)
                {
                    int att_point_idx = mapping[ap_points_atoms[l]];
                    if (att_point_idx > -1)
                        att_atoms.push(att_point_idx);
                }

                int out_bonds = 0;
                int used_att_points = 0;
                bool wrong_xbond_order = false;
                for (int j = 0; j < remove_atoms.size(); j++)
                {
                    bool ap_used = false;
                    const Vertex& v = getVertex(remove_atoms[j]);
                    for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                    {
                        if (remove_atoms.find(v.neiVertex(k)) == -1)
                        {
                            out_bonds++;
                            for (int l = 0; l < ap_points_atoms.size(); l++)
                            {
                                int att_point_idx = mapping[ap_points_atoms[l]];
                                if (att_point_idx == remove_atoms[j] && !ap_used)
                                {
                                    used_att_points += 1;
                                    ap_used = true;
                                    int q_xbond_idx = fragment.findEdgeIndex(ap_points_atoms[l], ap_lgrp_atoms[l]);
                                    int t_xbond_idx = findEdgeIndex(att_point_idx, v.neiVertex(k));
                                    if (fragment.getBondOrder(q_xbond_idx) != this->asMolecule().getBondOrder(t_xbond_idx))
                                        wrong_xbond_order = true;
                                }
                            }
                        }
                    }
                }
                if ((out_bonds > ap_points_atoms.size()) || (out_bonds > used_att_points) || wrong_xbond_order)
                    continue;

                bool charged = false;
                for (int j = 0; j < remove_atoms.size(); j++)
                {
                    if (this->asMolecule().getAtomCharge(remove_atoms[j]) != 0)
                        charged = true;
                }
                //            if (charged)
                //               continue;

                target.clear();
                target.makeSubmolecule(*this, remove_atoms, 0);

                if (charged)
                {
                    for (auto j : target.vertices())
                    {
                        if (target.asMolecule().getAtomCharge(j) != 0)
                            target.asMolecule().setAtomCharge(j, 0);
                    }
                }

                query.clear();
                query.makeSubmolecule(fragment, query_atoms, 0);

                indigo_inchi.saveMoleculeIntoInchi(target, inchi_target);
                indigo_inchi.saveMoleculeIntoInchi(query, inchi_query);

                int inchi_comp = inchi_target.memcmp(inchi_query);

                if (inchi_comp != 0)
                    continue;

                /*
                            bool wrong_bond_order = false;
                            for (int j = 0; j < remove_atoms.size(); j++)
                            {
                               const Vertex &v = getVertex(remove_atoms[j]);
                               for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                               {
                                  int neib_ind = remove_atoms.find(v.neiVertex(k));
                                  if (neib_ind != -1)
                                  {
                                      int q_xbond_idx = fragment.findEdgeIndex(query_atoms[j], query_atoms[neib_ind]);
                                      int t_xbond_idx = findEdgeIndex(remove_atoms[j], v.neiVertex(k));
                                      if (q_xbond_idx == -1)
                                         wrong_bond_order = true;
                                      else if (fragment.getBondOrder(q_xbond_idx) != this->asMolecule().getBondOrder(t_xbond_idx))
                                         wrong_bond_order = true;
                                  }
                               }
                            }
                            if (wrong_bond_order)
                               continue;
                */

                bool ap_closed = false;
                ap_neibs.clear();
                for (int l = 0; l < ap_points_atoms.size(); l++)
                {
                    int att_point_idx = mapping[ap_points_atoms[l]];
                    const Vertex& v = getVertex(att_point_idx);
                    for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                    {
                        if (att_atoms.find(v.neiVertex(k)) != -1)
                        {
                            ap_closed = true;
                        }
                        else if (ap_neibs.find(v.neiVertex(k)) != -1)
                        {
                            ap_closed = true;
                        }
                        else if (remove_atoms.find(v.neiVertex(k)) == -1)
                        {
                            ap_neibs.push(v.neiVertex(k));
                        }
                    }
                }
                if (ap_closed)
                    continue;

                bool lgrp_absent = false;
                for (int m = 0; m < ap_points_atoms.size(); m++)
                {
                    int att_point_idx = mapping[ap_points_atoms[m]];
                    const Vertex& v1 = getVertex(att_point_idx);
                    const Vertex& v2 = fragment.getVertex(ap_points_atoms[m]);
                    if (v1.degree() != v2.degree() && fragment.getAtomNumber(ap_lgrp_atoms[m]) != ELEM_H)
                        lgrp_absent = true;
                }
                if (lgrp_absent)
                    continue;

                bool y_delta_found = false;
                for (int j = 0; j < query_atoms.size(); j++)
                {
                    if (ap_points_atoms.find(query_atoms[j]) == -1)
                    {
                        const Vertex& v1 = getVertex(remove_atoms[j]);
                        const Vertex& v2 = fragment.getVertex(query_atoms[j]);
                        if (v1.degree() != v2.degree())
                        {
                            y_delta_found = true;
                            break;
                        }
                    }
                }
                if (y_delta_found)
                    continue;

                templ_found = true;

            } while (!templ_found && matcher.findNext());

            if (!templ_found)
            {
                ignore_atoms.concat(remove_atoms);
                continue;
            }
            else if (unrestricted_search)
            {
                sgs.clear();
                sgroups.findSGroups(SGroup::SG_ATOMS, remove_atoms, sgs);

                int sg_idx = -1;

                if (sgs.size() > 0)
                {
                    for (int j = 0; j < sgs.size(); j++)
                    {
                        SGroup& sg = sgroups.getSGroup(sgs[j]);
                        if ((sg.sgroup_type == SGroup::SG_TYPE_SUP) && (sg.atoms.size() == remove_atoms.size()))
                        {
                            Superatom& su = (Superatom&)sg;
                            sg_idx = sgs[j];
                            if (tg.tgroup_natreplace.size() > 1)
                                su.sa_natreplace.copy(tg.tgroup_natreplace);
                            else
                            {
                                su.sa_natreplace.copy(tg.tgroup_class);
                                su.sa_natreplace.pop();
                                su.sa_natreplace.push('/');
                                su.sa_natreplace.concat(tg.tgroup_alias);
                            }
                        }
                        else if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
                        {
                            bool skip_sg = false;
                            for (int m = 0; m < sg.atoms.size(); m++)
                            {
                                if (isTemplateAtom(sg.atoms[m]))
                                {
                                    skip_sg = true;
                                    break;
                                }
                            }
                            if (skip_sg)
                                continue;

                            sg_idx = _createSGroupFromFragment(remove_atoms, tg, mapping);
                        }
                    }
                }
                else
                {
                    bool skip_match = false;
                    for (int m = 0; m < remove_atoms.size(); m++)
                    {
                        if (isAtomBelongsSGroup(remove_atoms[m]))
                        {
                            skip_match = true;
                            break;
                        }
                    }
                    if (!skip_match)
                        sg_idx = _createSGroupFromFragment(remove_atoms, tg, mapping);
                }

                if (sg_idx != -1)
                {
                    int templ_idx = -1;
                    int templ_occ_ind = _transformSGroupToTGroup(sg_idx, templ_idx); //  Try to transform this Superatom into template
                    if (templ_occ_ind > -1)
                    {
                        new_templates.push(templ_idx);
                        new_template_occurs.push(templ_occ_ind);
                    }
                }
                ignore_atoms.concat(remove_atoms);
                continue;
            }

            int idx = this->asMolecule().addAtom(-1);
            this->asMolecule().setTemplateAtom(idx, tg.tgroup_name.ptr());
            this->asMolecule().setTemplateAtomClass(idx, tg.tgroup_class.ptr());

            count_occur++;

            for (int j = 0; j < ap_points_atoms.size(); j++)
            {
                int att_point_idx = mapping[ap_points_atoms[j]];
                if (remove_atoms.find(att_point_idx) != -1)
                {
                    const Vertex& v = getVertex(att_point_idx);
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
                        int v_k = neighbors[k];
                        if (findEdgeIndex(v_k, att_point_idx) != -1)
                        {
                            const Vertex& v_n = getVertex(v_k);
                            if (getAtomNumber(v_k) == ELEM_H)
                            {
                                remove_atoms.push(v_k);
                                continue;
                            }

                            if ((getAtomNumber(v_k) == ELEM_O && v_n.degree() == 1 && getAtomCharge(v_k) == 0) && getAtomNumber(att_point_idx) == ELEM_C)
                            {
                                remove_atoms.push(v_k);
                                continue;
                            }

                            if (findEdgeIndex(v_k, idx) == -1)
                                flipBond(v_k, att_point_idx, idx);
                            this->asMolecule().setTemplateAtomAttachmentOrder(idx, v_k, ap_points_ids.at(ap_ids[j]));
                            if (isTemplateAtom(v_k))
                            {
                                int ap_count = getTemplateAtomAttachmentPointsCount(v_k);
                                for (int m = 0; m < ap_count; m++)
                                {
                                    if (getTemplateAtomAttachmentPoint(v_k, m) == att_point_idx)
                                    {
                                        QS_DEF(Array<char>, ap_id);
                                        getTemplateAtomAttachmentPointId(v_k, m, ap_id);
                                        _flipTemplateAtomAttachmentPoint(v_k, att_point_idx, ap_id, idx);
                                    }
                                }
                            }
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

            sgs.clear();
            sgroups.findSGroups(SGroup::SG_ATOMS, remove_atoms, sgs);

            for (int j = 0; j < sgs.size(); j++)
            {
                SGroup& sg = sgroups.getSGroup(sgs[j]);
                if ((sg.sgroup_type == SGroup::SG_TYPE_SUP) && (sg.atoms.size() == remove_atoms.size()))
                {
                    sgroups.remove((sgs[j]));
                }
                else
                {
                    sg.atoms.push(idx);
                }
            }

            removeAtoms(remove_atoms);
        }
        //    }

        if (count_occur > 0 && added_templates.find(i) == -1)
            added_templates.push(i);
    }

    /*
       if (new_templates.size() > 0)
       {
          for (auto i = 0; i < new_templates.size(); i++)
          {
             _transformTGroupToSGroup(new_template_occurs[i], new_templates[i]);
             tgroups.remove(new_templates[i]);
          }
       }
    */

    /*
       for (auto i = 0; i < templates.size(); i++)
       {
          const TGroup &tg = templates.at(i);

          fragment.clear();
          fragment.clone_KeepIndices(*tg.fragment.get());

          sgs.clear();
          base_sgs.clear();
          fragment.sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", sgs);
          for (int j = fragment.sgroups.begin(); j != fragment.sgroups.end(); j = fragment.sgroups.next(j))
          {
             if (sgs.find(j) == -1)
                base_sgs.push(j);
          }

          ap_points_atoms.clear();
          ap_lgrp_atoms.clear();
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
                      ap_lgrp_atoms.push(ap.lvidx);
                      ap_ids.push(ap_points_ids.add(ap.apid));
                   }
                }
             }
             else
                throw Error("Wrong template structure was found (base SGroup is not Superatom type)");
          }

          ignore_query_atoms.clear();
          for (int j = 0; j < sgs.size(); j++)
          {
             SGroup &sg = fragment.sgroups.getSGroup(sgs[j]);
             ignore_query_atoms.concat(sg.atoms);
          }

          int count_occur = 0;
          ignore_atoms.clear();

          sgs.clear();
          sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_SUP, sgs);

          sgs_to_remove.clear();
          atoms_to_remove.clear();
          added_template_occurs.clear();

          for (int l = 0; l < sgs.size(); l++)
          {
             Superatom &su = (Superatom &) sgroups.getSGroup(sgs[l]);


             if (su.sa_class.size() > 3 && strncmp(su.sa_class.ptr(), "LGRP", 4) == 0)
             {
                if (remove_scsr_lgrp)
                   sgs_to_remove.push(sgs[l]);
                continue;
             }

             if (use_scsr_name)
             {
                if ( (tg.tgroup_name.memcmp(su.subscript) == -1) &&
                     (tg.tgroup_alias.memcmp(su.subscript) == -1) )
                {
                   continue;
                }
             }

             MoleculeExactMatcher matcher(fragment, this->asMolecule());

             matcher.flags =  MoleculeExactMatcher::CONDITION_ELECTRONS |  // bond types, atom charges, valences, radicals must match
                              MoleculeExactMatcher::CONDITION_ISOTOPE   |  // atom isotopes must match
                              MoleculeExactMatcher::CONDITION_STEREO;      // tetrahedral and cis-trans configurations must match


             for (auto j : vertices())
             {
                if (su.atoms.find(j) == -1)
                   matcher.ignoreTargetAtom(j);
             }

             for (auto j = 0; j < ignore_query_atoms.size(); j++)
             {
                matcher.ignoreQueryAtom(ignore_query_atoms[j]);
             }

             if (!matcher.find())   //  This is Superatom, but no suitable template is found
                continue;

             mapping.clear();
             remove_atoms.clear();
             mapping.copy(matcher.getQueryMapping(), fragment.vertexEnd());

             for (int j = 0; j < mapping.size(); j++)
             {
                if (mapping[j] > -1)
                   remove_atoms.push(mapping[j]);
             }

             bool charged = false;
             for (int j = 0; j < remove_atoms.size(); j++)
             {
                if (this->asMolecule().getAtomCharge(remove_atoms[j]) != 0)
                   charged = true;
             }

             int out_bonds = 0;
             int used_att_points = 0;
             bool wrong_xbond_order = false;
             for (int j = 0; j < remove_atoms.size(); j++)
             {
                bool ap_used = false;
                const Vertex &v = getVertex(remove_atoms[j]);
                for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                {
                   if (remove_atoms.find(v.neiVertex(k)) == -1)
                   {
                      out_bonds++;
                      for (int m = 0; m < ap_points_atoms.size(); m++)
                      {
                         int att_point_idx = mapping[ap_points_atoms[m]];
                         if (att_point_idx == remove_atoms[j] && !ap_used)
                         {
                            used_att_points += 1;
                            ap_used = true;
                            int q_xbond_idx = fragment.findEdgeIndex(ap_points_atoms[m], ap_lgrp_atoms[m]);
                            int t_xbond_idx = findEdgeIndex(att_point_idx, v.neiVertex(k));
                            if (fragment.getBondOrder(q_xbond_idx) != this->asMolecule().getBondOrder(t_xbond_idx))
                               wrong_xbond_order = true;
                         }
                      }
                   }
                }
             }


             if ( (out_bonds > ap_points_atoms.size()) || (out_bonds > used_att_points) || wrong_xbond_order)
             {
                continue;
             }


             bool lgrp_absent = false;
             for (int m = 0; m < ap_points_atoms.size(); m++)
             {
                int att_point_idx = mapping[ap_points_atoms[m]];
                const Vertex &v1 = getVertex(att_point_idx);
                const Vertex &v2 = fragment.getVertex(ap_points_atoms[m]);
                if (v1.degree() != v2.degree() && fragment.getAtomNumber(ap_lgrp_atoms[m]) != ELEM_H)
                   lgrp_absent = true;
             }

             if (lgrp_absent)
             {
                continue;
             }


             int idx = this->asMolecule().addAtom(-1);
             this->asMolecule().setTemplateAtom(idx, tg.tgroup_name.ptr());
             this->asMolecule().setTemplateAtomClass(idx, tg.tgroup_class.ptr());
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
                      int v_k = neighbors[k];
                      if (findEdgeIndex(v_k, att_point_idx) != -1)
                      {
                         const Vertex &v_n = getVertex(v_k);
                         if (getAtomNumber(v_k) == ELEM_H)
                         {
                            remove_atoms.push(v_k);
                            continue;
                         }

                         if ( (getAtomNumber(v_k) == ELEM_O && v_n.degree() == 1 && getAtomCharge(v_k) == 0) &&
                              getAtomNumber(att_point_idx) == ELEM_C )
                         {
                            remove_atoms.push(v_k);
                            continue;
                         }

                         if (findEdgeIndex(v_k, idx) == -1)
                            flipBond(v_k, att_point_idx, idx);
                         this->asMolecule().setTemplateAtomAttachmentOrder(idx, v_k, ap_points_ids.at(ap_ids[j]));
                         if (isTemplateAtom(v_k))
                         {
                            int ap_count = getTemplateAtomAttachmentPointsCount(v_k);
                            for (int m = 0; m < ap_count; m++)
                            {
                               if (getTemplateAtomAttachmentPoint(v_k, m) == att_point_idx)
                               {
                                  QS_DEF(Array<char>, ap_id);
                                  getTemplateAtomAttachmentPointId(v_k, m, ap_id);
                                  _flipTemplateAtomAttachmentPoint(v_k, att_point_idx, ap_id, idx);
                               }
                            }
                         }
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

             sgs_to_remove.push(sgs[l]);
             added_template_occurs.push(idx);

             atoms_to_remove.concat(remove_atoms);
          }

          if (count_occur > 0)
             added_templates.push(i);

          for (int j = 0; j < sgs_to_remove.size(); j++)
          {
             SGroup &sg = sgroups.getSGroup(sgs_to_remove[j]);

             sgs.clear();
             sgroups.findSGroups(SGroup::SG_ATOMS, sg.atoms, sgs);

             for (int k= 0; k < sgs.size(); k++)
             {
                SGroup &sg_sim = sgroups.getSGroup(sgs[k]);
                if ( (sg_sim.sgroup_type == SGroup::SG_TYPE_SUP) && (sg_sim.atoms.size() == sg.atoms.size()) )
                {
                   sgroups.remove(sgs_to_remove[j]);
                }
                else
                {
                   sg_sim.atoms.push(added_template_occurs[j]);
                }
             }
          }

          if (atoms_to_remove.size() > 0)
             removeAtoms(atoms_to_remove);
       }
    */
    /*
       sgs.clear();
       sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_SUP, sgs);

       for (int l = 0; l < sgs.size(); l++)
       {
          int templ_idx;
          int templ_occ_ind = _transformSGroupToTGroup(sgs[l], templ_idx);   //  Try to transform this Superatom into template
          if (templ_occ_ind > -1)
          {
             new_templates.push(templ_idx);
             new_template_occurs.push(templ_occ_ind);
          }
       }
    */
    if (use_scsr_sgroups_only)
    {
        for (auto i = 0; i < added_templates.size(); i++)
        {
            addTemplate(templates.at(added_templates[i]));
        }
        return result;
    }

    new_templates.clear();
    new_template_occurs.clear();

    for (auto i = 0; i < templates.size(); i++)
    {
        const TGroup& tg = templates.at(i);

        fragment.clear();
        fragment.clone_KeepIndices(*tg.fragment.get());

        if (ignore_chem_templates)
        {
            if (((tg.tgroup_class.size() > 3) && strncmp(tg.tgroup_class.ptr(), "CHEM", 4) == 0) ||
                ((tg.tgroup_class.size() > 5) && strncmp(tg.tgroup_class.ptr(), "LINKER", 6) == 0) || fragment.vertexCount() < 6)
                continue;
        }

        if (arom)
            fragment.aromatize(arom_opt);

        sgs.clear();
        base_sgs.clear();
        fragment.sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", sgs);
        for (int j = fragment.sgroups.begin(); j != fragment.sgroups.end(); j = fragment.sgroups.next(j))
        {
            if (sgs.find(j) == -1)
                base_sgs.push(j);
        }

        ap_points_atoms.clear();
        ap_lgrp_atoms.clear();
        ap_points_ids.clear();
        ap_ids.clear();
        for (int j = 0; j < base_sgs.size(); j++)
        {
            SGroup& sg = fragment.sgroups.getSGroup(base_sgs[j]);
            if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                Superatom& su = (Superatom&)sg;

                if (su.attachment_points.size() > 0)
                {
                    for (int k = su.attachment_points.begin(); k < su.attachment_points.end(); k = su.attachment_points.next(k))
                    {
                        Superatom::_AttachmentPoint& ap = su.attachment_points.at(k);
                        ap_points_atoms.push(ap.aidx);
                        ap_lgrp_atoms.push(ap.lvidx);
                        ap_ids.push(ap_points_ids.add(ap.apid));
                    }
                }
            }
            else
                throw Error("Wrong template structure was found (base SGroup is not Superatom type)");
        }

        ignore_query_atoms.clear();
        for (int j = 0; j < sgs.size(); j++)
        {
            SGroup& sg = fragment.sgroups.getSGroup(sgs[j]);
            ignore_query_atoms.concat(sg.atoms);
        }

        int count_occur = 0;
        ignore_atoms.clear();

        int flags = MoleculeExactMatcher::CONDITION_ELECTRONS | // bond types, atom charges, valences, radicals must match
                    MoleculeExactMatcher::CONDITION_ISOTOPE |   // atom isotopes must match
                    MoleculeExactMatcher::CONDITION_STEREO;     // tetrahedral and cis-trans configurations must match

        bool unrestricted_search = false;

        for (;;)
        {
            MoleculeExactSubstructureMatcher matcher(fragment, this->asMolecule());

            for (int j = 0; j < ignore_atoms.size(); j++)
                matcher.ignoreTargetAtom(ignore_atoms[j]);

            for (int j = 0; j < ignore_query_atoms.size(); j++)
                matcher.ignoreQueryAtom(ignore_query_atoms[j]);

            matcher.flags = flags;

            if (!matcher.find())
            {
                if (flags == 0)
                    break;
                else
                {
                    flags = 0;
                    unrestricted_search = true;
                    continue;
                }
            }

            bool templ_found = false;
            do
            {
                mapping.clear();
                remove_atoms.clear();
                query_atoms.clear();
                mapping.copy(matcher.getQueryMapping(), fragment.vertexEnd());
                for (int j = 0; j < mapping.size(); j++)
                {
                    if (mapping[j] > -1)
                    {
                        query_atoms.push(j);
                        remove_atoms.push(mapping[j]);
                    }
                }

                att_atoms.clear();
                for (int l = 0; l < ap_points_atoms.size(); l++)
                {
                    int att_point_idx = mapping[ap_points_atoms[l]];
                    if (att_point_idx > -1)
                        att_atoms.push(att_point_idx);
                }

                int out_bonds = 0;
                int used_att_points = 0;
                bool wrong_xbond_order = false;
                for (int j = 0; j < remove_atoms.size(); j++)
                {
                    bool ap_used = false;
                    const Vertex& v = getVertex(remove_atoms[j]);
                    for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                    {
                        if (remove_atoms.find(v.neiVertex(k)) == -1)
                        {
                            out_bonds++;
                            for (int l = 0; l < ap_points_atoms.size(); l++)
                            {
                                int att_point_idx = mapping[ap_points_atoms[l]];
                                if (att_point_idx == remove_atoms[j] && !ap_used)
                                {
                                    used_att_points += 1;
                                    ap_used = true;
                                    int q_xbond_idx = fragment.findEdgeIndex(ap_points_atoms[l], ap_lgrp_atoms[l]);
                                    int t_xbond_idx = findEdgeIndex(att_point_idx, v.neiVertex(k));
                                    if (fragment.getBondOrder(q_xbond_idx) != this->asMolecule().getBondOrder(t_xbond_idx))
                                        wrong_xbond_order = true;
                                }
                            }
                        }
                    }
                }
                if ((out_bonds > ap_points_atoms.size()) || (out_bonds > used_att_points) || wrong_xbond_order)
                    continue;

                bool charged = false;
                for (int j = 0; j < remove_atoms.size(); j++)
                {
                    if (this->asMolecule().getAtomCharge(remove_atoms[j]) != 0)
                        charged = true;
                }
                //            if (charged)
                //               continue;

                bool isotopic = false;
                for (int j = 0; j < remove_atoms.size(); j++)
                {
                    if (this->asMolecule().getAtomIsotope(remove_atoms[j]) != 0)
                    {
                        isotopic = true;
                    }
                }

                target.clear();
                target.makeSubmolecule(*this, remove_atoms, 0);

                if (charged || isotopic)
                {
                    for (auto j : target.vertices())
                    {
                        if (target.asMolecule().getAtomCharge(j) != 0)
                            target.asMolecule().setAtomCharge(j, 0);
                        if (isotopic)
                        {
                            if (target.asMolecule().getAtomIsotope(j) != 0)
                                target.asMolecule().setAtomIsotope(j, 0);
                        }
                    }
                }

                query.clear();
                query.makeSubmolecule(fragment, query_atoms, 0);

                indigo_inchi.saveMoleculeIntoInchi(target, inchi_target);
                indigo_inchi.saveMoleculeIntoInchi(query, inchi_query);

                int inchi_comp = inchi_target.memcmp(inchi_query);

                if (inchi_comp != 0)
                    continue;

                /*
                            bool wrong_bond_order = false;
                            for (int j = 0; j < remove_atoms.size(); j++)
                            {
                               const Vertex &v = getVertex(remove_atoms[j]);
                               for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                               {
                                  int neib_ind = remove_atoms.find(v.neiVertex(k));
                                  if (neib_ind != -1)
                                  {
                                      int q_xbond_idx = fragment.findEdgeIndex(query_atoms[j], query_atoms[neib_ind]);
                                      int t_xbond_idx = findEdgeIndex(remove_atoms[j], v.neiVertex(k));
                                      if (q_xbond_idx == -1)
                                         wrong_bond_order = true;
                                      else if (fragment.getBondOrder(q_xbond_idx) != this->asMolecule().getBondOrder(t_xbond_idx))
                                         wrong_bond_order = true;
                                  }
                               }
                            }
                            if (wrong_bond_order)
                               continue;
                */

                bool ap_closed = false;
                ap_neibs.clear();
                for (int l = 0; l < ap_points_atoms.size(); l++)
                {
                    int att_point_idx = mapping[ap_points_atoms[l]];
                    const Vertex& v = getVertex(att_point_idx);
                    for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                    {
                        if (att_atoms.find(v.neiVertex(k)) != -1)
                        {
                            ap_closed = true;
                        }
                        else if (ap_neibs.find(v.neiVertex(k)) != -1)
                        {
                            ap_closed = true;
                        }
                        else if (remove_atoms.find(v.neiVertex(k)) == -1)
                        {
                            ap_neibs.push(v.neiVertex(k));
                        }
                    }
                }
                if (ap_closed)
                    continue;

                bool lgrp_absent = false;
                for (int m = 0; m < ap_points_atoms.size(); m++)
                {
                    int att_point_idx = mapping[ap_points_atoms[m]];
                    const Vertex& v1 = getVertex(att_point_idx);
                    const Vertex& v2 = fragment.getVertex(ap_points_atoms[m]);
                    if (v1.degree() != v2.degree() && fragment.getAtomNumber(ap_lgrp_atoms[m]) != ELEM_H)
                        lgrp_absent = true;
                }
                if (lgrp_absent)
                    continue;

                bool y_delta_found = false;
                for (int j = 0; j < query_atoms.size(); j++)
                {
                    if (ap_points_atoms.find(query_atoms[j]) == -1)
                    {
                        const Vertex& v1 = getVertex(remove_atoms[j]);
                        const Vertex& v2 = fragment.getVertex(query_atoms[j]);
                        if (v1.degree() != v2.degree())
                        {
                            y_delta_found = true;
                            break;
                        }
                    }
                }
                if (y_delta_found)
                    continue;

                templ_found = true;

            } while (!templ_found && matcher.findNext());

            if (!templ_found)
            {
                ignore_atoms.concat(remove_atoms);
                continue;
            }
            else if (unrestricted_search)
            {
                sgs.clear();
                sgroups.findSGroups(SGroup::SG_ATOMS, remove_atoms, sgs);

                int sg_idx = -1;

                if (sgs.size() > 0)
                {
                    for (int j = 0; j < sgs.size(); j++)
                    {
                        SGroup& sg = sgroups.getSGroup(sgs[j]);
                        if ((sg.sgroup_type == SGroup::SG_TYPE_SUP) && (sg.atoms.size() == remove_atoms.size()))
                        {
                            Superatom& su = (Superatom&)sg;
                            sg_idx = sgs[j];
                            if (tg.tgroup_natreplace.size() > 1)
                                su.sa_natreplace.copy(tg.tgroup_natreplace);
                            else
                            {
                                su.sa_natreplace.copy(tg.tgroup_class);
                                su.sa_natreplace.pop();
                                su.sa_natreplace.push('/');
                                su.sa_natreplace.concat(tg.tgroup_alias);
                            }
                        }
                        else if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
                        {
                            bool skip_sg = false;
                            for (int m = 0; m < sg.atoms.size(); m++)
                            {
                                if (isTemplateAtom(sg.atoms[m]))
                                {
                                    skip_sg = true;
                                    break;
                                }
                            }
                            if (skip_sg)
                                continue;

                            sg_idx = _createSGroupFromFragment(remove_atoms, tg, mapping);
                        }
                    }
                }
                else
                {
                    bool skip_match = false;
                    for (int m = 0; m < remove_atoms.size(); m++)
                    {
                        if (isAtomBelongsSGroup(remove_atoms[m]))
                        {
                            skip_match = true;
                            break;
                        }
                    }
                    if (!skip_match)
                        sg_idx = _createSGroupFromFragment(remove_atoms, tg, mapping);
                }

                if (sg_idx != -1)
                {
                    int templ_idx = -1;
                    int templ_occ_ind = _transformSGroupToTGroup(sg_idx, templ_idx); //  Try to transform this Superatom into template
                    if (templ_occ_ind > -1)
                    {
                        new_templates.push(templ_idx);
                        new_template_occurs.push(templ_occ_ind);
                    }
                }
                ignore_atoms.concat(remove_atoms);
                continue;
            }

            int idx = this->asMolecule().addAtom(-1);
            this->asMolecule().setTemplateAtom(idx, tg.tgroup_name.ptr());
            this->asMolecule().setTemplateAtomClass(idx, tg.tgroup_class.ptr());

            count_occur++;

            for (int j = 0; j < ap_points_atoms.size(); j++)
            {
                int att_point_idx = mapping[ap_points_atoms[j]];
                if (remove_atoms.find(att_point_idx) != -1)
                {
                    const Vertex& v = getVertex(att_point_idx);
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
                        int v_k = neighbors[k];
                        if (findEdgeIndex(v_k, att_point_idx) != -1)
                        {
                            const Vertex& v_n = getVertex(v_k);
                            if (getAtomNumber(v_k) == ELEM_H)
                            {
                                remove_atoms.push(v_k);
                                continue;
                            }

                            if ((getAtomNumber(v_k) == ELEM_O && v_n.degree() == 1 && getAtomCharge(v_k) == 0) && getAtomNumber(att_point_idx) == ELEM_C)
                            {
                                remove_atoms.push(v_k);
                                continue;
                            }

                            if (findEdgeIndex(v_k, idx) == -1)
                                flipBond(v_k, att_point_idx, idx);
                            this->asMolecule().setTemplateAtomAttachmentOrder(idx, v_k, ap_points_ids.at(ap_ids[j]));
                            if (isTemplateAtom(v_k))
                            {
                                int ap_count = getTemplateAtomAttachmentPointsCount(v_k);
                                for (int m = 0; m < ap_count; m++)
                                {
                                    if (getTemplateAtomAttachmentPoint(v_k, m) == att_point_idx)
                                    {
                                        QS_DEF(Array<char>, ap_id);
                                        getTemplateAtomAttachmentPointId(v_k, m, ap_id);
                                        _flipTemplateAtomAttachmentPoint(v_k, att_point_idx, ap_id, idx);
                                    }
                                }
                            }
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

            sgs.clear();
            sgroups.findSGroups(SGroup::SG_ATOMS, remove_atoms, sgs);

            for (int j = 0; j < sgs.size(); j++)
            {
                SGroup& sg = sgroups.getSGroup(sgs[j]);
                if ((sg.sgroup_type == SGroup::SG_TYPE_SUP) && (sg.atoms.size() == remove_atoms.size()))
                {
                    sgroups.remove((sgs[j]));
                }
                else
                {
                    sg.atoms.push(idx);
                }
            }

            removeAtoms(remove_atoms);
        }

        if (count_occur > 0 && added_templates.find(i) == -1)
            added_templates.push(i);
    }

    for (auto i = 0; i < added_templates.size(); i++)
    {
        addTemplate(templates.at(added_templates[i]));
    }

    /*
       sgs.clear();
       sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_SUP, sgs);

       for (int l = 0; l < sgs.size(); l++)
       {
          SGroup &sg = sgroups.getSGroup(sgs[l]);
          bool skip_sg = false;
          for (int m = 0; m < sg.atoms.size(); m++)
          {
             if (isTemplateAtom(sg.atoms[m]))
             {
                skip_sg = true;
                break;
             }
          }
          if (skip_sg)
             continue;

          int templ_idx;
          int templ_occ_ind = _transformSGroupToTGroup(sgs[l], templ_idx);   //  Try to transform this Superatom into template
          if (templ_occ_ind > -1)
          {
             new_templates.push(templ_idx);
             new_template_occurs.push(templ_occ_ind);
          }
       }
    */

    if ((added_templates.size() + new_templates.size()) > 0)
    {
        _fillTemplateSeqIds();
    }

    if (new_templates.size() > 0)
    {
        for (auto i = 0; i < new_templates.size(); i++)
        {
            _transformTGroupToSGroup(new_template_occurs[i], new_templates[i]);
            tgroups.remove(new_templates[i]);
        }
    }

    if (expand_mod_templates)
    {
        QS_DEF(Array<int>, tinds);
        tinds.clear();

        for (auto i : vertices())
        {
            if (isTemplateAtom(i))
            {
                const Vertex& v = getVertex(i);
                for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                {
                    int v_k = v.neiVertex(k);
                    if (!isTemplateAtom(v_k))
                    {
                        const Vertex& v_n = getVertex(v_k);

                        if (getAtomNumber(v_k) == ELEM_H)
                            continue;

                        if (getAtomNumber(v_k) == ELEM_O && v_n.degree() == 1 && getAtomCharge(v_k) == 0)
                            continue;

                        sgs.clear();
                        sgroups.findSGroups(SGroup::SG_TYPE, SGroup::SG_TYPE_SUP, sgs);
                        bool mod_found = true;
                        for (int l = 0; l < sgs.size(); l++)
                        {
                            Superatom& su = (Superatom&)sgroups.getSGroup(sgs[l]);
                            if (su.atoms.find(v_k) != -1)
                            {
                                mod_found = false;
                            }
                        }
                        if (!mod_found)
                            continue;

                        tinds.push(i);
                        break;
                    }
                }
            }
        }

        for (auto i = 0; i < tinds.size(); i++)
        {
            _transformTGroupToSGroup(tinds[i], -1);
        }

        QS_DEF(Array<int>, rem_templ);
        QS_DEF(Array<char>, ta_desc);
        rem_templ.clear();
        ta_desc.clear();
        bool templ_used;
        for (auto i = tgroups.begin(); i != tgroups.end(); i = tgroups.next(i))
        {
            TGroup& tgroup = tgroups.getTGroup(i);
            templ_used = false;
            for (auto j : vertices())
            {
                if (isTemplateAtom(j))
                {
                    getAtomDescription(j, ta_desc);
                    if (tgroup.tgroup_name.memcmp(ta_desc) == 0)
                    {
                        templ_used = true;
                        break;
                    }
                }
            }
            if (!templ_used)
                rem_templ.push(i);
        }
        for (auto i = 0; i < rem_templ.size(); i++)
        {
            tgroups.remove(rem_templ[i]);
        }
    }
    /*
       StereocentersOptions stereochemistry_options;
       stereocenters.buildFromBonds(stereochemistry_options, 0);
       allene_stereo.buildFromBonds(stereochemistry_options.ignore_errors, 0);
       cis_trans.build(0);
    */
    if (arom)
        this->asMolecule().dearomatize(arom_opt);

    return result;
}

void BaseMolecule::_fillTemplateSeqIds()
{
    QS_DEF(Array<int>, ignored_vertices);
    QS_DEF(Array<int>, vertex_ranks);
    QS_DEF(Molecule, tmp);

    tmp.clear();
    tmp.clone_KeepIndices(*this);

    ignored_vertices.clear_resize(tmp.vertexEnd());
    ignored_vertices.zerofill();

    QS_DEF(Array<char>, left_apid);
    QS_DEF(Array<char>, right_apid);
    QS_DEF(Array<char>, xlink_apid);
    left_apid.readString("Al", true);
    right_apid.readString("Br", true);
    xlink_apid.readString("Cx", true);

    vertex_ranks.clear_resize(tmp.vertexEnd());
    vertex_ranks.zerofill();

    for (auto i : tmp.vertices())
        vertex_ranks[i] = i;

    for (auto i : tmp.vertices())
    {
        if (!tmp.isTemplateAtom(i))
        {
            ignored_vertices[i] = 1;
        }
        else
        {
            int left_neib = tmp.getTemplateAtomAttachmentPointById(i, left_apid);
            int right_neib = tmp.getTemplateAtomAttachmentPointById(i, right_apid);

            if (left_neib == -1)
            {
                vertex_ranks[i] = -2;
            }
            else if (!tmp.isTemplateAtom(left_neib) && (right_neib != -1))
            {
                vertex_ranks[i] = -1;
            }

            int xlink_neib = tmp.getTemplateAtomAttachmentPointById(i, xlink_apid);
            if (xlink_neib > -1)
            {
                int eidx = tmp.findEdgeIndex(i, xlink_neib);
                if (eidx > -1)
                    tmp.removeEdge(eidx);
            }
        }
    }

    DfsWalk walk(tmp);

    walk.ignored_vertices = ignored_vertices.ptr();
    walk.vertex_ranks = vertex_ranks.ptr();

    walk.walk();

    const Array<DfsWalk::SeqElem>& v_seq = walk.getSequence();

    QS_DEF(Array<int>, branch_counters);
    QS_DEF(Array<int>, cycle_numbers);
    QS_DEF(Array<int>, atom_sequence);

    branch_counters.clear_resize(tmp.vertexEnd());
    branch_counters.zerofill();
    cycle_numbers.clear();
    atom_sequence.clear();

    /* first template */
    if (v_seq.size() > 0)
    {
        atom_sequence.push(v_seq[0].idx);

        int j, openings = walk.numOpenings(v_seq[0].idx);

        for (j = 0; j < openings; j++)
        {
            cycle_numbers.push(v_seq[0].idx);
        }
    }

    int i, j, k;

    for (i = 1; i < v_seq.size(); i++)
    {
        int v_idx = v_seq[i].idx;
        int e_idx = v_seq[i].parent_edge;
        int v_prev_idx = v_seq[i].parent_vertex;
        bool write_template = true;

        if (v_prev_idx >= 0)
        {
            int branches = walk.numBranches(v_prev_idx);

            branch_counters[v_prev_idx]++;

            if (branch_counters[v_prev_idx] > branches)
                throw Error("unexpected branch");

            if (walk.isClosure(e_idx))
            {
                for (j = 0; j < cycle_numbers.size(); j++)
                    if (cycle_numbers[j] == v_idx)
                        break;

                if (j == cycle_numbers.size())
                    throw Error("cycle number not found");

                cycle_numbers[j] = -1;
                write_template = false;
            }
        }

        if (write_template)
        {
            atom_sequence.push(v_idx);

            int openings = walk.numOpenings(v_idx);

            for (j = 0; j < openings; j++)
            {
                for (k = 0; k < cycle_numbers.size(); k++)
                    if (cycle_numbers[k] == -1)
                        break;
                if (k == cycle_numbers.size())
                    cycle_numbers.push(v_idx);
                else
                    cycle_numbers[k] = v_idx;
            }
        }
    }

    int seq_id = 1;
    for (i = 0; i < atom_sequence.size(); i++)
    {
        int v_idx = atom_sequence[i];
        this->asMolecule().setTemplateAtomSeqid(v_idx, seq_id);
        seq_id += 1;
    }
}

int BaseMolecule::addTemplate(TGroup& tgroup)
{
    int idx = tgroups.addTGroup();
    (tgroups.getTGroup(idx)).copy(tgroup);
    return idx;
}

bool BaseMolecule::isAtomBelongsSGroup(int idx)
{
    QS_DEF(Array<int>, sgs);
    QS_DEF(Array<int>, atoms);

    sgs.clear();
    atoms.clear();
    atoms.push(idx);
    sgroups.findSGroups(SGroup::SG_ATOMS, atoms, sgs);
    return sgs.size() > 0;
}

int BaseMolecule::_transformTGroupToSGroup(int idx, int t_idx)
{
    int result = 0;
    QS_DEF(Molecule, fragment);
    QS_DEF(Array<int>, sgs);
    QS_DEF(Array<int>, base_sgs);
    QS_DEF(Array<int>, mapping);
    QS_DEF(Array<int>, att_atoms);
    QS_DEF(Array<int>, tg_atoms);
    QS_DEF(Array<int>, lvgroups);
    QS_DEF(StringPool, ap_points_ids);
    QS_DEF(Array<int>, ap_ids);
    QS_DEF(Array<char>, ap_id);

    int tg_idx = t_idx;
    if (t_idx == -1)
        tg_idx = tgroups.findTGroup(getTemplateAtom(idx));

    TGroup& tgroup = tgroups.getTGroup(tg_idx);
    fragment.clear();
    fragment.clone_KeepIndices(*tgroup.fragment.get());

    sgs.clear();
    att_atoms.clear();
    tg_atoms.clear();
    lvgroups.clear();
    base_sgs.clear();
    ap_points_ids.clear();
    ap_ids.clear();

    fragment.sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", sgs);
    for (int j = fragment.sgroups.begin(); j != fragment.sgroups.end(); j = fragment.sgroups.next(j))
    {
        if (sgs.find(j) == -1)
            base_sgs.push(j);
    }

    if (base_sgs.size() == 0)
        throw Error("transformTGroupToSGroup(): wrong template structure found (no base SGroup detected)");

    if (base_sgs.size() > 1)
        throw Error("transformTGroupToSGroup(): wrong template structure found (more then one base SGroup detected)");

    SGroup& sg = fragment.sgroups.getSGroup(base_sgs[0]);
    if (sg.sgroup_type != SGroup::SG_TYPE_SUP)
        throw Error("transformTGroupToSGroup(): wrong template structure found (base SGroup is not Superatom type)");

    Superatom& su = (Superatom&)sg;

    // printf("Template = %s (%d)\n", tgroup.tgroup_name.ptr(), idx);

    if (su.attachment_points.size() > 0)
    {
        for (int j = su.attachment_points.begin(); j < su.attachment_points.end(); j = su.attachment_points.next(j))
        {
            Superatom::_AttachmentPoint& ap = su.attachment_points.at(j);

            int att_atom_idx = getTemplateAtomAttachmentPointById(idx, ap.apid);
            if (att_atom_idx > -1)
            {
                att_atoms.push(att_atom_idx);
                tg_atoms.push(ap.aidx);
                lvgroups.push(ap.lvidx);
                ap_ids.push(ap_points_ids.add(ap.apid));

                // printf("idx = %d, att_atom_idx = %d, ap.aidx = %d, ap.lvidx = %d, ap_id = %s\n", idx, att_atom_idx, ap.aidx, ap.lvidx, ap.apid.ptr());
            }
        }
    }

    for (const auto sg_index : sgs)
    {
        const SGroup& lvg = fragment.sgroups.getSGroup(sg_index);
        for (const auto lvgroup_index : lvgroups)
        {
            if (lvg.atoms.find(lvgroup_index) > -1)
            {
                fragment.removeSGroupWithBasis(sg_index);
                if (!fragment.sgroups.hasSGroup(sg_index))
                {
                    break;
                }
            }
        }
    }

    mergeWithMolecule(fragment, &mapping);

    for (auto i : fragment.vertices())
    {
        int aidx = mapping[i];
        if (aidx > -1)
        {
            setAtomXyz(aidx, getAtomXyz(idx));
        }
    }

    QS_DEF(Array<int>, added_atoms);

    added_atoms.clear();
    for (auto i = 0; i < su.atoms.size(); i++)
    {
        int aidx = mapping[su.atoms[i]];
        if (aidx > -1)
        {
            added_atoms.push(aidx);
        }
    }

    base_sgs.clear();
    sgroups.findSGroups(SGroup::SG_ATOMS, added_atoms, base_sgs);
    if (base_sgs.size() == 1)
    {
        SGroup& sg = sgroups.getSGroup(base_sgs[0]);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            Superatom& su = (Superatom&)sg;
            su.seqid = getTemplateAtomSeqid(idx);
            su.sa_natreplace.copy(tgroup.tgroup_natreplace);

            for (int i = 0; i < att_atoms.size(); i++)
            {
                if (findEdgeIndex(att_atoms[i], idx) > -1)
                {
                    // printf("Flip bond = %d, att_atom[i] = %d, tg_atoms[i] = %d, mapping[tg_atoms[i]] = %d\n", findEdgeIndex(att_atoms[i], idx), att_atoms[i],
                    // tg_atoms[i], mapping[tg_atoms[i]]);
                    flipBond(att_atoms[i], idx, mapping[tg_atoms[i]]);
                }
                else if (isTemplateAtom(att_atoms[i]))
                {
                    int ap_count = getTemplateAtomAttachmentPointsCount(att_atoms[i]);
                    for (int m = 0; m < ap_count; m++)
                    {
                        if (getTemplateAtomAttachmentPoint(att_atoms[i], m) == idx)
                        {
                            getTemplateAtomAttachmentPointId(att_atoms[i], m, ap_id);
                            int added_bond = this->asMolecule().addBond(att_atoms[i], mapping[tg_atoms[i]], BOND_SINGLE);
                            (void)added_bond;
                            // printf("Add bond = %d, att_atom[i] = %d, tg_atoms[i] = %d, mapping[tg_atoms[i]] = %d\n", added_bond, att_atoms[i], tg_atoms[i],
                            // mapping[tg_atoms[i]]); printf("Flip AP  att_atom[i] = %d, tg_atoms[i] = %d, mapping[tg_atoms[i]] = %d, ap_id = %s\n",
                            // att_atoms[i], tg_atoms[i], mapping[tg_atoms[i]], ap_id.ptr());
                            _flipTemplateAtomAttachmentPoint(att_atoms[i], idx, ap_id, mapping[tg_atoms[i]]);
                        }
                    }
                    //               int added_bond = this->asMolecule().addBond(att_atoms[i], mapping[tg_atoms[i]], BOND_SINGLE);
                    // printf("Add bond = %d, att_atom[i] = %d, tg_atoms[i] = %d, mapping[tg_atoms[i]] = %d\n", added_bond, att_atoms[i], tg_atoms[i],
                    // mapping[tg_atoms[i]]);
                }

                if (isTemplateAtom(att_atoms[i]))
                {
                    int ap_count = getTemplateAtomAttachmentPointsCount(att_atoms[i]);
                    for (int m = 0; m < ap_count; m++)
                    {
                        if (getTemplateAtomAttachmentPoint(att_atoms[i], m) == idx)
                        {
                            getTemplateAtomAttachmentPointId(att_atoms[i], m, ap_id);
                            // printf("Flip AP  att_atom[i] = %d, tg_atoms[i] = %d, mapping[tg_atoms[i]] = %d, ap_id = %s\n", att_atoms[i], tg_atoms[i],
                            // mapping[tg_atoms[i]], ap_id.ptr());
                            _flipTemplateAtomAttachmentPoint(att_atoms[i], idx, ap_id, mapping[tg_atoms[i]]);
                            break;
                        }
                    }
                }

                int bond_idx = findEdgeIndex(att_atoms[i], mapping[tg_atoms[i]]);
                if (bond_idx > -1)
                {
                    su.bonds.push(bond_idx);

                    for (int j = su.attachment_points.begin(); j < su.attachment_points.end(); j = su.attachment_points.next(j))
                    {
                        Superatom::_AttachmentPoint& ap = su.attachment_points.at(j);

                        // printf("SUP AP  ap.aidx = %d, tg_atoms[i] = %d, mapping[tg_atoms[i]] = %d\n", ap.aidx, tg_atoms[i], mapping[tg_atoms[i]]);

                        if (ap.aidx == mapping[tg_atoms[i]])
                            ap.lvidx = att_atoms[i];
                    }
                }
            }
        }
    }

    QS_DEF(Array<int>, templ_atoms);
    templ_atoms.clear();
    templ_atoms.push(idx);

    sgs.clear();
    sgroups.findSGroups(SGroup::SG_ATOMS, templ_atoms, sgs);

    for (int i = 0; i < sgs.size(); i++)
    {
        SGroup& sg = sgroups.getSGroup(sgs[i]);
        sg.atoms.concat(added_atoms);
    }

    removeAtom(idx);

    return result;
}

int BaseMolecule::_transformSGroupToTGroup(int sg_idx, int& tg_idx)
{
    QS_DEF(Array<int>, remove_atoms);
    QS_DEF(Array<int>, sg_atoms);
    QS_DEF(Array<int>, mapping);
    QS_DEF(Array<int>, ap_points_atoms);
    QS_DEF(StringPool, ap_points_ids);
    QS_DEF(Array<int>, ap_ids);
    QS_DEF(Array<int>, sgs);
    QS_DEF(Array<int>, sgs_tmp);

    mapping.clear();

    if (!sgroups.hasSGroup(sg_idx))
        return -1;

    Superatom& su = (Superatom&)sgroups.getSGroup(sg_idx);

    ap_points_atoms.clear();
    ap_points_ids.clear();
    ap_ids.clear();

    if (su.attachment_points.size() == 0 && su.bonds.size() == 0)
        return -1;

    if (su.attachment_points.size() > 0)
    {
        for (int k = su.attachment_points.begin(); k < su.attachment_points.end(); k = su.attachment_points.next(k))
        {
            Superatom::_AttachmentPoint& ap = su.attachment_points.at(k);
            ap_points_atoms.push(ap.aidx);
            ap_ids.push(ap_points_ids.add(ap.apid));
        }
    }
    else // Try to create attachment points from crossing bond information
    {
        for (int k = 0; k < su.bonds.size(); k++)
        {
            const Edge& edge = getEdge(su.bonds[k]);
            int ap_aidx = -1;
            int ap_lvidx = -1;
            if (su.atoms.find(edge.beg) != -1)
            {
                ap_aidx = edge.beg;
                ap_lvidx = edge.end;
            }
            else if (su.atoms.find(edge.end) != -1)
            {
                ap_aidx = edge.end;
                ap_lvidx = edge.beg;
            }
            else // Crossing bond connects atoms out of Sgroup?
            {
                continue;
            }

            int idap = su.attachment_points.add();
            Superatom::_AttachmentPoint& ap = su.attachment_points.at(idap);

            if (_isNTerminus(su, ap_aidx)) //  N-terminus ?
                ap.apid.readString("Al", true);
            else if (_isCTerminus(su, ap_aidx)) //  C-terminus ?
                ap.apid.readString("Br", true);
            else
                ap.apid.readString("Cx", true);

            ap.aidx = ap_aidx;
            ap.lvidx = ap_lvidx;

            ap_points_atoms.push(ap.aidx);
            ap_ids.push(ap_points_ids.add(ap.apid));
        }
    }

    if (su.sa_class.size() == 0)
        return -1;
    else if (strncmp(su.sa_class.ptr(), "AA", 2) != 0)
        return -1;

    tg_idx = this->tgroups.addTGroup();
    TGroup& tg = this->tgroups.getTGroup(tg_idx);
    tg.tgroup_id = tg_idx;

    tg.tgroup_class.copy(su.sa_class);

    if (su.subscript.size() > 0)
        tg.tgroup_name.copy(su.subscript);
    tg.tgroup_alias.clear();
    tg.tgroup_comment.clear();
    if (su.sa_natreplace.size() > 0)
        tg.tgroup_natreplace.copy(su.sa_natreplace);

    tg.fragment.reset(this->neu());
    tg.fragment->makeSubmolecule(*this, su.atoms, &mapping, SKIP_TGROUPS | SKIP_TEMPLATE_ATTACHMENT_POINTS);

    sg_atoms.clear();
    for (int j = 0; j < su.atoms.size(); j++)
    {
        if (mapping[su.atoms[j]] != -1)
            sg_atoms.push(mapping[su.atoms[j]]);
    }

    sgs.clear();
    tg.fragment->sgroups.findSGroups(SGroup::SG_ATOMS, sg_atoms, sgs);

    int new_sg_idx = -1;
    for (int j = 0; j < sgs.size(); j++)
    {
        SGroup& sg = tg.fragment->sgroups.getSGroup(sgs[j]);
        if ((sg.sgroup_type != SGroup::SG_TYPE_SUP) || (sg.atoms.size() != su.atoms.size()))
        {
            tg.fragment->sgroups.remove((sgs[j]));
        }
        else
        {
            Superatom& sup_new = (Superatom&)sg;
            if ((strcmp(su.subscript.ptr(), sup_new.subscript.ptr()) == 0) && (su.attachment_points.size() == sup_new.attachment_points.size()))
            {
                new_sg_idx = sgs[j];
            }
        }
    }

    sgs.clear();
    for (int j = tg.fragment->sgroups.begin(); j != tg.fragment->sgroups.end(); j = tg.fragment->sgroups.next(j))
    {
        if (j != new_sg_idx)
            sgs.push(j);
    }

    for (int j = 0; j < sgs.size(); j++)
    {
        tg.fragment->sgroups.remove((sgs[j]));
    }

    int idx = this->asMolecule().addAtom(-1);
    this->asMolecule().setTemplateAtom(idx, tg.tgroup_name.ptr());
    this->asMolecule().setTemplateAtomClass(idx, tg.tgroup_class.ptr());

    for (int j = 0; j < ap_points_atoms.size(); j++)
    {
        int att_point_idx = ap_points_atoms[j];
        if (su.atoms.find(att_point_idx) != -1)
        {
            const Vertex& v = getVertex(att_point_idx);
            QS_DEF(Array<int>, neighbors);
            neighbors.clear();
            for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
            {
                if (su.atoms.find(v.neiVertex(k)) == -1)
                {
                    neighbors.push(v.neiVertex(k));
                }
            }

            for (int k = 0; k < neighbors.size(); k++)
            {
                int v_k = neighbors[k];
                if (findEdgeIndex(v_k, att_point_idx) != -1)
                {
                    if (findEdgeIndex(v_k, idx) == -1)
                        flipBond(v_k, att_point_idx, idx);
                    this->asMolecule().setTemplateAtomAttachmentOrder(idx, v_k, ap_points_ids.at(ap_ids[j]));
                    if (isTemplateAtom(v_k))
                    {
                        int ap_count = getTemplateAtomAttachmentPointsCount(v_k);
                        for (int m = 0; m < ap_count; m++)
                        {
                            if (getTemplateAtomAttachmentPoint(v_k, m) == att_point_idx)
                            {
                                QS_DEF(Array<char>, ap_id);
                                getTemplateAtomAttachmentPointId(v_k, m, ap_id);
                                _flipTemplateAtomAttachmentPoint(v_k, att_point_idx, ap_id, idx);
                            }
                        }
                    }
                }
            }
        }
    }

    QS_DEF(Vec2f, cp);
    QS_DEF(Vec3f, p);
    p.set(0, 0, 0);
    getAtomsCenterPoint(su.atoms, cp);
    p.x = cp.x;
    p.y = cp.y;
    setAtomXyz(idx, p);

    remove_atoms.copy(su.atoms);

    sgs.clear();
    sgroups.findSGroups(SGroup::SG_ATOMS, su.atoms, sgs);

    for (int j = 0; j < sgs.size(); j++)
    {
        SGroup& sg = sgroups.getSGroup(sgs[j]);
        if ((sg.sgroup_type == SGroup::SG_TYPE_SUP) && (sg.atoms.size() == su.atoms.size()))
        {
            sgroups.remove((sgs[j]));
        }
        else
        {
            sg.atoms.push(idx);
        }
    }

    removeAtoms(remove_atoms);

    return idx;
}

bool BaseMolecule::_isCTerminus(Superatom& su, int idx)
{
    if (getAtomNumber(idx) != ELEM_C)
        return false;

    QS_DEF(Array<int>, mapping);
    BufferScanner sc("[#7]-[#6]-[#6]=O");
    SmilesLoader loader(sc);
    QS_DEF(QueryMolecule, aminoacid);
    loader.loadSMARTS(aminoacid);

    MoleculeSubstructureMatcher matcher(this->asMolecule());
    matcher.setQuery(aminoacid);

    for (auto i : vertices())
    {
        if (su.atoms.find(i) == -1)
            matcher.ignoreTargetAtom(i);
    }

    if (!matcher.find())
        return false;

    mapping.clear();
    mapping.copy(matcher.getQueryMapping(), aminoacid.vertexEnd());

    if (mapping.find(idx) == -1)
        return false;
    else
        return true;
}

bool BaseMolecule::_isNTerminus(Superatom& su, int idx)
{
    if (getAtomNumber(idx) != ELEM_N)
        return false;

    QS_DEF(Array<int>, mapping);
    BufferScanner sc("[#7]-[#6]-[#6]=O");
    SmilesLoader loader(sc);
    QS_DEF(QueryMolecule, aminoacid);
    loader.loadSMARTS(aminoacid);

    MoleculeSubstructureMatcher matcher(this->asMolecule());
    matcher.setQuery(aminoacid);

    for (auto i : vertices())
    {
        if (su.atoms.find(i) == -1)
            matcher.ignoreTargetAtom(i);
    }

    if (!matcher.find())
        return false;

    mapping.clear();
    mapping.copy(matcher.getQueryMapping(), aminoacid.vertexEnd());

    if (mapping.find(idx) == -1)
        return false;
    else
        return true;
}

int BaseMolecule::_createSGroupFromFragment(Array<int>& sg_atoms, const TGroup& tg, Array<int>& mapping)
{
    QS_DEF(Molecule, fragment);
    QS_DEF(Array<int>, sgs);
    QS_DEF(Array<int>, base_sgs);
    QS_DEF(Array<int>, att_atoms);
    QS_DEF(Array<int>, tg_atoms);
    QS_DEF(Array<int>, lvgroups);
    QS_DEF(StringPool, ap_points_ids);
    QS_DEF(Array<int>, ap_ids);
    QS_DEF(Array<char>, ap_id);

    fragment.clear();
    fragment.clone_KeepIndices(*tg.fragment.get());

    sgs.clear();
    att_atoms.clear();
    tg_atoms.clear();
    lvgroups.clear();
    base_sgs.clear();
    ap_points_ids.clear();
    ap_ids.clear();

    fragment.sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", sgs);
    for (int j = fragment.sgroups.begin(); j != fragment.sgroups.end(); j = fragment.sgroups.next(j))
    {
        if (sgs.find(j) == -1)
            base_sgs.push(j);
    }

    if (base_sgs.size() == 0)
        throw Error("_createSGroupFromFragment(): wrong template structure found (no base SGroup detected)");

    if (base_sgs.size() > 1)
        throw Error("_createSGroupFromFragment(): wrong template structure found (more then one base SGroup detected)");

    SGroup& sg = fragment.sgroups.getSGroup(base_sgs[0]);
    if (sg.sgroup_type != SGroup::SG_TYPE_SUP)
        throw Error("_createSGroupFromFragment(): wrong template structure found (base SGroup is not Superatom type)");

    Superatom& su = (Superatom&)sg;

    if (su.attachment_points.size() > 0)
    {
        for (int j = su.attachment_points.begin(); j < su.attachment_points.end(); j = su.attachment_points.next(j))
        {
            Superatom::_AttachmentPoint& ap = su.attachment_points.at(j);

            tg_atoms.push(ap.aidx);
            lvgroups.push(ap.lvidx);
            ap_ids.push(ap_points_ids.add(ap.apid));
        }
    }

    int new_sg_idx = sgroups.addSGroup("SUP");
    Superatom& su_new = (Superatom&)sgroups.getSGroup(new_sg_idx);

    su_new.atoms.copy(sg_atoms);
    su_new.subscript.copy(tg.tgroup_name);
    su_new.sa_class.copy(tg.tgroup_class);
    su_new.sa_natreplace.copy(tg.tgroup_natreplace);

    for (auto j = 0; j < sg_atoms.size(); j++)
    {
        const Vertex& v = getVertex(sg_atoms[j]);
        for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
        {
            if (sg_atoms.find(v.neiVertex(k)) == -1)
            {
                for (int l = 0; l < tg_atoms.size(); l++)
                {
                    int att_point_idx = mapping[tg_atoms[l]];
                    if (att_point_idx != -1 && sg_atoms[j] == att_point_idx)
                    {
                        int q_xbond_idx = fragment.findEdgeIndex(tg_atoms[l], lvgroups[l]);
                        int t_xbond_idx = findEdgeIndex(att_point_idx, v.neiVertex(k));
                        if (fragment.getBondOrder(q_xbond_idx) == this->asMolecule().getBondOrder(t_xbond_idx))
                        {
                            su_new.bonds.push(t_xbond_idx);
                            int idap = su_new.attachment_points.add();
                            Superatom::_AttachmentPoint& ap = su_new.attachment_points.at(idap);
                            ap.aidx = att_point_idx;
                            ap.lvidx = v.neiVertex(k);
                            ap.apid.readString(ap_points_ids.at(l), true);
                        }
                    }
                }
            }
        }
    }

    return new_sg_idx;
}

void BaseMolecule::_removeAtomsFromSGroup(SGroup& sgroup, Array<int>& mapping)
{
    int i;

    for (i = sgroup.atoms.size() - 1; i >= 0; i--)
    {
        if (mapping[sgroup.atoms[i]] == -1)
            sgroup.atoms.remove(i);
    }
    for (i = sgroup.bonds.size() - 1; i >= 0; i--)
    {
        const Edge& edge = getEdge(sgroup.bonds[i]);
        if (mapping[edge.beg] == -1 || mapping[edge.end] == -1)
            sgroup.bonds.remove(i);
    }
    updateEditRevision();
}

void BaseMolecule::_removeAtomsFromMultipleGroup(MultipleGroup& mg, Array<int>& mapping)
{
    int i;

    for (i = mg.parent_atoms.size() - 1; i >= 0; i--)
    {
        if (mapping[mg.parent_atoms[i]] == -1)
            mg.parent_atoms.remove(i);
    }
    updateEditRevision();
}

void BaseMolecule::_removeAtomsFromSuperatom(Superatom& sa, Array<int>& mapping)
{

    if (sa.bond_connections.size() > 0)
    {
        for (int j = sa.bond_connections.size() - 1; j >= 0; j--)
        {
            Superatom::_BondConnection& bond = sa.bond_connections[j];
            const Edge& edge = getEdge(bond.bond_idx);
            if (mapping[edge.beg] == -1 || mapping[edge.end] == -1)
                sa.bond_connections.remove(j);
        }
    }
    if (sa.attachment_points.size() > 0)
    {
        for (int j = sa.attachment_points.begin(); j < sa.attachment_points.end(); j = sa.attachment_points.next(j))
        {
            Superatom::_AttachmentPoint& ap = sa.attachment_points.at(j);
            if (ap.aidx >= 0 && mapping[ap.aidx] == -1)
                sa.attachment_points.remove(j);
            else if (ap.lvidx >= 0 && mapping[ap.lvidx] == -1)
                ap.lvidx = -1;
        }
    }
    updateEditRevision();
}

void BaseMolecule::_removeBondsFromSGroup(SGroup& sgroup, Array<int>& mapping)
{
    int i;

    for (i = sgroup.bonds.size() - 1; i >= 0; i--)
    {
        if (mapping[sgroup.bonds[i]] == -1)
            sgroup.bonds.remove(i);
    }
    updateEditRevision();
}

void BaseMolecule::_removeBondsFromSuperatom(Superatom& sa, Array<int>& mapping)
{
    if (sa.bond_connections.size() > 0)
    {
        for (int j = sa.bond_connections.size() - 1; j >= 0; j--)
        {
            Superatom::_BondConnection& bond = sa.bond_connections[j];
            if (mapping[bond.bond_idx] == -1)
                sa.bond_connections.remove(j);
        }
    }
    updateEditRevision();
}

bool BaseMolecule::_mergeSGroupWithSubmolecule(SGroup& sgroup, SGroup& super, BaseMolecule& supermol, Array<int>& mapping, Array<int>& edge_mapping)
{
    int i;
    bool merged = false;
    sgroup.parent_group = super.parent_group;
    sgroup.sgroup_subtype = super.sgroup_subtype;
    sgroup.brackets.copy(super.brackets);

    QS_DEF(Array<int>, parent_atoms);
    parent_atoms.clear();
    if (supermol.sgroups.getParentAtoms(super, parent_atoms))
    {
        // parent exists
        for (i = 0; i < parent_atoms.size(); i++)
        {
            if (mapping[parent_atoms[i]] >= 0)
            {
                merged = true;
            }
        }
    }

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
        const Edge& edge = supermol.getEdge(super.bonds[i]);

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

void BaseMolecule::unhighlightAll()
{
    _hl_atoms.clear();
    _hl_bonds.clear();
    updateEditRevision();
}

void BaseMolecule::unselectAll()
{
    _sl_atoms.clear();
    _sl_bonds.clear();
    updateEditRevision();
}

void BaseMolecule::highlightAtom(int idx)
{
    _hl_atoms.expandFill(idx + 1, 0);
    _hl_atoms[idx] = 1;
    updateEditRevision();
}

void BaseMolecule::selectAtom(int idx)
{
    _sl_atoms.expandFill(idx + 1, 0);
    _sl_atoms[idx] = 1;
    updateEditRevision();
}

void BaseMolecule::highlightBond(int idx)
{
    _hl_bonds.expandFill(idx + 1, 0);
    _hl_bonds[idx] = 1;
    updateEditRevision();
}

void BaseMolecule::selectBond(int idx)
{
    _sl_bonds.expandFill(idx + 1, 0);
    _sl_bonds[idx] = 1;
    updateEditRevision();
}

void BaseMolecule::highlightAtoms(const Filter& filter)
{
    int i;

    for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        if (filter.valid(i))
            highlightAtom(i);
    updateEditRevision();
}

void BaseMolecule::selectAtoms(const Filter& filter)
{
    int i;

    for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        if (filter.valid(i))
            selectAtom(i);
    updateEditRevision();
}

void BaseMolecule::highlightBonds(const Filter& filter)
{
    int i;

    for (i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
        if (filter.valid(i))
            highlightBond(i);
    updateEditRevision();
}

void BaseMolecule::selectBonds(const Filter& filter)
{
    int i;

    for (i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
        if (filter.valid(i))
            selectBond(i);
    updateEditRevision();
}

void BaseMolecule::unhighlightAtom(int idx)
{
    if (_hl_atoms.size() > idx)
    {
        _hl_atoms[idx] = 0;
        updateEditRevision();
    }
}

void BaseMolecule::unselectAtom(int idx)
{
    if (_sl_atoms.size() > idx)
    {
        _sl_atoms[idx] = 0;
        updateEditRevision();
    }
}

void BaseMolecule::unhighlightBond(int idx)
{
    if (_hl_bonds.size() > idx)
    {
        _hl_bonds[idx] = 0;
        updateEditRevision();
    }
}

void BaseMolecule::unselectBond(int idx)
{
    if (_sl_bonds.size() > idx)
    {
        _sl_bonds[idx] = 0;
        updateEditRevision();
    }
}

int BaseMolecule::countHighlightedAtoms()
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

int BaseMolecule::countSelectedAtoms()
{
    int i, res = 0;

    for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
    {
        if (i >= _sl_atoms.size())
            break;
        res += _sl_atoms[i];
    }

    return res;
}

void BaseMolecule::getAtomSelection(std::set<int>& selection)
{
    selection.clear();
    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
    {
        if (i >= _sl_atoms.size())
            break;
        if (_sl_atoms[i])
            selection.insert(i);
    }
}

int BaseMolecule::countHighlightedBonds()
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

int BaseMolecule::countSelectedBonds()
{
    int i, res = 0;

    for (i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
    {
        if (i >= _sl_bonds.size())
            break;
        res += _sl_bonds[i];
    }

    return res;
}

bool BaseMolecule::hasHighlighting()
{
    return countHighlightedAtoms() > 0 || countHighlightedBonds() > 0;
}

bool BaseMolecule::hasSelection()
{
    return countSelectedAtoms() > 0 || countSelectedBonds() > 0;
}

bool BaseMolecule::isAtomHighlighted(int idx)
{
    return _hl_atoms.size() > idx && _hl_atoms[idx] == 1;
}

bool BaseMolecule::isAtomSelected(int idx)
{
    return _sl_atoms.size() > idx && _sl_atoms[idx] == 1;
}

bool BaseMolecule::isBondHighlighted(int idx)
{
    return _hl_bonds.size() > idx && _hl_bonds[idx] == 1;
}

bool BaseMolecule::isBondSelected(int idx)
{
    return _sl_bonds.size() > idx && _sl_bonds[idx] == 1;
}

void BaseMolecule::highlightSubmolecule(BaseMolecule& subgraph, const int* mapping, bool entire)
{
    int i;

    for (i = subgraph.vertexBegin(); i != subgraph.vertexEnd(); i = subgraph.vertexNext(i))
        if (mapping[i] >= 0 && (entire || subgraph.isAtomHighlighted(i)))
            highlightAtom(mapping[i]);

    for (i = subgraph.edgeBegin(); i != subgraph.edgeEnd(); i = subgraph.edgeNext(i))
    {
        if (!entire && !subgraph.isBondHighlighted(i))
            continue;

        const Edge& edge = subgraph.getEdge(i);

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

void BaseMolecule::selectSubmolecule(BaseMolecule& subgraph, const int* mapping, bool entire)
{
    int i;

    for (i = subgraph.vertexBegin(); i != subgraph.vertexEnd(); i = subgraph.vertexNext(i))
        if (mapping[i] >= 0 && (entire || subgraph.isAtomSelected(i)))
            selectAtom(mapping[i]);

    for (i = subgraph.edgeBegin(); i != subgraph.edgeEnd(); i = subgraph.edgeNext(i))
    {
        if (!entire && !subgraph.isBondSelected(i))
            continue;

        const Edge& edge = subgraph.getEdge(i);

        int beg = mapping[edge.beg];
        int end = mapping[edge.end];

        if (beg >= 0 && end >= 0)
        {
            int edge_idx = findEdgeIndex(beg, end);
            if (edge_idx >= 0)
                selectBond(edge_idx);
        }
    }
}

int BaseMolecule::countSGroups()
{
    return sgroups.getSGroupCount();
}

void BaseMolecule::getAttachmentIndicesForAtom(int atom_idx, Array<int>& res)
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

int BaseMolecule::getEditRevision()
{
    return _edit_revision;
}

void BaseMolecule::updateEditRevision()
{
    _edit_revision++;
}

int BaseMolecule::getChiralFlag()
{
    return _chiral_flag;
}

void BaseMolecule::setChiralFlag(int flag)
{
    _chiral_flag = flag;
}

int BaseMolecule::getBondDirection(int idx) const
{
    if (idx > _bond_directions.size() - 1)
        return 0;

    return _bond_directions[idx];
}

int BaseMolecule::getBondDirection2(int center_idx, int nei_idx)
{
    int idx = findEdgeIndex(center_idx, nei_idx);

    if (idx == -1)
        throw Error("getBondDirection2(): can not find bond");

    if (center_idx != getEdge(idx).beg)
        return 0;

    return getBondDirection(idx);
}

void BaseMolecule::setBondDirection(int idx, int dir)
{
    _bond_directions.expandFill(idx + 1, 0);
    _bond_directions[idx] = dir;
}

void BaseMolecule::clearBondDirections()
{
    _bond_directions.clear();
}

bool BaseMolecule::isChiral()
{
    // Molecule is Chiral if it has at least one Abs stereocenter and all the stereocenters are Abs or Any
    return stereocenters.size() != 0 && stereocenters.haveAllAbsAny() && stereocenters.haveAbs();
}

void BaseMolecule::invalidateAtom(int index, int mask)
{
    if (mask & CHANGED_ATOM_NUMBER)
    {
        // Cis-trans and stereocenters can be removed
        if (stereocenters.exists(index))
        {
            if (!isPossibleStereocenter(index))
                stereocenters.remove(index);
        }

        const Vertex& v = getVertex(index);
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

void BaseMolecule::getSGroupAtomsCenterPoint(SGroup& sgroup, Vec2f& res)
{
    getAtomsCenterPoint(sgroup.atoms, res);
}

void BaseMolecule::getAtomsCenterPoint(Array<int>& atoms, Vec2f& res)
{
    res.set(0, 0);
    for (int j = 0; j < atoms.size(); j++)
    {
        int ai = atoms[j];
        Vec3f& p = getAtomXyz(ai);
        res.x += p.x;
        res.y += p.y;
    }
    if (atoms.size() != 0)
        res.scale(1.0f / atoms.size());
}

void BaseMolecule::getAtomSymbol(int v, Array<char>& result)
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

        if (isQueryMolecule() && (query_atom_type = QueryMolecule::parseQueryAtom(asQueryMolecule(), v, list)) != -1)
        {
            if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST || query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
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
            else
                QueryMolecule::getQueryAtomLabel(query_atom_type, result);
        }
    }
    if (result.size() == 0)
        result.readString("*", true);
}

int BaseMolecule::atomCode(int vertex_idx)
{
    if (isPseudoAtom(vertex_idx))
        return CRC32::get(getPseudoAtom(vertex_idx));

    if (isTemplateAtom(vertex_idx))
        return CRC32::get(getTemplateAtom(vertex_idx));

    if (isRSite(vertex_idx))
        return 0;

    return getAtomNumber(vertex_idx);
}

int BaseMolecule::bondCode(int edge_idx)
{
    return getBondOrder(edge_idx);
}

int BaseMolecule::transformHELMtoSGroups(Array<char>& helm_class, Array<char>& name, Array<char>& code, Array<char>& natreplace, StringPool& r_names)
{
    QS_DEF(Array<int>, sg_atoms);
    sg_atoms.clear();

    if (countRSites() > r_names.size())
        throw Error("transformHELMtoSGroups: inconsistent number of R-sites and R-groups");

    for (auto i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
    {
        if (isRSite(i))
            continue;
        else
            sg_atoms.push(i);
    }

    if (sg_atoms.size() == 0)
        throw Error("transformHELMtoSGroups: atoms for base S-group are not found");

    int idx = sgroups.addSGroup("SUP");
    Superatom& sg = (Superatom&)sgroups.getSGroup(idx);
    sg.atoms.copy(sg_atoms);
    sg.subscript.copy(name);
    if (helm_class.size() > 6 && strncmp(helm_class.ptr(), "PEPTIDE", 7) == 0)
        sg.sa_class.readString("AA", true);
    else
        sg.sa_class.copy(helm_class);
    sg.sa_natreplace.copy(natreplace);

    for (auto i : vertices())
    {
        if (isRSite(i))
        {
            QS_DEF(Array<int>, rg_list);
            getAllowedRGroups(i, rg_list);
            int r_num = rg_list[0] - 1;

            int lvidx = sgroups.addSGroup("SUP");
            Superatom& lvsg = (Superatom&)sgroups.getSGroup(lvidx);
            lvsg.atoms.push(i);
            if (strncmp(r_names.at(r_num), "O", 1) == 0 && strlen(r_names.at(r_num)) == 1)
                lvsg.subscript.readString("OH", true);
            else
                lvsg.subscript.readString(r_names.at(r_num), true);
            lvsg.sa_class.readString("LGRP", true);
            this->asMolecule().resetAtom(i, Element::fromString(r_names.at(r_num)));

            int ap_idx = -1;
            const Vertex& v = getVertex(i);
            for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
            {
                if (sg_atoms.find(v.neiVertex(k)) != -1)
                {
                    ap_idx = v.neiVertex(k);
                    int b_idx = findEdgeIndex(v.neiVertex(k), i);
                    sg.bonds.push(b_idx);
                    lvsg.bonds.push(b_idx);
                }
            }

            if (ap_idx < 0)
            {
                throw Error("internal error: attachment point was not found");
            }
            int idap = sg.attachment_points.add();
            Superatom::_AttachmentPoint& ap = sg.attachment_points.at(idap);
            ap.aidx = ap_idx;
            ap.lvidx = i;
            if (r_num == 0)
                ap.apid.readString("Al", true);
            else if (r_num == 1)
                ap.apid.readString("Br", true);
            else if (r_num == 2)
                ap.apid.readString("Cx", true);
            else if (r_num == 3)
                ap.apid.readString("Dx", true);
        }
    }
    return 1;
}

const int* BaseMolecule::getPyramidStereocenters(int idx) const
{
    return stereocenters.getPyramid(idx);
}

void BaseMolecule::setStereoFlagPosition(int frag_index, const Vec3f& pos)
{
    try
    {
        _stereo_flag_positions.insert(frag_index, pos);
    }
    catch (Exception& ex)
    {
    }
}

bool BaseMolecule::getStereoFlagPosition(int frag_index, Vec3f& pos)
{
    auto* pval = _stereo_flag_positions.at2(frag_index);
    if (pval)
    {
        pos = *pval;
        return true;
    }
    return false;
}

int BaseMolecule::countStereoFlags()
{
    return _stereo_flag_positions.size();
}

void BaseMolecule::markBondsStereocenters()
{
    stereocenters.markBonds(*this);
}

bool BaseMolecule::hasAtropoStereoBonds()
{
    for (int i = stereocenters.begin(); i != stereocenters.end(); i = stereocenters.next(i))
    {
        auto atom_idx = stereocenters.getAtomIndex(i);
        if (stereocenters.hasAtropoStereoBonds(*this, atom_idx))
            return true;
    }
    return false;
}

void BaseMolecule::markBondStereocenters(int atom_idx)
{
    stereocenters.markBond(*this, atom_idx);
}

void BaseMolecule::addStereocenters(int atom_idx, int type, int group, const int pyramid[4])
{
    stereocenters.add(*this, atom_idx, type, group, pyramid);
}

void BaseMolecule::addStereocenters(int atom_idx, int type, int group, bool inverse_pyramid)
{
    stereocenters.add(*this, atom_idx, type, group, inverse_pyramid);
}

void BaseMolecule::addStereocentersIgnoreBad(int atom_idx, int type, int group, bool inverse_pyramid)
{
    stereocenters.add_ignore(*this, atom_idx, type, group, inverse_pyramid);
}

void BaseMolecule::removeAtomsStereocenters(const Array<int>& indices)
{
    stereocenters.removeAtoms(*this, indices);
}

void BaseMolecule::removeBondsStereocenters(const Array<int>& indices)
{
    stereocenters.removeBonds(*this, indices);
}

void BaseMolecule::buildFromBondsStereocenters(const StereocentersOptions& options, int* sensible_bonds_out)
{
    stereocenters.buildFromBonds(*this, options, sensible_bonds_out);
}

void BaseMolecule::buildFrom3dCoordinatesStereocenters(const StereocentersOptions& options)
{
    stereocenters.buildFrom3dCoordinates(*this, options);
}

bool BaseMolecule::isPossibleStereocenter(int atom_idx, bool* possible_implicit_h, bool* possible_lone_pair)
{
    return stereocenters.isPossibleStereocenter(*this, atom_idx, possible_implicit_h, possible_lone_pair);
}

bool BaseMolecule::isPossibleAtropocenter(int atom_idx, int& possible_atropo_bond)
{
    return stereocenters.isPossibleAtropocenter(*this, atom_idx, possible_atropo_bond);
}

void BaseMolecule::buildOnSubmoleculeStereocenters(const BaseMolecule& super, int* mapping)
{
    stereocenters.buildOnSubmolecule(*this, super, mapping);
}

void BaseMolecule::getSubstituents_All(int bond_idx, int subst[4])
{
    cis_trans.getSubstituents_All(*this, bond_idx, subst);
}

void BaseMolecule::restoreSubstituents(int bond_idx)
{
    cis_trans.restoreSubstituents(*this, bond_idx);
}

void BaseMolecule::buildCisTrans(int* exclude_bonds)
{
    cis_trans.build(*this, exclude_bonds);
}

bool BaseMolecule::registerBondAndSubstituentsCisTrans(int idx)
{
    return cis_trans.registerBondAndSubstituents(*this, idx);
}

void BaseMolecule::registerUnfoldedHydrogenCisTrans(int atom_idx, int added_hydrogen)
{
    cis_trans.registerUnfoldedHydrogen(*this, atom_idx, added_hydrogen);
}

void BaseMolecule::buildFromSmilesCisTrans(int* dirs)
{
    cis_trans.buildFromSmiles(*this, dirs);
}

void BaseMolecule::buildOnSubmoleculeCisTrans(BaseMolecule& super, int* mapping)
{
    cis_trans.buildOnSubmolecule(*this, super, mapping);
}

void BaseMolecule::validateCisTrans()
{
    cis_trans.validate(*this);
}

bool BaseMolecule::convertableToImplicitHydrogenCisTrans(int idx)
{
    return cis_trans.convertableToImplicitHydrogen(*this, idx);
}

void BaseMolecule::markBondsAlleneStereo()
{
    allene_stereo.markBonds(*this);
}

void BaseMolecule::buildOnSubmoleculeAlleneStereo(BaseMolecule& super, int* mapping)
{
    allene_stereo.buildOnSubmolecule(*this, super, mapping);
}

void BaseMolecule::removeAtomsAlleneStereo(const Array<int>& indices)
{
    allene_stereo.removeAtoms(*this, indices);
}

void BaseMolecule::removeBondsAlleneStereo(const Array<int>& indices)
{
    allene_stereo.removeBonds(*this, indices);
}

void BaseMolecule::buildFromBondsAlleneStereo(bool ignore_errors, int* sensible_bonds_out)
{
    allene_stereo.buildFromBonds(*this, ignore_errors, sensible_bonds_out);
}

void BaseMolecule::addCIP()
{
    MoleculeCIPCalculator mcc;
    have_cip = mcc.addCIPStereoDescriptors(*this);
}

void BaseMolecule::clearCIP()
{
    _cip_atoms.clear();
    _cip_bonds.clear();
    have_cip = false;
}

CIPDesc BaseMolecule::getAtomCIP(int atom_idx)
{
    auto* pval = _cip_atoms.at2(atom_idx);
    return pval ? *pval : CIPDesc::NONE;
}

CIPDesc BaseMolecule::getBondCIP(int bond_idx)
{
    auto* pval = _cip_bonds.at2(bond_idx);
    return pval ? *pval : CIPDesc::NONE;
}

void BaseMolecule::setAtomCIP(int atom_idx, CIPDesc cip)
{
    _cip_atoms.insert(atom_idx, cip);
    have_cip = true;
}

void BaseMolecule::setBondCIP(int bond_idx, CIPDesc cip)
{
    _cip_bonds.insert(bond_idx, cip);
    have_cip = true;
}

void BaseMolecule::getBoundingBox(Vec2f& a, Vec2f& b) const
{
    for (int atom_idx = 0; atom_idx < vertexCount(); ++atom_idx)
    {
        const auto& vec3d = _xyz[atom_idx];
        Vec2f vec(vec3d.x, vec3d.y);
        if (!atom_idx)
            a = b = vec;
        else
        {
            a.min(vec);
            b.max(vec);
        }
    }
}

void BaseMolecule::getBoundingBox(Rect2f& bbox, const Vec2f& minbox) const
{
    getBoundingBox(bbox);
    if (bbox.width() < minbox.x || bbox.height() < minbox.y)
    {
        Vec2f center(bbox.center());
        const auto half_width = std::max(bbox.width() / 2, minbox.x / 2);
        const auto half_height = std::max(bbox.height() / 2, minbox.y / 2);
        Rect2f new_bbox(Vec2f(center.x - half_width, center.y - half_height), Vec2f(center.x + half_width, center.y + half_height));
        bbox.copy(new_bbox);
    }
}

void BaseMolecule::getBoundingBox(Rect2f& bbox) const
{
    Vec2f a, b;
    getBoundingBox(a, b);
    bbox = Rect2f(a, b);
}

bool BaseMolecule::isAlias(int atom_idx) const
{
    return aliases.find(atom_idx);
}

const char* BaseMolecule::getAlias(int atom_idx) const
{
    return aliases.at(atom_idx).ptr();
}

void BaseMolecule::setAlias(int atom_idx, const char* alias)
{
    Array<char>& array = aliases.findOrInsert(atom_idx);
    array.readString(alias, true);
}

void BaseMolecule::removeAlias(int atom_idx)
{
    aliases.remove(atom_idx);
}
