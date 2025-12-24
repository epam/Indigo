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

// clang-format off
#include "molecule/query_molecule.h"
#include "molecule/elements.h"
// clang-format on
#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;

bool QueryMolecule::isKnownAttr(QueryMolecule::Atom& qa)
{
    return (qa.type == QueryMolecule::ATOM_CHARGE || qa.type == QueryMolecule::ATOM_ISOTOPE || qa.type == QueryMolecule::ATOM_RADICAL ||
            qa.type == QueryMolecule::ATOM_VALENCE || qa.type == QueryMolecule::ATOM_TOTAL_H || qa.type == QueryMolecule::ATOM_SUBSTITUENTS ||
            qa.type == QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN || qa.type == QueryMolecule::ATOM_RING_BONDS ||
            qa.type == QueryMolecule::ATOM_RING_BONDS_AS_DRAWN || qa.type == QueryMolecule::ATOM_UNSATURATION) &&
           qa.value_max == qa.value_min;
}

bool QueryMolecule::isNotAtom(QueryMolecule::Atom& qa, int elem)
{
    if (qa.type == QueryMolecule::OP_NOT)
    {
        QueryMolecule::Atom& c = *qa.child(0);
        if (c.type == QueryMolecule::ATOM_NUMBER && c.value_min == elem && c.value_max == elem)
            return true;
    }
    return false;
}

bool QueryMolecule::collectAtomList(QueryMolecule::Atom& qa, Array<int>& list, bool& notList)
{
    list.clear();
    if (qa.type == QueryMolecule::OP_OR || qa.type == QueryMolecule::OP_NOT)
    {
        notList = (qa.type == QueryMolecule::OP_NOT);
        QueryMolecule::Atom* qap = &qa;
        if (notList)
        {
            qap = qa.child(0);
            if ((qap->type != QueryMolecule::OP_OR) && qa.children.size() > 1)
            {
                return false;
            }
            else // Single NOT atom
            {
                if (qap->type != QueryMolecule::ATOM_NUMBER || qap->value_min != qap->value_max)
                    return false;
                list.push(qap->value_min);
                return true;
            }
        }
        for (int i = 0; i < qap->children.size(); ++i)
        {
            QueryMolecule::Atom& qc = *qap->child(i);
            if (qc.type != QueryMolecule::ATOM_NUMBER || qc.value_min != qc.value_max)
                return false;
            list.push(qc.value_min);
        }
    }
    else if (qa.type == QueryMolecule::OP_AND)
    {
        notList = true;
        for (int i = 0; i < qa.children.size(); ++i)
        {
            QueryMolecule::Atom& qc = *qa.child(i);
            if (qc.type != QueryMolecule::OP_NOT)
                return false;
            QueryMolecule::Atom& qd = *qc.child(0);
            if (qd.type != QueryMolecule::ATOM_NUMBER || qd.value_min != qd.value_max)
                return false;
            list.push(qd.value_min);
        }
    }
    return true;
}

QueryMolecule::Atom* QueryMolecule::stripKnownAttrs(QueryMolecule::Atom& qa)
{
    QueryMolecule::Atom* qd = NULL;
    if (qa.type == QueryMolecule::OP_AND)
    {
        for (int i = 0; i < qa.children.size(); ++i)
        {
            QueryMolecule::Atom* qc = qa.child(i);
            if (!isKnownAttr(*qc))
            {
                if (qd != NULL)
                    return NULL;
                qd = qc;
            }
        }
    }
    return qd == NULL ? &qa : qd;
}

// TODO: develop function to convert tree to CNF to simplify checks
bool QueryMolecule::_tryToConvertToList(Atom* p_query_atom, std::vector<std::unique_ptr<Atom>>& atoms, std::map<int, std::unique_ptr<Atom>>& properties)
{
    // Try to convert a1p1p2..pn, a2p1p2..pn, .. , akp1p2..pn to (a1, a2, .. an)p1p2..pn
    if (!p_query_atom)
        return false;
    if (p_query_atom->type != OP_OR)
        return false;
    int size = p_query_atom->children.size();
    if (size < 2)
        return false;
    std::vector<std::unique_ptr<Atom>> atoms_properties;
    std::vector<std::unique_ptr<Atom>> atoms_list;
    int list_element_child_count = -1;
    for (int i = 0; i < size; i++)
    {
        std::unique_ptr<Atom> child(p_query_atom->child(i)->clone());
        if (child->type != OP_AND)
            return false;
        if (list_element_child_count < 0)
        {
            list_element_child_count = child->children.size();
            if (list_element_child_count < 2)
                return false;
        }
        else
        {
            if (list_element_child_count != child->children.size())
                return false; // All list elements should have same count of properties
        }
        std::vector<std::unique_ptr<Atom>> props;
        bool atom_not_found = true;
        for (int j = 0; j < child->children.size(); j++)
        {
            std::unique_ptr<Atom> child_prop(child->child(j)->clone());
            switch (child_prop->type)
            {
            case OP_AND:
            case OP_OR:
            case OP_NOT:
            case OP_NONE:
                return false;
                break;
            case ATOM_NUMBER:
            case ATOM_PSEUDO:
                if (child_prop->value_min != child_prop->value_max)
                    return false;
                atoms_list.emplace_back(std::move(child_prop));
                atom_not_found = false;
                break;
            default:
                if (i == 0) // first atom
                    atoms_properties.emplace_back(std::move(child_prop));
                else
                    props.emplace_back(std::move(child_prop));
                break;
            }
        }
        if (atom_not_found)
            return false;
        auto compare = [](const std::unique_ptr<Atom>& a, const std::unique_ptr<Atom>& b) { return a->type > b->type; };
        if (i == 0) // first atom
        {
            std::sort(atoms_properties.begin(), atoms_properties.end(), compare);
        }
        else
        {
            // check that children[i] props == children[0] props
            std::sort(props.begin(), props.end(), compare);
            if (props.size() != atoms_properties.size())
                return false;
            for (size_t j = 0; j < props.size(); j++)
            {
                if (props[j]->type != atoms_properties[j]->type || props[j]->value_min != atoms_properties[j]->value_min ||
                    props[j]->value_max != atoms_properties[j]->value_max)
                    return false;
            }
        }
    }

    atoms.clear();
    for (auto& qa : atoms_list)
        atoms.emplace_back(std::move(qa));

    for (auto& prop : atoms_properties)
        properties[prop->type] = std::move(prop);

    return true;
}

bool QueryMolecule::_isAtomListOr(Atom* p_query_atom, std::vector<std::unique_ptr<Atom>>& list)
{
    // Check if p_query_atom atom list like or(a1,a2,a3, or(a4,a5,a6), a7)
    if (!p_query_atom)
        return false;
    if (p_query_atom->type != OP_OR)
        return false;
    std::vector<std::unique_ptr<Atom>> collected;
    for (auto i = 0; i < p_query_atom->children.size(); i++)
    {
        Atom* p_query_atom_child = p_query_atom->child(i);
        if ((p_query_atom_child->type == ATOM_PSEUDO) ||
            (p_query_atom_child->type == ATOM_NUMBER && p_query_atom_child->value_min == p_query_atom_child->value_max))
        {
            collected.emplace_back(p_query_atom_child->clone());
        }
        else if (p_query_atom_child->type == OP_OR)
        {
            if (!_isAtomListOr(p_query_atom_child, collected))
                return false;
        }
        else
            return false;
    }
    if (collected.size() < 1)
        return false;
    for (auto& qa : collected)
        list.emplace_back(std::move(qa));
    return true;
}

bool QueryMolecule::_isAtomOrListAndProps(Atom* p_query_atom, std::vector<std::unique_ptr<Atom>>& list, bool& neg,
                                          std::map<int, std::unique_ptr<Atom>>& properties)
{
    // Check if p_query_atom contains only atom or atom list and atom properties connected by "and"
    // atom list is positive i.e. or(a1,a2,a3,or(a4,a5),a6) or negative
    // negative list like is set of op_not(atom_number) or op_not(positive list), this set connected by "and"
    if (!p_query_atom)
        return false;
    if (p_query_atom->type != OP_AND)
    {
        Atom* p_query_atom_child = p_query_atom;
        bool is_neg = false;
        if (p_query_atom->type == OP_NOT)
        {
            p_query_atom_child = p_query_atom->child(0);
            is_neg = true;
        }
        if ((p_query_atom_child->type == ATOM_PSEUDO) ||
            (p_query_atom_child->type == ATOM_NUMBER && p_query_atom_child->value_min == p_query_atom_child->value_max))
        {
            list.emplace_back(p_query_atom_child->clone());
            neg = is_neg;
            return true;
        }
        else if (p_query_atom_child->type == ATOM_STAR)
        {
            return true;
        }
        else if (!is_neg && isAtomProperty(p_query_atom_child->type)) // atom property, no negative props here
        {
            properties[p_query_atom_child->type] = std::unique_ptr<Atom>(p_query_atom_child->clone());
            return true;
        }
        else
        {
            if (!is_neg && _tryToConvertToList(p_query_atom, list, properties))
            {
                return true;
            }
            std::vector<std::unique_ptr<Atom>> collected;
            if (_isAtomListOr(p_query_atom_child, collected))
            {
                neg = is_neg;
                for (auto& item : collected)
                    list.emplace_back(std::move(item));
                return true;
            }
        }

        return false;
    }
    // OP_AND
    for (auto i = 0; i < p_query_atom->children.size(); i++)
    {
        Atom* p_query_atom_child = const_cast<Atom*>(p_query_atom)->child(i);
        bool is_neg = false;
        std::vector<std::unique_ptr<Atom>> collected;
        std::map<int, std::unique_ptr<Atom>> collected_properties;
        if (_isAtomOrListAndProps(p_query_atom_child, collected, is_neg, collected_properties))
        {
            if (isAtomProperty(p_query_atom_child->type))
            {
                properties[p_query_atom_child->type] = std::unique_ptr<Atom>(p_query_atom_child->clone());
                continue;
            }
            if (list.size() > 0 && is_neg != neg) // allowed only one list type in set - positive or negative
                return false;
            neg = is_neg;
            for (auto& item : collected)
                list.emplace_back(std::move(item));
            for (auto& prop : collected_properties)
                properties[prop.first] = std::move(prop.second);
        }
        else
            return false;
    }
    return true;
}

int QueryMolecule::parseQueryAtomSmarts(QueryMolecule& qm, int aid, std::vector<std::unique_ptr<Atom>>& list, std::map<int, std::unique_ptr<Atom>>& properties)
{
    std::vector<std::unique_ptr<Atom>> query_atom_list;
    std::map<int, std::unique_ptr<Atom>> atom_props;
    bool negative = false;
    QueryMolecule::Atom& qa = qm.getAtom(aid);
    if (qa.type == QueryMolecule::OP_NONE)
        return QUERY_ATOM_AH;
    if (qa.type == QueryMolecule::ATOM_STAR)
        return QUERY_ATOM_STAR;
    if (_isAtomOrListAndProps(&qa, query_atom_list, negative, atom_props))
    {
        bool can_be_query_atom = true;
        std::vector<int> atom_list;
        for (auto& prop : atom_props)
            properties[prop.first] = std::move(prop.second);
        for (auto& qatom : query_atom_list)
        {
            if (qatom->type == ATOM_PSEUDO)
                can_be_query_atom = false;
            else if (qatom->type == ATOM_NUMBER)
                atom_list.emplace_back(qatom->value_max);
            else
                throw Error("Wrong atom type %d", qatom->type);
            list.emplace_back(std::move(qatom));
        }

        if (can_be_query_atom)
        {
            std::sort(atom_list.begin(), atom_list.end());
            if (negative)
            {
                if (atom_list.size() == 1 && atom_list[0] == ELEM_H)
                    return QUERY_ATOM_A;
                else if (atom_list == std::vector<int>{ELEM_H, ELEM_C})
                    return QUERY_ATOM_Q;
                else if (atom_list == std::vector<int>{ELEM_C})
                    return QUERY_ATOM_QH;
                else if (atom_list == std::vector<int>{ELEM_H, ELEM_He, ELEM_C, ELEM_N, ELEM_O, ELEM_F, ELEM_Ne, ELEM_P, ELEM_S, ELEM_Cl, ELEM_Ar, ELEM_Se,
                                                       ELEM_Br, ELEM_Kr, ELEM_I, ELEM_Xe, ELEM_At, ELEM_Rn})
                    return QUERY_ATOM_M;
                else if (atom_list == std::vector<int>{ELEM_He, ELEM_C, ELEM_N, ELEM_O, ELEM_F, ELEM_Ne, ELEM_P, ELEM_S, ELEM_Cl, ELEM_Ar, ELEM_Se, ELEM_Br,
                                                       ELEM_Kr, ELEM_I, ELEM_Xe, ELEM_At, ELEM_Rn})
                    return QUERY_ATOM_MH;
            }
            else
            {
                if (atom_list == std::vector<int>{ELEM_F, ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At})
                    return QUERY_ATOM_X;
                else if (atom_list == std::vector<int>{ELEM_H, ELEM_F, ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At})
                    return QUERY_ATOM_XH;
            }
        }
        if (negative)
        {
            return QUERY_ATOM_NOTLIST;
        }
        else
        {
            if (query_atom_list.size() == 0)
                return QUERY_ATOM_STAR;
            else if (query_atom_list.size() == 1)
                return QUERY_ATOM_SINGLE;
            else
                return QUERY_ATOM_LIST;
        }
    }
    return QUERY_ATOM_UNKNOWN;
}

int QueryMolecule::parseQueryAtom(QueryMolecule& qm, int aid, Array<int>& list)
{
    return parseQueryAtom(qm.getAtom(aid), list);
}

int QueryMolecule::parseQueryAtom(QueryMolecule::Atom& qa, Array<int>& list)
{
    QueryMolecule::Atom* qc = stripKnownAttrs(qa);
    if (qa.type == QueryMolecule::OP_NONE)
        return QUERY_ATOM_AH;
    if (qa.type == QueryMolecule::ATOM_STAR)
        return QUERY_ATOM_STAR;
    if (qc != NULL && isNotAtom(*qc, ELEM_H))
        return QUERY_ATOM_A;
    bool notList = false;
    if (collectAtomList(qa, list, notList) || (qa.type == QueryMolecule::OP_NOT && collectAtomList(*qa.child(0), list, notList) && !notList))
    { // !notList is to check there's no double negation
        if (list.size() == 0)
            return -1;
        notList = notList || qa.type == QueryMolecule::OP_NOT;
        if (!notList && list.size() == 5 && list[0] == ELEM_F && list[1] == ELEM_Cl && list[2] == ELEM_Br && list[3] == ELEM_I && list[4] == ELEM_At)
            return QUERY_ATOM_X;
        if (!notList && list.size() == 6 && list[0] == ELEM_F && list[1] == ELEM_Cl && list[2] == ELEM_Br && list[3] == ELEM_I && list[4] == ELEM_At &&
            list[5] == ELEM_H)
            return QUERY_ATOM_XH;
        if (notList && list.size() == 2 && ((list[0] == ELEM_C && list[1] == ELEM_H) || (list[0] == ELEM_H && list[1] == ELEM_C)))
            return QUERY_ATOM_Q;
        if (notList && list.size() == 1 && (list[0] == ELEM_C))
            return QUERY_ATOM_QH;
        if (notList && list.size() == 17 && list[0] == ELEM_C && list[1] == ELEM_N && list[2] == ELEM_O && list[3] == ELEM_F && list[4] == ELEM_P &&
            list[5] == ELEM_S && list[6] == ELEM_Cl && list[7] == ELEM_Se && list[8] == ELEM_Br && list[9] == ELEM_I && list[10] == ELEM_At &&
            list[11] == ELEM_He && list[12] == ELEM_Ne && list[13] == ELEM_Ar && list[14] == ELEM_Kr && list[15] == ELEM_Xe && list[16] == ELEM_Rn)
            return QUERY_ATOM_MH;
        if (notList && list.size() == 18 && list[0] == ELEM_C && list[1] == ELEM_N && list[2] == ELEM_O && list[3] == ELEM_F && list[4] == ELEM_P &&
            list[5] == ELEM_S && list[6] == ELEM_Cl && list[7] == ELEM_Se && list[8] == ELEM_Br && list[9] == ELEM_I && list[10] == ELEM_At &&
            list[11] == ELEM_He && list[12] == ELEM_Ne && list[13] == ELEM_Ar && list[14] == ELEM_Kr && list[15] == ELEM_Xe && list[16] == ELEM_Rn &&
            list[17] == ELEM_H)
            return QUERY_ATOM_M;

        return notList ? QUERY_ATOM_NOTLIST : QUERY_ATOM_LIST;
    }
    return -1;
}

