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
        setTemplateAtomSeqid(v_idx, seq_id);
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
    QS_DEF(Array<int>, lv_dirs);
    QS_DEF(Array<char>, lv_atom_is_beg);

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
    lv_dirs.clear();
    lv_atom_is_beg.clear();

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
            int edge = fragment->findEdgeIndex(ap.aidx, ap.lvidx);
            int dir = BOND_DIRECTION_MONO;
            bool lv_beg = false;
            if (edge >= 0)
            {
                dir = fragment->getBondDirection(edge);
                lv_beg = fragment->_edges[edge].beg == ap.lvidx;
            }
            lv_dirs.push(dir);
            lv_atom_is_beg.push((char)lv_beg);
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
                int leaving_atom = -1;
                if (i < lv_atoms.size() && lv_atoms[i] >= 0 && lv_atoms[i] < mapping.size())
                    leaving_atom = mapping[lv_atoms[i]];

                int edge_idx = findEdgeIndex(att_atoms[i], idx);
                if (edge_idx > -1)
                {
                    flipBondWithDirection(att_atoms[i], idx, mapping[tg_atoms[i]], leaving_atom);
                }
                else
                {
                    if (isTemplateAtom(att_atoms[i]))
                    {
                        int ap_count = getTemplateAtomAttachmentPointsCount(att_atoms[i]);
                        for (int m = 0; m < ap_count; m++)
                        {
                            if (getTemplateAtomAttachmentPoint(att_atoms[i], m) == idx)
                            {
                                getTemplateAtomAttachmentPointId(att_atoms[i], m, ap_id);
                                int added_bond = addBond(att_atoms[i], mapping[tg_atoms[i]], BOND_SINGLE);
                                (void)added_bond;
                                _flipTemplateAtomAttachmentPoint(att_atoms[i], idx, ap_id, mapping[tg_atoms[i]]);
                            }
                        }
                    }
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
                    if (su.bonds.find(bond_idx) == -1)
                        su.bonds.push(bond_idx);
                }
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

    // skip LGRP
    if (sa.sa_class.size() && std::string(sa.sa_class.ptr()) == "LGRP")
        return false;

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
    bool is_added = it_added == added_templates.end();
    int tg_index = is_added ? tgroups.addTGroup() : it_added->second;
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
    if (!res && is_added)
    {
        tgroups.remove(tg_index);
        tg_id--;
    }

    return res;
}

