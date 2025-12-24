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


#include "molecule/query_molecule.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;

QueryMolecule::Node::Node(int type_)
{
    type = (OpType)type_;
}

QueryMolecule::Node::~Node()
{
}

IMPL_ERROR(QueryMolecule::Atom, "query atom");

QueryMolecule::Atom::Atom() : Node(OP_NONE), occurrence_idx(0)
{
    value_min = 0;
    value_max = 0;
}

QueryMolecule::Atom::Atom(int type_, int value) : Node(type_), occurrence_idx(0)
{
    if (type_ == ATOM_NUMBER || type_ == ATOM_CHARGE || type_ == ATOM_ISOTOPE || type_ == ATOM_RADICAL || type_ == ATOM_AROMATICITY || type_ == ATOM_VALENCE ||
        type_ == ATOM_RING_BONDS || type_ == ATOM_RING_BONDS_AS_DRAWN || type_ == ATOM_SUBSTITUENTS || type_ == ATOM_SUBSTITUENTS_AS_DRAWN ||
        type_ == ATOM_TOTAL_H || type_ == ATOM_CONNECTIVITY || type_ == ATOM_TOTAL_BOND_ORDER || type_ == ATOM_UNSATURATION || type == ATOM_SSSR_RINGS ||
        type == ATOM_SMALLEST_RING_SIZE || type == ATOM_RSITE || type == HIGHLIGHTING || type == ATOM_TEMPLATE_SEQID || type == ATOM_PI_BONDED ||
        type == ATOM_IMPLICIT_H)

        value_min = value_max = value;
    else
        throw Error("bad type: %d", type_);
}

QueryMolecule::Atom::Atom(int type_, int val_min, int val_max) : Node(type_), occurrence_idx(0)
{
    value_min = val_min;
    value_max = val_max;
}

QueryMolecule::Atom::Atom(int type_, const char* value) : Node(type_), occurrence_idx(0)
{
    if (type_ == ATOM_PSEUDO || type_ == ATOM_TEMPLATE || type_ == ATOM_TEMPLATE_CLASS)
        alias.readString(value, true);
    else
        throw Error("bad type: %d", type_);
}

QueryMolecule::Atom::Atom(int type_, QueryMolecule* value) : Node(type_), occurrence_idx(0)
{
    if (type_ == ATOM_FRAGMENT)
        fragment.reset(value);
    else
        throw Error("bad type: %d", type_);
}

QueryMolecule::Atom::~Atom()
{
}

QueryMolecule::Bond::Bond() : Node(OP_NONE), value(0), direction(0)
{
}

QueryMolecule::Bond::Bond(int type_) : Node(type_), value(0), direction(0)
{
}

QueryMolecule::Bond::Bond(int type_, int value_) : Node(type_), value(value_), direction(0)
{
}

QueryMolecule::Bond::Bond(int type_, int value_, int direction_) : Node(type_), value(value_), direction(direction_)
{
}

QueryMolecule::Bond::~Bond()
{
}

QueryMolecule::Node* QueryMolecule::Node::_und(QueryMolecule::Node* node1, QueryMolecule::Node* node2)
{
    if (node1->type == QueryMolecule::OP_NONE)
    {
        delete node1;
        return node2;
    }

    if (node2->type == QueryMolecule::OP_NONE)
    {
        delete node2;
        return node1;
    }

    if (node1->type == QueryMolecule::OP_AND)
    {
        if (node2->type == QueryMolecule::OP_AND)
        {
            while (node2->children.size() != 0)
                node1->children.add(node2->children.pop());
        }
        else
            node1->children.add(node2);

        return node1;
    }

    if (node2->type == QueryMolecule::OP_AND)
    {
        node2->children.add(node1);
        return node2;
    }

    std::unique_ptr<QueryMolecule::Node> newnode(node1->_neu());

    newnode->type = QueryMolecule::OP_AND;
    newnode->children.add(node1);
    newnode->children.add(node2);
    return newnode.release();
}

QueryMolecule::Node* QueryMolecule::Node::_oder(QueryMolecule::Node* node1, QueryMolecule::Node* node2)
{
    if (node1->type == QueryMolecule::OP_NONE)
    {
        delete node2;
        return node1;
    }

    if (node2->type == QueryMolecule::OP_NONE)
    {
        delete node1;
        return node2;
    }

    if (node1->type == QueryMolecule::OP_OR)
    {
        if (node2->type == QueryMolecule::OP_OR)
        {
            while (node2->children.size() != 0)
                node1->children.add(node2->children.pop());
        }
        else
            node1->children.add(node2);

        return node1;
    }

    if (node2->type == QueryMolecule::OP_OR)
    {
        node2->children.add(node1);
        return node2;
    }

    std::unique_ptr<QueryMolecule::Node> newnode(node1->_neu());

    newnode->type = QueryMolecule::OP_OR;
    newnode->children.add(node1);
    newnode->children.add(node2);
    return newnode.release();
}

QueryMolecule::Node* QueryMolecule::Node::_nicht(QueryMolecule::Node* node)
{
    if (node->type == QueryMolecule::OP_NOT)
    {
        Node* res = (Node*)node->children.pop();
        delete node;
        return res;
    }

    std::unique_ptr<QueryMolecule::Node> newnode(node->_neu());

    newnode->type = QueryMolecule::OP_NOT;
    newnode->children.add(node);
    return newnode.release();
}

QueryMolecule::Atom* QueryMolecule::Atom::und(Atom* atom1, Atom* atom2)
{
    return (Atom*)_und(atom1, atom2);
}

QueryMolecule::Atom* QueryMolecule::Atom::oder(Atom* atom1, Atom* atom2)
{
    return (Atom*)_oder(atom1, atom2);
}

QueryMolecule::Atom* QueryMolecule::Atom::nicht(Atom* atom)
{
    return (Atom*)_nicht(atom);
}

bool QueryMolecule::Atom::hasConstraintWithValue(int what_type, int what_value)
{
    if (type == what_type)
        return value_max == what_value && value_min == what_value;

    if (type == OP_NONE)
        return false;

    if (type == OP_AND || type == OP_OR || type == OP_NOT)
    {
        int i;

        for (i = 0; i < children.size(); i++)
            if (((Atom*)children[i])->hasConstraintWithValue(what_type, what_value))
                return true;
    }

    return false;
}

bool QueryMolecule::Atom::updateConstraintWithValue(int what_type, int new_value)
{
    if (type == what_type)
    {
        value_max = new_value;
        value_min = new_value;
        return true;
    }

    if (type == OP_NONE)
        return false;

    if (type == OP_AND || type == OP_OR || type == OP_NOT)
    {
        int i;

        for (i = 0; i < children.size(); i++)
            if (((Atom*)children[i])->updateConstraintWithValue(what_type, new_value))
                return true;
    }

    return false;
}

QueryMolecule::Bond* QueryMolecule::Bond::und(Bond* bond1, Bond* bond2)
{
    return (Bond*)_und(bond1, bond2);
}

QueryMolecule::Bond* QueryMolecule::Bond::oder(Bond* bond1, Bond* bond2)
{
    return (Bond*)_oder(bond1, bond2);
}

QueryMolecule::Bond* QueryMolecule::Bond::nicht(Bond* bond)
{
    return (Bond*)_nicht(bond);
}

void QueryMolecule::_flipBond(int atom_parent, int atom_from, int atom_to)
{
    int src_bond_idx = findEdgeIndex(atom_parent, atom_from);

    int new_bond_idx = addBond(atom_parent, atom_to, releaseBond(src_bond_idx));
    // Copy aromaticity information
    aromaticity.setCanBeAromatic(new_bond_idx, aromaticity.canBeAromatic(src_bond_idx));
    setBondStereoCare(new_bond_idx, bondStereoCare(src_bond_idx));
    updateEditRevision();
}

