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
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_standardize.h"
#include <string>
#include <unordered_map>

using namespace indigo;

QueryMolecule::QueryMolecule() : spatial_constraints(*this)
{
}

QueryMolecule::~QueryMolecule()
{
}

QueryMolecule& QueryMolecule::asQueryMolecule()
{
    return *this;
}

bool QueryMolecule::isQueryMolecule()
{
    return true;
}

int QueryMolecule::getAtomNumber(int idx)
{
    int res;
    QueryMolecule::Atom* atom = _atoms[idx];

    if (atom->sureValue(ATOM_NUMBER, res))
        return res;

    return -1;
}

int QueryMolecule::getAtomIsotope(int idx)
{
    int res;

    if (_atoms[idx]->sureValue(ATOM_ISOTOPE, res))
        return res;

    return -1;
}

int QueryMolecule::getAtomCharge(int idx)
{
    int res;

    if (_atoms[idx]->sureValue(ATOM_CHARGE, res))
        return res;

    return CHARGE_UNKNOWN;
}

int QueryMolecule::getAtomRadical(int idx)
{
    int res;

    if (_atoms[idx]->sureValue(ATOM_RADICAL, res))
        return res;

    return -1;
}

int QueryMolecule::getExplicitValence(int idx)
{
    int res;

    if (_atoms[idx]->sureValue(ATOM_VALENCE, res))
        return res;

    return -1;
}

int QueryMolecule::getAtomAromaticity(int idx)
{
    int res;

    if (_atoms[idx]->sureValue(ATOM_AROMATICITY, res))
        return res;

    return -1;
}

int QueryMolecule::getBondOrder(int idx) const
{
    int res;

    if (_bonds[idx]->sureValue(BOND_ORDER, res))
        return res;

    return -1;
}

int QueryMolecule::getBondTopology(int idx)
{
    int res;

    if (getEdgeTopology(idx) == TOPOLOGY_RING)
        return TOPOLOGY_RING;

    if (_bonds[idx]->sureValue(BOND_TOPOLOGY, res))
        return res;

    return -1;
}

int QueryMolecule::getAtomValence(int idx)
{
    throw Error("not implemented");
}

int QueryMolecule::getAtomSubstCount(int idx)
{
    int res;

    if (_atoms[idx]->sureValue(ATOM_SUBSTITUENTS, res))
        return res;
    if (_atoms[idx]->sureValue(ATOM_SUBSTITUENTS_AS_DRAWN, res))
        return res;

    return -1;
}

int QueryMolecule::getAtomRingBondsCount(int idx)
{
    int res;

    if (_atoms[idx]->sureValue(ATOM_RING_BONDS, res))
        return res;
    if (_atoms[idx]->sureValue(ATOM_RING_BONDS_AS_DRAWN, res))
        return res;

    return -1;
}

int QueryMolecule::getAtomConnectivity(int idx)
{
    return 0;
}

bool QueryMolecule::atomNumberBelongs(int idx, const int* numbers, int count)
{
    return _atoms[idx]->sureValueBelongs(ATOM_NUMBER, numbers, count);
}

bool QueryMolecule::possibleAtomNumber(int idx, int number)
{
    QueryMolecule::Atom* atom = _atoms[idx];

    if (!atom->possibleValue(ATOM_NUMBER, number))
        return false;

    return true;
}

bool QueryMolecule::possibleAtomNumberAndCharge(int idx, int number, int charge)
{
    return _atoms[idx]->possibleValuePair(ATOM_NUMBER, number, ATOM_CHARGE, charge);
}

bool QueryMolecule::possibleAtomNumberAndIsotope(int idx, int number, int isotope)
{
    return _atoms[idx]->possibleValuePair(ATOM_NUMBER, number, ATOM_ISOTOPE, isotope);
}

bool QueryMolecule::possibleAtomIsotope(int idx, int isotope)
{
    return _atoms[idx]->possibleValue(ATOM_ISOTOPE, isotope);
}

bool QueryMolecule::possibleAtomCharge(int idx, int charge)
{
    return _atoms[idx]->possibleValue(ATOM_CHARGE, charge);
}

bool QueryMolecule::possibleAtomRadical(int idx, int radical)
{
    return _atoms[idx]->possibleValue(ATOM_RADICAL, radical);
}

void QueryMolecule::getAtomDescription(int idx, Array<char>& description)
{
    ArrayOutput out(description);

    out.writeChar('[');
    _getAtomDescription(_atoms[idx], out, 0);
    out.writeChar(']');
    out.writeChar(0);
}

void QueryMolecule::_getAtomDescription(Atom* atom, Output& out, int depth)
{
    int i;

    switch (atom->type)
    {
    case OP_NONE:
        out.writeChar('*');
        return;
    case OP_AND: {
        if (depth > 0)
            out.writeChar('(');
        for (i = 0; i < atom->children.size(); i++)
        {
            if (i > 0)
                out.writeString(";");
            _getAtomDescription((Atom*)atom->children[i], out, depth + 1);
        }
        if (depth > 0)
            out.writeChar(')');
        return;
    }
    case OP_OR: {
        if (depth > 0)
            out.writeChar('(');
        for (i = 0; i < atom->children.size(); i++)
        {
            if (i > 0)
                out.writeString(",");
            _getAtomDescription((Atom*)atom->children[i], out, depth + 1);
        }
        if (depth > 0)
            out.writeChar(')');
        return;
    }
    case OP_NOT:
        out.writeString("!");
        _getAtomDescription((Atom*)atom->children[0], out, depth + 1);
        return;
    case ATOM_NUMBER:
        out.writeString(Element::toString(atom->value_min));
        return;
    case ATOM_PSEUDO:
        out.writeString(atom->alias.ptr());
        return;
    case ATOM_TEMPLATE:
        out.writeString(atom->alias.ptr());
        return;
    case ATOM_CHARGE:
        out.printf("%+d", atom->value_min);
        return;
    case ATOM_ISOTOPE:
        out.printf("i%d", atom->value_min);
        return;
    case ATOM_AROMATICITY:
        if (atom->value_min == ATOM_AROMATIC)
            out.printf("a");
        else
            out.printf("A");
        return;
    case ATOM_RADICAL:
        out.printf("^%d", atom->value_min);
        return;
    case ATOM_FRAGMENT:
        out.printf("$(");
        if (atom->fragment->fragment_smarts.ptr() != 0 && atom->fragment->fragment_smarts.size() > 0)
            out.printf("%s", atom->fragment->fragment_smarts.ptr());
        out.printf(")");
        return;
    case ATOM_TOTAL_H:
        out.printf("H%d", atom->value_min);
        return;
    case ATOM_IMPLICIT_H:
        if (atom->value_min == 1 && atom->value_max == 100)
        {
            out.printf("h");
        }
        else
        {
            out.printf("h%d", atom->value_min);
        }
        return;
    case ATOM_CONNECTIVITY:
        out.printf("X%d", atom->value_min);
        return;
    case ATOM_SUBSTITUENTS:
        out.printf("s%d", atom->value_min);
        return;
    case ATOM_SUBSTITUENTS_AS_DRAWN:
        out.printf("s*");
        return;
    case ATOM_RING_BONDS:
        out.printf("rb%d", atom->value_min);
        return;
    case ATOM_RING_BONDS_AS_DRAWN:
        out.printf("rb*");
        return;
    case ATOM_UNSATURATION:
        out.printf("u");
        return;
    case ATOM_TOTAL_BOND_ORDER:
        out.printf("v%d", atom->value_min);
        return;
    case ATOM_SSSR_RINGS:
        out.printf("R%d", atom->value_min);
        return;
    case ATOM_SMALLEST_RING_SIZE:
        out.printf("r%d", atom->value_min);
        return;
    case ATOM_VALENCE:
        return;
    default:
        throw new Error("Unrecognized constraint type %d", atom->type);
    }
}

std::string QueryMolecule::getSmartsBondStr(Bond* bond)
{
    Array<char> out;
    ArrayOutput output(out);
    writeSmartsBond(output, bond, false);
    std::string result{out.ptr(), static_cast<std::size_t>(out.size())};
    return result;
}

void QueryMolecule::writeSmartsBond(Output& output, Bond* bond, bool has_or_parent)
{
    int i;

    switch (bond->type)
    {
    case OP_NONE:
        output.writeChar('~');
        break;
    case OP_NOT: {
        output.writeChar('!');
        writeSmartsBond(output, bond->child(0), has_or_parent);
        break;
    }
    case OP_OR: {
        for (i = 0; i < bond->children.size(); i++)
        {
            if (i > 0)
                output.printf(",");
            writeSmartsBond(output, bond->child(i), true);
        }
        break;
    }
    case OP_AND: {
        for (i = 0; i < bond->children.size(); i++)
        {
            if (i > 0)
                output.writeChar(has_or_parent ? '&' : ';');
            writeSmartsBond(output, bond->child(i), has_or_parent);
        }
        break;
    }
    case BOND_ORDER: {
        int bond_order = bond->value;
        if (bond_order == BOND_SINGLE)
        {
            if (bond->direction == BOND_UP)
                output.writeChar('/');
            else if (bond->direction == BOND_DOWN)
                output.writeChar('\\');
            else
                output.writeChar('-');
        }
        if (bond_order == BOND_DOUBLE)
            output.writeChar('=');
        else if (bond_order == BOND_TRIPLE)
            output.writeChar('#');
        else if (bond_order == BOND_AROMATIC)
            output.writeChar(':');
        break;
    }
    case BOND_TOPOLOGY: {
        if (bond->value == TOPOLOGY_RING)
            output.writeChar('@');
        break;
    }
    default:
        throw Error("Unexpected bond type: %d", bond->type);
    }
}

std::string QueryMolecule::getSmartsAtomStr(QueryMolecule::Atom* atom)
{
    Array<char> out;
    ArrayOutput output(out);
    writeSmartsAtom(output, atom, -1, -1, 1, false, false);
    std::string result{out.ptr(), static_cast<std::size_t>(out.size())};
    return result;
}

static void _write_num(indigo::Output& output, unsigned char ch, int num)
{
    output.writeChar(ch);
    if (num != 1)
        output.printf("%d", num);
}

static void _write_num_if_set(indigo::Output& output, unsigned char ch, int min, int max)
{
    if (min == 1 && max == 100)
        output.writeChar(ch);
    else
    {
        output.printf("%c%d", ch, min);
    }
}

static void writeAnd(Output& _output, QueryMolecule::Node* node, bool has_or_parent)
{
    if (has_or_parent)
        _output.writeChar('&');
    else if (node->hasOP_OR())
        _output.writeChar(';');
}

void QueryMolecule::writeSmartsAtom(Output& output, Atom* atom, int aam, int chirality, int depth, bool has_or_parent, bool has_not_parent)
{
    int i;

    if (depth == 0)
        output.printf("[");

    switch (atom->type)
    {
    case OP_NOT: {
        if (atom->artificial) // Skip atoms added by loader (!#1)
        {
            break;
        }
        else if (isNotAtom(*atom, ELEM_H))
        {
            output.printf("*");
            break;
        }
        output.writeChar('!');
        writeSmartsAtom(output, atom->child(0), aam, chirality, depth + 1, has_or_parent, true);
        break;
    }
    case OP_AND: {
        bool has_number = false;
        bool has_aromatic = false;
        bool aromatic = false;
        char atom_name[10];
        int cur_pos = output.tell();
        for (i = 0; i < atom->children.size(); i++)
        {
            if (atom->children[i]->type == ATOM_NUMBER)
            {
                has_number = true;
                strncpy(atom_name, Element::toString(atom->child(i)->value_max), sizeof(atom_name));
            }
            if (atom->children[i]->type == ATOM_AROMATICITY)
            {
                has_aromatic = true;
                aromatic = atom->child(i)->value_min == ATOM_AROMATIC;
            }
        }
        if (has_aromatic && has_number)
        { // Convert a & #6 -> c,  A & #6 -> C
            if (aromatic)
                atom_name[0] = tolower(atom_name[0]);
            output.printf("%s", atom_name);
        }
        for (i = 0; i < atom->children.size(); i++)
        {
            if (has_aromatic && has_number && (atom->children[i]->type == ATOM_AROMATICITY || atom->children[i]->type == ATOM_NUMBER))
            {
                continue;
            }
            if (atom->children[i]->type == ATOM_RADICAL || atom->children[i]->type == ATOM_VALENCE)
            {
                continue;
            }
            if (atom->children[i]->type == OP_NOT && atom->children[i]->artificial)
            {
                continue;
            }

            if (output.tell() > cur_pos)
            {
                output.writeChar(has_or_parent ? '&' : ';');
                cur_pos = output.tell();
            }
            writeSmartsAtom(output, atom->child(i), aam, chirality, depth + 1, has_or_parent, has_not_parent);
        }
        break;
    }
    case OP_OR: {
        for (i = 0; i < atom->children.size(); i++)
        {
            if (atom->children[i]->type == QueryMolecule::ATOM_RADICAL || atom->children[i]->type == QueryMolecule::ATOM_VALENCE)
            {
                continue;
            }

            if (i > 0)
                output.printf(has_not_parent ? "!" : ",");
            writeSmartsAtom(output, atom->child(i), aam, chirality, depth + 1, true, has_not_parent);
        }
        break;
    }
    case ATOM_ISOTOPE:
        output.printf("%d", atom->value_max);
        break;
    case ATOM_NUMBER: {
        output.printf("#%d", atom->value_max);
        if (chirality == 1)
            output.printf("@");
        else if (chirality == 2)
            output.printf("@@");

        if (aam > 0)
            output.printf(":%d", aam);

        break;
    }
    case ATOM_CHARGE: {
        int charge = atom->value_max;

        if (charge > 1)
            output.printf("+%d", charge);
        else if (charge < -1)
            output.printf("-%d", -charge);
        else if (charge == 1)
            output.printf("+");
        else if (charge == -1)
            output.printf("-");
        else
            output.printf("+0");
        break;
    }
    case ATOM_FRAGMENT: {
        if (atom->fragment->fragment_smarts.ptr() == 0)
            throw Error("fragment_smarts has unexpectedly gone");
        output.printf("$(%s)", atom->fragment->fragment_smarts.ptr());
        break;
    }
    case ATOM_AROMATICITY: {
        if (atom->value_min == ATOM_AROMATIC)
            output.printf("a");
        else
            output.printf("A");
        break;
    }
    case OP_NONE:
        output.writeChar('*');
        break;
    case ATOM_TOTAL_H: {
        _write_num(output, 'H', atom->value_min);
        break;
    }

    case ATOM_SSSR_RINGS: {
        _write_num_if_set(output, 'R', atom->value_min, atom->value_max);
        break;
    }

    case ATOM_RING_BONDS_AS_DRAWN: {
        output.printf("x:%d", atom->value_min);
        break;
    }

    case ATOM_RING_BONDS: {
        _write_num_if_set(output, 'x', atom->value_min, atom->value_max);
        break;
    }

    case ATOM_IMPLICIT_H: {
        _write_num_if_set(output, 'h', atom->value_min, atom->value_max);
        break;
    }

    case ATOM_UNSATURATION: {
        output.printf("$([*,#1]=,#,:[*,#1])");
        break;
    }

    case ATOM_SMALLEST_RING_SIZE: {
        _write_num_if_set(output, 'r', atom->value_min, atom->value_max);
        break;
    }

    case ATOM_SUBSTITUENTS: {
        output.printf("D%d", atom->value_min);
        break;
    }

    case ATOM_SUBSTITUENTS_AS_DRAWN: {
        output.printf("D%d", atom->value_min);
        break;
    }

    case ATOM_PSEUDO: {
        output.printf("*", atom->alias.ptr());
        break;
    }

    case ATOM_CONNECTIVITY: {
        output.printf("X%d", atom->value_min);
        break;
    }

    case ATOM_TOTAL_BOND_ORDER: {
        _write_num(output, 'v', atom->value_min);
        break;
    }

    default: {
        throw Error("Unknown atom attribute %d", atom->type);
        break;
    }
    }

    if (depth == 0)
        output.writeChar(']');
}

void QueryMolecule::getBondDescription(int idx, Array<char>& description)
{
    ArrayOutput out(description);

    _getBondDescription(_bonds[idx], out);
    out.writeChar(0);
}

void QueryMolecule::_getBondDescription(Bond* bond, Output& out)
{
    int i;

    switch (bond->type)
    {
    case OP_NONE:
        out.writeChar('~');
        return;
    case OP_AND: {
        out.writeChar('(');
        for (i = 0; i < bond->children.size(); i++)
        {
            if (i > 0)
                out.writeString(" & ");
            _getBondDescription((Bond*)bond->children[i], out);
        }
        out.writeChar(')');
        return;
    }
    case OP_OR: {
        out.writeChar('(');
        for (i = 0; i < bond->children.size(); i++)
        {
            if (i > 0)
                out.writeString(" | ");
            _getBondDescription((Bond*)bond->children[i], out);
        }
        out.writeChar(')');
        return;
    }
    case OP_NOT:
        out.writeString("!(");
        _getBondDescription((Bond*)bond->children[0], out);
        out.writeChar(')');
        return;
    case BOND_ORDER:
        out.printf("order = %d", bond->value);
        return;
    case BOND_TOPOLOGY:
        out.printf("%s", bond->value == TOPOLOGY_RING ? "ring" : "chain");
        return;
    default:
        out.printf("<constraint of type %d>", bond->type);
    }
}

bool QueryMolecule::possibleBondOrder(int idx, int order)
{
    return _bonds[idx]->possibleValue(BOND_ORDER, order);
}

bool QueryMolecule::possibleNitrogenV5(int idx)
{
    if (!possibleAtomNumber(idx, ELEM_N))
        return false;

    if (!possibleAtomCharge(idx, 0))
        return false;

    // Other conditions also can be checked as in Molecule::isNitrogenV5 but
    // two the meaning of this function can be different: check if nitrogen
    // can valence 5 in the embedding or check if nitrogen can have valence 5
    // in the original query molecule as self-contained molecule

    return true;
}

bool QueryMolecule::isPseudoAtom(int idx)
{
    // This is dirty hack; however, it is legal here, as pseudo atoms
    // can not be present in deep query trees due to Molfile and SMILES
    // format limitations. If they could, we would have to implement
    // sureValue() for ATOM_PSEUDO

    if (_atoms[idx]->type == ATOM_PSEUDO)
        return true;

    if (_atoms[idx]->type == OP_AND)
    {
        int i;

        for (i = 0; i < _atoms[idx]->children.size(); i++)
            if (_atoms[idx]->children[i]->type == ATOM_PSEUDO)
                return true;
    }

    return false;
}

const char* QueryMolecule::getPseudoAtom(int idx)
{
    // see the comment above in isPseudoAtom()

    if (_atoms[idx]->type == ATOM_PSEUDO)
        return _atoms[idx]->alias.ptr();

    if (_atoms[idx]->type == OP_AND)
    {
        int i;

        for (i = 0; i < _atoms[idx]->children.size(); i++)
            if (_atoms[idx]->children[i]->type == ATOM_PSEUDO)
                return ((Atom*)_atoms[idx]->children[i])->alias.ptr();
    }

    throw Error("getPseudoAtom() applied to something that is not a pseudo-atom");
}

bool QueryMolecule::isTemplateAtom(int idx)
{
    // This is dirty hack; however, it is legal here, as template atoms
    // can not be present in deep query trees due to Molfile and SMILES
    // format limitations. If they could, we would have to implement
    // sureValue() for ATOM_TEMPLATE

    if (_atoms[idx]->type == ATOM_TEMPLATE)
        return true;

    if (_atoms[idx]->type == OP_AND)
    {
        int i;

        for (i = 0; i < _atoms[idx]->children.size(); i++)
            if (_atoms[idx]->children[i]->type == ATOM_TEMPLATE)
                return true;
    }

    return false;
}

const char* QueryMolecule::getTemplateAtom(int idx)
{
    // see the comment above in isTemplateAtom()

    if (_atoms[idx]->type == ATOM_TEMPLATE)
        return _atoms[idx]->alias.ptr();

    if (_atoms[idx]->type == OP_AND)
    {
        int i;

        for (i = 0; i < _atoms[idx]->children.size(); i++)
            if (_atoms[idx]->children[i]->type == ATOM_TEMPLATE)
                return ((Atom*)_atoms[idx]->children[i])->alias.ptr();
    }

    throw Error("getTemplateAtom() applied to something that is not a template atom");
}

const char* QueryMolecule::getTemplateAtomClass(int idx)
{
    return 0;
}

const int QueryMolecule::getTemplateAtomSeqid(int idx)
{
    return -1;
}

const int QueryMolecule::getTemplateAtomDisplayOption(int idx)
{
    return -1;
}

bool QueryMolecule::isSaturatedAtom(int idx)
{
    throw Error("not implemented");
}

QueryMolecule::Node::Node(int type_) : artificial(false)
{
    type = (OpType)type_;
}

QueryMolecule::Node::~Node()
{
}

IMPL_ERROR(QueryMolecule::Atom, "query atom");

QueryMolecule::Atom::Atom() : Node(OP_NONE)
{
    value_min = 0;
    value_max = 0;
}

QueryMolecule::Atom::Atom(int type_, int value) : Node(type_)
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

QueryMolecule::Atom::Atom(int type_, int val_min, int val_max) : Node(type_)
{
    value_min = val_min;
    value_max = val_max;
}

QueryMolecule::Atom::Atom(int type_, const char* value) : Node(type_)
{
    if (type_ == ATOM_PSEUDO || type_ == ATOM_TEMPLATE || type_ == ATOM_TEMPLATE_CLASS)
        alias.readString(value, true);
    else
        throw Error("bad type: %d", type_);
}

QueryMolecule::Atom::Atom(int type_, QueryMolecule* value) : Node(type_)
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

void QueryMolecule::_postMergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping,
                                              int skip_flags)
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

        if ((sure1 = _sureValue(what_type1, val1)) && !hasConstraint(what_type2) && val1 == what_value1)
            return false;

        if ((sure2 = _sureValue(what_type2, val2)) && !hasConstraint(what_type1) && val2 == what_value2)
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

bool QueryMolecule::Bond::_sureValueBelongs(int what_type, const int* arr, int count)
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

BaseMolecule* QueryMolecule::neu()
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
    throw Error("Dearomatization of query molecules is not implemented");
}

int QueryMolecule::getAtomMaxH(int idx)
{
    int total;

    if (_atoms[idx]->sureValue(ATOM_TOTAL_H, total))
        return total;

    int number = getAtomNumber(idx);

    if (number == -1)
        return -1;

    int conn = _calcAtomConnectivity(idx);

    if (conn == -1)
        return -1;

    int explicit_val = getExplicitValence(idx);

    int max_h = 0;

    int charge, radical;

    for (charge = -5; charge <= 8; charge++)
    {
        if (!possibleAtomCharge(idx, charge))
            continue;

        for (radical = 0; radical <= RADICAL_DOUBLET; radical++)
        {
            if (!possibleAtomRadical(idx, radical))
                continue;

            if (explicit_val != -1)
            {
                int h = explicit_val - Element::calcValenceMinusHyd(number, charge, radical, conn);
                if (h > max_h)
                    max_h = h;
            }
            else
            {
                int h, val;
                if (Element::calcValence(number, charge, radical, conn, val, h, false))
                {
                    if (h > max_h)
                        max_h = h;
                }
            }
        }
    }

    const Vertex& vertex = getVertex(idx);
    int i;

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (possibleAtomNumber(vertex.neiVertex(i), ELEM_H))
            max_h++;
    }

    return max_h;
}