int BaseMolecule::_transformSGroupToTGroup(int sg_idx, int& tg_id)
{
    QS_DEF(Array<int>, leaving_atoms);
    QS_DEF(Array<int>, tgroup_atoms);
    QS_DEF(Array<int>, residue_atoms);
    QS_DEF(Array<int>, mapping);
    QS_DEF(Array<int>, ap_points_atoms);
    QS_DEF(StringPool, ap_points_ids);
    QS_DEF(Array<int>, ap_ids);
    QS_DEF(Array<int>, fragment_sgroups);
    QS_DEF(Array<int>, sgs_tmp);

    mapping.clear();
    tgroup_atoms.clear();
    leaving_atoms.clear();

    if (!sgroups.hasSGroup(sg_idx))
        return -1;

    Superatom& su = (Superatom&)sgroups.getSGroup(sg_idx);

    if (su.subscript.size() == 0)
        return -1;

    if (su.sa_class.size() && std::string(su.sa_class.ptr()) == "LGRP")
        return -1;

    for (auto k : su.atoms)
    {
        auto& vx = getVertex(k);
        for (auto nei_idx = vx.neiBegin(); nei_idx != vx.neiEnd(); nei_idx = vx.neiNext(nei_idx))
        {
            if (su.atoms.find(vx.neiVertex(nei_idx)) == -1)
            {
                if (su.atoms.find(vx.neiVertex(nei_idx)) == -1)
                {
                    if (getBondDirection(vx.neiEdge(nei_idx)))
                        return -1;
                }
            }
        }
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
            if (ap.lvidx >= 0)
            {
                Array<int> latoms, lgroups;
                latoms.push(ap.lvidx);
                sgroups.findSGroups(SGroup::SG_ATOMS, latoms, lgroups);
                for (auto lg_idx : lgroups)
                {
                    SGroup& lsg = sgroups.getSGroup(lg_idx);
                    if (lsg.sgroup_type == SGroup::SG_TYPE_SUP)
                    {
                        Superatom& lsa = (Superatom&)lsg;
                        if (lsa.sa_class.size() && std::string(lsa.sa_class.ptr()) == "LGRP")
                            leaving_atoms.push(ap.lvidx);
                    }
                }
            }
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
    tgroup_atoms.copy(su.atoms);
    tgroup_atoms.concat(leaving_atoms);
    tg.fragment->makeSubmolecule(*this, tgroup_atoms, &mapping, SKIP_TGROUPS | SKIP_TEMPLATE_ATTACHMENT_POINTS);

    residue_atoms.clear();
    // collect residue atoms
    for (int j = 0; j < su.atoms.size(); j++)
    {
        if (mapping[su.atoms[j]] != -1)
            residue_atoms.push(mapping[su.atoms[j]]);
    }

    fragment_sgroups.clear();
    // find all s-groups that contain residue atoms
    tg.fragment->sgroups.findSGroups(SGroup::SG_ATOMS, residue_atoms, fragment_sgroups);

    // find residue superatom s-group
    int residue_sg_idx = -1;
    for (int j = 0; j < fragment_sgroups.size(); j++)
    {
        SGroup& sg = tg.fragment->sgroups.getSGroup(fragment_sgroups[j]);
        // remove all s-groups that are not superatoms
        if ((sg.sgroup_type != SGroup::SG_TYPE_SUP) || (sg.atoms.size() != su.atoms.size()))
        {
            tg.fragment->sgroups.remove((fragment_sgroups[j]));
        }
        else
        {
            Superatom& sup_new = (Superatom&)sg;
            if ((strcmp(su.subscript.ptr(), sup_new.subscript.ptr()) == 0) && (su.attachment_points.size() == sup_new.attachment_points.size()))
            {
                residue_sg_idx = fragment_sgroups[j];
            }
        }
    }

    fragment_sgroups.clear();
    // delete all superatoms that are not residue superatom and not leaving groups
    for (int j = tg.fragment->sgroups.begin(); j != tg.fragment->sgroups.end(); j = tg.fragment->sgroups.next(j))
    {
        auto& sa = (Superatom&)tg.fragment->sgroups.getSGroup(j);
        if (j != residue_sg_idx && (!sa.sa_class.size() || sa.sa_class.ptr() != std::string("LGRP")))
            fragment_sgroups.push(j);
    }

    for (int j = 0; j < fragment_sgroups.size(); j++)
        tg.fragment->sgroups.remove((fragment_sgroups[j]));

    int idx = addTemplateAtom(tg.tgroup_name.ptr());
    setTemplateAtomClass(idx, tg.tgroup_class.ptr());
    setTemplateAtomSeqid(idx, su.seqid);
    setTemplateAtomDisplayOption(idx, su.contracted);
    setTemplateAtomTemplateIndex(idx, tg_idx);

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
                if (su.atoms.find(v.neiVertex(k)) == -1 && leaving_atoms.find(v.neiVertex(k)) == -1)
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
                        flipBondWithDirection(v_k, att_point_idx, idx, -1);
                    setTemplateAtomAttachmentOrder(idx, v_k, ap_points_ids.at(ap_ids[j]));
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
    QS_DEF(Vec3f, pcenter);
    pcenter.set(0, 0, 0);
    getAtomsCenterPoint(su.atoms, cp);
    pcenter.x = cp.x;
    pcenter.y = cp.y;
    setAtomXyz(idx, pcenter);
    removeAtoms(tgroup_atoms);
    // fix template atoms' coordinates
    for (auto vidx : tg.fragment->vertices())
    {
        auto p = tg.fragment->getAtomXyz(vidx);
        p.sub(pcenter);
        tg.fragment->setAtomXyz(vidx, p);
    }
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

    MoleculeSubstructureMatcher matcher(*this);
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

    MoleculeSubstructureMatcher matcher(*this);
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
                        if (fragment.getBondOrder(q_xbond_idx) == getBondOrder(t_xbond_idx))
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif
