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

#include <unordered_map>

#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/molecule_scaffold_detection.h"
#include "tinyxml.h"
typedef unsigned short int UINT16;
typedef int INT32;
typedef unsigned int UINT32;
#include "molecule/CDXConstants.h"

using namespace indigo;

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

IMPL_ERROR(MoleculeCdxmlLoader, "CDXML loader");

MoleculeCdxmlLoader::MoleculeCdxmlLoader(Scanner& scanner)
{
    _scanner = &scanner;
}

void MoleculeCdxmlLoader::loadMolecule(BaseMolecule& mol)
{
    mol.clear();
    _pmol = NULL;
    _pqmol = NULL;
    if (mol.isQueryMolecule())
    {
        _pqmol = &mol.asQueryMolecule();
    }
    else
    {
        _pmol = &mol.asMolecule();
        _pmol->setIgnoreBadValenceFlag(ignore_bad_valence);
    }

    if (_scanner != 0)
    {
        QS_DEF(Array<char>, buf);
        _scanner->readAll(buf);
        buf.push(0);
        TiXmlDocument xml;
        xml.Parse(buf.ptr());

        if (xml.Error())
            throw Error("XML parsing error: %s", xml.ErrorDesc());

        TiXmlElement* fragment = _findFragment(xml.RootElement());
        if (!fragment)
            throw Error("CDXML has no fragment tag");

        if (fragment)
            _loadFragment(mol, fragment);
    }
}

TiXmlElement* MoleculeCdxmlLoader::_findFragment(TiXmlElement* elem)
{
    auto pElem = elem->FirstChildElement();
    for (pElem; pElem; pElem = pElem->NextSiblingElement())
    {
        printf("%s\n", pElem->Value());
        if (std::string(pElem->Value()).compare("fragment") == 0)
            return pElem;
        auto pFrag = _findFragment(pElem);
        if (pFrag)
            return pFrag;
    }
    return NULL;
}

void MoleculeCdxmlLoader::_addAtomsAndBonds(BaseMolecule& mol, const std::vector<CdxmlAtom>& atoms, const std::vector<CdxmlBond>& bonds)
{
    mol.reaction_atom_mapping.clear_resize(atoms.size());
    mol.reaction_atom_mapping.zerofill();
    mol.reaction_atom_inversion.clear_resize(atoms.size());
    mol.reaction_atom_inversion.zerofill();
    mol.reaction_atom_exact_change.clear_resize(atoms.size());
    mol.reaction_atom_exact_change.zerofill();

    std::unordered_map<int, int> id_to_idx;
    int atom_idx;
    for (const auto& atom : atoms)
    {
        if (_pmol)
        {
            atom_idx = _pmol->addAtom(atom.element);
            id_to_idx.emplace(atom.id, atom_idx);
            mol.setAtomXyz(atom_idx, atom.pos);
            _pmol->setAtomCharge_Silent(atom_idx, atom.charge);
            if ( atom.valence )
                _pmol->setExplicitValence(atom_idx, atom.valence);
            _pmol->setAtomRadical(atom_idx, atom.radical);
            _pmol->setAtomIsotope(atom_idx, atom.isotope);
            // _pmol->setPseudoAtom(atom_idx, label.c_str());
        }
        else
        {
            throw Error("Queries not supported");
        }
    }

    for (const auto& bond : bonds)
    {
        int bond_idx;
        if (_pmol)
        {
            if (bond.swap_bond)
                bond_idx = _pmol->addBond_Silent(id_to_idx.at(bond.be.second), id_to_idx.at(bond.be.first), bond.order);
            else
                bond_idx = _pmol->addBond_Silent(id_to_idx.at(bond.be.first), id_to_idx.at(bond.be.second), bond.order);
            if (bond.dir > 0)
                _pmol->setBondDirection(bond_idx, bond.dir);
        }
    }
}

void MoleculeCdxmlLoader::_loadFragment(BaseMolecule& mol, TiXmlElement* fragment)
{
    std::vector<CdxmlAtom> atoms;
    std::vector<CdxmlBond> bonds;

    auto pElem = fragment->FirstChildElement();
    for (pElem; pElem; pElem = pElem->NextSiblingElement())
    {
        const char* pKey = pElem->Value();
        switch (*pKey)
        {
        case 'n': // atom
        {
            CdxmlAtom atom;
            auto pAttr = pElem->FirstAttribute();
            _parseAtom(atom, pAttr);
            atoms.push_back(atom);
        }
        break;
        case 'b': // bond
        {
            CdxmlBond bond;
            auto pAttr = pElem->FirstAttribute();
            _parseBond(bond, pAttr);
            bonds.push_back(bond);
        }
        break;
        }
    }

    _addAtomsAndBonds(mol, atoms, bonds);

    printf("atoms: %d\n", atoms.size());
    printf("bonds: %d\n", bonds.size());
    for (auto& atom : atoms)
    {
        printf("id=%d, element=%d, x=%f y=%f\n ", (int)atom.id, (int)atom.element, atom.pos.x, atom.pos.y);
    }

    for (auto& bond : bonds)
    {
        printf("begin=%d, end=%d\n ", (int)bond.be.first, (int)bond.be.second);
    }
}

void MoleculeCdxmlLoader::_applyDispatcher(TiXmlAttribute* pAttr, const std::unordered_map<std::string, std::function<void(std::string&)>>& dispatcher)
{
    for (pAttr; pAttr; pAttr = pAttr->Next())
    {
        auto it = dispatcher.find(pAttr->Name());
        if (it != dispatcher.end())
        {
            it->second(std::string(pAttr->Value()));
        }
        else
            printf("Unknown attribute: %s\n", pAttr->Name());
    }
}

void MoleculeCdxmlLoader::_parseAtom(CdxmlAtom& atom, TiXmlAttribute* pAttr)
{
    // Atom parsing lambdas definition
    auto id_lambda = [&atom](std::string& data) { atom.id = data; };
    auto hydrogens_lambda = [&atom](std::string& data) { atom.hydrogens = data; };
    auto charge_lambda = [&atom](std::string& data) { atom.charge = data; };
    auto element_lambda = [&atom](std::string& data) { atom.element = data; };
    auto isotope_lambda = [&atom](std::string& data) { atom.isotope = data; };
    auto radical_lambda = [&atom](std::string& data) { atom.radical = data; };
    auto label_lambda = [&atom](std::string& data) { atom.label = data; };

    auto pos_lambda = [&atom](std::string& data) {
        std::vector<std::string> coords = split(data, ' ');
        if (coords.size() >= 2)
        {
            atom.pos.x = std::stof(coords[0]);
            atom.pos.y = std::stof(coords[1]);
            if (coords.size() == 3)
                atom.pos.z = std::stof(coords[2]);
            else
                atom.pos.z = 0;
        }
        else
            throw Error("Not enought coordinates for atom position");
    };
    auto stereo_lambda = [&atom](std::string& data) {
        static const std::unordered_map<std::string, int> cip_map = {{"U", 0}, {"N", 1}, {"R", 2}, {"S", 3}, {"r", 4}, {"s", 5}, {"u", 6}};
        atom.stereo = cip_map.at(data);
    };

    auto node_type_lambda = [&atom](std::string& data) {
        static const std::unordered_map<std::string, int> node_type_map = {{"Unspecified", kCDXNodeType_Unspecified},
                                                                           {"Element", kCDXNodeType_Element},
                                                                           {"ElementList", kCDXNodeType_ElementList},
                                                                           {"ElementListNickname", kCDXNodeType_ElementListNickname},
                                                                           {"Nickname", kCDXNodeType_Nickname},
                                                                           {"Fragment", kCDXNodeType_Fragment},
                                                                           {"Formula", kCDXNodeType_Formula},
                                                                           {"GenericNickname", kCDXNodeType_GenericNickname},
                                                                           {"AnonymousAlternativeGroup", kCDXNodeType_AnonymousAlternativeGroup},
                                                                           {"NamedAlternativeGroup", kCDXNodeType_NamedAlternativeGroup},
                                                                           {"MultiAttachment", kCDXNodeType_MultiAttachment},
                                                                           {"VariableAttachment", kCDXNodeType_VariableAttachment},
                                                                           {"ExternalConnectionPoint", kCDXNodeType_ExternalConnectionPoint},
                                                                           {"LinkNode", kCDXNodeType_LinkNode}};
        atom.stereo = node_type_map.at(data);
    };

    auto element_list_lambda = [&atom](std::string& data) {
        std::vector<std::string> elements = split(data, ' ');
        if (elements.size() && elements.front().compare("NOT") == 0)
        {
            elements.erase(elements.begin());
            atom.is_not_list = true;
        }
        atom.element_list.assign(elements.begin(), elements.end());
    };

    //
    static const std::unordered_map<std::string, std::function<void(std::string&)>> atom_dispatcher = {{"id", id_lambda},
                                                                                                       {"p", pos_lambda},
                                                                                                       {"xyz", pos_lambda},
                                                                                                       {"NumHydrogens", hydrogens_lambda},
                                                                                                       {"Charge", charge_lambda},
                                                                                                       {"Isotope", isotope_lambda},
                                                                                                       {"Radical", radical_lambda},
                                                                                                       {"AS", stereo_lambda},
                                                                                                       {"NodeType", node_type_lambda},
                                                                                                       {"Element", element_lambda},
                                                                                                       {"GenericNickname", label_lambda},
                                                                                                       {"ElementList", element_list_lambda}};

    _applyDispatcher(pAttr, atom_dispatcher);
}

void MoleculeCdxmlLoader::_parseBond(CdxmlBond& bond, TiXmlAttribute* pAttr)
{
    auto bond_begin_lambda = [&bond](std::string& data) { bond.be.first = data; };
    auto bond_end_lambda = [&bond](std::string& data) { bond.be.second = data; };
    auto bond_order = [&bond](std::string& data) {
        static const std::unordered_map<std::string, int> order_map = {
            {"1", BOND_SINGLE}, {"2", BOND_DOUBLE}, {"3", BOND_TRIPLE}, {"1.5", BOND_AROMATIC}, {"dative", _BOND_COORDINATION}, {"hydrogen", _BOND_HYDROGEN}};
        bond.order = order_map.at(data);
    };

    auto bond_dir = [&bond](std::string& data) {
        static const std::unordered_map<std::string, std::pair<int, bool>> dir_map = {{"WedgedHashBegin", {BOND_DOWN, false}},
                                                                                                  {"WedgedHashEnd", {BOND_DOWN, true}},
                                                                                                  {"WedgeBegin", {BOND_UP, false}},
                                                                                                  {"WedgeEnd", {BOND_UP, true}},
                                                                                                  {"Wavy", {BOND_EITHER, false}}};
        try
        {
            auto& dir = dir_map.at(data);
            bond.dir = dir.first;
            bond.swap_bond = dir.second;
        }
        catch (std::out_of_range& const e)
        {
        }
    };

    static const std::unordered_map<std::string, std::function<void(std::string&)>> bond_dispatcher = {
        {"B", bond_begin_lambda}, {"E", bond_end_lambda}, {"Order", bond_order}, {"Display", bond_dir}};

    _applyDispatcher(pAttr, bond_dispatcher);
}

void MoleculeCdxmlLoader::_appendQueryAtom(const char* atom_label, std::unique_ptr<QueryMolecule::Atom>& atom)
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
