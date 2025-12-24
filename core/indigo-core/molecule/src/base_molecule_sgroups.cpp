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

    bool arom = aromatize(arom_opt);

    templates.qsort(TGroup::cmp, 0);

    for (auto i = 0; i < templates.size(); i++)
    {
        const TGroup& tg = templates.at(i);

        fragment.clear();
        // ambiguous templates have no fragment
        if (tg.fragment.get() == nullptr)
            continue;
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
            MoleculeExactSubstructureMatcher matcher(fragment, asMolecule());

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
                                    if (fragment.getBondOrder(q_xbond_idx) != getBondOrder(t_xbond_idx))
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
                    if (getAtomCharge(remove_atoms[j]) != 0)
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
                        if (target.getAtomCharge(j) != 0)
                            target.setAtomCharge(j, 0);
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

            int idx = addTemplateAtom(tg.tgroup_name.ptr());
            setTemplateAtomClass(idx, tg.tgroup_class.ptr());

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
        // ambiguous templates have no fragment
        if (tg.fragment.get() == nullptr)
            continue;
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
            MoleculeExactSubstructureMatcher matcher(fragment, asMolecule());

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
                                    if (fragment.getBondOrder(q_xbond_idx) != getBondOrder(t_xbond_idx))
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
                    if (getAtomCharge(remove_atoms[j]) != 0)
                        charged = true;
                }
                //            if (charged)
                //               continue;

                bool isotopic = false;
                for (int j = 0; j < remove_atoms.size(); j++)
                {
                    if (getAtomIsotope(remove_atoms[j]) != 0)
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
                        if (target.getAtomCharge(j) != 0)
                            target.setAtomCharge(j, 0);
                        if (isotopic)
                        {
                            if (target.getAtomIsotope(j) != 0)
                                target.setAtomIsotope(j, 0);
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

            int idx = addTemplateAtom(tg.tgroup_name.ptr());
            setTemplateAtomClass(idx, tg.tgroup_class.ptr());

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
        dearomatize(arom_opt);

    return result;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