void QueryMolecule::_mergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping, int skip_flags)
{
    QueryMolecule& mol = bmol.asQueryMolecule();
    int i;

    // atoms
    for (i = 0; i < vertices.size(); i++)
    {
        int newidx = mapping[vertices[i]];

        _atoms.expand(newidx + 1);
        _atoms.set(newidx, mol.getAtom(vertices[i]).clone());
    }

    // bonds
    if (edges != 0)
        for (i = 0; i < edges->size(); i++)
        {
            const Edge& edge = mol.getEdge(edges->at(i));
            int beg = mapping[edge.beg];
            int end = mapping[edge.end];

            if (beg == -1 || end == -1)
                // must have been thrown before in mergeWithSubgraph()
                throw Error("_mergeWithSubmolecule: internal");

            int idx = findEdgeIndex(beg, end);

            _bonds.expand(idx + 1);
            _bonds.set(idx, mol.getBond(edges->at(i)).clone());

            // Aromaticity
            if (!(skip_flags & SKIP_AROMATICITY))
                aromaticity.setCanBeAromatic(idx, mol.aromaticity.canBeAromatic(edges->at(i)));
            if (!(skip_flags & SKIP_CIS_TRANS) && mol.cis_trans.getParity(edges->at(i)) != 0)
                setBondStereoCare(idx, mol.bondStereoCare(edges->at(i)));
        }
    else
        for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
        {
            const Edge& edge = mol.getEdge(i);
            int beg = mapping[edge.beg];
            int end = mapping[edge.end];

            if (beg == -1 || end == -1)
                continue;

            int idx = findEdgeIndex(beg, end);

            _bonds.expand(idx + 1);
            _bonds.set(idx, mol.getBond(i).clone());

            // Aromaticity
            if (!(skip_flags & SKIP_AROMATICITY))
                aromaticity.setCanBeAromatic(idx, mol.aromaticity.canBeAromatic(i));
            if (!(skip_flags & SKIP_CIS_TRANS) && mol.cis_trans.getParity(i) != 0)
                setBondStereoCare(idx, mol.bondStereoCare(i));
        }

    // 3D constraints
    if (!(skip_flags & SKIP_3D_CONSTRAINTS))
        spatial_constraints.buildOnSubmolecule(mol.spatial_constraints, mapping.ptr());

    // fixed atoms
    if (!(skip_flags & SKIP_FIXED_ATOMS))
    {
        for (i = 0; i < mol.fixed_atoms.size(); i++)
        {
            int idx = mapping[mol.fixed_atoms[i]];

            if (idx >= 0)
                fixed_atoms.push(idx);
        }
    }

    // components
    if (!(skip_flags & SKIP_COMPONENTS))
    {
        for (i = 0; i < vertices.size(); i++)
        {
            int v_idx = vertices[i];
            if (mol.components.size() > v_idx)
            {
                int newidx = mapping[v_idx];
                components.expandFill(newidx + 1, 0);
                components[newidx] = mol.components[v_idx];
            }
        }
    }

    updateEditRevision();
}

void QueryMolecule::_postMergeWithSubmolecule(BaseMolecule& /*bmol*/, const Array<int>& /*vertices*/, const Array<int>* /*edges*/,
                                              const Array<int>& /*mapping*/, int /*skip_flags*/)
{
    // Remove stereocare flags for bonds that are not cis-trans
    for (int i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
        if (cis_trans.getParity(i) == 0)
            setBondStereoCare(i, false);
}

void QueryMolecule::_removeAtoms(const Array<int>& indices, const int* mapping)
{
    spatial_constraints.removeAtoms(mapping);

    if (attachmentPointCount() > 0)
    {
        int i;

        for (i = 0; i < indices.size(); i++)
            this->removeAttachmentPointsFromAtom(indices[i]);

        bool empty = true;

        for (i = 1; i <= this->attachmentPointCount(); i++)
            if (this->getAttachmentPoint(i, 0) != -1)
            {
                empty = false;
                break;
            }

        if (empty)
            _attachment_index.clear();
    }

    for (int i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];
        _atoms.reset(idx);
        if (idx < _rsite_attachment_points.size())
            _rsite_attachment_points[idx].clear();
    }

    // Collect bonds to remove
    QS_DEF(Array<int>, edges_to_remove);
    edges_to_remove.clear();
    for (int i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
    {
        const Edge& e = getEdge(i);
        if (mapping[e.beg] == -1 || mapping[e.end] == -1)
            edges_to_remove.push(i);
    }
    _removeBonds(edges_to_remove);
    updateEditRevision();
}

void QueryMolecule::_removeBonds(const Array<int>& indices)
{
    for (int i = 0; i < indices.size(); i++)
        _bonds.reset(indices[i]);

    _min_h.clear();
    updateEditRevision();
}

bool QueryMolecule::Node::sureValue(int what_type, int& value_out) const
{
    int i;

    switch (type)
    {
    case OP_AND: {
        // of course one can fool this around, specifying something like
        // "(C || O) && (C || N)", but we do not claim to have perfect
        // logic here, and we do not need one.

        int child_value = -1;
        bool child_value_valid = false;

        for (i = 0; i < children.size(); i++)
        {
            int child_value_tmp;
            if (children[i]->sureValue(what_type, child_value_tmp))
            {
                if (!child_value_valid)
                {
                    child_value_valid = true;
                    child_value = child_value_tmp;
                }
                else
                {
                    if (child_value_tmp != child_value)
                        return false; // self-contradictory query atom it is
                }
            }
        }

        if (child_value_valid)
        {
            value_out = child_value;
            return true;
        }
        return false;
    }
    case OP_OR: {
        int child_value = -1;

        for (i = 0; i < children.size(); i++)
        {
            int child_value_tmp;
            if (children[i]->sureValue(what_type, child_value_tmp))
            {
                if (i == 0)
                    child_value = child_value_tmp;
                else if (child_value_tmp != child_value)
                    return false;
            }
            else
                return false;
        }
        value_out = child_value;
        return true;
    }
    case OP_NOT:
        return children[0]->sureValueInv(what_type, value_out);
    case OP_NONE:
        return false;
    default:
        return _sureValue(what_type, value_out);
    }
}

bool QueryMolecule::Node::sureValueInv(int what_type, int& value_out) const
{
    int i;

    switch (type)
    {
    case OP_OR: {
        int child_value = -1;
        bool child_value_valid = false;

        for (i = 0; i < children.size(); i++)
        {
            int child_value_tmp;
            if (children[i]->sureValueInv(what_type, child_value_tmp))
            {
                if (!child_value_valid)
                {
                    child_value_valid = true;
                    child_value = child_value_tmp;
                }
                else
                {
                    if (child_value_tmp != child_value)
                        return false; // self-contradictory query atom it is
                }
            }
        }

        if (child_value_valid)
        {
            value_out = child_value;
            return true;
        }
        return false;
    }
    case OP_AND: {
        int child_value = -1;

        for (i = 0; i < children.size(); i++)
        {
            int child_value_tmp;
            if (children[i]->sureValueInv(what_type, child_value_tmp))
            {
                if (i == 0)
                    child_value = child_value_tmp;
                else if (child_value_tmp != child_value)
                    return false;
            }
            else
                return false;
        }
        value_out = child_value;
        return true;
    }
    case OP_NOT:
        return children[0]->sureValue(what_type, value_out);
    case OP_NONE:
        throw QueryMolecule::Error("sureValueInv(OP_NONE) not implemented");
    default:
        return false;
    }
}

bool QueryMolecule::Node::possibleValue(int what_type, int what_value)
{
    int i;

    switch (type)
    {
    case OP_AND: {
        for (i = 0; i < children.size(); i++)
            if (!children[i]->possibleValue(what_type, what_value))
                return false;

        return true;
    }
    case OP_OR: {
        for (i = 0; i < children.size(); i++)
            if (children[i]->possibleValue(what_type, what_value))
                return true;

        return false;
    }
    case OP_NOT:
        return children[0]->possibleValueInv(what_type, what_value);
    case OP_NONE:
        return true;
    default:
        return _possibleValue(what_type, what_value);
    }
}

bool QueryMolecule::Node::possibleValueInv(int what_type, int what_value)
{
    int i;

    switch (type)
    {
    case OP_AND: {
        for (i = 0; i < children.size(); i++)
            if (children[i]->possibleValueInv(what_type, what_value))
                return true;

        return false;
    }
    case OP_OR: {
        for (i = 0; i < children.size(); i++)
            if (!children[i]->possibleValueInv(what_type, what_value))
                return false;

        return true;
    }
    case OP_NOT:
        return children[0]->possibleValue(what_type, what_value);
    case OP_NONE:
        throw QueryMolecule::Error("possibleValueInv(OP_NONE) not implemented");
    default: {
        int val;

        if (!_sureValue(what_type, val))
            return true;

        return val != what_value;
    }
    }
}

bool QueryMolecule::Node::possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2)
{
    int i;

    switch (type)
    {
    case OP_AND: {
        for (i = 0; i < children.size(); i++)
            if (!children[i]->possibleValuePair(what_type1, what_value1, what_type2, what_value2))
                return false;

        return true;
    }
    case OP_OR: {
        for (i = 0; i < children.size(); i++)
            if (children[i]->possibleValuePair(what_type1, what_value1, what_type2, what_value2))
                return true;

        return false;
    }
    case OP_NOT:
        return children[0]->possibleValuePairInv(what_type1, what_value1, what_type2, what_value2);
    case OP_NONE:
        return true;
    default:
        return _possibleValuePair(what_type1, what_value1, what_type2, what_value2);
    }
}

bool QueryMolecule::Node::possibleValuePairInv(int what_type1, int what_value1, int what_type2, int what_value2)
{
    int i;

    switch (type)
    {
    case OP_AND: {
        for (i = 0; i < children.size(); i++)
            if (children[i]->possibleValuePairInv(what_type1, what_value1, what_type2, what_value2))
                return true;

        return false;
    }
    case OP_OR: {
        for (i = 0; i < children.size(); i++)
            if (!children[i]->possibleValuePairInv(what_type1, what_value1, what_type2, what_value2))
                return false;

        return true;
    }
    case OP_NOT:
        return children[0]->possibleValuePair(what_type1, what_value1, what_type2, what_value2);
    case OP_NONE:
        throw QueryMolecule::Error("possibleValuePairInv(OP_NONE) not implemented");
    default: {
        int val1, val2;
        bool sure1, sure2;

        sure1 = _sureValue(what_type1, val1);
        if (sure1 && !hasConstraint(what_type2) && val1 == what_value1)
            return false;

        sure2 = _sureValue(what_type2, val2);
        if (sure2 && !hasConstraint(what_type1) && val2 == what_value2)
            return false;

        if (sure1 && sure2 && val1 == what_value1 && val2 == what_value2)
            return false;

        return true;
    }
    }
}

bool QueryMolecule::Node::sureValueBelongs(int what_type, const int* arr, int count)
{
    int i;

    switch (type)
    {
    case OP_AND: {
        // Of course one can fool this around;
        // see comment in sureValue()
        for (i = 0; i < children.size(); i++)
            if (children[i]->sureValueBelongs(what_type, arr, count))
                return true;

        return false;
    }
    case OP_OR: {
        for (i = 0; i < children.size(); i++)
            if (!children[i]->sureValueBelongs(what_type, arr, count))
                return false;

        return true;
    }
    case OP_NOT:
        return children[0]->sureValueBelongsInv(what_type, arr, count);
    case OP_NONE:
        return false;
    default:
        return _sureValueBelongs(what_type, arr, count);
    }
}

bool QueryMolecule::Node::hasOP_OR()
{
    int i;

    switch (type)
    {
    case OP_AND: {
        for (i = 0; i < children.size(); i++)
            if (children[i]->hasOP_OR())
                return true;

        return false;
    }
    case OP_OR: {
        return true;
    }
    case OP_NOT:
        return false;
    default:
        return false;
    }
}

QueryMolecule::Atom* QueryMolecule::Atom::sureConstraint(int what_type)
{
    int count = 0;
    Atom* found = (Atom*)_findSureConstraint(what_type, count);
    if (count == 1)
        return found;
    return NULL;
}

QueryMolecule::Node* QueryMolecule::Node::_findSureConstraint(int what_type, int& count)
{
    switch (type)
    {
    case OP_AND:
    case OP_OR: {
        Node* subnode_found = NULL;
        for (int i = 0; i < children.size(); i++)
        {
            Node* subnode = children[i]->_findSureConstraint(what_type, count);
            if (subnode != NULL)
                subnode_found = subnode;
        }
        return subnode_found;
    }
    case OP_NOT: {
        children[0]->_findSureConstraint(what_type, count);
        return NULL; // Do not return anything in this case but increase count if found
    }
    case OP_NONE:
        return NULL;
    default:
        if (type == what_type)
        {
            count++;
            return this;
        }
        return NULL;
    }
}

bool QueryMolecule::Node::sureValueBelongsInv(int what_type, const int* arr, int count)
{
    int i;

    switch (type)
    {
    case OP_OR: {
        for (i = 0; i < children.size(); i++)
            if (children[i]->sureValueBelongsInv(what_type, arr, count))
                return true;

        return false;
    }
    case OP_AND: {
        for (i = 0; i < children.size(); i++)
        {
            if (!children[i]->sureValueBelongsInv(what_type, arr, count))
                return false;
        }
        return true;
    }
    case OP_NOT:
        return children[0]->sureValueBelongs(what_type, arr, count);
    case OP_NONE:
        throw QueryMolecule::Error("sureValueBelongsInv(OP_NONE) not implemented");
    default:
        return false;
    }
}

void QueryMolecule::Node::optimize()
{
    switch (type)
    {
    case OP_AND:
    case OP_OR:
    case OP_NOT:
        for (int i = 0; i < children.size(); i++)
            children[i]->optimize();
        break;
    case OP_NONE:
        return;
    default:
        break;
    }
    _optimize();
}

bool QueryMolecule::Atom::_sureValue(int what_type, int& value_out) const
{
    if (type == what_type && value_max == value_min)
    {
        value_out = value_min;
        return true;
    }

    // common case for fragment 'atoms' in SMARTS queries
    if (type == ATOM_FRAGMENT && fragment->vertexCount() > 0)
    {
        if (fragment->getAtom(fragment->vertexBegin()).sureValue(what_type, value_out))
            return true;
    }

    return false;
}

bool QueryMolecule::Atom::_sureValueBelongs(int what_type, const int* arr, int count)
{
    int i;

    if (type == what_type)
    {
        for (i = 0; i < count; i++)
            if (arr[i] < value_min || arr[i] > value_max)
                return false;
        return true;
    }

    return false;
}

bool QueryMolecule::Atom::_possibleValue(int what_type, int what_value)
{
    if (type == what_type)
        return what_value >= value_min && what_value <= value_max;

    if (type == ATOM_FRAGMENT && fragment->vertexCount() > 0)
    {
        if (!fragment->getAtom(fragment->vertexBegin()).possibleValue(what_type, what_value))
            return false;
    }

    return true;
}

bool QueryMolecule::Atom::_possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2)
{
    if (type == what_type1)
        return what_value1 >= value_min && what_value1 <= value_max;

    if (type == what_type2)
        return what_value2 >= value_min && what_value2 <= value_max;

    if (type == ATOM_FRAGMENT && fragment->vertexCount() > 0)
    {
        if (!fragment->getAtom(fragment->vertexBegin()).possibleValuePair(what_type1, what_value1, what_type2, what_value2))
            return false;
    }
    return true;
}

void QueryMolecule::Atom::_optimize()
{
    // Check if fragment has one atom
    if (type == ATOM_FRAGMENT && fragment->vertexCount() == 1)
    {
        std::unique_ptr<QueryMolecule> saved_fragment(fragment.release());
        copy(saved_fragment->getAtom(saved_fragment->vertexBegin()));
    }
}

bool QueryMolecule::Bond::_sureValue(int what_type, int& value_out) const
{
    if (type == what_type)
    {
        value_out = value;
        return true;
    }

    return false;
}

bool QueryMolecule::Bond::_sureValueBelongs(int /*what_type*/, const int* /*arr*/, int /*count*/)
{
    throw QueryMolecule::Error("not implemented");
}

bool QueryMolecule::Bond::_possibleValue(int what_type, int what_value)
{
    if (type == what_type)
        return what_value == value;

    return true;
}

bool QueryMolecule::Bond::_possibleValuePair(int what_type1, int what_value1, int what_type2, int what_value2)
{
    if (type == what_type1)
        return what_value1 == value;
    if (type == what_type2)
        return what_value2 == value;
    if (type == BOND_ANY)
        return true;
    return false;
}

bool QueryMolecule::Atom::valueWithinRange(int value)
{
    bool result = value >= value_min && value <= value_max;
    return result;
}

QueryMolecule::Atom* QueryMolecule::Atom::child(int idx)
{
    return static_cast<Atom*>(children[idx]);
}

QueryMolecule::Bond* QueryMolecule::Bond::child(int idx)
{
    return static_cast<Bond*>(children[idx]);
}

QueryMolecule::Node* QueryMolecule::Atom::_neu()
{
    return new Atom();
}

QueryMolecule::Node* QueryMolecule::Bond::_neu()
{
    return new Bond();
}

int QueryMolecule::addAtom(Atom* atom)
{
    int idx = _addBaseAtom();

    _atoms.expand(idx + 1);
    _atoms.set(idx, atom);

    updateEditRevision();
    return idx;
}

QueryMolecule::Atom& QueryMolecule::getAtom(int idx)
{
    return *_atoms[idx];
}

QueryMolecule::Atom* QueryMolecule::releaseAtom(int idx)
{
    updateEditRevision();
    return _atoms.release(idx);
}

void QueryMolecule::resetAtom(int idx, QueryMolecule::Atom* atom)
{
    if (atom != _atoms[idx])
        _atoms.reset(idx);
    _atoms[idx] = atom;
    updateEditRevision();
}

int QueryMolecule::addBond(int beg, int end, QueryMolecule::Bond* bond)
{
    int idx = _addBaseBond(beg, end);

    _bonds.expand(idx + 1);
    _bonds.set(idx, bond);

    invalidateAtom(beg, CHANGED_CONNECTIVITY);
    invalidateAtom(end, CHANGED_CONNECTIVITY);

    aromaticity.setCanBeAromatic(idx, false);
    setBondStereoCare(idx, false);

    updateEditRevision();

    return idx;
}

int QueryMolecule::addBond_Silent(int beg, int end, int order)
{
    updateEditRevision();
    int idx = _addBaseBond(beg, end);

    _bonds.expand(idx + 1);
    _bonds.set(idx, QueryMolecule::createQueryMoleculeBond(order, BOND_ZERO, BOND_ZERO));

    aromaticity.setCanBeAromatic(idx, false);
    setBondStereoCare(idx, false);

    updateEditRevision();

    return idx;
}

QueryMolecule::Bond& QueryMolecule::getBond(int idx)
{
    return *_bonds[idx];
}

QueryMolecule::Bond* QueryMolecule::releaseBond(int idx)
{
    updateEditRevision();
    return _bonds.release(idx);
}

void QueryMolecule::resetBond(int idx, QueryMolecule::Bond* bond)
{
    _bonds.reset(idx);
    _bonds[idx] = bond;
    _min_h.clear();
    updateEditRevision();
}

void QueryMolecule::Atom::copy(const Atom& other)
{
    type = other.type;
    value_max = other.value_max;
    value_min = other.value_min;
    occurrence_idx = other.occurrence_idx;

    fragment.reset(nullptr);
    if (other.fragment.get() != 0)
    {
        fragment = std::make_unique<QueryMolecule>();
        fragment->clone(*other.fragment, 0, 0);
        fragment->fragment_smarts.copy(other.fragment->fragment_smarts);
    }
    alias.copy(other.alias);

    children.clear();
    for (int i = 0; i < other.children.size(); i++)
        children.add(((Atom*)other.children[i])->clone());
}

