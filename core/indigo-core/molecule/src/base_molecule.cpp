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
#include "molecule/ket_document.h"
#include "molecule/ket_document_json_loader.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_exact_substructure_matcher.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_json_saver.h"
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

BaseMolecule::BaseMolecule() : original_format(BaseMolecule::UNKNOWN), _document(new KetDocument), _edit_revision(0)
{
}

BaseMolecule::~BaseMolecule()
{
    delete _document;
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
    delete _document;
    _document = new KetDocument();
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

    for (auto tidx : tinds)
        _transformTGroupToSGroup(tidx, -1);

    if (tinds.size() > 0)
    {
        tgroups.clear();
        template_attachment_points.clear();
        template_attachment_indexes.clear();
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

            int idx = this->addTemplateAtom(tg.tgroup_name.ptr());
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
             int idx = this->addTemplateAtom(tg.tgroup_name.ptr());
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

            int idx = this->addTemplateAtom(tg.tgroup_name.ptr());
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

void BaseMolecule::getTemplateAtomDirectionsMap(std::vector<std::map<int, int>>& directions_map)
{
    directions_map.clear();
    if (vertexCount())
    {
        directions_map.resize(vertexEnd());
        for (int i = template_attachment_points.begin(); i != template_attachment_points.end(); i = template_attachment_points.next(i))
        {
            auto& tap = template_attachment_points[i];
            if (tap.ap_id.size())
            {
                Array<char> atom_label;
                getAtomSymbol(tap.ap_occur_idx, atom_label);
                int ap_id = tap.ap_id[0] - 'A';
                directions_map[tap.ap_occur_idx].emplace(ap_id, tap.ap_aidx);
            }
        }
    }
}

int BaseMolecule::_transformTGroupToSGroup(int idx, int t_idx)
{
    int result = 0;
    QS_DEF(Array<int>, leaving_groups);
    QS_DEF(Array<int>, residue_sgroups);
    QS_DEF(Array<int>, mapping);
    QS_DEF(Array<int>, att_atoms);
    QS_DEF(Array<int>, tg_atoms);
    QS_DEF(Array<int>, lv_atoms);
    QS_DEF(Array<int>, atoms_to_delete);
    QS_DEF(StringPool, ap_points_ids);
    QS_DEF(Array<int>, ap_ids);
    QS_DEF(Array<char>, ap_id);

    int tg_idx = t_idx;
    if (t_idx == -1)
        tg_idx = tgroups.findTGroup(getTemplateAtom(idx));

    TGroup& tgroup = tgroups.getTGroup(tg_idx);
    if (tgroup.ambiguous)
        throw Error("Ambiguous monomer cannot be transform to SGroup.");

    // create transformed fragment
    std::unique_ptr<BaseMolecule> fragment = tgroup.fragment->applyTransformation(getTemplateAtomTransform(idx), getAtomXyz(idx));

    leaving_groups.clear();
    att_atoms.clear();
    tg_atoms.clear();
    lv_atoms.clear();
    residue_sgroups.clear();
    ap_points_ids.clear();
    ap_ids.clear();

    // collect leaving groups into sgs and residue to base_sgs
    for (int j = fragment->sgroups.begin(); j != fragment->sgroups.end(); j = fragment->sgroups.next(j))
    {
        // how to check if group is connected?
        auto& sg = fragment->sgroups.getSGroup(j);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            Superatom& sa = (Superatom&)sg;
            BufferScanner sc(sa.sa_class);
            if (sc.findWordIgnoreCase("LGRP"))
                leaving_groups.push(j);
            else
                residue_sgroups.push(j);
        }
    }

    if (residue_sgroups.size() == 0)
        throw Error("transformTGroupToSGroup(): wrong template structure found (no base SGroup detected)");

    if (residue_sgroups.size() > 1)
        throw Error("transformTGroupToSGroup(): wrong template structure found (more then one base SGroup detected)");

    SGroup& sgu = fragment->sgroups.getSGroup(residue_sgroups[0]);
    if (sgu.sgroup_type != SGroup::SG_TYPE_SUP)
        throw Error("transformTGroupToSGroup(): wrong template structure found (base SGroup is not Superatom type)");

    Superatom& residue_super_atom = (Superatom&)sgu;

    // printf("Template = %s (%d)\n", tgroup.tgroup_name.ptr(), idx);

    for (int j = residue_super_atom.attachment_points.begin(); j < residue_super_atom.attachment_points.end(); j = residue_super_atom.attachment_points.next(j))
    {
        Superatom::_AttachmentPoint& ap = residue_super_atom.attachment_points.at(j);

        // find template atom attachment point with the same name
        int att_atom_idx = getTemplateAtomAttachmentPointById(idx, ap.apid);
        if (att_atom_idx > -1)
        {
            att_atoms.push(att_atom_idx);
            tg_atoms.push(ap.aidx);
            lv_atoms.push(ap.lvidx);
            ap_ids.push(ap_points_ids.add(ap.apid));
            ap.lvidx = -1;
            // printf("idx = %d, att_atom_idx = %d, ap.aidx = %d, ap.lvidx = %d, ap_id = %s\n", idx, att_atom_idx, ap.aidx, ap.lvidx, ap.apid.ptr());
        }
    }

    // merge full template fragment with leaving groups
    mergeWithMolecule(*fragment, &mapping);

    // collect leaving atoms
    for (const auto sg_index : leaving_groups)
    {
        const SGroup& lvg = fragment->sgroups.getSGroup(sg_index);
        for (const auto lv_atom_index : lv_atoms)
        {
            if (lvg.atoms.find(lv_atom_index) > -1)
            {
                atoms_to_delete.push(mapping[lvg.atoms[0]]);
                fragment->removeSGroup(sg_index);
                if (!fragment->sgroups.hasSGroup(sg_index))
                {
                    break;
                }
            }
        }
    }

    QS_DEF(Array<int>, residue_atoms);

    // collect residue atoms
    residue_atoms.clear();
    for (auto i = 0; i < residue_super_atom.atoms.size(); i++)
    {
        int aidx = mapping[residue_super_atom.atoms[i]];
        if (aidx > -1)
            residue_atoms.push(aidx); // collect converted atoms
    }

    residue_sgroups.clear();
    sgroups.findSGroups(SGroup::SG_ATOMS, residue_atoms, residue_sgroups);
    if (residue_sgroups.size() == 1)
    {
        SGroup& sg = sgroups.getSGroup(residue_sgroups[0]);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            Superatom& su = (Superatom&)sg;
            su.seqid = getTemplateAtomSeqid(idx);
            su.sa_natreplace.copy(tgroup.tgroup_natreplace);
            su.contracted = getTemplateAtomDisplayOption(idx);

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
                            int added_bond = asMolecule().addBond(att_atoms[i], mapping[tg_atoms[i]], BOND_SINGLE);
                            (void)added_bond;
                            // printf("Add bond = %d, att_atom[i] = %d, tg_atoms[i] = %d, mapping[tg_atoms[i]] = %d\n", added_bond, att_atoms[i], tg_atoms[i],
                            // mapping[tg_atoms[i]]); printf("Flip AP  att_atom[i] = %d, tg_atoms[i] = %d, mapping[tg_atoms[i]] = %d, ap_id = %s\n",
                            // att_atoms[i], tg_atoms[i], mapping[tg_atoms[i]], ap_id.ptr());
                            _flipTemplateAtomAttachmentPoint(att_atoms[i], idx, ap_id, mapping[tg_atoms[i]]);
                        }
                    }
                    // int added_bond = this->asMolecule().addBond(att_atoms[i], mapping[tg_atoms[i]], BOND_SINGLE);
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
                    su.bonds.push(bond_idx);
            }
        }
    }

    QS_DEF(Array<int>, templ_atoms);
    templ_atoms.clear();
    templ_atoms.push(idx);

    leaving_groups.clear();
    sgroups.findSGroups(SGroup::SG_ATOMS, templ_atoms, leaving_groups);

    for (int i = 0; i < leaving_groups.size(); i++)
    {
        SGroup& sg = sgroups.getSGroup(leaving_groups[i]);
        sg.atoms.concat(residue_atoms);
    }

    atoms_to_delete.push(idx);
    removeAtoms(atoms_to_delete);

    return result;
}