int QueryMolecule::getAtomMinH(int idx)
{
    if (_min_h.size() > idx && _min_h[idx] >= 0)
        return _min_h[idx];

    int i, min_h = _getAtomMinH(_atoms[idx]);

    if (min_h >= 0)
    {
        _min_h.expandFill(idx + 1, -1);
        _min_h[idx] = min_h;
        return min_h;
    }

    const Vertex& vertex = getVertex(idx);
    min_h = 0;

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (getAtomNumber(vertex.neiVertex(i)) == ELEM_H)
            min_h++;
    }

    _min_h.expandFill(idx + 1, -1);
    _min_h[idx] = min_h;
    return min_h;
}

int QueryMolecule::getAtomMaxExteralConnectivity(int idx)
{
    int number = getAtomNumber(idx);
    if (number == -1)
        return -1;

    int min_local_h = _getAtomMinH(_atoms[idx]);
    if (min_local_h == -1)
        min_local_h = 0;
    int min_conn = _calcAtomConnectivity(idx);
    if (min_conn == -1)
        min_conn = 0;

    int max_conn = 0;
    for (int charge = -5; charge <= 8; charge++)
    {
        if (!possibleAtomCharge(idx, charge))
            continue;

        for (int radical = 0; radical <= RADICAL_DOUBLET; radical++)
        {
            if (!possibleAtomRadical(idx, radical))
                continue;

            int cur_max_conn = Element::getMaximumConnectivity(number, charge, radical, true);
            if (max_conn < cur_max_conn)
                max_conn = cur_max_conn;
        }
    }

    int ext_conn = max_conn - min_conn - min_local_h;
    if (ext_conn < 0)
        return 0;
    return ext_conn;
}

int QueryMolecule::_getAtomMinH(QueryMolecule::Atom* atom)
{
    if (atom->type == ATOM_TOTAL_H)
        return atom->value_min;

    if (atom->type == OP_AND)
    {
        int i;

        for (i = 0; i < atom->children.size(); i++)
        {
            int h = _getAtomMinH((Atom*)atom->children[i]);

            if (h >= 0)
                return h;
        }
    }

    return -1;
}

int QueryMolecule::getAtomTotalH(int idx)
{
    int value;

    if (_atoms[idx]->sureValue(ATOM_TOTAL_H, value))
        return value;

    int minh = getAtomMinH(idx);
    int maxh = getAtomMaxH(idx);

    if (minh == maxh)
        return minh;

    return -1;
}

int QueryMolecule::_calcAtomConnectivity(int idx)
{
    const Vertex& vertex = getVertex(idx);
    int i, conn = 0;

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int order = getBondOrder(vertex.neiEdge(i));

        if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE)
            conn += order;
        else
            conn += 1;
    }

    return conn;
}

bool QueryMolecule::isRSite(int atom_idx)
{
    int bits;

    return _atoms[atom_idx]->sureValue(ATOM_RSITE, bits);
}

dword QueryMolecule::getRSiteBits(int atom_idx)
{
    int bits;

    if (!_atoms[atom_idx]->sureValue(ATOM_RSITE, bits))
        throw Error("getRSiteBits(): atom #%d is not an r-site", atom_idx);

    return (dword)bits;
}

void QueryMolecule::allowRGroupOnRSite(int atom_idx, int rg_idx)
{
    if (rg_idx < 1 || rg_idx > 32)
        throw Error("allowRGroupOnRSite(): rgroup number %d is invalid", rg_idx);

    rg_idx--;

    // This is dirty hack; however, it is legal here, as rgroups
    // can not be present in deep query trees due to Molfile
    // format limitations.

    if (_atoms[atom_idx]->type == ATOM_RSITE)
    {
        _atoms[atom_idx]->value_max |= (1 << rg_idx);
        _atoms[atom_idx]->value_min |= (1 << rg_idx);
        return;
    }

    if (_atoms[atom_idx]->type == OP_AND)
    {
        int i;

        for (i = 0; i < _atoms[atom_idx]->children.size(); i++)
            if (_atoms[atom_idx]->children[i]->type == ATOM_RSITE)
            {
                ((Atom*)_atoms[atom_idx]->children[i])->value_max |= (1 << rg_idx);
                ((Atom*)_atoms[atom_idx]->children[i])->value_min |= (1 << rg_idx);
            }
    }

    throw Error("allowRGroupOnRSite(): atom #%d does not seem to be an r-site", atom_idx);
}

bool QueryMolecule::isKnownAttr(QueryMolecule::Atom& qa)
{
    return (qa.type == QueryMolecule::ATOM_CHARGE || qa.type == QueryMolecule::ATOM_ISOTOPE || qa.type == QueryMolecule::ATOM_RADICAL ||
            qa.type == QueryMolecule::ATOM_VALENCE || qa.type == QueryMolecule::ATOM_TOTAL_H || qa.type == QueryMolecule::ATOM_SUBSTITUENTS ||
            qa.type == QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN || qa.type == QueryMolecule::ATOM_RING_BONDS ||
            qa.type == QueryMolecule::ATOM_RING_BONDS_AS_DRAWN || qa.type == QueryMolecule::ATOM_UNSATURATION || qa.type == QueryMolecule::ATOM_AROMATICITY ||
            qa.type == QueryMolecule::ATOM_SSSR_RINGS || qa.type == QueryMolecule::ATOM_SMALLEST_RING_SIZE || qa.type == QueryMolecule::ATOM_CONNECTIVITY) &&
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

bool QueryMolecule::_isAtomListOr(const Atom* pqa, std::set<int>& list)
{
    // Check if pqa atom list like or(a1,a2,a3, or(a4,a5,a6), a7)
    if (!pqa)
        return false;
    if (pqa->type != OP_OR)
        return false;
    std::set<int> collected;
    for (auto i = 0; i < pqa->children.size(); i++)
    {
        Atom* pqc = const_cast<Atom*>(pqa)->child(i);
        if (pqc->type == ATOM_NUMBER && pqc->value_max == pqc->value_max)
        {
            collected.insert(pqc->value_min);
        }
        else if (pqc->type == OP_OR)
        {
            if (!_isAtomListOr(pqc, collected))
                return false;
        }
        else
            return false;
    }
    if (collected.size() < 1)
        return false;
    list.insert(collected.begin(), collected.end());
}

bool QueryMolecule::_isAtomOrListAndProps(const Atom* pqa, std::set<int>& list, bool& neg, std::map<int, const Atom*>& properties)
{
    // Check if pqa contains only atom or atom list and atom properties connected by "and"
    // atom list is positive i.e. or(a1,a2,a3,or(a4,a5),a6) or negative
    // negative list like is set of op_not(atom_number) or op_not(positevi list), this set connected by "and"
    if (!pqa)
        return false;
    std::set<int> collected;
    std::map<int, const Atom*> collected_properties;
    if (pqa->type != OP_AND)
    {
        const Atom* pqc = pqa;
        bool is_neg = false;
        if (pqa->type == OP_NOT)
        {
            Atom* pqc = const_cast<Atom*>(pqa)->child(0);
            is_neg = true;
        }
        if (pqc->type == ATOM_NUMBER && pqc->value_min == pqc->value_max)
        {
            list.insert(pqc->value_min);
            neg = is_neg;
            return true;
        }
        else if (!is_neg && pqc->type > ATOM_NUMBER && pqc->type <= ATOM_CHILARITY) // atom property, no negative props here
        {
            properties.emplace(pqc->type, pqc);
            return true;
        }
        else if (_isAtomListOr(pqc, collected))
        {
            neg = is_neg;
            list.insert(collected.begin(), collected.end());
            return true;
        }
        return false;
    }
    // OP_AND
    for (auto i = 0; i < pqa->children.size(); i++)
    {
        Atom* pqc = const_cast<Atom*>(pqa)->child(i);
        bool is_neg = false;
        if (_isAtomOrListAndProps(pqc, collected, is_neg, collected_properties))
        {
            if (list.size() > 0 && is_neg != neg) // allowed only one list type in set - positive or negative
                return false;
            neg = is_neg;
            list.insert(collected.begin(), collected.end());
            properties.insert(collected_properties.begin(), collected_properties.end());
        }
        else
            return false;
    }
    return true;
}

int QueryMolecule::parseQueryAtomSmarts(QueryMolecule& qm, int aid, std::set<int>& list, std::map<int, const Atom*>& properties)
{
    std::set<int> atom_list;
    std::map<int, const Atom*> atom_pros;
    bool negative;
    using special = std::pair<bool, std::set<int>>;
    std::set<int> atom_x{ELEM_F, ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    std::set<int> atom_xh{ELEM_F, ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At, ELEM_H};
    std::set<int> atom_q{ELEM_C, ELEM_H};
    std::set<int> atom_qh{ELEM_C};
    std::set<int> atom_mh{ELEM_C, ELEM_N,  ELEM_O,  ELEM_F,  ELEM_P,  ELEM_S,  ELEM_Cl, ELEM_Se, ELEM_Br,
                          ELEM_I, ELEM_At, ELEM_He, ELEM_Ne, ELEM_Ar, ELEM_Kr, ELEM_Xe, ELEM_Rn};
    std::set<int> atom_m{ELEM_C, ELEM_N,  ELEM_O,  ELEM_F,  ELEM_P,  ELEM_S,  ELEM_Cl, ELEM_Se, ELEM_Br,
                         ELEM_I, ELEM_At, ELEM_He, ELEM_Ne, ELEM_Ar, ELEM_Kr, ELEM_Xe, ELEM_Rn, ELEM_H};
    QueryMolecule::Atom& qa = qm.getAtom(aid);
    if (qa.type == QueryMolecule::OP_NONE)
        return QUERY_ATOM_AH;
    if (_isAtomOrListAndProps(&qa, atom_list, negative, atom_pros))
    {
        list.insert(atom_list.begin(), atom_list.end());
        properties.insert(atom_pros.begin(), atom_pros.end());

        if (negative)
        {
            if (atom_list.size() == 1 && atom_list.count(ELEM_H) > 0)
                return QUERY_ATOM_A; // !H
            else if (list == atom_q)
                return QUERY_ATOM_Q;
            else if (list == atom_qh)
                return QUERY_ATOM_QH;
            else if (list == atom_m)
                return QUERY_ATOM_M;
            else if (list == atom_mh)
                return QUERY_ATOM_MH;
        }
        else
        {
            if (list == atom_x)
                return QUERY_ATOM_X;
            else if (list == atom_xh)
                return QUERY_ATOM_XH;
        }
        if (negative)
        {
            return QUERY_ATOM_NOTLIST;
        }
        else
        {
            if (list.size() == 0)
                return QUERY_ATOM_A;
            else if (list.size() == 1)
                return QUERY_ATOM_SINGLE;
            else
                QUERY_ATOM_LIST;
        }
    }
    else
        return -1;
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
    return qc && qc->type == QueryMolecule::ATOM_NUMBER;
}

bool QueryMolecule::queryAtomIsSpecial(QueryMolecule& qm, int aid)
{
    QS_DEF(Array<int>, list);
    int query_atom_type;

    if ((query_atom_type = QueryMolecule::parseQueryAtom(qm, aid, list)) != -1)
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
        (query_atom_type == QueryMolecule::QUERY_ATOM_AH))
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
    int topology;
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

void QueryMolecule::invalidateAtom(int index, int mask)
{
    BaseMolecule::invalidateAtom(index, mask);
    if (_min_h.size() > index)
        _min_h[index] = -1;
}

void QueryMolecule::optimize()
{
    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        getAtom(i).optimize();
    updateEditRevision();
}

bool QueryMolecule::standardize(const StandardizeOptions& options)
{
    updateEditRevision();
    return MoleculeStandardizer::standardize(*this, options);
}

int QueryMolecule::getAtomType(const char* label)
{
    static const std::unordered_map<std::string, int> atom_types = {{"R", _ATOM_R},   {"A", _ATOM_A},   {"X", _ATOM_X},   {"Q", _ATOM_Q},
                                                                    {"M", _ATOM_M},   {"AH", _ATOM_AH}, {"XH", _ATOM_XH}, {"QH", _ATOM_QH},
                                                                    {"XH", _ATOM_XH}, {"QH", _ATOM_QH}, {"MH", _ATOM_MH}};
    auto it = atom_types.find(label);
    if (it != atom_types.end())
        return it->second;
    return _ATOM_PSEUDO;
}

void QueryMolecule::getQueryAtomLabel(int qa, Array<char>& result)
{
    static const std::unordered_map<int, std::string> query_atom_labels = {{QUERY_ATOM_A, "A"},   {QUERY_ATOM_Q, "Q"},   {QUERY_ATOM_X, "X"},
                                                                           {QUERY_ATOM_AH, "AH"}, {QUERY_ATOM_XH, "XH"}, {QUERY_ATOM_QH, "QH"},
                                                                           {QUERY_ATOM_MH, "MH"}, {QUERY_ATOM_M, "M"}};

    auto it = query_atom_labels.find(qa);
    if (it != query_atom_labels.end())
        result.readString(it->second.c_str(), true);
}

void QueryMolecule::getComponentNeighbors(std::list<std::unordered_set<int>>& componentNeighbors)
{
    std::unordered_map<int, std::unordered_set<int>> componentAtoms;
    for (int i = 0; i < components.size(); ++i)
    {
        int componentId = components[i];
        if (componentId > 0)
        { // vertice[i] belongs to component #Id
            componentAtoms[componentId].insert(i);
        }
    }
    for (auto elem : componentAtoms)
    {
        auto atoms = elem.second;
        if (atoms.size() > 1)
            componentNeighbors.emplace_back(atoms);
    }
}