QueryMolecule::Atom* QueryMolecule::Atom::clone() const
{
    std::unique_ptr<Atom> res = std::make_unique<Atom>();
    res->copy(*this);
    return res.release();
}

QueryMolecule::Bond* QueryMolecule::Bond::clone()
{
    std::unique_ptr<Bond> res = std::make_unique<Bond>();
    int i;

    res->type = type;
    res->value = value;
    res->direction = direction;

    for (i = 0; i < children.size(); i++)
        res->children.add(((Bond*)children[i])->clone());

    return res.release();
}

bool QueryMolecule::Node::hasConstraint(int what_type)
{
    if (type == what_type)
        return true;

    if (type == OP_NONE)
        return false;

    if (type == OP_AND || type == OP_OR || type == OP_NOT)
    {
        for (int i = 0; i < children.size(); i++)
            if (children[i]->hasConstraint(what_type))
                return true;
    }

    return false;
}

bool QueryMolecule::Node::hasNoConstraintExcept(int what_type)
{
    return hasNoConstraintExcept(what_type, what_type);
}

bool QueryMolecule::Node::hasNoConstraintExcept(int what_type1, int what_type2)
{
    if (type == OP_NONE)
        return true;

    if (type == OP_AND || type == OP_OR || type == OP_NOT)
    {
        int i;

        for (i = 0; i < children.size(); i++)
            if (!children[i]->hasNoConstraintExcept(what_type1, what_type2))
                return false;

        return true;
    }

    return type == what_type1 || type == what_type2;
}

bool QueryMolecule::Node::hasNoConstraintExcept(std::vector<int> what_types)
{
    if (type == OP_NONE)
        return true;

    if (type == OP_AND || type == OP_OR || type == OP_NOT)
    {
        int i;

        for (i = 0; i < children.size(); i++)
            if (!children[i]->hasNoConstraintExcept(what_types))
                return false;

        return true;
    }

    return std::any_of(what_types.cbegin(), what_types.cend(), [this](int i) { return type == i; });
}

void QueryMolecule::Node::removeConstraints(int what_type)
{
    if (type == what_type)
    {
        type = OP_NONE;
        return;
    }

    if (type == OP_AND || type == OP_OR || type == OP_NOT)
    {
        int i;

        for (i = children.size() - 1; i >= 0; i--)
        {
            children[i]->removeConstraints(what_type);
            if (children[i]->type == OP_NONE)
                children.remove(i);
        }
        if (children.size() == 0)
            type = OP_NONE;
    }
}

void QueryMolecule::clear()
{
    BaseMolecule::clear();

    _atoms.clear();
    _bonds.clear();
    spatial_constraints.clear();
    fixed_atoms.clear();
    _bond_stereo_care.clear();
    _min_h.clear();
    aromaticity.clear();
    components.clear();
    fragment_smarts.clear();
    updateEditRevision();
}

BaseMolecule* QueryMolecule::neu() const
{
    return new QueryMolecule();
}

bool QueryMolecule::bondStereoCare(int idx)
{
    if (idx >= _bond_stereo_care.size())
        return false;

    if (_bond_stereo_care[idx] && cis_trans.getParity(idx) == 0)
        throw Error("bond #%d has stereo-care flag, but is not cis-trans bond", idx);

    return _bond_stereo_care[idx];
}

void QueryMolecule::setBondStereoCare(int idx, bool stereo_care)
{
    if (stereo_care == false && idx >= _bond_stereo_care.size())
        return;

    _bond_stereo_care.expandFill(idx + 1, false);
    _bond_stereo_care[idx] = stereo_care;
    updateEditRevision();
}

bool QueryMolecule::aromatize(const AromaticityOptions& options)
{
    updateEditRevision();
    return QueryMoleculeAromatizer::aromatizeBonds(*this, options);
}

bool QueryMolecule::dearomatize(const AromaticityOptions& options)
{
    updateEditRevision();
    return MoleculeDearomatizer::dearomatizeMolecule(*this, options);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
