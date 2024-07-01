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

#include "molecule/cml_loader.h"

#include <tinyxml2.h>

#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_scaffold_detection.h"

using namespace indigo;
using namespace tinyxml2;

static float readFloat(const char* point_str)
{
    float res = 0;
    if (point_str != 0)
    {
        BufferScanner strscan(point_str);
        res = strscan.readFloat();
    }
    return res;
}

IMPL_ERROR(CmlLoader, "CML loader");

CmlLoader::CmlLoader(Scanner& scanner)
{
    _scanner = &scanner;
    _handle = 0;
}

CmlLoader::CmlLoader(XMLHandle& handle)
{
    _handle = &handle;
    _scanner = 0;
}

void CmlLoader::loadMolecule(Molecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = &mol;
    _qmol = 0;
    _loadMolecule();

    mol.setIgnoreBadValenceFlag(ignore_bad_valence);
}

void CmlLoader::loadQueryMolecule(QueryMolecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = 0;
    _qmol = &mol;
    _loadMolecule();
}

void CmlLoader::_loadMolecule()
{
    if (_scanner != 0)
    {
        QS_DEF(Array<char>, buf);
        _scanner->readAll(buf);
        buf.push(0);
        XMLDocument xml;

        xml.Parse(buf.ptr());

        if (xml.Error())
            throw Error("XML parsing error: %s", xml.ErrorStr());

        XMLHandle hxml(&xml);
        XMLNode* node;

        if (_findMolecule(hxml.ToNode()))
        {
            node = _molecule;
            XMLHandle molecule(_molecule);
            _loadMoleculeElement(molecule);

            for (node = node->NextSibling(); node != 0; node = node->NextSibling())
            {
                if (strncmp(node->Value(), "Rgroup", 6) == 0)
                {
                    XMLHandle rgroup(node);
                    _loadRgroupElement(rgroup);
                }
            }
        }
    }
    else
        _loadMoleculeElement(*_handle);
}

bool CmlLoader::_findMolecule(XMLNode* elem)
{
    XMLNode* node;
    for (node = elem->FirstChild(); node != 0; node = node->NextSibling())
    {
        if (strncmp(node->Value(), "molecule", 8) == 0)
        {
            _molecule = node;
            return true;
        }
        if (_findMolecule(node))
            return true;
    }
    return false;
}

// Atoms
struct Atom
{
    std::string id, element_type, label, alias, isotope, formal_charge, spin_multiplicity, radical, valence, rgroupref, attpoint, attorder, query_props,
        hydrogen_count, atom_mapping, atom_inversion, atom_exact_change, x, y, z;
};

// This methods splits a space-separated string and writes each values into an arbitrary string
// property of Atom structure for each atom in the specified list
static void splitStringIntoProperties(const char* s, std::vector<Atom>& atoms, std::string Atom::*property)
{
    if (s == 0)
        return;

    std::stringstream stream(s);

    size_t i = 0;
    std::string token;
    while (stream >> token)
    {
        if (atoms.size() <= i)
            atoms.resize(i + 1);
        atoms[i].*property = token;
        i++;
    }
}