void BaseMolecule::_collectSuparatomAttachmentPoints(Superatom& sa, std::unordered_map<int, std::string>& ap_ids_map)
{
    if (sa.attachment_points.size() > 0)
    {
        for (int k = sa.attachment_points.begin(); k < sa.attachment_points.end(); k = sa.attachment_points.next(k))
        {
            Superatom::_AttachmentPoint& ap = sa.attachment_points.at(k);
            ap_ids_map.emplace(ap.aidx, ap.apid.ptr());
        }
    }
    else // Try to create attachment points from crossing bond information
    {
        for (int k = 0; k < sa.bonds.size(); k++)
        {
            const Edge& edge = getEdge(sa.bonds[k]);
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
            else // Crossing bond connects atoms out of Sgroup?
            {
                continue;
            }

            int idap = sa.attachment_points.add();
            Superatom::_AttachmentPoint& ap = sa.attachment_points.at(idap);

            if (_isNTerminus(sa, ap_aidx)) //  N-terminus ?
                ap.apid.readString("Al", true);
            else if (_isCTerminus(sa, ap_aidx)) //  C-terminus ?
                ap.apid.readString("Br", true);
            else
                ap.apid.readString("Cx", true);

            ap.aidx = ap_aidx;
            ap.lvidx = ap_lvidx;
            ap_ids_map.emplace(ap.aidx, ap.apid.ptr());
        }
    }
}

void BaseMolecule::_connectTemplateAtom(Superatom& sa, int t_idx, Array<int>& orphaned_atoms)
{
    orphaned_atoms.concat(sa.atoms);
    for (int i = sa.attachment_points.begin(); i < sa.attachment_points.end(); i = sa.attachment_points.next(i))
    {
        auto& ap = sa.attachment_points.at(i);
        if (ap.lvidx < 0)
        {
            int edge_idx = -1;
            for (auto xbond_idx : sa.bonds)
            {
                const Edge& e = getEdge(xbond_idx);
                int dest_atom = e.beg == ap.aidx ? e.end : (e.end == ap.aidx ? e.beg : -1);
                if (dest_atom != -1)
                {
                    Array<int> sgs, sg_atoms;
                    sg_atoms.push(dest_atom);
                    sgroups.findSGroups(SGroup::SG_ATOMS, sg_atoms, sgs);
                    bool is_lgrp = false;
                    for (auto sgs_index : sgs)
                    {
                        auto& sg = sgroups.getSGroup(sgs_index);
                        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
                        {
                            auto& sal = static_cast<Superatom&>(sg);
                            if (sal.sa_class.size() && std::string(sal.sa_class.ptr()) == "LGRP")
                            {
                                is_lgrp = true;
                                break;
                            }
                        }
                    }
                    if (!is_lgrp)
                    {
                        edge_idx = xbond_idx;
                        break;
                    }
                }
            }

            if (edge_idx < 0) // find the first one
            {
                const Vertex& v = getVertex(ap.aidx);
                for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                {
                    if (sa.atoms.find(v.neiVertex(k)) == -1)
                    {
                        int v_k = v.neiVertex(k);
                        edge_idx = findEdgeIndex(v_k, ap.aidx);
                        if (edge_idx != -1)
                            break;
                    }
                }
            }

            if (edge_idx != -1)
            {
                const Edge& e = getEdge(edge_idx);
                int v_k = e.beg == ap.aidx ? e.end : (e.end == ap.aidx ? e.beg : -1);
                if (v_k != -1)
                {
                    if (findEdgeIndex(v_k, t_idx) == -1)
                        flipBond(v_k, ap.aidx, t_idx);
                    setTemplateAtomAttachmentOrder(t_idx, v_k, ap.apid.ptr());
                    if (isTemplateAtom(v_k))
                    {
                        int ap_count = getTemplateAtomAttachmentPointsCount(v_k);
                        for (int m = 0; m < ap_count; m++)
                        {
                            if (getTemplateAtomAttachmentPoint(v_k, m) == ap.aidx)
                            {
                                QS_DEF(Array<char>, ap_id);
                                getTemplateAtomAttachmentPointId(v_k, m, ap_id);
                                _flipTemplateAtomAttachmentPoint(v_k, ap.aidx, ap_id, t_idx);
                            }
                        }
                    }
                }
            }
        }
        else
            orphaned_atoms.push(ap.lvidx);
    }
}

bool BaseMolecule::_findAffineTransform(BaseMolecule& src, BaseMolecule& dst, Mat23& M, const int* mapping)
{
    constexpr double PIVOT_EPS = 1e-12;
    constexpr double MATCH_EPS = 5e-4;

    if (src.vertexCount() != dst.vertexCount() || src.vertexCount() < 3)
        return false;

    double A[6][6]{};
    double b[6]{};

    auto accumulate = [&](const double J[6], double d) {
        for (int r = 0; r < 6; ++r)
        {
            b[r] += J[r] * d;
            for (int c = 0; c < 6; ++c)
                A[r][c] += J[r] * J[c];
        }
    };

    for (auto v : src.vertices())
    {
        const auto s = src.getAtomXyz(v);
        const auto t = dst.getAtomXyz(mapping[v]);
        Array<char> src_label, dst_label;
        src.getAtomSymbol(v, src_label);
        dst.getAtomSymbol(mapping[v], dst_label);
        const double Jx[6]{s.x, s.y, 0, 0, 1, 0};
        const double Jy[6]{0, 0, s.x, s.y, 0, 1};
        accumulate(Jx, t.x);
        accumulate(Jy, t.y);
    }

    // Gauss solve
    for (int k = 0; k < 6; ++k)
    {
        int piv = k;
        for (int i = k + 1; i < 6; ++i)
            if (std::fabs(A[i][k]) > std::fabs(A[piv][k]))
                piv = i;

        if (std::fabs(A[piv][k]) < PIVOT_EPS)
            return false;

        if (piv != k)
        {
            for (int j = k; j < 6; ++j)
                std::swap(A[k][j], A[piv][j]);
            std::swap(b[k], b[piv]);
        }

        const double diag = A[k][k];
        for (int j = k; j < 6; ++j)
            A[k][j] /= diag;
        b[k] /= diag;

        for (int i = 0; i < 6; ++i)
            if (i != k)
            {
                const double f = A[i][k];
                for (int j = k; j < 6; ++j)
                    A[i][j] -= f * A[k][j];
                b[i] -= f * b[k];
            }
    }

    const double a11 = b[0], a12 = b[1], a21 = b[2], a22 = b[3], tx = b[4], ty = b[5];

    for (auto v : src.vertices())
    {
        const auto s = src.getAtomXyz(v);
        const auto t = dst.getAtomXyz(mapping[v]);

        const double px = a11 * s.x + a12 * s.y + tx;
        const double py = a21 * s.x + a22 * s.y + ty;

        if (std::fabs(px - t.x) > MATCH_EPS || std::fabs(py - t.y) > MATCH_EPS)
            return false;
    }

    M = {{{static_cast<float>(a11), static_cast<float>(a12), static_cast<float>(tx)},
          {static_cast<float>(a21), static_cast<float>(a22), static_cast<float>(ty)}}};
    return true;
}

bool BaseMolecule::_restoreTemplateFromLibrary(TGroup& tg, MonomerTemplateLibrary& mtl, const std::string& residue_inchi)
{
    auto& class_map = MonomerTemplates::getStrToMonomerType();
    auto class_it = class_map.find(tg.tgroup_class.ptr());
    if (class_it != class_map.end())
    {
        std::string id = mtl.getMonomerTemplateIdByAlias(class_it->second, tg.tgroup_name.ptr());
        if (id.size() == 0 && tg.tgroup_alias.size() > 0)
            id = mtl.getMonomerTemplateIdByAlias(class_it->second, tg.tgroup_alias.ptr());
        if (id.size() > 0)
        {
            // template with same class and alias found
            // now we can compare residues' InChI
            try
            {
                const auto& templ = mtl.getMonomerTemplateById(id);
                auto templ_tgroup = templ.getTGroup();
                auto templ_residue = templ_tgroup->getResidue();
                if (templ_residue)
                {
                    // calc inchi for template residue
                    std::string templ_inchi_str;
                    {
                        StringOutput templ_inchi_output(templ_inchi_str);
                        MoleculeInChI templ_inchi(templ_inchi_output);
                        templ_inchi.outputInChI(templ_residue->asMolecule());
                    }

                    if (templ_inchi_str == residue_inchi)
                    {
                        tg.copy(*templ_tgroup);
                        tg.tgroup_text_id.readString(id.c_str(), true);
                        tg.fragment.reset(neu());
                        tg.fragment->clone(templ_tgroup->fragment->asMolecule()); // clone the whole template as is
                        return true;
                    }
                }
            }
            catch (...)
            {
            }
        }
    }
    return false;
}

bool BaseMolecule::_replaceExpandedMonomerWithTemplate(int sg_idx, int& tg_id, MonomerTemplateLibrary& mtl,
                                                       std::unordered_map<std::string, int>& added_templates, Array<int>& remove_atoms)
{
    auto& sa = static_cast<Superatom&>(sgroups.getSGroup(sg_idx));
    if (!sgroups.hasSGroup(sg_idx) || sa.subscript.size() == 0 || sa.sa_class.size() == 0)
        return false;

    // No special handling needed for LGRP. Just mark it to remove.
    if (sa.sa_class.size() && std::string(sa.sa_class.ptr()) == "LGRP")
        return true;

    // create template atom and set all properties except template index and transform
    bool res = true;

    // Calculate residue InChI
    std::unique_ptr<BaseMolecule> residue(neu());

    Array<int> mapping;
    residue->makeSubmolecule(*this, sa.atoms, &mapping, SKIP_TGROUPS | SKIP_TEMPLATE_ATTACHMENT_POINTS | SKIP_STEREOCENTERS);
    residue->sgroups.clear();

    std::string residue_inchi_str;
    {
        StringOutput inchi_output(residue_inchi_str);
        MoleculeInChI inchi(inchi_output);
        inchi.outputInChI(residue->asMolecule());
    }

    // find or create template group for residue
    auto template_inchi_id = monomerNameByAlias(sa.sa_class.ptr(), sa.subscript.ptr()) + "/" + std::string(sa.sa_class.ptr()) + "/" + residue_inchi_str;
    auto it_added = added_templates.find(template_inchi_id);
    int tg_index = it_added != added_templates.end() ? it_added->second : tgroups.addTGroup();
    // no we know template index to link template atom with it
    TGroup& tg = tgroups.getTGroup(tg_index);
    if (tg.tgroup_id < 0)
    {
        tg.tgroup_id = ++tg_id;
        tg.tgroup_class.copy(sa.sa_class);
        if (sa.subscript.size())
            tg.tgroup_name.copy(sa.subscript);
        if (sa.sa_natreplace.size() > 0)
            tg.tgroup_natreplace.copy(sa.sa_natreplace);
        res = _restoreTemplateFromLibrary(tg, mtl, residue_inchi_str);
        if (!res)
            tgroups.remove(tg_index);
    }
    // handle transformation
    if (res)
    {
        auto templ_residue = tg.getResidue();
        MoleculeExactMatcher matcher(*residue, *templ_residue);
        matcher.flags = MoleculeExactMatcher::CONDITION_ELECTRONS;
        if (matcher.find())
        {
            // check if transform is possible
            auto map = matcher.getTargetMapping();
            Mat23 transform;
            bool affine = _findAffineTransform(*templ_residue, *residue, transform, map);
            Transformation tform;
            if (affine && tform.fromAffineMatrix(transform))
            {
                int ta_idx = addTemplateAtom(sa.subscript.ptr());
                setAtomXyz(ta_idx, tform.shift);
                tform.shift.clear();
                if (tform.hasTransformation())
                    setTemplateAtomTransform(ta_idx, tform);
                setTemplateAtomClass(ta_idx, sa.sa_class.ptr());
                setTemplateAtomSeqid(ta_idx, sa.seqid);
                setTemplateAtomDisplayOption(ta_idx, sa.contracted);
                setTemplateAtomTemplateIndex(ta_idx, tg_index);
                added_templates.emplace(template_inchi_id, tg_index);
                _connectTemplateAtom(sa, ta_idx, remove_atoms);
            }
            else
                res = false;
        }
    }
    return res;
}

int BaseMolecule::_transformSGroupToTGroup(int sg_idx, int& tg_id)
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

    if (su.subscript.size() == 0)
        return -1;

    // TODO: special handling needed for LGRP
    if (su.sa_class.size() && std::string(su.sa_class.ptr()) == "LGRP")
    {
        removeSGroup(sg_idx);
        return -1;
    }

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
        std::vector<int> xbonds;
        for (auto k : su.atoms)
        {
            auto& vx = getVertex(k);
            for (auto nei_idx = vx.neiBegin(); nei_idx != vx.neiEnd(); nei_idx = vx.neiNext(nei_idx))
            {
                if (su.atoms.find(vx.neiVertex(nei_idx)) == -1)
                    xbonds.push_back(vx.neiEdge(nei_idx));
            }
        }
        for (auto k : xbonds)
        {
            const Edge& edge = getEdge(k);
            int ap_aidx = -1;
            int ap_lvidx = -1;

            if (su.atoms.find(edge.beg) != -1)
            {
                ap_aidx = edge.beg;
                ap_lvidx = edge.end;
            }
            else
            {
                ap_aidx = edge.end;
                ap_lvidx = edge.beg;
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
        su.sa_class.appendString(kMonomerClassCHEM, true);

    int tg_idx = this->tgroups.addTGroup();
    TGroup& tg = this->tgroups.getTGroup(tg_idx);
    tg.tgroup_id = ++tg_id;

    tg.tgroup_class.copy(su.sa_class);

    if (su.subscript.size() > 0)
        tg.tgroup_name.copy(su.subscript);
    tg.tgroup_alias.clear();
    tg.tgroup_comment.clear();
    tg.tgroup_full_name.clear();
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

    int idx = this->addTemplateAtom(tg.tgroup_name.ptr());
    this->asMolecule().setTemplateAtomClass(idx, tg.tgroup_class.ptr());
    this->asMolecule().setTemplateAtomSeqid(idx, su.seqid);
    this->asMolecule().setTemplateAtomTemplateIndex(idx, tg_idx);

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

    if (super.sgroup_type == SGroup::SG_TYPE_SUP && static_cast<Superatom&>(super).unresolved)
    {
        static_cast<Superatom&>(sgroup).unresolved = true;
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
    _bond_directions.expandFill(idx + 1, BOND_DIRECTION_MONO);
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

void BaseMolecule::getAtomsCenterPoint(Vec2f& res)
{
    Array<int> atoms;
    for (auto i : vertices())
        atoms.push(i);
    getAtomsCenterPoint(atoms, res);
}

void BaseMolecule::setAtomsCenterPoint(const Vec3f& center)
{
    Vec2f old_center;
    getAtomsCenterPoint(old_center);
    Vec2f shift = Vec2f(center.x, center.y) - old_center;
    for (auto i : vertices())
    {
        Vec3f& p = getAtomXyz(i);
        p.x += shift.x;
        p.y += shift.y;
    }
}

float BaseMolecule::getBondsMeanLength()
{
    double bondSum = 0.0;
    for (auto j : edges())
    {
        const Edge& edge = getEdge(j);
        auto& v1 = getAtomXyz(edge.beg);
        auto& v2 = getAtomXyz(edge.end);
        float bondLength = std::hypot(v1.x - v2.x, v1.y - v2.y);
        bondSum += bondLength;
    }
    if (edgeCount())
        bondSum /= edgeCount();
    return static_cast<float>(bondSum);
}

void BaseMolecule::scale(const Vec2f& center, float scale)
{
    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
    {
        Vec3f& p = getAtomXyz(i);
        p.x = center.x + (p.x - center.x) * scale;
        p.y = center.y + (p.y - center.y) * scale;
        p.z *= scale;
    }
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
        QS_DEF(Array<int>, r_groups);
        int i;
        getAllowedRGroups(v, r_groups);

        if (r_groups.size() == 0)
        {
            result.readString("R", true);
            return;
        }

        ArrayOutput output(result);
        for (i = 0; i < r_groups.size(); i++)
        {
            if (i > 0)
                output.writeChar(',');
            output.printf("R%d", r_groups[i]);
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

void BaseMolecule::transformSuperatomsToTemplates(int template_id, MonomerTemplateLibrary* mtl)
{
    std::unordered_map<std::string, int> added_templates;
    std::vector<int> remove_sgroups;
    Array<int> remove_atoms;

    for (int tg_idx = tgroups.begin(); tg_idx != tgroups.end(); tg_idx = tgroups.next(tg_idx))
    {
        auto& tg = tgroups.getTGroup(tg_idx);
        auto res = tg.getResidue();
        if (res)
        {
            std::string templ_inchi_str;
            {
                StringOutput templ_inchi_output(templ_inchi_str);
                MoleculeInChI templ_inchi(templ_inchi_output);
                templ_inchi.outputInChI(res->asMolecule());
            }

            std::string template_inchi_id = std::string(tg.tgroup_name.ptr()) + "/" + std::string(tg.tgroup_class.ptr()) + "/" + templ_inchi_str;
            if (added_templates.count(template_inchi_id) == 0)
                added_templates.emplace(template_inchi_id, tg_idx);
        }
    }

    for (auto sg_idx = sgroups.begin(); sg_idx != sgroups.end(); sg_idx = sgroups.next(sg_idx))
    {
        auto& sg = sgroups.getSGroup(sg_idx);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            if (mtl && _replaceExpandedMonomerWithTemplate(sg_idx, template_id, *mtl, added_templates, remove_atoms))
                remove_sgroups.push_back(sg_idx);
            else if (tgroups.getTGroupCount()) // transform only for mixed case. Otherwise keep sgroups.
                _transformSGroupToTGroup(sg_idx, template_id);
        }
    }
    // remove S-groups that were transformed to templates
    std::sort(remove_sgroups.begin(), remove_sgroups.end(), std::greater<int>());
    for (auto sg_idx : remove_sgroups)
        removeSGroup(sg_idx);
    removeAtoms(remove_atoms);
}

int BaseMolecule::transformHELMtoSGroups(Array<char>& helm_class, Array<char>& helm_name, Array<char>& /*code*/, Array<char>& natreplace, StringPool& r_names)
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
    sg.subscript.copy(helm_name);
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
    catch (Exception&)
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
    _show_cip_atoms.clear();
    _cip_bonds.clear();
    have_cip = false;
}

CIPDesc BaseMolecule::getAtomCIP(int atom_idx)
{
    auto* pval = _cip_atoms.at2(atom_idx);
    return pval ? *pval : CIPDesc::NONE;
}

bool BaseMolecule::getShowAtomCIP(const int atomIndex)
{
    auto* pval = _show_cip_atoms.at2(atomIndex);
    return pval ? *pval : false;
}

CIPDesc BaseMolecule::getBondCIP(int bond_idx)
{
    auto* pval = _cip_bonds.at2(bond_idx);
    return pval ? *pval : CIPDesc::NONE;
}

void BaseMolecule::setAtomCIP(int atom_idx, CIPDesc cip)
{
    _cip_atoms.insert(atom_idx, cip);
    _show_cip_atoms.insert(atom_idx, true);
    have_cip = true;
}

void BaseMolecule::setShowAtomCIP(const int atomIndex, const bool display)
{
    auto* pval = _show_cip_atoms.at2(atomIndex);
    if (pval == nullptr)
    {
        _show_cip_atoms.insert(atomIndex, display);
    }
    else
    {
        *pval = display;
    }
}

void BaseMolecule::setBondCIP(int bond_idx, CIPDesc cip)
{
    _cip_bonds.insert(bond_idx, cip);
    have_cip = true;
}

void BaseMolecule::offsetCoordinates(const Vec3f& offset)
{
    for (int i = 0; i < _xyz.size(); i++)
        _xyz[i].add(offset);
}

void BaseMolecule::getAtomBoundingBox(int atom_idx, float font_size, LABEL_MODE label_mode, Vec2f& bottom_left, Vec2f& top_right)
{
    Vec2f vec = _xyz[atom_idx].projectZ();
    bottom_left = top_right = vec;
    if (font_size <= EPSILON)
        return;

    float constexpr WIDTH_FACTOR = 0.7f; // width of font symbols

    float symbol_size = font_size * WIDTH_FACTOR;

    if (isPseudoAtom(atom_idx) || isTemplateAtom(atom_idx))
    {
        const char* str = isPseudoAtom(atom_idx) ? getPseudoAtom(atom_idx) : getTemplateAtom(atom_idx);
        size_t len = strlen(str);
        Vec2f shift(len * symbol_size / 2.0f, symbol_size); // TODO: Add pseudoatom parsing
        bottom_left.sub(shift);
        top_right.add(shift);
    }
    else
    {

        int charge = getAtomCharge(atom_idx);
        int isotope = getAtomIsotope(atom_idx);
        int radical = -1;
        int valence = getExplicitValence(atom_idx);
        bool query = isQueryMolecule();
        int implicit_h = 0;
        const Vertex& vertex = getVertex(atom_idx);
        int atomNumber = getAtomNumber(atom_idx);
        int label = 0;
        bool atom_regular = !query || QueryMolecule::queryAtomIsRegular(asQueryMolecule(), atom_idx);

        if (!isRSite(atom_idx))
        {
            if (atom_regular)
                label = atomNumber;
            radical = getAtomRadical_NoThrow(atom_idx, -1);
            if (!query)
                implicit_h = asMolecule().getImplicitH_NoThrow(atom_idx, 0);
        }

        bool plainCarbon = label == ELEM_C && charge == (query ? CHARGE_UNKNOWN : 0) && isotope == (query ? -1 : 0) && radical <= 0 && valence == -1;
        bool showLabel = true;
        if (label_mode == LABEL_MODE_ALL || vertex.degree() == 0)
            ;
        else if (label_mode == LABEL_MODE_NONE)
            showLabel = false;
        else if (plainCarbon && (label_mode == LABEL_MODE_HETERO || vertex.degree() > 1))
        {
            showLabel = false;
            if (vertex.degree() == 2)
            {
                int k1 = vertex.neiBegin();
                int k2 = vertex.neiNext(k1);
                if (getBondOrder(vertex.neiEdge(k1)) == getBondOrder(vertex.neiEdge(k2)))
                {
                    int a1 = vertex.neiVertex(k1);
                    int a2 = vertex.neiVertex(k2);
                    Vec2f vk1(_xyz[a1].x, _xyz[a1].y);
                    Vec2f vk2(_xyz[a2].x, _xyz[a2].y);
                    Vec2f dir_k1, dir_k2;
                    dir_k1.diff(vec, vk1);
                    dir_k1.normalize();
                    dir_k2.diff(vec, vk2);
                    dir_k2.normalize();
                    float dot = Vec2f::dot(dir_k1, dir_k2);
                    if (dot < -0.97)
                        showLabel = true;
                }
            }
        }
        if (showLabel)
        {
            // calc label size
            size_t len = 0;
            if (query && !atom_regular)
            {
                Array<int> list;
                int atom_type = QueryMolecule::parseQueryAtom(asQueryMolecule(), atom_idx, list);
                switch (atom_type)
                {
                case QueryMolecule::QUERY_ATOM_A:
                case QueryMolecule::QUERY_ATOM_X:
                case QueryMolecule::QUERY_ATOM_Q:
                case QueryMolecule::QUERY_ATOM_M:
                    len = 1;
                    break;
                case QueryMolecule::QUERY_ATOM_AH:
                case QueryMolecule::QUERY_ATOM_XH:
                case QueryMolecule::QUERY_ATOM_QH:
                case QueryMolecule::QUERY_ATOM_MH:
                case QueryMolecule::QUERY_ATOM_SINGLE:
                    len = 2;
                    break;
                case QueryMolecule::QUERY_ATOM_LIST:
                case QueryMolecule::QUERY_ATOM_NOTLIST:
                    len = 1 + list.size() / 2;
                    for (int i = 0; i < list.size(); i++)
                    {
                        len += strlen(Element::toString(list[i]));
                    }
                    break;
                }
            }
            else
            {
                len = strlen(Element::toString(label));
            }
            Vec2f shift(len * symbol_size / 2.0f, symbol_size);
            bottom_left.sub(shift);
            top_right.add(shift);
            // Add isotope at left
            if (isotope > 0 && !(label = ELEM_H && (isotope == DEUTERIUM || isotope == TRITIUM)))
            {
                if (isotope > 99)
                    len = 3;
                else if (isotope > 9)
                    len = 2;
                else
                    len = 1;
                bottom_left.x -= len * symbol_size;
            }
            // Add valence at right
            if (valence > 0)
            {
                static constexpr int count[] = {
                    1, // 0
                    1, // I
                    2, // II
                    3, // III
                    2, // IV
                    1, // V
                    2, // VI
                    3, // VII
                    4, // VIII
                    2, // IX
                    1, // X

                };
                top_right.x += count[valence] * symbol_size;
            }
            // Add charge at right
            if (charge != 0)
            {
                if (abs(charge) > 9)
                    len = 3;
                else if (abs(charge) > 1)
                    len = 2;
                else
                    len = 1;
                top_right.x += len * symbol_size;
            }
            if (implicit_h > 0)
            {
                // add implicit H size
                if (implicit_h > 1)
                    len = 2;
                else
                    len = 1;
                bool h_at_right = true;
                if (vertex.degree() == 0)
                {
                    if (ElementHygrodenOnLeft(label))
                        h_at_right = false;
                }
                else
                {
                    constexpr float min_sin = 0.49f;
                    float right_weight = 0.3f;
                    float left_weight = 0.2f;
                    float left_sin = 0, right_sin = 0;
                    for (int j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
                    {
                        Vec2f d = _xyz[vertex.neiVertex(j)].projectZ();
                        d.sub(vec);
                        d.normalize();
                        if (d.x > 0)
                            right_sin = std::max(right_sin, d.x);
                        else
                            left_sin = std::max(left_sin, -d.x);
                    }
                    if (left_sin > min_sin)
                        left_weight -= left_sin;
                    if (right_sin > min_sin)
                        right_weight -= right_sin;
                    if (left_weight > right_weight)
                        h_at_right = false;
                }
                if (h_at_right)
                    top_right.x += len * symbol_size;
                else
                    bottom_left.x -= len * symbol_size;
            }
        }
    }
    // process AAM
}

void BaseMolecule::getBoundingBox(float font_size, LABEL_MODE label_mode, Vec2f& bottom_left, Vec2f& top_right)
{
    Vec2f atom_bottom_left, atom_top_right;
    for (int atom_idx = 0; atom_idx < vertexCount(); ++atom_idx)
    {
        getAtomBoundingBox(atom_idx, font_size, label_mode, atom_bottom_left, atom_top_right);
        if (!atom_idx)
        {
            bottom_left = atom_bottom_left;
            top_right = atom_top_right;
        }
        else
        {
            bottom_left.min(atom_bottom_left);
            top_right.max(atom_top_right);
        }
    }
}

void BaseMolecule::getBoundingBox(float font_size, LABEL_MODE label_mode, Rect2f& bbox)
{
    Vec2f a, b;
    getBoundingBox(font_size, label_mode, a, b);
    bbox = Rect2f(a, b);
}

// Andrew's monotone chain convex hull algorithm
std::vector<Vec2f> BaseMolecule::getConvexHull(const Vec2f& min_box) const
{
    std::vector<Vec2f> vertices;
    std::transform(_xyz.ptr(), _xyz.ptr() + _xyz.size(), std::back_inserter(vertices), [](const Vec3f& v) -> Vec2f { return Vec2f(v.x, v.y); });
    if (vertices.size() < 3)
    {
        Rect2f bbox;
        getBoundingBox(bbox, min_box);
        vertices.clear();
        vertices.emplace_back(bbox.leftTop());
        vertices.emplace_back(bbox.rightTop());
        vertices.emplace_back(bbox.rightBottom());
        vertices.emplace_back(bbox.leftBottom());
        return vertices;
    }
    std::sort(vertices.begin(), vertices.end());
    std::vector<Vec2f> hull;
    for (const auto& p : vertices)
    {
        while (hull.size() >= 2 && hull[hull.size() - 2].relativeCross(hull.back(), p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }
    size_t lower_size = hull.size();
    for (auto it = vertices.rbegin(); it != vertices.rend(); ++it)
    {
        while (hull.size() > lower_size && hull[hull.size() - 2].relativeCross(hull.back(), *it) <= 0)
            hull.pop_back();
        hull.push_back(*it);
    }
    hull.pop_back();
    return hull;
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

int BaseMolecule::countTemplateAtoms()
{
    int mon_count = 0;
    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
    {
        if (isTemplateAtom(i))
            mon_count++;
    }
    return mon_count;
}

void BaseMolecule::unfoldHydrogens(Array<int>* markers_out, int max_h_cnt, bool impl_h_no_throw, bool only_selected)
{
    int v_end = vertexEnd();

    QS_DEF(Array<int>, imp_h_count);
    imp_h_count.clear_resize(vertexEnd());
    imp_h_count.zerofill();

    // getImplicitH can throw an exception, and we need to get the number of hydrogens
    // before unfolding them
    for (int i = vertexBegin(); i < v_end; i = vertexNext(i))
    {
        if (!only_selected || isAtomSelected(i))
        {
            if (isPseudoAtom(i) || isRSite(i) || isTemplateAtom(i))
                continue;

            imp_h_count[i] = getImplicitH(i, impl_h_no_throw);
        }
    }

    if (markers_out != 0)
    {
        markers_out->clear_resize(vertexEnd());
        markers_out->zerofill();
    }

    for (int i = vertexBegin(); i < v_end; i = vertexNext(i))
    {
        if (!only_selected || isAtomSelected(i))
        {
            int impl_h = imp_h_count[i];
            if (impl_h == 0)
                continue;

            int h_cnt;
            if ((max_h_cnt == -1) || (max_h_cnt > impl_h))
                h_cnt = impl_h;
            else
                h_cnt = max_h_cnt;

            for (int j = 0; j < h_cnt; j++)
            {
                int new_h_idx = addAtom(ELEM_H);
                int new_bond_idx = addBond(i, new_h_idx, BOND_SINGLE);

                if (only_selected) // if only selected atoms - select new H too
                {
                    selectAtom(new_h_idx);
                    selectBond(new_bond_idx);
                }

                if (markers_out != 0)
                {
                    markers_out->expandFill(new_h_idx + 1, 0);
                    markers_out->at(new_h_idx) = 1;
                }

                stereocenters.registerUnfoldedHydrogen(i, new_h_idx);
                cis_trans.registerUnfoldedHydrogen(*this, i, new_h_idx);
                allene_stereo.registerUnfoldedHydrogen(i, new_h_idx);
                sgroups.registerUnfoldedHydrogen(i, new_h_idx);
            }

            setImplicitH(i, impl_h - h_cnt);
        }
    }

    updateEditRevision();
}

bool BaseMolecule::convertableToImplicitHydrogen(int idx)
{
    // TODO: add check for query features defined for H, do not remove such hydrogens
    if (getAtomNumber(idx) == ELEM_H && getAtomIsotope(idx) <= 0 && getVertex(idx).degree() == 1)
    {
        int nei = getVertex(idx).neiVertex(getVertex(idx).neiBegin());
        if (getAtomNumber(nei) == ELEM_H && getAtomIsotope(nei) <= 0)
        {
            // This is H-H connection
            int edge_idx = findEdgeIndex(idx, nei);
            if (edge_idx < 0)
                return false;
            const Edge& edge = getEdge(edge_idx);
            if (idx == edge.end) // if this is second H - remove it
                return true;
            else
                return false;
        }
        if (stereocenters.getType(nei) > 0)
            if (getVertex(nei).degree() == 3)
                return false; // not ignoring hydrogens around stereocenters with lone pair

        if (!cis_trans.convertableToImplicitHydrogen(*this, idx))
            return false;

        return true;
    }
    return false;
}

bool BaseMolecule::getUnresolvedTemplatesList(BaseMolecule& bmol, std::string& unresolved)
{
    unresolved.clear();
    if (!bmol.isQueryMolecule())
    {
        if (bmol.tgroups.getTGroupCount())
        {
            for (auto tgidx = bmol.tgroups.begin(); tgidx != bmol.tgroups.end(); tgidx = bmol.tgroups.next(tgidx))
            {
                auto& tg = bmol.tgroups.getTGroup(tgidx);
                if (tg.unresolved && tg.tgroup_alias.size())
                {
                    if (unresolved.size())
                        unresolved += ',';
                    unresolved += tg.tgroup_alias.ptr();
                }
            }
        }
    }
    return unresolved.size();
}

void BaseMolecule::getTemplatesMap(std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash>& templates_map)
{
    templates_map.clear();
    for (int i = tgroups.begin(); i != tgroups.end(); i = tgroups.next(i))
    {
        auto& tg = tgroups.getTGroup(i);
        if (tg.tgroup_name.size() > 0)
        {
            templates_map.emplace(std::make_pair(tg.tgroup_name.ptr(), tg.tgroup_class.ptr()), std::ref(tg));
            if (tg.tgroup_alias.size() > 0)
                templates_map.emplace(std::make_pair(tg.tgroup_alias.ptr(), tg.tgroup_class.ptr()), std::ref(tg));
        }
        else
        {
            templates_map.emplace(std::make_pair(monomerAlias(tg), tg.tgroup_class.ptr()), std::ref(tg));
        }
    }
}

void BaseMolecule::transformTemplatesToSuperatoms()
{
    if (tgroups.getTGroupCount())
    {
        std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash> templates;
        bool modified = false;
        getTemplatesMap(templates);
        for (auto atom_idx = vertexBegin(); atom_idx < vertexEnd(); atom_idx = vertexNext(atom_idx))
        {
            if (isTemplateAtom(atom_idx))
            {
                auto tg_idx = getTemplateAtomTemplateIndex(atom_idx);
                if (tg_idx < 0)
                {
                    std::string alias = getTemplateAtomClass(atom_idx);
                    std::string mon_class = getTemplateAtom(atom_idx);
                    auto tg_ref = findTemplateInMap(alias, mon_class, templates);
                    if (tg_ref.has_value())
                    {
                        auto& tg = tg_ref.value().get();
                        tg_idx = tg.tgroup_id;
                    }
                }
                if (tg_idx != -1)
                {
                    _transformTGroupToSGroup(atom_idx, tg_idx);
                    modified = true;
                }
            }
        }
        tgroups.clear();
        template_attachment_points.clear();
        template_attachment_indexes.clear();
    }
}

std::string BaseMolecule::getAtomDescription(int idx)
{
    Array<char> description;
    getAtomDescription(idx, description);
    return std::string(description.ptr(), description.size());
}

KetDocument& BaseMolecule::getKetDocument()
{
    // static thread_local std::optional<std::unique_ptr<KetDocument>> document; // Temporary until direct conversion to document supported
    if (_document == nullptr || _edit_revision != _document_revision)
    {
        if (_document != nullptr)
        {
            delete _document;
            _document = nullptr;
        }
        // save to ket
        std::string json;
        StringOutput out(json);
        MoleculeJsonSaver saver(out);
        saver.saveMolecule(*this);
        // load document from ket
        rapidjson::Document data;
        /*auto& res*/ std::ignore = data.Parse(json.c_str());
        // if res.hasParseError()
        _document = new KetDocument;
        KetDocumentJsonLoader loader{};
        loader.parseJson(json, *_document);
        _document_revision = _edit_revision;
    }
    return *_document;
}

const char* BaseMolecule::getTemplateAtom(int idx)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    const char* res = _template_names.at(occur.name_idx);

    if (res == 0)
        throw Error("template atom string is zero");

    return res;
}

const char* BaseMolecule::getTemplateAtomClass(int idx)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    const char* res = _template_classes.at(occur.class_idx);

    return res;
}

const char* BaseMolecule::getTemplateAtomSeqName(int idx)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    return occur.seq_name.ptr();
}

const int BaseMolecule::getTemplateAtomTemplateIndex(int idx)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    const int res = occur.template_idx;
    return res;
}

const int BaseMolecule::getTemplateAtomSeqid(int idx)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    const int res = occur.seq_id;

    return res;
}

const DisplayOption BaseMolecule::getTemplateAtomDisplayOption(int idx) const
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    const _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);

    return occur.contracted;
}

const Transformation& BaseMolecule::getTemplateAtomTransform(int idx) const
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    const _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);

    return occur.transform;
}

void BaseMolecule::renameTemplateAtom(int idx, const char* text)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    _template_names.set(occur.name_idx, text);
    updateEditRevision();
}

void BaseMolecule::setTemplateAtomName(int idx, const char* text)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    occur.name_idx = _template_names.add(text);
    updateEditRevision();
}

void BaseMolecule::setTemplateAtomClass(int idx, const char* text)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    occur.class_idx = _template_classes.add(text);
    updateEditRevision();
}

void BaseMolecule::setTemplateAtomSeqid(int idx, int seq_id)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    occur.seq_id = seq_id;
    updateEditRevision();
}

void BaseMolecule::setTemplateAtomSeqName(int idx, const char* seq_name)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    occur.seq_name.readString(seq_name, true);
    updateEditRevision();
}

void BaseMolecule::setTemplateAtomTemplateIndex(int idx, int temp_idx)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    occur.template_idx = temp_idx;
    updateEditRevision();
}

void BaseMolecule::setTemplateAtomDisplayOption(int idx, DisplayOption option)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    occur.contracted = option;
    updateEditRevision();
}

void BaseMolecule::setTemplateAtomTransform(int idx, const Transformation& transform)
{
    int template_occur_idx = getTemplateAtomOccurrence(idx);
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    occur.transform = transform;
    updateEditRevision();
}