bool QueryMolecule::queryAtomIsRegular(QueryMolecule& qm, int aid)
{
    QueryMolecule::Atom& qa = qm.getAtom(aid);
    QueryMolecule::Atom* qc = stripKnownAttrs(qa);
    if (qm.original_format == SMARTS || qm.original_format == KET)
    {
        // Regular means that atom name will be used as label - Cl, Br, C, N
        // only "organic" subset atoms and aliphatic atoms should be rendered as "regular"
        // aromatic atoms should be rendered in lowercase, other - in backets [Au]
        if (isOrganicSubset(&qa) && !isAromaticByCaseAtom(&qa))
            return true;
        if (qc || qa.type != OP_AND || qa.children.size() != 2)
            return false;
        bool aliphatic = false;
        int atom_number = -1;
        for (int i = 0; i < 2; i++)
            if (isAromaticByCaseAtom(qa.child(i)))
                atom_number = qa.child(i)->value_min;
            else if (qa.child(i)->type == ATOM_AROMATICITY && qa.child(i)->value_min == ATOM_ALIPHATIC)
                aliphatic = true;
        return aliphatic && atom_number >= ELEM_MIN;
    }
    else
    {
        return qc && qc->type == QueryMolecule::ATOM_NUMBER;
    }
}

bool QueryMolecule::queryAtomIsSpecial(QueryMolecule& qm, int aid)
{
    QS_DEF(Array<int>, list);
    int query_atom_type;

    if ((query_atom_type = QueryMolecule::parseQueryAtom(qm, aid, list)) != QUERY_ATOM_UNKNOWN)
    {
        return queryAtomIsSpecial(query_atom_type);
    }
    return false;
}

bool QueryMolecule::queryAtomIsSpecial(int query_atom_type)
{
    if ((query_atom_type == QueryMolecule::QUERY_ATOM_Q) || (query_atom_type == QueryMolecule::QUERY_ATOM_QH) ||
        (query_atom_type == QueryMolecule::QUERY_ATOM_X) || (query_atom_type == QueryMolecule::QUERY_ATOM_XH) ||
        (query_atom_type == QueryMolecule::QUERY_ATOM_M) || (query_atom_type == QueryMolecule::QUERY_ATOM_MH) ||
        (query_atom_type == QueryMolecule::QUERY_ATOM_AH) || (query_atom_type == QueryMolecule::QUERY_ATOM_STAR))
    {
        return true;
    }
    else
    {
        return false;
    }
}

QueryMolecule::Bond* QueryMolecule::getBondOrderTerm(QueryMolecule::Bond& qb, bool& complex)
{
    if (qb.type == QueryMolecule::OP_AND)
    {
        QueryMolecule::Bond* r = NULL;
        for (int i = 0; i < qb.children.size(); ++i)
        {
            QueryMolecule::Bond* c = getBondOrderTerm(*qb.child(i), complex);
            if (complex)
                return NULL;
            if (c != NULL)
            {
                if (r != NULL)
                {
                    complex = true;
                    return NULL;
                }
                r = c;
            }
        }
        return r;
    }

    if (qb.type == QueryMolecule::BOND_TOPOLOGY)
        return NULL;
    return &qb;
}

bool QueryMolecule::isOrBond(QueryMolecule::Bond& qb, int type1, int type2)
{
    if ((qb.type == OP_AND || qb.type == OP_OR) && qb.children.size() == 1)
        return isOrBond(*qb.child(0), type1, type2);

    if (qb.type != QueryMolecule::OP_OR || qb.children.size() != 2)
        return false;
    QueryMolecule::Bond& b1 = *qb.child(0);
    QueryMolecule::Bond& b2 = *qb.child(1);
    if (b1.type != QueryMolecule::BOND_ORDER || b2.type != QueryMolecule::BOND_ORDER)
        return false;
    if ((b1.value == type1 && b2.value == type2) || (b1.value == type2 && b2.value == type1))
        return true;
    return false;
}

bool QueryMolecule::isSingleOrDouble(QueryMolecule::Bond& qb)
{
    if ((qb.type == OP_AND || qb.type == OP_OR) && qb.children.size() == 1)
        return isSingleOrDouble(*qb.child(0));

    if (qb.type != QueryMolecule::OP_AND || qb.children.size() != 2)
        return false;
    QueryMolecule::Bond& b1 = *qb.child(0);
    QueryMolecule::Bond& b2 = *qb.child(1);
    if (!isOrBond(b2, BOND_SINGLE, BOND_DOUBLE))
        return false;
    if (b1.type != QueryMolecule::OP_NOT)
        return false;
    QueryMolecule::Bond& b11 = *b1.child(0);
    if (b11.type != QueryMolecule::BOND_ORDER || b11.value != BOND_AROMATIC)
        return false;
    return true;
}

int QueryMolecule::getQueryBondType(QueryMolecule::Bond& qb)
{
    if (!qb.hasConstraint(BOND_ORDER))
        return _BOND_ANY;

    Bond* qb2 = &qb;
    std::unique_ptr<Bond> qb_modified;
    int topology;
    if (qb.sureValue(BOND_TOPOLOGY, topology))
    {
        qb_modified.reset(qb.clone());
        qb_modified->removeConstraints(BOND_TOPOLOGY);
        qb2 = qb_modified.get();
    }

    if (isSingleOrDouble(*qb2) || isOrBond(*qb2, BOND_SINGLE, BOND_DOUBLE))
        return _BOND_SINGLE_OR_DOUBLE;
    if (isOrBond(*qb2, BOND_SINGLE, BOND_AROMATIC))
        return _BOND_SINGLE_OR_AROMATIC;
    if (isOrBond(*qb2, BOND_DOUBLE, BOND_AROMATIC))
        return _BOND_DOUBLE_OR_AROMATIC;
    return -1;
}

int QueryMolecule::getQueryBondType(Bond& qb, int& direction, bool& negative)
{
    Bond* qbond = &qb;
    if (qbond->type == OP_NOT)
    {
        qbond = qbond->child(0);
        negative = true;
    }
    if (qbond->type == OP_AND)
    {
        int idx = qbond->children.size() - 1;
        // Skip topology if any
        if (qbond->children[idx]->type == BOND_TOPOLOGY)
            --idx;
        if (idx > 0) // Looks like _BOND_SINGLE_OR_DOUBLE
        {
            Bond* tbond = qbond->child(0);
            if (tbond->type != OP_NOT)
                return -1;
            tbond = tbond->child(0);
            if (tbond->type != BOND_ORDER || tbond->value != BOND_AROMATIC)
                return -1;
            tbond = qbond->child(1);
            if (tbond->type != OP_OR || tbond->children.size() != 2)
                return -1;
            if (tbond->child(0)->type != BOND_ORDER || tbond->child(0)->value != BOND_SINGLE)
                return -1;
            if (tbond->child(1)->type != BOND_ORDER || tbond->child(1)->value != BOND_DOUBLE)
                return -1;
            return _BOND_SINGLE_OR_DOUBLE;
        }
        qbond = qbond->child(0);
    }

    if (qbond->type == OP_NONE)
        return _BOND_ANY;

    if (qbond->type == BOND_ORDER)
    {
        direction = qbond->direction;
        return qbond->value;
    }

    if (qbond->type != OP_OR || qbond->children.size() != 2)
        return -1;
    Bond* qb0 = qbond->child(0);
    Bond* qb1 = qbond->child(1);
    if (qb0->type != BOND_ORDER || qb1->type != BOND_ORDER)
        return -1;
    if (qb0->value == BOND_SINGLE && qb1->value == BOND_AROMATIC)
        return _BOND_SINGLE_OR_AROMATIC;
    if (qb0->value == BOND_DOUBLE && qb1->value == BOND_AROMATIC)
        return _BOND_DOUBLE_OR_AROMATIC;
    return -1;
}

QueryMolecule::Bond* QueryMolecule::createQueryMoleculeBond(int order, int topology, int direction)
{
    std::unique_ptr<Bond> bond;
    if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC || order == _BOND_COORDINATION ||
        order == _BOND_HYDROGEN)
        bond = std::make_unique<Bond>(BOND_ORDER, order, direction);
    else if (order == _BOND_SINGLE_OR_DOUBLE)
        bond.reset(
            Bond::und(Bond::nicht(new Bond(BOND_ORDER, BOND_AROMATIC)), Bond::oder(new Bond(BOND_ORDER, BOND_SINGLE), new Bond(BOND_ORDER, BOND_DOUBLE))));
    else if (order == _BOND_SINGLE_OR_AROMATIC)
        bond.reset(Bond::oder(new Bond(BOND_ORDER, BOND_SINGLE), new Bond(BOND_ORDER, BOND_AROMATIC)));
    else if (order == _BOND_DOUBLE_OR_AROMATIC)
        bond.reset(Bond::oder(new Bond(BOND_ORDER, BOND_DOUBLE), new Bond(BOND_ORDER, BOND_AROMATIC)));
    else if (order == _BOND_ANY)
        bond = std::make_unique<Bond>();
    else
        throw Error("unknown bond type: %d", order);
    if (topology != 0)
    {
        bond.reset(Bond::und(bond.release(), new Bond(BOND_TOPOLOGY, topology == 1 ? TOPOLOGY_RING : TOPOLOGY_CHAIN)));
    }
    return bond.release();
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
