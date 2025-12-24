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
            if (isotope > 0 && !(label == ELEM_H && (isotope == DEUTERIUM || isotope == TRITIUM)))
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
    if (!tgroups.getTGroupCount())
        return;

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
                std::string alias = getTemplateAtom(atom_idx);
                std::string mon_class = getTemplateAtomClass(atom_idx);
                auto tg_ref = findTemplateInMap(alias, mon_class, templates);
                if (tg_ref.has_value())
                {
                    auto& tg = tg_ref.value().get();
                    tg_idx = tg.tgroup_id - 1;
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

std::string BaseMolecule::getAtomDescription(int idx)
{
    Array<char> description;
    getAtomDescription(idx, description);
    return std::string(description.ptr(), description.size());
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

const KetObjectAnnotation& BaseMolecule::getTemplateAtomAnnotation(int idx) const
{
    return _atom_annotations.at(idx);
}

bool BaseMolecule::hasTemplateAtomAnnotation(int idx) const
{
    return _atom_annotations.count(idx) > 0;
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

void BaseMolecule::setTemplateAtomAnnotation(int idx, const KetObjectAnnotation& annotation)
{
    _atom_annotations[idx] = annotation;
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

void BaseMolecule::setBondAnnotation(int idx, const KetObjectAnnotation& annotation)
{
    _bond_annotations[idx] = annotation;
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

std::unique_ptr<BaseMolecule> BaseMolecule::expandedMonomersToAtoms()
{
    std::unique_ptr<BaseMolecule> result(neu());
    result->clone(*this);

    std::list<int> monomer_ids;
    for (int monomer_id = result->vertexBegin(); monomer_id != result->vertexEnd(); monomer_id = result->vertexNext(monomer_id))
    {
        if (result->isTemplateAtom(monomer_id))
            monomer_ids.push_front(monomer_id);
    }

    QS_DEF(Array<int>, all_atoms_to_remove);
    all_atoms_to_remove.clear();

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
        std::map<int, std::pair<int, int>> attached_atom;
        Array<int> atoms_to_remove;

        _processMonomerAttachmentPoints(monomer_id, *result, *monomer_mol, attached_atom, atoms_to_remove);

        Array<int> atom_map;
        result->mergeWithMolecule(*monomer_mol, &atom_map);
        // update atom indexes
        for (int* i = atoms_to_remove.begin(); i != atoms_to_remove.end(); i++)
        {
            *i = atom_map[*i];
        }

        // add bonds from template atom neighbors to attachment points
        _connectMonomerToNeighbors(monomer_id, *result, *monomer_mol, atom_map, attached_atom, templates);

        // remove template atom and all bonds to it
        all_atoms_to_remove.push(monomer_id);

        // remove leaved atoms
        for (int k = 0; k < atoms_to_remove.size(); ++k)
            all_atoms_to_remove.push(atoms_to_remove[k]);
    }

    result->removeAtoms(all_atoms_to_remove);

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
        std::vector<int> verts;
        // VerticesAuto fails if graph changed inside cycle
        for (auto vi : vertices())
            verts.emplace_back(vi);
        for (auto vi : verts)
        {
            if (isTemplateAtom(vi) &&
                (!only_transformed || getTemplateAtomTransform(vi).hasTransformation() || getTemplateAtomDisplayOption(vi) == DisplayOption::Expanded))
            {
                auto tg_idx = getTemplateAtomTemplateIndex(vi);
                if (tg_idx < 0)
                {
                    std::string alias = getTemplateAtom(vi);
                    std::string mon_class = getTemplateAtomClass(vi);
                    auto tg_ref = findTemplateInMap(alias, mon_class, templates);
                    if (tg_ref.has_value())
                    {
                        auto& tg = tg_ref.value().get();
                        tg_idx = tg.tgroup_id - 1;
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

bool BaseMolecule::restoreAromaticHydrogens(bool unambiguous_only)
{
    return MoleculeDearomatizer::restoreHydrogens(*this, unambiguous_only);
}

bool BaseMolecule::isPiBonded(const int atom_index) const
{
    const Vertex& vertex = getVertex(atom_index);
    for (auto i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        const int order = getBondOrder(vertex.neiEdge(i));
        if (order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC)
        {
            return true;
        }
    }
    return false;
}

void BaseMolecule::_processMonomerAttachmentPoints(int monomer_id, BaseMolecule& result, BaseMolecule& monomer_mol,
                                                   std::map<int, std::pair<int, int>>& attached_atom, Array<int>& atoms_to_remove)
{
    auto& monomer_sgroups = monomer_mol.sgroups;
    int sg_idx = monomer_sgroups.begin();
    while (sg_idx != monomer_sgroups.end())
    {
        int next_sg_idx = monomer_sgroups.next(sg_idx);
        SGroup& sg = monomer_sgroups.getSGroup(sg_idx);
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
                if (monomer_id < result.template_attachment_indexes.size()) // check if monomer has attachment points in use
                {
                    auto& indexes = result.template_attachment_indexes.at(monomer_id);
                    for (int att_idx = indexes.begin(); att_idx != indexes.end(); att_idx = indexes.next(att_idx))
                    {
                        auto& ap = result.template_attachment_points.at(indexes.at(att_idx));
                        assert(ap.ap_occur_idx == monomer_id);
                        auto it = sorted_attachment_points.find(ap.ap_id.ptr());
                        if (it != sorted_attachment_points.end())
                        {
                            auto& atp = sa.attachment_points[it->second];
                            attached_atom.insert(
                                std::make_pair(ap.ap_aidx, std::make_pair(atp.aidx, atp.lvidx))); // molecule atom ap.ap_aidx is attached to atp.aidx in monomer
                            if (atp.lvidx >= 0)
                                atoms_to_remove.push(atp.lvidx);
                        }
                        result.template_attachment_points.remove(indexes.at(att_idx));
                    }
                }
            }
            monomer_sgroups.remove(sg_idx);
        }
        sg_idx = next_sg_idx;
    }
}

std::pair<int, bool> BaseMolecule::_getNeighborLeavingBondDir(
    int other, int monomer_id, BaseMolecule& result,
    std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash>& templates)
{
    int leaving_bond_dir2 = 0;
    bool leaving_bond_is_beg2 = false;
    if (other < result.template_attachment_indexes.size())
    {
        auto& b_indexes = result.template_attachment_indexes.at(other);
        for (int att_idx = b_indexes.begin(); att_idx != b_indexes.end(); att_idx = b_indexes.next(att_idx))
        {
            auto& ap = result.template_attachment_points.at(b_indexes.at(att_idx));
            if (ap.ap_aidx == monomer_id)
            { // Found B's AP connecting to A.
                // Need leaving direction from B's template.
                int b_occur_idx = result.getTemplateAtomOccurrence(other);
                int b_tmpl_idx = result._template_occurrences.at(b_occur_idx).template_idx;
                std::optional<std::reference_wrapper<TGroup>> b_tgroup_opt;
                if (b_tmpl_idx == -1)
                {
                    auto b_tg_ref = findTemplateInMap(std::string(result.getTemplateAtom(other)), std::string(result.getTemplateAtomClass(other)), templates);
                    if (b_tg_ref.has_value())
                        b_tgroup_opt = b_tg_ref.value();
                }
                else
                {
                    b_tgroup_opt = result.tgroups.getTGroup(b_tmpl_idx);
                }

                if (b_tgroup_opt.has_value())
                {
                    TGroup& b_tgroup = b_tgroup_opt.value().get();
                    // Find SGroup with AP ID
                    std::string ap_id(ap.ap_id.ptr());
                    // We need B's fragment to look up SGroup APs by ID.
                    BaseMolecule* b_mol = b_tgroup.fragment.get();
                    // Search SGroups in B
                    for (int k = b_mol->sgroups.begin(); k != b_mol->sgroups.end(); k = b_mol->sgroups.next(k))
                    {
                        SGroup& sg = b_mol->sgroups.getSGroup(k);
                        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
                        {
                            Superatom& sa = (Superatom&)sg;
                            for (int m = sa.attachment_points.begin(); m != sa.attachment_points.end(); m = sa.attachment_points.next(m))
                            {
                                if (strcmp(sa.attachment_points[m].apid.ptr(), ap_id.c_str()) == 0)
                                {
                                    // Found matching AP. Get Leaving info.
                                    int la_idx = sa.attachment_points[m].lvidx;
                                    int aa_idx = sa.attachment_points[m].aidx;
                                    int lb_idx = b_mol->findEdgeIndex(aa_idx, la_idx);
                                    if (lb_idx != -1)
                                    {
                                        leaving_bond_dir2 = b_mol->getBondDirection(lb_idx);
                                        leaving_bond_is_beg2 = (b_mol->getEdge(lb_idx).beg == la_idx);
                                    }
                                }
                            }
                        }
                    }
                }
                // break template_attachment_indexes loop after first match
                break;
            }
        }
    }
    return {leaving_bond_dir2, leaving_bond_is_beg2};
}

void BaseMolecule::_connectMonomerToNeighbors(int monomer_id, BaseMolecule& result, BaseMolecule& monomer_mol, const Array<int>& atom_map,
                                              const std::map<int, std::pair<int, int>>& attached_atom,
                                              std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash>& templates)
{
    const Vertex& v = result.getVertex(monomer_id);
    Array<int> bonds_to_delete;
    std::vector<int> neighbors;
    for (int k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
    {
        neighbors.push_back(v.neiVertex(k));
    }

    for (int nei : neighbors)
    {
        int edge_idx = result.findEdgeIndex(monomer_id, nei);
        if (edge_idx >= 0 && result.getBondOrder(edge_idx) == BOND_SINGLE)
        {
            int a1 = result.getEdge(edge_idx).beg;
            int a2 = result.getEdge(edge_idx).end;
            int other = a1 == monomer_id ? a2 : a1;
            auto it = attached_atom.find(other);
            if (it != attached_atom.end())
            {
                int ap_new_idx = atom_map[it->second.first];
                int local_leaving_atom_idx = it->second.second;
                int mapped_leaving_atom_idx = (local_leaving_atom_idx >= 0) ? atom_map[local_leaving_atom_idx] : -1;

                int leaving_bond_dir1 = 0;
                bool leaving_bond_is_beg1 = false;

                if (local_leaving_atom_idx >= 0)
                {
                    int lb_idx = monomer_mol.findEdgeIndex(it->second.first, local_leaving_atom_idx);
                    leaving_bond_dir1 = monomer_mol.getBondDirection(lb_idx);
                    leaving_bond_is_beg1 = (monomer_mol.getEdge(lb_idx).beg == local_leaving_atom_idx);
                }

                auto [leaving_bond_dir2, leaving_bond_is_beg2] = std::make_pair(0, false);

                if (result.isTemplateAtom(other)) // Check neighbor
                {
                    std::tie(leaving_bond_dir2, leaving_bond_is_beg2) = _getNeighborLeavingBondDir(other, monomer_id, result, templates);
                }

                int atom_parent = other;
                // If 'other' is already expanded, the bond (other, monomer_id) might have been moved to (ap_other, monomer_id).
                // We need to find which attachment point of 'other' is connected to 'monomer_id'.
                if (other < result.template_attachment_indexes.size())
                {
                    auto& indexes = result.template_attachment_indexes.at(other);
                    for (int att_idx = indexes.begin(); att_idx != indexes.end(); att_idx = indexes.next(att_idx))
                    {
                        auto& ap = result.template_attachment_points.at(indexes.at(att_idx));
                        // Check if 'monomer_id' is connected to this AP
                        if (result.findEdgeIndex(monomer_id, ap.ap_aidx) >= 0)
                        {
                            atom_parent = ap.ap_aidx;
                            break;
                        }
                    }
                }

                result.flipBondWithDirection(atom_parent, monomer_id, ap_new_idx, mapped_leaving_atom_idx);

                // fix neighbor template_attachment_points
                if (other < result.template_attachment_indexes.size())
                {
                    auto& indexes = result.template_attachment_indexes.at(other);
                    for (int att_idx = indexes.begin(); att_idx != indexes.end(); att_idx = indexes.next(att_idx))
                    {
                        auto& ap = result.template_attachment_points.at(indexes.at(att_idx));
                        if (ap.ap_aidx == monomer_id)
                            ap.ap_aidx = ap_new_idx;
                    }
                }
            }
            else
            {
                bonds_to_delete.push(edge_idx);
            }
        }
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
