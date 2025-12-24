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
#include "molecule/molecule_dearom.h"
#include "molecule/molecule_standardize.h"
#include <string>
#include <unordered_map>

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;

bool QueryMolecule::isAtomProperty(OpType type)
{
    return (type > ATOM_PSEUDO && type <= ATOM_CHIRALITY);
}

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

// explicit valence plays role of required connectivity
// (ATOM_TOTAL_ORDER) in QueryMolecule
int QueryMolecule::getExplicitValence(int idx)
{
    int res;
    if (_atoms[idx]->sureValue(ATOM_TOTAL_BOND_ORDER, res))
        return res;

    return -1;
}

void QueryMolecule::setExplicitValence(int idx, int valence)
{
    resetAtom(idx, QueryMolecule::Atom::und(_atoms[idx], new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_BOND_ORDER, valence)));
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

int QueryMolecule::getAtomValence(int /*idx*/)
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
    // Some data stored as min=value, max=100(e.g. MOL format)
    auto atom = _atoms[idx]->sureConstraint(ATOM_SUBSTITUENTS);
    if (atom != nullptr)
        return atom->value_min;

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

bool QueryMolecule::isAromaticByCaseAtom(int num)
{
    if (num == ELEM_C || num == ELEM_N || num == ELEM_O || num == ELEM_P || num == ELEM_S || num == ELEM_Si || num == ELEM_Se || num == ELEM_As ||
        num == ELEM_Te)
        return true;
    return false;
}

bool QueryMolecule::isAromaticByCaseAtom(QueryMolecule::Node* atom)
{
    if (atom->type != ATOM_NUMBER)
        return false;
    return QueryMolecule::isAromaticByCaseAtom(static_cast<QueryMolecule::Atom*>(atom)->value_max);
}

bool QueryMolecule::isOrganicSubset(int num)
{
    if (num == ELEM_B || num == ELEM_C || num == ELEM_N || num == ELEM_O || num == ELEM_P || num == ELEM_S || num == ELEM_F || num == ELEM_Cl ||
        num == ELEM_Br || num == ELEM_I)
        return true;
    return false;
}

bool QueryMolecule::isOrganicSubset(QueryMolecule::Atom* atom)
{
    if (atom->type != ATOM_NUMBER)
        return false;
    return QueryMolecule::isOrganicSubset(atom->value_max);
}

int QueryMolecule::getAtomConnectivity(int /*idx*/)
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
    if (_atoms[idx]->hasConstraint(ATOM_CHARGE))
        return _atoms[idx]->possibleValue(ATOM_CHARGE, charge);
    return 0 == charge; // No charge set - means charge 0
}

bool QueryMolecule::possibleAtomRadical(int idx, int radical)
{
    if (_atoms[idx]->hasConstraint(ATOM_RADICAL))
        return _atoms[idx]->possibleValue(ATOM_RADICAL, radical);
    return 0 == radical; // No radical set - means radical 0
}

void QueryMolecule::getAtomDescription(int idx, Array<char>& description)
{
    ArrayOutput out(description);

    // out.writeChar('[');
    writeSmartsAtom(out, _atoms[idx], -1, -1, 0, false, false, original_format);
    // out.writeChar(']');
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
    case ATOM_CHIRALITY:
        _getAtomChiralityDescription(atom, out);
        break;
    default:
        throw new Error("Unrecognized constraint type %d", atom->type);
    }
}