void CmlLoader::_loadMoleculeElement(XMLHandle& handle)
{
    std::unordered_map<std::string, int> atoms_id;
    std::unordered_map<std::string, size_t> atoms_id_int;

    // Function that mappes atom id as a string to an atom index
    auto getAtomIdx = [&](const char* id) {
        auto it = atoms_id.find(id);
        if (it == atoms_id.end())
            throw Error("atom id %s cannot be found", id);
        return it->second;
    };

    QS_DEF(Array<int>, total_h_count);
    QS_DEF(Array<int>, query_h_count);
    atoms_id.clear();
    atoms_id_int.clear();
    total_h_count.clear();
    query_h_count.clear();

    const char* title = handle.ToElement()->Attribute("title");

    if (title != 0)
        _bmol->name.readString(title, true);

    QS_DEF(std::vector<Atom>, atoms);
    atoms.clear();

    //
    // Read elements into an atoms array first and the parse them
    //

    XMLHandle atom_array = handle.FirstChildElement("atomArray");
    // Read atoms as xml attributes
    // <atomArray
    //       atomID="a1 a2 a3 ... "
    //       elementType="O C O ..."
    //       hydrogenCount="1 0 0 ..."
    // />
    splitStringIntoProperties(atom_array.ToElement()->Attribute("atomID"), atoms, &Atom::id);

    // Fill id if any were found
    size_t atom_index;
    for (atom_index = 0; atom_index < atoms.size(); atom_index++)
        atoms_id_int.emplace(atoms[atom_index].id, atom_index);

    splitStringIntoProperties(atom_array.ToElement()->Attribute("elementType"), atoms, &Atom::element_type);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("mrvPseudo"), atoms, &Atom::label);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("hydrogenCount"), atoms, &Atom::hydrogen_count);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("isotope"), atoms, &Atom::isotope);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("isotopeNumber"), atoms, &Atom::isotope);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("formalCharge"), atoms, &Atom::formal_charge);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("spinMultiplicity"), atoms, &Atom::spin_multiplicity);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("radical"), atoms, &Atom::radical);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("mrvValence"), atoms, &Atom::valence);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("rgroupRef"), atoms, &Atom::rgroupref);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("attachmentPoint"), atoms, &Atom::attpoint);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("attachmentOrder"), atoms, &Atom::attorder);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("mrvQueryProps"), atoms, &Atom::query_props);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("mrvAlias"), atoms, &Atom::alias);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("mrvMap"), atoms, &Atom::atom_mapping);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("reactionStereo"), atoms, &Atom::atom_inversion);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("exactChage"), atoms, &Atom::atom_exact_change);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("x2"), atoms, &Atom::x);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("y2"), atoms, &Atom::y);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("x3"), atoms, &Atom::x);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("y3"), atoms, &Atom::y);
    splitStringIntoProperties(atom_array.ToElement()->Attribute("z3"), atoms, &Atom::z);

    // Read atoms as nested xml elements
    //   <atomArray>
    //     <atom id="a1" elementType="H" />
    for (XMLElement* elem = atom_array.FirstChild().ToElement(); elem; elem = elem->NextSiblingElement())
    {
        if (strncmp(elem->Value(), "atom", 4) != 0)
            continue;

        const char* id = elem->Attribute("id");
        if (id == 0)
            throw Error("atom without an id");

        // Check if this atom has already been parsed
        auto it = atoms_id_int.find(id);
        if (it != end(atoms_id_int))
            atom_index = it->second;
        else
        {
            atom_index = atoms.size();
            atoms.emplace_back();
            atoms_id_int.emplace(id, atom_index);
        }

        Atom& a = atoms[atom_index];

        a.id = id;

        const char* element_type = elem->Attribute("elementType");

        if (element_type == 0)
            throw Error("atom without an elementType");
        a.element_type = element_type;

        const char* pseudo = elem->Attribute("mrvPseudo");

        if (pseudo != 0)
            a.label = pseudo;

        const char* alias = elem->Attribute("mrvAlias");

        if (alias != 0)
            a.alias = alias;

        const char* isotope = elem->Attribute("isotope");

        if (isotope == 0)
            isotope = elem->Attribute("isotopeNumber");

        if (isotope != 0)
            a.isotope = isotope;

        const char* charge = elem->Attribute("formalCharge");

        if (charge != 0)
            a.formal_charge = charge;

        const char* spinmultiplicity = elem->Attribute("spinMultiplicity");

        if (spinmultiplicity != 0)
            a.spin_multiplicity = spinmultiplicity;

        const char* radical = elem->Attribute("radical");

        if (radical != 0)
            a.radical = radical;

        const char* valence = elem->Attribute("mrvValence");

        if (valence != 0)
            a.valence = valence;

        const char* hcount = elem->Attribute("hydrogenCount");

        if (hcount != 0)
            a.hydrogen_count = hcount;

        const char* atom_map = elem->Attribute("mrvMap");

        if (atom_map != 0)
            a.atom_mapping = atom_map;

        const char* atom_inv = elem->Attribute("reactionStereo");

        if (atom_inv != 0)
            a.atom_inversion = atom_inv;

        const char* atom_exact = elem->Attribute("exactChange");

        if (atom_exact != 0)
            a.atom_exact_change = atom_exact;

        /*
         * Read coordinates
         */
        const char* x2 = elem->Attribute("x2");
        const char* y2 = elem->Attribute("y2");

        if (x2 != 0 && y2 != 0)
        {
            a.x = x2;
            a.y = y2;
        }
        else
        {
            const char* x3 = elem->Attribute("x3");
            const char* y3 = elem->Attribute("y3");
            const char* z3 = elem->Attribute("z3");

            if (x3 != 0 && y3 != 0 && z3 != 0)
            {
                a.x = x3;
                a.y = y3;
                a.z = z3;
            }
        }

        const char* rgroupref = elem->Attribute("rgroupRef");

        if (rgroupref != 0)
            a.rgroupref = rgroupref;

        const char* attpoint = elem->Attribute("attachmentPoint");

        if (attpoint != 0)
            a.attpoint = attpoint;

        const char* attorder = elem->Attribute("attachmentOrder");

        if (attorder != 0)
            a.attorder = attorder;

        const char* query_props = elem->Attribute("mrvQueryProps");

        if (query_props != 0)
        {
            if (_qmol != 0)
                a.query_props = query_props;
            else
                throw Error("'query features' are allowed only for queries");
        }
    }

    // Parse them
    for (auto& a : atoms)
    {
        int label = Element::fromString2(a.element_type.c_str());

        if ((label == -1) && (strncmp(a.element_type.c_str(), "R", 1) == 0))
            label = ELEM_RSITE;
        else if ((label == -1) && (strncmp(a.element_type.c_str(), "*", 1) == 0))
            label = ELEM_ATTPOINT;
        else if (!a.label.empty())
        {
            if (label == -1)
                label = ELEM_PSEUDO;
            else if (label == ELEM_C)
            {
                if ((strncmp(a.label.c_str(), "AH", 2) == 0) || (strncmp(a.label.c_str(), "QH", 2) == 0) || (strncmp(a.label.c_str(), "XH", 2) == 0) ||
                    (strncmp(a.label.c_str(), "MH", 2) == 0) || (strncmp(a.label.c_str(), "X", 1) == 0) || (strncmp(a.label.c_str(), "M", 1) == 0))
                {
                }
                else
                    label = ELEM_PSEUDO;
            }
        }
        else if (label == -1)
            label = ELEM_PSEUDO;

        int idx;
        if (label == ELEM_ATTPOINT)
        {
            atoms_id.emplace(a.id, -1);
        }
        else
        {
            if (_mol != 0)
            {
                idx = _mol->addAtom(label);

                if (label == ELEM_PSEUDO)
                {
                    if (!a.label.empty())
                        _mol->setPseudoAtom(idx, a.label.c_str());
                    else
                        _mol->setPseudoAtom(idx, a.element_type.c_str());
                }

                total_h_count.expandFill(idx + 1, -1);
                query_h_count.expandFill(idx + 1, -1);

                atoms_id.emplace(a.id, idx);

                if (!a.isotope.empty())
                {
                    int val;
                    if (sscanf(a.isotope.c_str(), "%d", &val) != 1)
                        throw Error("error parsing isotope");
                    _mol->setAtomIsotope(idx, val);
                }

                if (!a.formal_charge.empty())
                {
                    int val;
                    if (sscanf(a.formal_charge.c_str(), "%d", &val) != 1)
                        throw Error("error parsing charge");
                    _mol->setAtomCharge(idx, val);
                }

                if (!a.spin_multiplicity.empty())
                {
                    int val;
                    if (sscanf(a.spin_multiplicity.c_str(), "%d", &val) != 1)
                        throw Error("error parsing spin multiplicity");
                    _mol->setAtomRadical(idx, val);
                }

                if (!a.radical.empty())
                {
                    int val = 0;
                    if (strncmp(a.radical.c_str(), "divalent1", 9) == 0)
                        val = 1;
                    else if (strncmp(a.radical.c_str(), "monovalent", 10) == 0)
                        val = 2;
                    else if ((strncmp(a.radical.c_str(), "divalent3", 9) == 0) || (strncmp(a.radical.c_str(), "divalent", 8) == 0) ||
                             (strncmp(a.radical.c_str(), "triplet", 7) == 0))
                        val = 3;
                    _mol->setAtomRadical(idx, val);
                }

                if (a.spin_multiplicity.empty() && a.radical.empty())
                {
                    _mol->setAtomRadical(idx, 0);
                }

                if (!a.valence.empty())
                {
                    int val;
                    if (sscanf(a.valence.c_str(), "%d", &val) == 1)
                        _mol->setExplicitValence(idx, val);
                }

                if (!a.hydrogen_count.empty())
                {
                    int val;
                    if (sscanf(a.hydrogen_count.c_str(), "%d", &val) != 1)
                        throw Error("error parsing hydrogen count");
                    if (val < 0)
                        throw Error("negative hydrogen count");
                    total_h_count[idx] = val;
                }
            }
            else
            {
                std::unique_ptr<QueryMolecule::Atom> atom;
                int qhcount = -1;

                if (label == ELEM_PSEUDO)
                {
                    if (!a.label.empty())
                        atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PSEUDO, a.label.c_str());
                    else
                        atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PSEUDO, a.element_type.c_str());
                }
                else if (label == ELEM_RSITE)
                    atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RSITE, 0);
                else
                    atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, label);

                if (!a.query_props.empty() && (strncmp(a.query_props.c_str(), "L", 1) == 0)) // _ATOM_LIST
                {
                    std::unique_ptr<QueryMolecule::Atom> atomlist;

                    BufferScanner strscan(a.query_props.c_str());
                    QS_DEF(Array<char>, el);
                    QS_DEF(Array<char>, delim);
                    el.clear();
                    delim.clear();

                    strscan.skip(1);
                    delim.push(strscan.readChar());
                    delim.push(':');
                    delim.push(0);

                    while (!strscan.isEOF())
                    {
                        strscan.readWord(el, delim.ptr());
                        _appendQueryAtom(el.ptr(), atomlist);
                        if (strscan.readChar() == ':')
                            break;
                    }

                    if (delim[0] == '!')
                        atomlist.reset(QueryMolecule::Atom::nicht(atomlist.release()));

                    atom.reset(atomlist.release());
                }
                else if (!a.query_props.empty() && (strncmp(a.query_props.c_str(), "A", 1) == 0)) // _ATOM_A
                {
                    atom.get()->removeConstraints(QueryMolecule::ATOM_NUMBER);
                    atom.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
                }
                else if (!a.query_props.empty() && (strncmp(a.query_props.c_str(), "Q", 1) == 0)) // _ATOM_Q
                {
                    atom.get()->removeConstraints(QueryMolecule::ATOM_NUMBER);
                    atom.reset(QueryMolecule::Atom::und(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                                                        QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C))));
                }

                if (label == ELEM_C && !a.label.empty())
                {
                    if (strncmp(a.label.c_str(), "AH", 2) == 0)
                    {
                        std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();
                        x_atom->type = QueryMolecule::OP_NONE;
                        atom.get()->removeConstraints(QueryMolecule::ATOM_NUMBER);
                        atom.reset(x_atom.release());
                    }
                    else if (strncmp(a.label.c_str(), "QH", 2) == 0)
                    {
                        atom.get()->removeConstraints(QueryMolecule::ATOM_NUMBER);
                        atom.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                    }
                    else if (strncmp(a.label.c_str(), "XH", 2) == 0)
                    {
                        std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                        x_atom->type = QueryMolecule::OP_OR;
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H));

                        atom.get()->removeConstraints(QueryMolecule::ATOM_NUMBER);
                        atom.reset(x_atom.release());
                    }
                    else if (strncmp(a.label.c_str(), "X", 1) == 0)
                    {
                        std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                        x_atom->type = QueryMolecule::OP_OR;
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I));
                        x_atom->children.add(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At));

                        atom.get()->removeConstraints(QueryMolecule::ATOM_NUMBER);
                        atom.reset(x_atom.release());
                    }
                    else if (strncmp(a.label.c_str(), "MH", 2) == 0)
                    {
                        std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                        x_atom->type = QueryMolecule::OP_AND;
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rn)));

                        atom.get()->removeConstraints(QueryMolecule::ATOM_NUMBER);
                        atom.reset(x_atom.release());
                    }
                    else if (strncmp(a.label.c_str(), "M", 1) == 0)
                    {
                        std::unique_ptr<QueryMolecule::Atom> x_atom = std::make_unique<QueryMolecule::Atom>();

                        x_atom->type = QueryMolecule::OP_AND;
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_N)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_O)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_F)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_P)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_S)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Cl)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Se)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Br)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_I)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_At)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_He)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ne)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Ar)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Kr)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Xe)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_Rn)));
                        x_atom->children.add(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));

                        atom.get()->removeConstraints(QueryMolecule::ATOM_NUMBER);
                        atom.reset(x_atom.release());
                    }
                }
                else if (!a.query_props.empty()) // Query features
                {
                    BufferScanner strscan(a.query_props.c_str());
                    QS_DEF(Array<char>, qf);
                    QS_DEF(Array<char>, delim);
                    qf.clear();
                    delim.clear();

                    delim.push(';');
                    delim.push(0);

                    while (!strscan.isEOF())
                    {
                        strscan.readWord(qf, delim.ptr());
                        if (strncmp(qf.ptr(), "rb", 2) == 0)
                        {
                            BufferScanner qfscan(qf.ptr());
                            qfscan.skip(2);
                            int rbcount;
                            if (qfscan.lookNext() == '*')
                                rbcount = -2;
                            else
                                rbcount = qfscan.readInt1();

                            if (rbcount > 0)
                                atom.reset(QueryMolecule::Atom::und(
                                    atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, rbcount, (rbcount < 4 ? rbcount : 100))));
                            else if (rbcount == 0)
                                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, 0)));
                            else if (rbcount == -2)
                                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN, 0)));
                        }
                        else if (strncmp(qf.ptr(), "s", 1) == 0)
                        {
                            BufferScanner qfscan(qf.ptr());
                            qfscan.skip(1);
                            int subst;
                            if (qfscan.lookNext() == '*')
                            {
                                subst = -2;
                            }
                            else
                            {
                                subst = qfscan.readInt1();
                            }

                            if (subst == 0)
                            {
                                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, 0)));
                            }
                            else if (subst == -2)
                            {
                                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN, 0)));
                            }
                            else if (subst > 0)
                            {
                                atom.reset(QueryMolecule::Atom::und(
                                    atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, subst, (subst < 6 ? subst : 100))));
                            }
                        }
                        else if (strncmp(qf.ptr(), "u", 1) == 0)
                        {
                            BufferScanner qfscan(qf.ptr());
                            qfscan.skip(1);
                            bool unsat = (qfscan.readInt1() > 0);
                            if (unsat)
                                atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_UNSATURATION, 0)));
                        }
                        else if (strncmp(qf.ptr(), "H", 1) == 0)
                        {
                            BufferScanner qfscan(qf.ptr());
                            qfscan.skip(1);
                            qhcount = qfscan.readInt1();
                            /*
                                                 if (total_h > 0)
                                                    atom.reset(QueryMolecule::Atom::und(atom.release(),
                                                             new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_H, total_h)));
                            */
                        }

                        if (!strscan.isEOF())
                            strscan.skip(1);
                    }
                }

                if (!a.formal_charge.empty())
                {
                    int val;
                    if (sscanf(a.formal_charge.c_str(), "%d", &val) != 1)
                        throw Error("error parsing charge");
                    atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, val)));
                }

                if (!a.isotope.empty())
                {
                    int val;
                    if (sscanf(a.isotope.c_str(), "%d", &val) != 1)
                        throw Error("error parsing isotope");
                    atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_ISOTOPE, val)));
                }

                if (!a.spin_multiplicity.empty())
                {
                    int val;
                    if (sscanf(a.spin_multiplicity.c_str(), "%d", &val) != 1)
                        throw Error("error parsing spin multiplicity");
                    atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, val)));
                }

                if (!a.radical.empty())
                {
                    int val = 0;
                    if (strncmp(a.radical.c_str(), "divalent1", 9) == 0)
                        val = 1;
                    else if (strncmp(a.radical.c_str(), "monovalent", 10) == 0)
                        val = 2;
                    else if ((strncmp(a.radical.c_str(), "divalent3", 9) == 0) || (strncmp(a.radical.c_str(), "divalent", 8) == 0) ||
                             (strncmp(a.radical.c_str(), "triplet", 7) == 0))
                        val = 3;
                    atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, val)));
                }

                if (!a.valence.empty())
                {
                    int val;
                    if (sscanf(a.valence.c_str(), "%d", &val) == 1)
                        atom.reset(QueryMolecule::Atom::und(atom.release(), new QueryMolecule::Atom(QueryMolecule::ATOM_VALENCE, val)));
                }

                idx = _qmol->addAtom(atom.release());
                atoms_id.emplace(a.id, idx);
                total_h_count.expandFill(idx + 1, -1);
                query_h_count.expandFill(idx + 1, -1);
                query_h_count[idx] = qhcount;
            }

            if (!a.rgroupref.empty())
            {
                int val;
                if (sscanf(a.rgroupref.c_str(), "%d", &val) != 1)
                    throw Error("error parsing R-group reference");
                _bmol->allowRGroupOnRSite(idx, val);
            }

            if (!a.attpoint.empty())
            {
                int val;
                if (strncmp(a.attpoint.c_str(), "both", 4) == 0)
                    val = 3;
                else if (sscanf(a.attpoint.c_str(), "%d", &val) != 1)
                    throw Error("error parsing Attachment point");
                for (int att_idx = 0; (1 << att_idx) <= val; att_idx++)
                    if (val & (1 << att_idx))
                        _bmol->addAttachmentPoint(att_idx + 1, idx);
            }
            Vec3f a_pos;

            if (!a.x.empty())
            {
                a_pos.x = readFloat(a.x.c_str());
            }
            if (!a.y.empty())
            {
                a_pos.y = readFloat(a.y.c_str());
            }
            if (!a.z.empty())
            {
                a_pos.z = readFloat(a.z.c_str());
            }

            _bmol->setAtomXyz(idx, a_pos);

            if (!a.alias.empty())
            {
                if (strncmp(a.alias.c_str(), "0", 1) != 0)
                {
                    _bmol->setAlias(idx, a.alias.c_str());
                }
            }

            if (!a.atom_mapping.empty())
            {
                int val;
                if (sscanf(a.atom_mapping.c_str(), "%d", &val) != 1)
                    throw Error("error parsing atom-atom mapping");
                _bmol->reaction_atom_mapping[idx] = val;
            }

            if (!a.atom_inversion.empty())
            {
                if (strncmp(a.atom_inversion.c_str(), "Inv", 3) == 0)
                    _bmol->reaction_atom_inversion[idx] = 1;
                else if (strncmp(a.atom_inversion.c_str(), "Ret", 3) == 0)
                    _bmol->reaction_atom_inversion[idx] = 2;
            }

            if (!a.atom_exact_change.empty())
            {
                int val;
                if (sscanf(a.atom_exact_change.c_str(), "%d", &val) != 1)
                    throw Error("error parsing atom exact change flag");
                _bmol->reaction_atom_exact_change[idx] = val;
            }
        }
        /*
              if (!a.attorder.empty())
              {
                 int val;
                 if (sscanf(a.attorder.c_str(), "%d", &val) != 1)
                    throw Error("error parsing Attachment order");
                 mol.setRSiteAttachmentOrder(idx, nei_idx - 1, val - 1);
              }
        */
    }

    // Bonds
    bool have_cistrans_notation = false;

    for (XMLElement* elem = handle.FirstChildElement("bondArray").FirstChild().ToElement(); elem; elem = elem->NextSiblingElement())
    {
        if (strncmp(elem->Value(), "bond", 4) != 0)
            continue;
        const char* atom_refs = elem->Attribute("atomRefs2");
        if (atom_refs == 0)
            throw Error("bond without atomRefs2");

        int idx;

        BufferScanner strscan(atom_refs);
        QS_DEF(Array<char>, id);

        strscan.readWord(id, 0);
        int beg = getAtomIdx(id.ptr());
        strscan.skipSpace();
        strscan.readWord(id, 0);
        int end = getAtomIdx(id.ptr());

        if ((beg < 0) || (end < 0))
            continue;

        const char* order = elem->Attribute("order");
        if (order == 0)
            throw Error("bond without an order");

        int order_val;
        {
            if (order[0] == 'A' && order[1] == 0)
                order_val = BOND_AROMATIC;
            else if (order[0] == 'S' && order[1] == 0)
                order_val = BOND_SINGLE;
            else if (order[0] == 'D' && order[1] == 0)
                order_val = BOND_DOUBLE;
            else if (order[0] == 'T' && order[1] == 0)
                order_val = BOND_TRIPLE;
            else if (sscanf(order, "%d", &order_val) != 1)
                throw Error("error parsing order");
        }

        const char* query_type = elem->Attribute("queryType");
        const char* topology = elem->Attribute("topology");

        if (_mol != 0)
        {
            if ((query_type == 0) && (topology == 0))
                idx = _mol->addBond_Silent(beg, end, order_val);
            else
                throw Error("'query type' bonds are allowed only for queries");
        }
        else
        {
            std::unique_ptr<QueryMolecule::Bond> bond;

            if (query_type == 0)
            {
                if (order_val == BOND_SINGLE || order_val == BOND_DOUBLE || order_val == BOND_TRIPLE || order_val == BOND_AROMATIC)
                    bond.reset(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, order_val));
            }
            else if (strncmp(query_type, "SD", 2) == 0)
                bond.reset(QueryMolecule::Bond::und(QueryMolecule::Bond::nicht(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)),
                                                    QueryMolecule::Bond::oder(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE),
                                                                              new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_DOUBLE))));
            else if (strncmp(query_type, "SA", 2) == 0)
                bond.reset(QueryMolecule::Bond::oder(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE),
                                                     new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
            else if (strncmp(query_type, "DA", 2) == 0)
                bond.reset(QueryMolecule::Bond::oder(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_DOUBLE),
                                                     new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
            else if (strncmp(query_type, "Any", 3) == 0)
                bond = std::make_unique<QueryMolecule::Bond>();
            else
                throw Error("unknown bond type: %d", order);

            const char* bond_topology = elem->Attribute("topology");
            if (bond_topology != 0)
            {
                if (strncmp(bond_topology, "ring", 4) == 0)
                    bond.reset(QueryMolecule::Bond::und(bond.release(), new QueryMolecule::Bond(QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_RING)));
                else if (strncmp(bond_topology, "chain", 5) == 0)
                    bond.reset(QueryMolecule::Bond::und(bond.release(), new QueryMolecule::Bond(QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_CHAIN)));
                else
                    throw Error("unknown topology: %s", bond_topology);
            }

            idx = _qmol->addBond(beg, end, bond.release());
        }

        int dir = 0;

        XMLElement* bs_elem = elem->FirstChildElement("bondStereo");

        if (bs_elem != 0)
        {
            const char* text = bs_elem->GetText();
            if (text != 0)
            {
                if (text[0] == 'W' && text[1] == 0)
                    dir = BOND_UP;
                if (text[0] == 'H' && text[1] == 0)
                    dir = BOND_DOWN;
                if ((text[0] == 'C' || text[0] == 'T') && text[1] == 0)
                    have_cistrans_notation = true;
            }
            else
            {
                const char* convention = bs_elem->Attribute("convention");
                if (convention != 0)
                {
                    have_cistrans_notation = true;
                }
            }
        }

        if (dir != 0)
            _bmol->setBondDirection(idx, dir);

        const char* brcenter = elem->Attribute("mrvReactingCenter");

        if (brcenter != 0)
        {
            int val;
            if (sscanf(brcenter, "%d", &val) != 1)
                throw Error("error parsing reacting center flag");
            _bmol->reaction_bond_reacting_center[idx] = val;
        }
    }

    // Implicit H counts
    int i, j;

    for (i = _bmol->vertexBegin(); i != _bmol->vertexEnd(); i = _bmol->vertexNext(i))
    {
        int h = total_h_count[i];

        if (h < 0)
            continue;

        const Vertex& vertex = _bmol->getVertex(i);

        for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
            if (_bmol->getAtomNumber(vertex.neiVertex(j)) == ELEM_H)
                h--;

        if (h < 0)
            throw Error("hydrogenCount on atom %d is less than the number of explicit hydrogens");

        if (_mol != 0)
            _mol->setImplicitH(i, h);
    }

    // Query H counts
    if (_qmol != 0)
    {
        for (i = _bmol->vertexBegin(); i != _bmol->vertexEnd(); i = _bmol->vertexNext(i))
        {
            int expl_h = 0;

            if (query_h_count[i] >= 0)
            {
                // count explicit hydrogens
                const Vertex& vertex = _bmol->getVertex(i);

                for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                {
                    if (_bmol->getAtomNumber(vertex.neiVertex(j)) == ELEM_H)
                        expl_h++;
                }
            }

            if (query_h_count[i] == 0)
            {
                // no hydrogens unless explicitly drawn
                _qmol->resetAtom(i, QueryMolecule::Atom::und(_qmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_H, expl_h)));
            }
            else if (query_h_count[i] > 0)
            {
                // (_hcount[k] - 1) or more atoms in addition to explicitly drawn
                // no hydrogens unless explicitly drawn
                _qmol->resetAtom(
                    i, QueryMolecule::Atom::und(_qmol->releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_H, expl_h + query_h_count[i], 100)));
            }
        }
    }

    // Tetrahedral stereocenters
    for (XMLElement* elem = handle.FirstChildElement("atomArray").FirstChild().ToElement(); elem; elem = elem->NextSiblingElement())
    {
        const char* id = elem->Attribute("id");

        if (id == 0)
            throw Error("atom without an id");

        int idx = getAtomIdx(id);

        XMLElement* ap_elem = elem->FirstChildElement("atomParity");

        if (ap_elem == 0)
            continue;

        const char* ap_text = ap_elem->GetText();

        if (ap_text == 0)
            continue;

        int val;
        if (sscanf(ap_text, "%d", &val) != 1)
            throw Error("error parsing atomParity");

        const char* refs4 = ap_elem->Attribute("atomRefs4");

        if (refs4 != 0)
        {
            BufferScanner strscan(refs4);
            QS_DEF(Array<char>, a_id);
            int pyramid[4];

            for (int k = 0; k < 4; k++)
            {
                strscan.skipSpace();
                strscan.readWord(a_id, 0);
                pyramid[k] = getAtomIdx(a_id.ptr());
                if (pyramid[k] == idx)
                    pyramid[k] = -1;
            }

            if (val < 0)
                std::swap(pyramid[0], pyramid[1]);

            MoleculeStereocenters::moveMinimalToEnd(pyramid);

            _bmol->addStereocenters(idx, MoleculeStereocenters::ATOM_ABS, 0, pyramid);
        }
    }

    if (_bmol->stereocenters.size() == 0 && BaseMolecule::hasCoord(*_bmol))
    {
        std::vector<int> sensible_bond_orientations;
        sensible_bond_orientations.resize(_bmol->edgeEnd(), 0);
        _bmol->buildFromBondsStereocenters(stereochemistry_options, sensible_bond_orientations.data());
        if (!stereochemistry_options.ignore_errors)
            for (i = 0; i < _bmol->vertexCount(); i++)
                if (_bmol->getBondDirection(i) > 0 && !sensible_bond_orientations[i])
                    throw Error("direction of bond #%d makes no sense", i);
    }

    // Cis-trans stuff
    if (have_cistrans_notation)
    {
        int bond_idx = -1;

        for (XMLElement* elem = handle.FirstChildElement("bondArray").FirstChild().ToElement(); elem; elem = elem->NextSiblingElement())
        {
            if (strncmp(elem->Value(), "bond", 4) != 0)
                continue;
            bond_idx++;
            XMLElement* bs_elem = elem->FirstChildElement("bondStereo");

            if (bs_elem == 0)
                continue;

            const char* convention = bs_elem->Attribute("convention");
            if (convention != 0)
            {
                const char* convention_value = bs_elem->Attribute("conventionValue");
                if (convention_value != 0)
                {
                    if (strncmp(convention, "MDL", 3) == 0)
                    {
                        int val;
                        if (sscanf(convention_value, "%d", &val) != 1)
                            throw Error("error conventionValue attribute");
                        if (val == 3)
                        {
                            _bmol->cis_trans.ignore(bond_idx);
                        }
                    }
                }
            }

            const char* text = bs_elem->GetText();

            if (text == 0)
                continue;

            int parity;

            if (text[0] == 'C' && text[1] == 0)
                parity = MoleculeCisTrans::CIS;
            else if (text[0] == 'T' && text[1] == 0)
                parity = MoleculeCisTrans::TRANS;
            else
                continue;

            const char* atom_refs = bs_elem->Attribute("atomRefs4");
            // If there are only one substituent then atomRefs4 cano be omitted
            bool has_subst = atom_refs != nullptr;

            int substituents[4];

            if (!MoleculeCisTrans::isGeomStereoBond(*_bmol, bond_idx, substituents, false))
                throw Error("cis-trans notation on a non cis-trans bond #%d", bond_idx);

            if (!MoleculeCisTrans::sortSubstituents(*_bmol, substituents, 0))
                throw Error("cis-trans notation on a non cis-trans bond #%d", bond_idx);

            if (has_subst)
            {
                BufferScanner strscan(atom_refs);
                QS_DEF(Array<char>, id);
                int refs[4] = {-1, -1, -1, -1};

                for (int k = 0; k < 4; k++)
                {
                    strscan.skipSpace();
                    strscan.readWord(id, 0);

                    refs[k] = getAtomIdx(id.ptr());
                }

                const Edge& edge = _bmol->getEdge(bond_idx);

                if (refs[1] == edge.beg && refs[2] == edge.end)
                    ;
                else if (refs[1] == edge.end && refs[2] == edge.beg)
                {
                    std::swap(refs[0], refs[3]);
                    std::swap(refs[1], refs[2]);
                }
                else
                    throw Error("atomRefs4 in bondStereo do not match the bond ends");

                bool invert = false;

                if (refs[0] == substituents[0])
                    ;
                else if (refs[0] == substituents[1])
                    invert = !invert;
                else
                    throw Error("atomRefs4 in bondStereo do not match the substituents");

                if (refs[3] == substituents[2])
                    ;
                else if (refs[3] == substituents[3])
                    invert = !invert;
                else
                    throw Error("atomRefs4 in bondStereo do not match the substituents");

                if (invert)
                    parity = 3 - parity;
            }

            _bmol->cis_trans.add(bond_idx, substituents, parity);
        }
    }
    else if (BaseMolecule::hasCoord(*_bmol))
        _bmol->buildCisTrans(0);

    // Sgroups

    for (XMLElement* elem = handle.FirstChild().ToElement(); elem; elem = elem->NextSiblingElement())
    {
        if (strncmp(elem->Value(), "molecule", 8) != 0)
            continue;
        _loadSGroupElement(elem, atoms_id, 0);
    }
}

void CmlLoader::_loadSGroupElement(XMLElement* elem, std::unordered_map<std::string, int>& atoms_id, int sg_parent)
{
    auto getAtomIdx = [&](const char* id) {
        auto it = atoms_id.find(id);
        if (it == atoms_id.end())
            throw Error("atom id %s cannot be found", id);
        return it->second;
    };

    MoleculeSGroups* sgroups = &_bmol->sgroups;

    DataSGroup* dsg = 0;
    SGroup* gen = 0;
    RepeatingUnit* sru = 0;
    MultipleGroup* mul = 0;
    Superatom* sup = 0;

    int idx = 0;
    if (elem != 0)
    {
        const char* role = elem->Attribute("role");
        if (role == 0)
            throw Error("Sgroup without type");

        if (strncmp(role, "DataSgroup", 10) == 0)
        {
            idx = sgroups->addSGroup(SGroup::SG_TYPE_DAT);
            dsg = (DataSGroup*)&sgroups->getSGroup(idx);
        }
        else if (strncmp(role, "GenericSgroup", 13) == 0)
        {
            idx = sgroups->addSGroup(SGroup::SG_TYPE_GEN);
            gen = (SGroup*)&sgroups->getSGroup(idx);
        }
        else if (strncmp(role, "SruSgroup", 9) == 0)
        {
            idx = sgroups->addSGroup(SGroup::SG_TYPE_SRU);
            sru = (RepeatingUnit*)&sgroups->getSGroup(idx);
        }
        else if (strncmp(role, "MultipleSgroup", 14) == 0)
        {
            idx = sgroups->addSGroup(SGroup::SG_TYPE_MUL);
            mul = (MultipleGroup*)&sgroups->getSGroup(idx);
        }
        else if (strncmp(role, "SuperatomSgroup", 15) == 0)
        {
            idx = sgroups->addSGroup(SGroup::SG_TYPE_SUP);
            sup = (Superatom*)&sgroups->getSGroup(idx);
        }

        if ((dsg == 0) && (gen == 0) && (sru == 0) && (mul == 0) && (sup == 0))
            return;

        if (dsg != 0)
        {
            if (sg_parent > 0)
                dsg->parent_group = sg_parent;

            const char* atom_refs = elem->Attribute("atomRefs");
            if (atom_refs != 0)
            {
                BufferScanner strscan(atom_refs);
                QS_DEF(Array<char>, id);

                while (!strscan.isEOF())
                {
                    strscan.readWord(id, 0);
                    dsg->atoms.push(getAtomIdx(id.ptr()));
                    strscan.skipSpace();
                }
            }

            XMLElement* brackets = elem->FirstChildElement("MBracket");

            if (brackets != 0)
            {
                const char* brk_type = brackets->Attribute("type");
                if (brk_type != 0)
                {
                    if (strncmp(brk_type, "SQUARE", 6) == 0)
                    {
                        dsg->brk_style = 0;
                    }
                    else if (strncmp(brk_type, "ROUND", 5) == 0)
                    {
                        dsg->brk_style = 1;
                    }
                }

                int point_idx = 0;
                Vec2f* pbrackets = nullptr;
                XMLElement* pPoint = nullptr;
                for (pPoint = brackets->FirstChildElement(); pPoint; pPoint = pPoint->NextSiblingElement())
                {
                    if (strncmp(pPoint->Value(), "MPoint", 6) != 0)
                        continue;

                    if (point_idx == 0)
                        pbrackets = dsg->brackets.push();

                    const char* point_x = pPoint->Attribute("x");
                    const char* point_y = pPoint->Attribute("y");

                    float x = readFloat(point_x);
                    float y = readFloat(point_y);

                    pbrackets[point_idx].x = x;
                    pbrackets[point_idx].y = y;
                    point_idx++;
                    if (point_idx == 2)
                        point_idx = 0;
                }
            }

            const char* fieldname = elem->Attribute("fieldName");
            if (fieldname != 0)
                dsg->name.readString(fieldname, true);

            const char* fielddata = elem->Attribute("fieldData");
            if (fieldname != 0)
                dsg->data.readString(fielddata, true);

            const char* fieldtype = elem->Attribute("fieldType");
            if (fieldtype != 0)
                dsg->description.readString(fieldtype, true);

            const char* disp_x = elem->Attribute("x");
            if (disp_x != 0)
            {
                BufferScanner strscan(disp_x);
                dsg->display_pos.x = strscan.readFloat();
            }

            const char* disp_y = elem->Attribute("y");
            if (disp_y != 0)
            {
                BufferScanner strscan(disp_y);
                dsg->display_pos.y = strscan.readFloat();
            }

            const char* detached = elem->Attribute("dataDetached");
            dsg->detached = true;
            if (detached != 0)
            {
                if ((strncmp(detached, "true", 4) == 0) || (strncmp(detached, "on", 2) == 0) || (strncmp(detached, "1", 1) == 0) ||
                    (strncmp(detached, "yes", 3) == 0))
                {
                    dsg->detached = true;
                }
                else if ((strncmp(detached, "false", 5) == 0) || (strncmp(detached, "off", 3) == 0) || (strncmp(detached, "0", 1) == 0) ||
                         (strncmp(detached, "no", 2) == 0))
                {
                    dsg->detached = false;
                }
            }

            const char* relative = elem->Attribute("placement");
            dsg->relative = false;
            if (relative != 0)
            {
                if (strncmp(relative, "Relative", 8) == 0)
                {
                    dsg->relative = true;
                }
            }

            const char* disp_units = elem->Attribute("unitsDisplayed");
            if (disp_units != 0)
            {
                if ((strncmp(disp_units, "Unit displayed", 14) == 0) || (strncmp(disp_units, "yes", 3) == 0) || (strncmp(disp_units, "on", 2) == 0) ||
                    (strncmp(disp_units, "1", 1) == 0) || (strncmp(disp_units, "true", 4) == 0))
                {
                    dsg->display_units = true;
                }
            }

            dsg->num_chars = 0;
            const char* disp_chars = elem->Attribute("displayedChars");
            if (disp_chars != 0)
            {
                BufferScanner strscan(disp_chars);
                dsg->num_chars = strscan.readInt1();
            }

            const char* disp_tag = elem->Attribute("tag");
            if (disp_tag != 0)
            {
                BufferScanner strscan(disp_tag);
                dsg->tag = strscan.readChar();
            }

            const char* query_op = elem->Attribute("queryOp");
            if (query_op != 0)
                dsg->queryoper.readString(query_op, true);

            const char* query_type = elem->Attribute("queryType");
            if (query_type != 0)
                dsg->querycode.readString(query_type, true);

            XMLNode* pChild;
            for (pChild = elem->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
            {
                if (strncmp(pChild->Value(), "molecule", 8) != 0)
                    continue;
                XMLHandle next_mol(pChild);
                if (next_mol.ToElement() != 0)
                    _loadSGroupElement(next_mol.ToElement(), atoms_id, idx + 1);
            }
        }
        else if (gen != 0)
        {
            if (sg_parent > 0)
                gen->parent_group = sg_parent;

            const char* atom_refs = elem->Attribute("atomRefs");
            if (atom_refs != 0)
            {
                BufferScanner strscan(atom_refs);
                QS_DEF(Array<char>, id);

                while (!strscan.isEOF())
                {
                    strscan.readWord(id, 0);
                    gen->atoms.push(getAtomIdx(id.ptr()));
                    strscan.skipSpace();
                }
            }

            XMLElement* brackets = elem->FirstChildElement("MBracket");

            if (brackets != 0)
            {
                const char* brk_type = brackets->Attribute("type");
                if (brk_type != 0)
                {
                    if (strncmp(brk_type, "SQUARE", 6) == 0)
                    {
                        gen->brk_style = 0;
                    }
                    else if (strncmp(brk_type, "ROUND", 5) == 0)
                    {
                        gen->brk_style = 1;
                    }
                }

                int point_idx = 0;
                Vec2f* pbrackets = nullptr;
                XMLElement* pPoint = nullptr;
                for (pPoint = brackets->FirstChildElement(); pPoint; pPoint = pPoint->NextSiblingElement())
                {
                    if (strncmp(pPoint->Value(), "MPoint", 6) != 0)
                        continue;

                    if (point_idx == 0)
                        pbrackets = gen->brackets.push();

                    float x = 0, y = 0;
                    const char* point_x = pPoint->Attribute("x");
                    if (point_x != 0)
                    {
                        BufferScanner strscan(point_x);
                        x = strscan.readFloat();
                    }
                    const char* point_y = pPoint->Attribute("y");
                    if (point_y != 0)
                    {
                        BufferScanner strscan(point_y);
                        y = strscan.readFloat();
                    }
                    pbrackets[point_idx].x = x;
                    pbrackets[point_idx].y = y;
                    point_idx++;
                    if (point_idx == 2)
                        point_idx = 0;
                }
            }

            XMLNode* pChild;
            for (pChild = elem->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
            {
                if (strncmp(pChild->Value(), "molecule", 8) != 0)
                    continue;
                XMLHandle next_mol(pChild);
                if (next_mol.ToElement() != 0)
                    _loadSGroupElement(next_mol.ToElement(), atoms_id, idx + 1);
            }
        }
        else if (sru != 0)
        {
            if (sg_parent > 0)
                sru->parent_group = sg_parent;

            const char* atom_refs = elem->Attribute("atomRefs");
            if (atom_refs != 0)
            {
                BufferScanner strscan(atom_refs);
                QS_DEF(Array<char>, id);

                while (!strscan.isEOF())
                {
                    strscan.readWord(id, 0);
                    sru->atoms.push(getAtomIdx(id.ptr()));
                    strscan.skipSpace();
                }
            }

            XMLElement* brackets = elem->FirstChildElement("MBracket");

            if (brackets != 0)
            {
                const char* brk_type = brackets->Attribute("type");
                if (brk_type != 0)
                {
                    if (strncmp(brk_type, "SQUARE", 6) == 0)
                    {
                        sru->brk_style = 0;
                    }
                    else if (strncmp(brk_type, "ROUND", 5) == 0)
                    {
                        sru->brk_style = 1;
                    }
                }

                int point_idx = 0;
                Vec2f* pbrackets = nullptr;
                XMLElement* pPoint = nullptr;
                for (pPoint = brackets->FirstChildElement(); pPoint; pPoint = pPoint->NextSiblingElement())
                {
                    if (strncmp(pPoint->Value(), "MPoint", 6) != 0)
                        continue;

                    if (point_idx == 0)
                        pbrackets = sru->brackets.push();

                    float x = 0, y = 0;
                    const char* point_x = pPoint->Attribute("x");
                    if (point_x != 0)
                    {
                        BufferScanner strscan(point_x);
                        x = strscan.readFloat();
                    }
                    const char* point_y = pPoint->Attribute("y");
                    if (point_y != 0)
                    {
                        BufferScanner strscan(point_y);
                        y = strscan.readFloat();
                    }
                    pbrackets[point_idx].x = x;
                    pbrackets[point_idx].y = y;
                    point_idx++;
                    if (point_idx == 2)
                        point_idx = 0;
                }
            }

            const char* title = elem->Attribute("title");
            if (title != 0)
                sru->subscript.readString(title, true);

            const char* connect = elem->Attribute("connect");
            if (connect != 0)
            {
                if (strncmp(connect, "ht", 2) == 0)
                {
                    sru->connectivity = SGroup::HEAD_TO_TAIL;
                }
                else if (strncmp(connect, "hh", 2) == 0)
                {
                    sru->connectivity = SGroup::HEAD_TO_HEAD;
                }
            }

            XMLNode* pChild;
            for (pChild = elem->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
            {
                if (strncmp(pChild->Value(), "molecule", 8) != 0)
                    continue;
                XMLHandle next_mol(pChild);
                if (next_mol.ToElement() != 0)
                    _loadSGroupElement(next_mol.ToElement(), atoms_id, idx + 1);
            }
        }
        else if (mul != 0)
        {
            if (sg_parent > 0)
                mul->parent_group = sg_parent;

            const char* atom_refs = elem->Attribute("atomRefs");
            if (atom_refs != 0)
            {
                BufferScanner strscan(atom_refs);
                QS_DEF(Array<char>, id);

                while (!strscan.isEOF())
                {
                    strscan.readWord(id, 0);
                    mul->atoms.push(getAtomIdx(id.ptr()));
                    strscan.skipSpace();
                }
            }

            const char* patoms = elem->Attribute("patoms");
            if (patoms != 0)
            {
                BufferScanner strscan(patoms);
                QS_DEF(Array<char>, id);

                while (!strscan.isEOF())
                {
                    strscan.readWord(id, 0);
                    mul->parent_atoms.push(getAtomIdx(id.ptr()));
                    strscan.skipSpace();
                }
            }

            XMLElement* brackets = elem->FirstChildElement("MBracket");

            if (brackets != 0)
            {
                const char* brk_type = brackets->Attribute("type");
                if (brk_type != 0)
                {
                    if (strncmp(brk_type, "SQUARE", 6) == 0)
                    {
                        mul->brk_style = 0;
                    }
                    else if (strncmp(brk_type, "ROUND", 5) == 0)
                    {
                        mul->brk_style = 1;
                    }
                }

                int point_idx = 0;
                Vec2f* pbrackets = nullptr;
                XMLElement* pPoint = nullptr;
                for (pPoint = brackets->FirstChildElement(); pPoint; pPoint = pPoint->NextSiblingElement())
                {
                    if (strncmp(pPoint->Value(), "MPoint", 6) != 0)
                        continue;

                    if (point_idx == 0)
                        pbrackets = mul->brackets.push();

                    float x = 0, y = 0;
                    const char* point_x = pPoint->Attribute("x");
                    if (point_x != 0)
                    {
                        BufferScanner strscan(point_x);
                        x = strscan.readFloat();
                    }
                    const char* point_y = pPoint->Attribute("y");
                    if (point_y != 0)
                    {
                        BufferScanner strscan(point_y);
                        y = strscan.readFloat();
                    }
                    pbrackets[point_idx].x = x;
                    pbrackets[point_idx].y = y;
                    point_idx++;
                    if (point_idx == 2)
                        point_idx = 0;
                }
            }

            const char* title = elem->Attribute("title");
            if (title != 0)
            {
                BufferScanner strscan(title);
                mul->multiplier = strscan.readInt1();
            }

            XMLNode* pChild;
            for (pChild = elem->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
            {
                if (strncmp(pChild->Value(), "molecule", 8) != 0)
                    continue;
                XMLHandle next_mol(pChild);
                if (next_mol.ToElement() != 0)
                    _loadSGroupElement(next_mol.ToElement(), atoms_id, idx + 1);
            }
        }
        else if (sup != 0)
        {
            if (sg_parent > 0)
                sup->parent_group = sg_parent;

            const char* atom_refs = elem->Attribute("atomRefs");
            if (atom_refs != 0)
            {
                BufferScanner strscan(atom_refs);
                QS_DEF(Array<char>, id);

                while (!strscan.isEOF())
                {
                    strscan.readWord(id, 0);
                    sup->atoms.push(getAtomIdx(id.ptr()));
                    strscan.skipSpace();
                }
            }

            XMLElement* brackets = elem->FirstChildElement("MBracket");

            if (brackets != 0)
            {
                const char* brk_type = brackets->Attribute("type");
                if (brk_type != 0)
                {
                    if (strncmp(brk_type, "SQUARE", 6) == 0)
                    {
                        sup->brk_style = 0;
                    }
                    else if (strncmp(brk_type, "ROUND", 5) == 0)
                    {
                        sup->brk_style = 1;
                    }
                }

                int point_idx = 0;
                Vec2f* pbrackets = nullptr;
                XMLElement* pPoint = nullptr;
                for (pPoint = brackets->FirstChildElement(); pPoint; pPoint = pPoint->NextSiblingElement())
                {
                    if (strncmp(pPoint->Value(), "MPoint", 6) != 0)
                        continue;

                    if (point_idx == 0)
                        pbrackets = sup->brackets.push();

                    const char* point_x = pPoint->Attribute("x");
                    const char* point_y = pPoint->Attribute("y");

                    float x = readFloat(point_x);
                    float y = readFloat(point_y);

                    pbrackets[point_idx].x = x;
                    pbrackets[point_idx].y = y;
                    point_idx++;
                    if (point_idx == 2)
                        point_idx = 0;
                }
            }
            /*
                     XMLElement *atpoints = elem->FirstChildElement("AttachmentPointArray");

                     if (atpoints != 0)
                     {
                        XMLElement * aPoint;
                        for (aPoint = atpoints->FirstChildElement();
                             aPoint;
                             aPoint = aPoint->NextSiblingElement())
                        {
                           if (strncmp(aPoint->Value(), "attachmentPoint", 15) != 0)
                              continue;

                           int idap = sup->attachment_points.add();
                           Superatom::_AttachmentPoint &ap = sup->attachment_points.at(idap);

                           const char *a_idx = aPoint->Attribute("atom");
                           if (a_idx != 0)
                           {
                              ap.aidx = getAtomIdx(a_idx);
                           }

                           const char *b_idx = aPoint->Attribute("bond");
                           if (b_idx != 0)
                           {
                              ap.aidx = getAtomIdx(a_idx);
                           }

                           const char *ap_order = aPoint->Attribute("order");
                           if (ap_order != 0)
                           {
                              ap.aidx = getAtomIdx(a_idx);
                           }

                        }
                     }
            */

            const char* title = elem->Attribute("title");
            if (title != 0)
                sup->subscript.readString(title, true);

            XMLNode* pChild;
            for (pChild = elem->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
            {
                if (strncmp(pChild->Value(), "molecule", 8) != 0)
                    continue;
                XMLHandle next_mol(pChild);
                if (next_mol.ToElement() != 0)
                    _loadSGroupElement(next_mol.ToElement(), atoms_id, idx + 1);
            }
        }
    }
}

void CmlLoader::_loadRgroupElement(XMLHandle& handle)
{
    MoleculeRGroups* rgroups = &_bmol->rgroups;

    XMLElement* elem = handle.ToElement();
    if (elem != 0)
    {
        int rg_idx;
        const char* rgroup_id = elem->Attribute("rgroupID");
        if (rgroup_id == 0)
            throw Error("Rgroup without ID");
        BufferScanner strscan(rgroup_id);
        rg_idx = strscan.readInt1();

        RGroup& rgroup = rgroups->getRGroup(rg_idx);

        const char* rlogic_range = elem->Attribute("rlogicRange");
        if (rlogic_range != 0)
            _parseRlogicRange(rlogic_range, rgroup.occurrence);

        const char* rgroup_then = elem->Attribute("thenR");
        if (rgroup_then != 0)
        {
            BufferScanner rstrscan(rgroup_then);
            rgroup.if_then = rstrscan.readInt1();
        }

        rgroup.rest_h = 0;
        const char* rgroup_resth = elem->Attribute("restH");
        if (rgroup_resth != 0)
        {
            if ((strncmp(rgroup_resth, "true", 4) == 0) || (strncmp(rgroup_resth, "on", 2) == 0) || (strncmp(rgroup_resth, "1", 1) == 0))
            {
                rgroup.rest_h = 1;
            }
        }

        XMLNode* pChild;
        for (pChild = handle.FirstChild().ToNode(); pChild != 0; pChild = pChild->NextSibling())
        {
            if (strncmp(pChild->Value(), "molecule", 8) != 0)
                continue;
            XMLHandle molecule(pChild);
            if (molecule.ToElement() != 0)
            {
                std::unique_ptr<BaseMolecule> fragment(_bmol->neu());

                Molecule* _mol_save;
                BaseMolecule* _bmol_save;
                QueryMolecule* _qmol_save;

                _mol_save = _mol;
                _bmol_save = _bmol;
                _qmol_save = _qmol;

                _bmol = fragment.get();

                if (_bmol->isQueryMolecule())
                {
                    _qmol = &_bmol->asQueryMolecule();
                    _mol = 0;
                }
                else
                {
                    _mol = &_bmol->asMolecule();
                    _qmol = 0;
                }
                _loadMoleculeElement(molecule);

                _mol = _mol_save;
                _bmol = _bmol_save;
                _qmol = _qmol_save;

                rgroup.fragments.add(fragment.release());
            }
        }
    }
}

void CmlLoader::_parseRlogicRange(const char* str, Array<int>& ranges)
{
    int beg = -1, end = -1;
    int add_beg = 0, add_end = 0;

    while (*str != 0)
    {
        if (*str == '>')
        {
            end = 0xFFFF;
            add_beg = 1;
        }
        else if (*str == '<')
        {
            beg = 0;
            add_end = -1;
        }
        else if (isdigit(*str))
        {
            sscanf(str, "%d", beg == -1 ? &beg : &end);
            while (isdigit(*str))
                str++;
            continue;
        }
        else if (*str == ',')
        {
            if (end == -1)
                end = beg;
            else
                beg += add_beg, end += add_end;
            ranges.push((beg << 16) | end);
            beg = end = -1;
            add_beg = add_end = 0;
        }
        str++;
    }

    if (beg == -1 && end == -1)
        return;

    if (end == -1)
        end = beg;
    else
        beg += add_beg, end += add_end;
    ranges.push((beg << 16) | end);
}

void CmlLoader::_appendQueryAtom(const char* atom_label, std::unique_ptr<QueryMolecule::Atom>& atom)
{
    int atom_number = Element::fromString2(atom_label);
    std::unique_ptr<QueryMolecule::Atom> cur_atom;
    if (atom_number != -1)
        cur_atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, atom_number);
    else
        cur_atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PSEUDO, atom_label);

    if (atom.get() == 0)
        atom.reset(cur_atom.release());
    else
        atom.reset(QueryMolecule::Atom::oder(atom.release(), cur_atom.release()));
}