int BaseMolecule::getExpandedMonomerCount() const
{
    int count = 0;
    for (auto vertex : vertices())
    {
        if (isTemplateAtom(vertex) && getTemplateAtomDisplayOption(vertex) == DisplayOption::Expanded)
            count++;
    }
    return count;
}

std::unique_ptr<BaseMolecule>& BaseMolecule::expandedMonomersToAtoms()
{
    std::unique_ptr<BaseMolecule>& result = _with_expanded_monomers;
    result.reset(neu());
    result->clone(*this);
    std::list<int> att_indexes_to_remove;
    std::list<int> monomer_ids;
    for (int monomer_id = result->vertexBegin(); monomer_id != result->vertexEnd(); monomer_id = result->vertexNext(monomer_id))
    {
        if (result->isTemplateAtom(monomer_id))
            monomer_ids.push_front(monomer_id);
    }
    std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash> templates;
    getTemplatesMap(templates);
    for (int monomer_id : monomer_ids)
    {
        int template_occur_idx = result->getTemplateAtomOccurrence(monomer_id);
        _TemplateOccurrence& occur = result->_template_occurrences.at(template_occur_idx);
        if (occur.contracted != DisplayOption::Expanded)
            continue;

        std::optional<std::reference_wrapper<TGroup>> tgroup_opt;
        int template_idx = occur.template_idx;
        if (template_idx == -1)
        {
            auto tg_ref = findTemplateInMap(getTemplateAtom(monomer_id), getTemplateAtomClass(monomer_id), templates);
            if (!tg_ref.has_value())
                continue;
            tgroup_opt = tg_ref.value();
        }
        else
        {
            tgroup_opt = result->tgroups.getTGroup(template_idx);
        }

        auto& tgroup = tgroup_opt.value().get();
        if (tgroup.unresolved)
            continue;

        auto monomer_mol = tgroup.fragment->applyTransformation(result->getTemplateAtomTransform(monomer_id), result->getAtomXyz(monomer_id));
        auto& monomer_sgroups = monomer_mol->sgroups;
        std::map<int, int> attached_atom;
        Array<int> atoms_to_remove;
        // Attachment points stored in SuperAtom SGroups
        for (int j = monomer_sgroups.begin(); j != monomer_sgroups.end(); j = monomer_sgroups.next(j))
        {
            SGroup& sg = monomer_sgroups.getSGroup(j);
            if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                auto& sa = static_cast<Superatom&>(sg);
                if (sa.attachment_points.size())
                {
                    std::map<std::string, int> sorted_attachment_points; // AP id to index in attachment points
                    for (int i = sa.attachment_points.begin(); i != sa.attachment_points.end(); i = sa.attachment_points.next(i))
                    {
                        auto& atp = sa.attachment_points[i];
                        std::string atp_id_str(atp.apid.ptr());
                        if (atp_id_str.size())
                            sorted_attachment_points.insert(std::make_pair(atp_id_str, i));
                    }
                    // for all used AP mark leaving atom to remove, all leaving atom are leafs - so bonds will be removed automatically
                    if (monomer_id < result->template_attachment_indexes.size()) // check if monomer has attachment points in use
                    {
                        auto& indexes = result->template_attachment_indexes.at(monomer_id);
                        for (int att_idx = 0; att_idx < indexes.size(); att_idx++)
                        {
                            auto& ap = result->template_attachment_points.at(indexes.at(att_idx));
                            assert(ap.ap_occur_idx == monomer_id);
                            auto it = sorted_attachment_points.find(ap.ap_id.ptr());
                            if (it != sorted_attachment_points.end())
                            {
                                auto& atp = sa.attachment_points[it->second];
                                attached_atom.insert(std::make_pair(ap.ap_aidx, atp.aidx)); // molecule atom ap.ap_aidx is attached to atp.aidx in monomer
                                atoms_to_remove.push(atp.lvidx);
                            }
                            result->template_attachment_points.remove(indexes.at(att_idx));
                        }
                        att_indexes_to_remove.emplace_back(monomer_id);
                    }
                }
                monomer_sgroups.remove(j);
            }
        }
        Array<int> atom_map;
        result->mergeWithMolecule(*monomer_mol, &atom_map);
        // update atom indexes
        for (int* i = atoms_to_remove.begin(); i != atoms_to_remove.end(); i++)
        {
            *i = atom_map[*i];
        }
        // add bonds from template atom neighbors to attachment points
        const Vertex& v = result->getVertex(monomer_id);
        Array<int> bonds_to_delete;
        for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
        {
            int edge_idx = result->findEdgeIndex(monomer_id, v.neiVertex(k));
            if (edge_idx >= 0 && result->getBondOrder(edge_idx) == BOND_SINGLE)
            {
                int a1 = result->getEdge(edge_idx).beg;
                int a2 = result->getEdge(edge_idx).end;
                int other = a1 == monomer_id ? a2 : a1;
                auto it = attached_atom.find(other);
                if (it != attached_atom.end())
                {
                    int ap_new_idx = atom_map[it->second];
                    int new_bond = result->addBond_Silent(ap_new_idx, other, BOND_SINGLE);
                    result->setBondDirection(new_bond, BOND_DIRECTION_MONO);
                    // fix neighbor template_attachment_points
                    if (other < result->template_attachment_indexes.size())
                    {
                        auto& indexes = result->template_attachment_indexes.at(other);
                        for (int att_idx = 0; att_idx < indexes.size(); att_idx++)
                        {
                            auto& ap = result->template_attachment_points.at(indexes.at(att_idx));
                            if (ap.ap_aidx == monomer_id)
                                ap.ap_aidx = ap_new_idx;
                        }
                    }
                }
                bonds_to_delete.push(edge_idx);
            }
        }
        // remove template atom and all bonds to it
        result->removeBonds(bonds_to_delete);
        result->removeAtom(monomer_id);
        // remove leaved atoms
        result->removeAtoms(atoms_to_remove);
    }
    att_indexes_to_remove.sort(std::greater<int>());
    for (int idx : att_indexes_to_remove)
        result->template_attachment_indexes.remove(idx);
    return result;
}