void QueryMolecule::_getAtomChiralityDescription(Atom* atom, Output& output)
{
    int chirality_type = atom->value_min;
    int chirality_value = atom->value_max & ~CHIRALITY_OR_UNSPECIFIED;
    switch (chirality_type)
    {
    case CHIRALITY_GENERAL:
        switch (chirality_value)
        {
        case CHIRALITY_ANTICLOCKWISE:
            output.writeChar('@');
            break;
        case CHIRALITY_CLOCKWISE:
            output.writeString("@@");
            break;
        default:
            throw Error("Wrong chirality value %d.", chirality_value);
        }
        break;
    case CHIRALITY_TETRAHEDRAL:
        if (chirality_value > CHIRALITY_TETRAHEDRAL_MAX)
            throw Error("Wrong TH chirality value %d", chirality_value);
        output.printf("@TH%d", chirality_value);
        break;
    case CHIRALITY_ALLENE_LIKE:
        if (chirality_value > CHIRALITY_ALLENE_LIKE_MAX)
            throw Error("Wrong AL chirality value %d", chirality_value);
        output.printf("@AL%d", chirality_value);
        break;
    case CHIRALITY_SQUARE_PLANAR:
        if (chirality_value > CHIRALITY_SQUARE_PLANAR_MAX)
            throw Error("Wrong SP chirality value %d", chirality_value);
        output.printf("@SP%d", chirality_value);
        break;
    case CHIRALITY_TRIGONAL_BIPYRAMIDAL:
        if (chirality_value > CHIRALITY_TRIGONAL_BIPYRAMIDAL_MAX)
            throw Error("Wrong TB chirality value %d", chirality_value);
        output.printf("@TB%d", chirality_value);
        break;
    case CHIRALITY_OCTAHEDRAL:
        if (chirality_value > CHIRALITY_OCTAHEDRAL_MAX)
            throw Error("Wrong OH chirality value %d", chirality_value);
        output.printf("@OH%d", chirality_value);
        break;
    default:
        throw Error("Wrong chirality type value %d.", chirality_type);
    }
    if ((atom->value_max & CHIRALITY_OR_UNSPECIFIED) == CHIRALITY_OR_UNSPECIFIED)
        output.writeChar('?');
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
        if (bond->children.size() == 2)
        {
            if (((bond->child(0)->value == BOND_SINGLE && bond->child(0)->direction == BOND_ZERO) ||
                 (bond->child(1)->value == BOND_SINGLE && bond->child(1)->direction == BOND_ZERO)) &&
                (bond->child(0)->value == BOND_AROMATIC || bond->child(1)->value == BOND_AROMATIC))
                return; // empty bond means single or aromatic in smarts
        }
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
            else if (bond->direction == BOND_UP_OR_UNSPECIFIED)
                output.writeString("/?");
            else if (bond->direction == BOND_DOWN_OR_UNSPECIFIED)
                output.writeString("\\?");
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
        else
            output.writeString("!@");
        break;
    }
    case BOND_ANY: {
        output.writeChar('~');
        break;
    }
    default:
        throw Error("Unexpected bond type: %d", bond->type);
    }
}

std::string QueryMolecule::getSmartsAtomStr(QueryMolecule::Atom* atom, int original_format, bool is_substr)
{
    Array<char> out;
    ArrayOutput output(out);
    writeSmartsAtom(output, atom, -1, -1, is_substr ? 1 : 0, false, false, original_format);
    std::string result{out.ptr(), static_cast<std::size_t>(out.size())};
    return result;
}

