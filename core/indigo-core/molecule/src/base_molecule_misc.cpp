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
            auto& sa = (Superatom&)sg;
            if (sa.sa_class.size())
            {
                if (!mtl || mtl->monomerTemplates().empty() || !_replaceExpandedMonomerWithTemplate(sg_idx, template_id, *mtl, added_templates, remove_atoms))
                {
                    if (isAminoAcidClass(sa.sa_class.ptr()) || isChemClass(sa.sa_class.ptr()) || isNucleicClass(sa.sa_class.ptr()))
                        _transformSGroupToTGroup(sg_idx, template_id);
                }
            }
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
            asMolecule().resetAtom(i, Element::fromString(r_names.at(r_num)));

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

#ifdef _MSC_VER
#pragma warning(pop)
#endif