/*
 *  Returns the copy of molecule with next coordinate transformation:
 *  1) translate coords so that the center of bounding box is at the point 'position'
 *  2) rotate around the center of bounding box by angle 'rotation'
 *  3) shift by vector 'shift'
 */
std::unique_ptr<BaseMolecule> BaseMolecule::applyTransformation(const Transformation& transform, const Vec2f position)
{
    BaseMolecule* result = neu();
    result->clone_KeepIndices(*this);
    Transform3f matr;
    matr.identity();
    if (transform.hasTransformation())
    {
        Rect2f bbox;
        result->getBoundingBox(bbox);
        if (transform.flip != Transformation::FlipType::none)
        {
            // dx = px - cx, px = cx - dx = cx - px + cx = 2*cx - px
            Vec2f center = bbox.center() * 2.0;
            if (transform.flip == Transformation::FlipType::horizontal)
            {
                for (auto atom_idx : result->vertices())
                {
                    Vec3f& p = result->getAtomXyz(atom_idx);
                    p.x = center.x - p.x;
                    result->setAtomXyz(atom_idx, p);
                }
            }
            else if (transform.flip == Transformation::FlipType::vertical)
            {
                for (auto atom_idx : result->vertices())
                {
                    Vec3f& p = result->getAtomXyz(atom_idx);
                    p.y = center.y - p.y;
                    result->setAtomXyz(atom_idx, p);
                }
            }
            // Fix bonds - change up to down and vice versa
            for (int i = 0; i < result->edgeCount(); i++)
            {
                switch (result->getBondDirection(i))
                {
                case BOND_DOWN:
                    result->setBondDirection(i, BOND_UP);
                    break;
                case BOND_UP:
                    result->setBondDirection(i, BOND_DOWN);
                    break;
                case BOND_DOWN_OR_UNSPECIFIED:
                    result->setBondDirection(i, BOND_UP_OR_UNSPECIFIED);
                    break;
                case BOND_UP_OR_UNSPECIFIED:
                    result->setBondDirection(i, BOND_DOWN_OR_UNSPECIFIED);
                    break;
                default:
                    break;
                }
            }
        }
        if (false) // straight transformation
        {
            matr.translateInv(bbox.center()); // translate to move bounding center to (0,0)
            if (transform.rotate != 0)
            {
                Transform3f rot;
                rot.rotateZ(transform.rotate); // rotate around Z axis
                matr.transform(rot);           // rotate after translation
            }
        }
        else // 2DO: check if this is correct. Also add comment to translateLocal
        {
            if (transform.rotate != 0)
                matr.rotationZ(transform.rotate);
            matr.translateLocalInv(bbox.center());
        }

        if (transform.shift.x != 0 || transform.shift.y != 0)
            matr.translate(transform.shift); // apply shift
    }
    matr.translate(position); // translate to move bounding center to position point
    for (auto atom_idx : result->vertices())
    {
        Vec3f& p = result->getAtomXyz(atom_idx);
        p.transformPoint(matr);
    }

    return std::unique_ptr<BaseMolecule>{result};
}

bool BaseMolecule::convertTemplateAtomsToSuperatoms(bool only_transformed)
{
    bool modified = false;
    if (tgroups.getTGroupCount())
    {
        std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash> templates;
        getTemplatesMap(templates);
        for (auto vi : vertices())
        {
            if (isTemplateAtom(vi) &&
                (!only_transformed || getTemplateAtomTransform(vi).hasTransformation() || getTemplateAtomDisplayOption(vi) == DisplayOption::Expanded))
            {
                auto tg_idx = getTemplateAtomTemplateIndex(vi);
                if (tg_idx < 0)
                {
                    std::string alias = getTemplateAtomClass(vi);
                    std::string mon_class = getTemplateAtom(vi);
                    auto tg_ref = findTemplateInMap(alias, mon_class, templates);
                    if (tg_ref.has_value())
                    {
                        auto& tg = tg_ref.value().get();
                        tg_idx = tg.tgroup_id;
                    }
                }
                if (tg_idx != -1)
                {
                    _transformTGroupToSGroup(vi, tg_idx);
                    modified = true;
                }
            }
        }
    }
    return modified;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