std::string QueryMolecule::getMolMrvSmaExtension(QueryMolecule& qm, int aid)
{
    Array<char> out;
    ArrayOutput output(out);
    std::vector<std::unique_ptr<Atom>> atom_list;
    std::map<int, std::unique_ptr<Atom>> atom_props;
    bool negative = false;
    QueryMolecule::Atom& qa = qm.getAtom(aid);
    if (_isAtomOrListAndProps(&qa, atom_list, negative, atom_props))
    {
        // Just atom or list and list of properties.
        bool atoms_writed = false;
        for (int property : {ATOM_IMPLICIT_H, ATOM_CONNECTIVITY, ATOM_SSSR_RINGS, ATOM_SMALLEST_RING_SIZE, ATOM_AROMATICITY})
        {
            if (atom_props.count(property) < 1)
                continue;
            if (!atoms_writed)
            {
                // negative list Will be !a1;!a2...;!a3;props
                // positive list will be "a1,a2,..an;props"
                bool not_first_atom = false;
                for (auto& qatom : atom_list)
                {
                    if (not_first_atom)
                        if (negative)
                            output.writeChar(';');
                        else
                            output.writeChar(',');
                    else
                        not_first_atom = true;
                    if (negative)
                        output.writeChar('!');
                    if (qatom->type == ATOM_NUMBER)
                        output.printf("#%d", qatom->value_max);
                    else if (qatom->type == ATOM_PSEUDO)
                        output.writeString(qatom->alias.ptr());
                }
                if (atom_list.size())
                {
                    output.writeChar(';');
                    atoms_writed = true;
                }
            }
            writeSmartsAtom(output, atom_props[property].get(), -1, -1, 1, false, false, qm.original_format);
        }
    }
    else
    {
        if (qa.type != OP_NONE)
            //  Complex tree - just write nothing
            return "";
    }
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

/*/
static void writeAnd(Output& _output, QueryMolecule::Node* node, bool has_or_parent)
{
    if (has_or_parent)
        _output.writeChar('&');
    else if (node->hasOP_OR())
        _output.writeChar(';');
}
//*/

void QueryMolecule::writeSmartsAtom(Output& output, Atom* atom, int aam, int chirality, int depth, bool has_or_parent, bool has_not_parent, int original_format)
{
    int i;
    bool brackets_used = false;

    if (depth == 0) // "organic" subset can be used without [], but CNOPS need explicit aromatic to use letter
        if (!isOrganicSubset(atom) || isAromaticByCaseAtom(atom))
        {
            bool atom_by_case = false;
            bool aromaticity = false;
            if (atom->type == OP_AND && atom->children.size() == 2)
            {
                atom_by_case = isAromaticByCaseAtom(atom->child(0)) || isAromaticByCaseAtom(atom->child(1));
                aromaticity = (atom->child(0)->type == ATOM_AROMATICITY || atom->child(1)->type == ATOM_AROMATICITY);
            }
            if (!atom_by_case || !aromaticity)
            {
                output.writeChar('[');
                brackets_used = true;
            }
        }

    switch (atom->type)
    {
    case OP_NOT: {
        if (isNotAtom(*atom, ELEM_H))
        {
            output.printf("*");
            break;
        }
        output.writeChar('!');
        writeSmartsAtom(output, atom->child(0), aam, chirality, depth + 1, has_or_parent, true, original_format);
        break;
    }
    case OP_AND: {
        bool has_number = false;
        bool has_aromatic = false;
        bool aromatic = false;
        char atom_name[10];
        long long cur_pos = output.tell();
        for (i = 0; i < atom->children.size(); i++)
        {
            if (isAromaticByCaseAtom(atom->children[i]))
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
                atom_name[0] = static_cast<char>(tolower(atom_name[0]));
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

            if (output.tell() > cur_pos)
            {
                output.writeChar(has_or_parent ? '&' : ';');
                cur_pos = output.tell();
            }
            writeSmartsAtom(output, atom->child(i), aam, chirality, depth + 1, has_or_parent, has_not_parent, original_format);
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
            writeSmartsAtom(output, atom->child(i), aam, chirality, depth + 1, true, has_not_parent, original_format);
        }
        break;
    }
    case ATOM_ISOTOPE:
        output.printf("%d", atom->value_max);
        break;
    case ATOM_NUMBER: {
        if (isAromaticByCaseAtom(atom))
            output.printf("#%d", atom->value_max);
        else
            output.printf("%s", Element::toString(atom->value_max));
        switch (original_format)
        {
        case SMARTS:
        case KET:
            // SMARTS and ket save chirality in ATOM_CHIRALITY for query molecule
            break;
        default:
            if (chirality == CHIRALITY_ANTICLOCKWISE)
                output.printf("@");
            else if (chirality == CHIRALITY_CLOCKWISE)
                output.printf("@@");
            break;
        }

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
    case ATOM_STAR:
        output.writeString("*,H");
        break;
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
        output.printf("x0"); // exact value should be writed in extended part
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
        // output.writeString(atom->alias.ptr());
        output.writeChar('*');
        break;
    }
    case ATOM_TEMPLATE: {
        output.writeString(atom->alias.ptr());
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

    case ATOM_CHIRALITY: {
        _getAtomChiralityDescription(atom, output);
        break;
    }
    case ATOM_RSITE:
        output.printf("*:%d", atom->value_min);
        break;
    default: {
        throw Error("Unknown atom attribute %d", atom->type);
        break;
    }
    }

    if (brackets_used)
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
    case BOND_ANY:
        out.writeChar('~');
        return;
    default:
        out.printf("<constraint of type %d>", bond->type);
    }
}

bool QueryMolecule::possibleAromaticBond(int idx)
{
    // If bond can be aromatic or any by definition
    if (possibleBondOrder(idx, BOND_AROMATIC) || possibleBondOrder(idx, BOND_ANY))
    {
        Edge ed = getEdge(idx);
        // and atoms at both ends can be aromatic
        if (getAtomAromaticity(ed.beg) == ATOM_AROMATIC && getAtomAromaticity(ed.end) == ATOM_AROMATIC)
        {
            // Then bond can be aromatic
            return true;
        }
    }
    return false;
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

int QueryMolecule::addTemplateAtom(const char* alias)
{
    std::unique_ptr<Atom> atom(new Atom(ATOM_TEMPLATE, alias));
    int template_occur_idx = _template_occurrences.add();
    atom->occurrence_idx = template_occur_idx;
    _TemplateOccurrence& occur = _template_occurrences.at(template_occur_idx);
    occur.name_idx = _template_names.add(alias);
    occur.seq_id = -1;
    occur.template_idx = -1;
    occur.contracted = DisplayOption::Undefined;
    return addAtom(atom.release());
}

bool QueryMolecule::isTemplateAtom(int idx) const
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

int QueryMolecule::getTemplateAtomOccurrence(int idx) const
{
    // see the comment above in isTemplateAtom()
    if (!isTemplateAtom(idx))
        throw Error("getTemplateAtomOccurrence() applied to something that is not a template atom");

    return _atoms[idx]->occurrence_idx;
}

bool QueryMolecule::isSaturatedAtom(int /*idx*/)
{
    throw Error("not implemented");
}


int QueryMolecule::calcAtomMaxH(int idx, int conn)
{
    int number = getAtomNumber(idx);

    if (conn == -1)
        return -1;

    int explicit_val = getExplicitValence(idx);

    if (number == -1 && explicit_val < 0)
        return -1;

    int max_h = 0;

    int charge, radical;

    for (charge = -5; charge <= 8; charge++)
    {
        if (!possibleAtomCharge(idx, charge))
            continue;

        for (radical = 0; radical <= RADICAL_TRIPLET; radical++)
        {
            if (!possibleAtomRadical(idx, radical))
                continue;

            if (explicit_val != -1)
            {
                int h = number < 0 ? explicit_val - conn : explicit_val - Element::calcValenceMinusHyd(number, charge, radical, conn);
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
    return max_h;
}

int QueryMolecule::getAtomMaxH(int idx)
{
    int total;

    if (_atoms[idx]->sureValue(ATOM_TOTAL_H, total))
        return total;

    int max_h = calcAtomMaxH(idx, _calcAtomConnectivity(idx));
    if (max_h < 0)
        return -1;

    max_h += getAtomConnectedH(idx);

    return max_h;
}

int QueryMolecule::getAtomConnectedH(int idx)
{
    const Vertex& vertex = getVertex(idx);
    int connected_h = 0;

    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (possibleAtomNumber(vertex.neiVertex(i), ELEM_H))
            connected_h++;
    }
    return connected_h;
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
    int i = 0, conn = 0;
    bool was_aromatic = false;
    int atom_aromaticy = -1;
    // Smarts treat default bond as SINGLE_OR_AROMATIC so for this bonds look to atom aromaticy
    if (original_format == SMARTS)
    {
        std::ignore = getAtom(idx).sureValue(ATOM_AROMATICITY, atom_aromaticy);
    }

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int order = getBondOrder(vertex.neiEdge(i));
        if (order < 0)
            order = getQueryBondType(getBond(vertex.neiEdge(i)));

        if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE ||
            (original_format == SMARTS && order == _BOND_SINGLE_OR_AROMATIC && atom_aromaticy == ATOM_ALIPHATIC))
            conn += order;
        else if (order == BOND_AROMATIC || (original_format == SMARTS && order == _BOND_SINGLE_OR_AROMATIC && atom_aromaticy == ATOM_AROMATIC))
        {
            conn += 1;
            if (was_aromatic)
            {
                conn += 1;
                was_aromatic = false; // +1 connection for 2 aromatic bonds
            }
            else
            {
                was_aromatic = true;
            }
        }
        else
            return -1;
    }
    if (was_aromatic) // +1 connection for odd aromatic bond
        conn += 1;
    // Add atachment points
    Array<int> ap_indices;
    ap_indices.clear();
    getAttachmentIndicesForAtom(idx, ap_indices);
    conn += ap_indices.size();

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
    static const std::unordered_map<std::string, int> atom_types = {{"*", _ATOM_STAR}, {"R", _ATOM_R},   {"A", _ATOM_A},   {"X", _ATOM_X},
                                                                    {"Q", _ATOM_Q},    {"M", _ATOM_M},   {"AH", _ATOM_AH}, {"XH", _ATOM_XH},
                                                                    {"QH", _ATOM_QH},  {"XH", _ATOM_XH}, {"QH", _ATOM_QH}, {"MH", _ATOM_MH}};
    auto it = atom_types.find(label);
    if (it != atom_types.end())
        return it->second;
    return _ATOM_PSEUDO;
}

void QueryMolecule::getQueryAtomLabel(int qa, Array<char>& result)
{
    static const std::unordered_map<int, std::string> query_atom_labels = {{QUERY_ATOM_STAR, "*"}, {QUERY_ATOM_A, "A"},   {QUERY_ATOM_Q, "Q"},
                                                                           {QUERY_ATOM_X, "X"},    {QUERY_ATOM_AH, "AH"}, {QUERY_ATOM_XH, "XH"},
                                                                           {QUERY_ATOM_QH, "QH"},  {QUERY_ATOM_MH, "MH"}, {QUERY_ATOM_M, "M"}};

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

int QueryMolecule::addAtom(int label)
{
    return addAtom(new Atom(ATOM_NUMBER, label));
}

int QueryMolecule::addBond(int beg, int end, int order)
{
    return addBond(beg, end, QueryMolecule::createQueryMoleculeBond(order, BOND_ZERO, BOND_ZERO));
}

int QueryMolecule::getImplicitH(int idx, bool /*impl_h_no_throw*/)
{
    std::vector<std::unique_ptr<Atom>> atoms;
    std::map<int, std::unique_ptr<Atom>> properties;

    int query_atom_type = QueryMolecule::parseQueryAtomSmarts(*this, idx, atoms, properties);

    if (query_atom_type == QUERY_ATOM_UNKNOWN)
        return 0; // Complex query cannot calculate implicit H

    if (properties.count(ATOM_IMPLICIT_H) > 0)
        return properties[ATOM_IMPLICIT_H]->value_min; // Implicit H set in properties

    // If implicit h is not set - calculate it
    int max_h = 0;
    int conn = _calcAtomConnectivity(idx);
    if (conn < 0) // can't calculate - no implicit H
        return 0;
    if (properties.count(ATOM_TOTAL_H) > 0)
        max_h = properties[ATOM_TOTAL_H]->value_min - getAtomConnectedH(idx);
    else
        max_h = calcAtomMaxH(idx, conn); // count of H that can be added ( valence - conn )

    if (properties.count(ATOM_TOTAL_BOND_ORDER) > 0)
    {
        int max_conn = max_h + conn;
        int valence = properties[ATOM_TOTAL_BOND_ORDER]->value_min;
        if (max_conn > valence)
            max_h -= max_conn - valence;
    }

    if (max_h < 0)
        max_h = 0;

    return max_h;
}

void QueryMolecule::setImplicitH(int idx, int impl_h)
{
    getAtom(idx).updateConstraintWithValue(ATOM_IMPLICIT_H, impl_h);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
