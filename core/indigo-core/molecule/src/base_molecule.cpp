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

#include <algorithm>
#include <numeric>

#include "base_cpp/crc32.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "graph/dfs_walk.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/inchi_wrapper.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_dearom.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_exact_substructure_matcher.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#pragma warning(error : 4100 4101 4189 4244 4456 4458 4715)
#endif

using namespace indigo;

IMPL_ERROR(BaseMolecule, "molecule");

BaseMolecule::BaseMolecule() : original_format(BaseMolecule::UNKNOWN), _edit_revision(0)
{
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
    // #2851: when adding atoms to the molecule, we should keep existing cip labels
    // if (have_cip)
    //     clearCIP();
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
    template_attachment_indexes.clear();
    _template_occurrences.clear();
    _template_names.clear();
    _template_classes.clear();

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
    // should be uncommented
    // for (i = 0; i < mol.rgroups.getRGroupCount(); ++i)
    //{
    //    RGroup& rg = mol.rgroups.getRGroup(i + 1);
    //    for (int j = 0; j < rg.fragments.size(); ++j)
    //    {
    //        BaseMolecule* frag = rg.fragments[j];
    //        if (hasCoord(*frag))
    //            return true;
    //    }
    //}
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
    std::map<int, int> old_idx_to_new;

    for (i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& supersg = mol.sgroups.getSGroup(i);
        int idx = sgroups.addSGroup(supersg.sgroup_type);
        SGroup& sg = sgroups.getSGroup(idx);
        sg.parent_idx = supersg.parent_idx;
        sg.original_group = supersg.original_group;
        sg.parent_group = supersg.parent_group;

        if (_mergeSGroupWithSubmolecule(sg, supersg, mol, mapping, edge_mapping))
        {
            if (idx != i)
                old_idx_to_new.emplace(i, idx);

            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                DataSGroup& superdg = (DataSGroup&)supersg;

                dg.detached = superdg.detached;
                dg.display_pos = superdg.display_pos;
                dg.data.copy(superdg.data);
                dg.sa_natreplace.copy(superdg.sa_natreplace);
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
                sa.display_position.copy(supersa.display_position);
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

    // Update parent_idx
    if (sgroups.getSGroupCount() < mol.sgroups.getSGroupCount())
    {
        for (i = sgroups.begin(); i != sgroups.end(); i = sgroups.next(i))
        {
            SGroup& sgroup = sgroups.getSGroup(i);
            if (sgroup.parent_idx < 0)
                continue;

            const auto it = old_idx_to_new.find(sgroup.parent_idx);
            if (it != old_idx_to_new.end())
                sgroup.parent_idx = it->second;
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
            auto cip_atom_key = mol._cip_atoms.key(i);
            auto aidx = mapping[cip_atom_key];
            if (aidx >= 0)
            {
                _cip_atoms.insert(aidx, mol._cip_atoms.value(i));
                _show_cip_atoms.insert(aidx, mol.getShowAtomCIP(cip_atom_key));
            }
        }
        catch (Exception&)
        {
        }
    }

    for (auto i = mol._cip_bonds.begin(); i != mol._cip_bonds.end(); i = mol._cip_bonds.next(i))
    {
        try
        {
            auto eidx = edge_mapping[mol._cip_bonds.key(i)];
            if (eidx >= 0)
                _cip_bonds.insert(eidx, mol._cip_bonds.value(i));
        }
        catch (Exception&)
        {
        }
    }

    reaction_atom_mapping.expandFill(vertexEnd(), 0);
    reaction_atom_inversion.expandFill(vertexEnd(), 0);
    reaction_atom_exact_change.expandFill(vertexEnd(), 0);
    reaction_bond_reacting_center.expandFill(edgeEnd(), 0);
    _bond_directions.expandFill(edgeEnd(), -1);

    // Copy atom properties
    for (auto i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (mapping[i] < 0)
            continue;

        reaction_atom_mapping[mapping[i]] = mol.reaction_atom_mapping[i];
        reaction_atom_inversion[mapping[i]] = mol.reaction_atom_inversion[i];
        reaction_atom_exact_change[mapping[i]] = mol.reaction_atom_exact_change[i];
        if (mol.isAtomSelected(i))
            selectAtom(mapping[i]);
        if (mol.isAtomHighlighted(i))
            highlightAtom(mapping[i]);
        if (mol._atom_annotations.count(i) > 0)
            _atom_annotations[mapping[i]] = mol._atom_annotations[i];
    }

    for (int j = mol.edgeBegin(); j != mol.edgeEnd(); j = mol.edgeNext(j))
    {
        int edge_idx = edge_mapping[j];
        if (edge_idx < 0)
            continue;
        reaction_bond_reacting_center[edge_idx] = mol.reaction_bond_reacting_center[j];
        _bond_directions[edge_idx] = mol.getBondDirection(j);

        if (mol.isBondSelected(j))
            selectBond(edge_idx);
        if (mol.isBondHighlighted(j))
            highlightBond(edge_idx);
        if (mol._bond_annotations.count(j) > 0)
            _bond_annotations[edge_idx] = mol._bond_annotations[j];
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
    for (int j = 0; j < sa.bond_connections.size(); j++)
    {
        Superatom::_BondConnection& bond = sa.bond_connections[j];
        if (bond.bond_idx == src_bond_idx)
            bond.bond_idx = new_bond_idx;
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
    setBondDirection(new_bond_idx, BOND_DIRECTION_MONO);

    // sgroups

    for (int j = sgroups.begin(); j != sgroups.end(); j = sgroups.next(j))
    {
        SGroup& sg = sgroups.getSGroup(j);
        _flipSGroupBond(sg, src_bond_idx, new_bond_idx);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            _flipSuperatomBond((Superatom&)sg, src_bond_idx, new_bond_idx);
    }

    updateEditRevision();
}

int BaseMolecule::flipBondWithDirection(int atom_parent, int atom_from, int atom_to, int leaving_atom)
{
    // atom_parent = B (Neighbor)
    // atom_from = A (Monomer)
    // atom_to = AttA (Attachment Point on A)
    // leaving_atom = L (Leaving Atom)

    // Get existing bond (A-B) details
    int bond_AB_idx = findEdgeIndex(atom_parent, atom_from);
    if (bond_AB_idx < 0)
        return -1;

    int dir_AB = getBondDirection(bond_AB_idx);

    // Get leaving bond (AttA-L) details
    int bond_AttAL_idx = -1;
    if (leaving_atom >= 0)
        bond_AttAL_idx = findEdgeIndex(atom_to, leaving_atom);

    int dir_AttAL = BOND_DIRECTION_MONO;
    if (bond_AttAL_idx >= 0)
        dir_AttAL = getBondDirection(bond_AttAL_idx);

    static const int PRIORITIES[] = {
        0, // BOND_DIRECTION_MONO = 0
        3, // BOND_UP = 1
        3, // BOND_DOWN = 2
        2, // BOND_EITHER = 3
        1, // BOND_UP_OR_UNSPECIFIED = 4
        1  // BOND_DOWN_OR_UNSPECIFIED = 5
    };

    auto get_priority = [&](int dir) -> int {
        if (dir >= 0 && dir < sizeof(PRIORITIES) / sizeof(PRIORITIES[0]))
            return PRIORITIES[dir];
        return 0;
    };

    int p_AB = get_priority(dir_AB);
    int p_AttAL = get_priority(dir_AttAL);

    // Case 1: If leaving bond starts at leaving_atom, it has lowest priority.
    if (bond_AttAL_idx >= 0)
    {
        const Edge& edge = _edges[bond_AttAL_idx];
        if (edge.beg == leaving_atom)
            p_AttAL = -1;
    }

    int kept_bond_idx = -1;
    int final_dir = BOND_DIRECTION_MONO;

    // Helper Lambda: In-place Bond Flip
    auto inplaceFlipBond = [&](int pivot, int old_neighbor, int new_neighbor) {
        // Find the edge to flip (between pivot and old_neighbor)
        int edge_idx = findEdgeIndex(pivot, old_neighbor);
        if (edge_idx < 0)
            throw Error("flipBondWithDirection: edge not found between %d and %d", pivot, old_neighbor);

        // 1. Update internal molecular structures
        stereocenters.flipBond(pivot, old_neighbor, new_neighbor);
        cis_trans.flipBond(*this, pivot, old_neighbor, new_neighbor);

        // 2. Direct Graph Modification: Update Edge endpoints
        Edge& edge = _edges[edge_idx];
        if (edge.beg == old_neighbor)
            edge.beg = new_neighbor;
        else if (edge.end == old_neighbor)
            edge.end = new_neighbor;
        else
            throw Error("flipBondWithDirection: edge ends mismatch for %d", edge_idx);

        // 3. Update Adjacency Lists
        // A. Pivot (stays connected): change neighbor ref from old to new
        Vertex& v_pivot = _vertices->at(pivot);
        int item_pivot = v_pivot.findNeiEdge(edge_idx);
        if (item_pivot == -1)
            throw Error("flipBondWithDirection: inconsistency at pivot %d", pivot);
        v_pivot.neighbors_list[item_pivot].v = new_neighbor;

        // B. Old Neighbor (loses connection): remove edge from adjacency list
        Vertex& v_old = _vertices->at(old_neighbor);
        int item_old = v_old.findNeiEdge(edge_idx);
        if (item_old == -1)
            throw Error("flipBondWithDirection: inconsistency at old neighbor %d", old_neighbor);
        v_old.neighbors_list.remove(item_old);

        // C. New Neighbor (gains connection): add edge to adjacency list
        Vertex& v_new = _vertices->at(new_neighbor);
        int item_new = v_new.neighbors_list.add();
        VertexEdge& ve_new = v_new.neighbors_list[item_new];
        ve_new.e = edge_idx;
        ve_new.v = pivot;
    };

    // Selection Logic:
    // If Leaving Bond has STRICTLY higher priority, use Method 2.
    // Otherwise (Existing is better or equal), use Method 1.
    if (p_AttAL > p_AB && bond_AttAL_idx >= 0)
    {
        // Method 2: Keep Leaving Bond (AttA-L)
        // We preserve the bond (AttA-L) and move its endpoint L -> B.
        // Result: Bond (AttA, B) using the 'Leaving' edge object.
        inplaceFlipBond(atom_to, leaving_atom, atom_parent);

        // Remove the conflicting A-B bond.
        removeBond(bond_AB_idx);

        final_dir = dir_AttAL;
        kept_bond_idx = findEdgeIndex(atom_to, atom_parent);
    }
    else
    {
        // Method 1: Keep Existing Bond (A-B)
        // We preserve the bond (A-B) and move its endpoint A -> AttA.
        // Result: Bond (B, AttA) using the 'Existing' edge object.
        // Note: Guaranteed to have bond_AB_idx >= 0 here.

        // Capture orientation BEFORE modification for Case 2 checks
        bool bond_ends_at_atom_from = false;
        if (bond_AB_idx >= 0)
            bond_ends_at_atom_from = (_edges[bond_AB_idx].end == atom_from);

        inplaceFlipBond(atom_parent, atom_from, atom_to);

        final_dir = dir_AB;

        // Case 2: Conflict Resolution Logic
        // If not Case 1 (implied, as we are in else branch/Method 1),
        // and direction > 0 and priorities are equal:
        if (bond_AttAL_idx >= 0 && dir_AB > BOND_DIRECTION_MONO && p_AB == p_AttAL)
        {
            // If Existing Bond "Comes To Us", zero out.
            if (bond_ends_at_atom_from)
                final_dir = BOND_DIRECTION_MONO;
        }

        // Remove the unused AttA-L bond.
        if (bond_AttAL_idx >= 0)
            removeBond(bond_AttAL_idx);

        kept_bond_idx = findEdgeIndex(atom_parent, atom_to);
    }

    if (kept_bond_idx >= 0)
        setBondDirection(kept_bond_idx, final_dir);

    updateEditRevision();
    return kept_bond_idx;
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
    original_format = other.original_format;
    copyProperties(other, *mapping);
    for (int i = 0; i < other.monomer_shapes.size(); ++i)
        monomer_shapes.add(new KetMonomerShape(*other.monomer_shapes[i]));
    for (int i = 0; i < other._template_occurrences.size(); ++i)
        std::ignore = _template_occurrences.add(other._template_occurrences[i]);
    for (int i = 0; i < other._template_names.size(); ++i)
        _template_names.add(other._template_names.at(i));
    for (int i = 0; i < other._template_classes.size(); ++i)
        _template_classes.add(other._template_classes.at(i));
    if (other._annotation.has_value())
    {
        _annotation.emplace();
        _annotation->copy(*other._annotation);
    }
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
    original_format = other.original_format;
    copyProperties(other, mapping);
    for (int j = 0; j < other.monomer_shapes.size(); ++j)
        monomer_shapes.add(new KetMonomerShape(*other.monomer_shapes[j]));
    for (i = 0; i < other._template_occurrences.size(); ++i)
        std::ignore = _template_occurrences.add(other._template_occurrences[i]);
    for (i = 0; i < other._template_names.size(); ++i)
        _template_names.add(other._template_names.at(i));
    for (i = 0; i < other._template_classes.size(); ++i)
        _template_classes.add(other._template_classes.at(i));
    if (other._annotation.has_value())
    {
        _annotation.emplace();
        _annotation->copy(*other._annotation);
    }
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
                setBondDirection(b_idx, BOND_DIRECTION_MONO);
        }
    }

    // aliases && selection
    for (i = 0; i < indices.size(); i++)
    {
        unselectAtom(indices[i]);
        if (isAlias(indices[i]))
            removeAlias(indices[i]);
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

    for (i = 0; i < indices.size(); i++)
    {
        unhighlightBond(indices[i]);
        if (getBondDirection(indices[i]) > 0)
            setBondDirection(indices[i], BOND_DIRECTION_MONO);
        removeEdge(indices[i]);
        _bond_annotations.erase(indices[i]);
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

void BaseMolecule::_postMergeWithSubmolecule(BaseMolecule& /*mol*/, const Array<int>& /*vertices*/, const Array<int>* /*edges*/, const Array<int>& /*mapping*/,
                                             int /*skip_flags*/)
{
}

void BaseMolecule::_flipBond(int /*atom_parent*/, int /*atom_from*/, int /*atom_to*/)
{
}

void BaseMolecule::_removeAtoms(const Array<int>& /*indices*/, const int* /*mapping*/)
{
}

void BaseMolecule::_removeBonds(const Array<int>& /*indices*/)
{
}

Vec3f& BaseMolecule::getAtomXyz(int idx)
{
    return _xyz[idx];
}

bool BaseMolecule::getMiddlePoint(int idx1, int idx2, Vec3f& vec)
{
    if (idx1 == std::clamp(idx1, 0, vertexCount() - 1) && idx2 == std::clamp(idx2, 0, vertexCount() - 1))
    {
        vec = _xyz[idx1];
        vec.add(_xyz[idx2]);
        vec.scale(0.5);
        return true;
    }
    return false;
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
    catch (Exception&)
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
    catch (Exception&)
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
    auto& ap = template_attachment_points.at(att_idx);
    ap.ap_occur_idx = atom_idx;
    ap.ap_aidx = att_atom_idx;
    ap.ap_id.readString(att_id, false);
    ap.ap_id.push(0);
    if (atom_idx >= template_attachment_indexes.size())
        template_attachment_indexes.expand(atom_idx + 1);
    template_attachment_indexes.at(atom_idx).add(att_idx);
    updateEditRevision();
}

int BaseMolecule::getTemplateAtomAttachmentPoint(int atom_idx, int order)
{
    if (atom_idx < template_attachment_indexes.size())
    {
        auto& att_indexes = template_attachment_indexes.at(atom_idx);
        if (att_indexes.size() && order < att_indexes.size())
            return template_attachment_points.at(att_indexes[order]).ap_aidx;
    }
    return -1;
}

void BaseMolecule::getTemplateAtomAttachmentPointId(int atom_idx, int order, Array<char>& apid)
{
    if (atom_idx < template_attachment_indexes.size())
    {
        auto& att_indexes = template_attachment_indexes.at(atom_idx);
        if (att_indexes.size() && order < att_indexes.size())
        {
            apid.copy(template_attachment_points.at(att_indexes[order]).ap_id);
            return;
        }
    }
    throw Error("attachment point order %d is out of range", order);
}

std::optional<std::pair<int, std::reference_wrapper<ObjPool<int>>>> BaseMolecule::getTemplateAtomAttachmentPointIdxs(int atom_idx, int att_point_idx)
{
    if (atom_idx < template_attachment_indexes.size())
    {
        auto& att_idxs = template_attachment_indexes[atom_idx];
        for (int k = att_idxs.begin(); k != att_idxs.end(); k = att_idxs.next(k))
            if (att_idxs.at(k) == att_point_idx)
                return std::make_pair(k, std::ref(att_idxs));
    }
    return std::nullopt;
}

int BaseMolecule::getTemplateAtomAttachmentPointById(int atom_idx, Array<char>& att_id)
{
    QS_DEF(Array<char>, tmp);
    if (atom_idx < template_attachment_indexes.size())
    {
        auto& att_idxs = template_attachment_indexes[atom_idx];
        for (int k = att_idxs.begin(); k != att_idxs.end(); k = att_idxs.next(k))
        {
            auto& ap = template_attachment_points.at(att_idxs.at(k));
            if (ap.ap_id.memcmp(att_id) == 0)
                return ap.ap_aidx;
        }
    }
    return -1;
}

bool BaseMolecule::updateTemplateAtomAttachmentDestination(int atom_idx, int old_dest_atom_idx, int new_dest_atom_idx)
{
    if (atom_idx < template_attachment_indexes.size())
    {
        auto& att_idxs = template_attachment_indexes[atom_idx];
        for (int k = att_idxs.begin(); k != att_idxs.end(); k = att_idxs.next(k))
        {
            auto& ap = template_attachment_points.at(att_idxs.at(k));
            if (ap.ap_aidx == old_dest_atom_idx)
            {
                ap.ap_aidx = new_dest_atom_idx;
                return true;
            }
        }
    }
    return false;
}

void BaseMolecule::setTemplateAtomAttachmentDestination(int atom_idx, int new_dest_atom_idx, Array<char>& att_id)
{
    if (atom_idx < template_attachment_indexes.size())
    {
        auto& att_idxs = template_attachment_indexes[atom_idx];
        for (int k = att_idxs.begin(); k != att_idxs.end(); k = att_idxs.next(k))
        {
            auto& ap = template_attachment_points.at(att_idxs.at(k));
            if (ap.ap_id.memcmp(att_id) == 0)
            {
                ap.ap_aidx = new_dest_atom_idx;
                return;
            }
        }
    }

    setTemplateAtomAttachmentOrder(atom_idx, new_dest_atom_idx, att_id.ptr());
}

int BaseMolecule::getTemplateAtomAttachmentPointsCount(int atom_idx)
{
    return atom_idx < template_attachment_indexes.size() ? template_attachment_indexes.at(atom_idx).size() : 0;
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif
