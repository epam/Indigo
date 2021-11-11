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

#include "molecule/molecule_cdxml_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_scaffold_detection.h"
#include "tinyxml.h"
#include <unordered_map>
#include <unordered_set>

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

        std::vector<TiXmlElement*> fragments, brackets;
        _parseCDXMLAttributes(xml.RootElement()->FirstAttribute());
        _enumerateData(xml.RootElement(), fragments, brackets);
        if (!fragments.size())
            throw Error("CDXML has no fragment tag");
        printf("fragments found: %d\n", fragments.size());
        printf("brackets found: %d\n", brackets.size());
        _loadFragments(mol, fragments, brackets);
    }
}

void MoleculeCdxmlLoader::_parseCDXMLAttributes(TiXmlAttribute* pAttr)
{
    auto cdxml_bbox_lambda = [this](std::string& data) {
        std::vector<std::string> coords = split(data, ' ');
        if ( coords.size() == 4 )
        {
            this->_has_bounding_box = true;
            this->_cdxml_bbox = Rect2f(Vec2f(std::stof(coords[0]), std::stof(coords[1])), Vec2f(std::stof(coords[2]), std::stof(coords[3])));
        }
        else
            throw Error("Not enought coordinates for atom position");
    };

    auto& bond_length = _cdxml_bond_length;
    auto cdxml_bond_length_lambda = [&bond_length](std::string& data) { bond_length = data; };
    static const std::unordered_map<std::string, std::function<void(std::string&)>> cdxml_dispatcher = {{"BoundingBox", cdxml_bbox_lambda},
                                                                                                       {"BondLength", cdxml_bond_length_lambda}};
    _applyDispatcher(pAttr, cdxml_dispatcher);
}

void MoleculeCdxmlLoader::_enumerateData(TiXmlElement* elem, std::vector<TiXmlElement*>& fragments, std::vector<TiXmlElement*>& brackets)
{
    auto pElem = elem->FirstChildElement();
    for (pElem; pElem; pElem = pElem->NextSiblingElement())
    {
        if (std::string(pElem->Value()).compare("fragment") == 0)
            fragments.push_back(pElem);
        if (std::string(pElem->Value()).compare("bracketedgroup") == 0)
            brackets.push_back(pElem);
        _enumerateData(pElem, fragments, brackets);
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

void MoleculeCdxmlLoader::_addAtomsAndBonds(BaseMolecule& mol, const std::vector<CdxmlNode>& atoms, const std::vector<CdxmlBond>& bonds)
{
    const float COORD_COEF = 1.0f / 1857710.0f;
    _id_to_idx.clear();
    mol.reaction_atom_mapping.clear_resize(atoms.size());
    mol.reaction_atom_mapping.zerofill();
    mol.reaction_atom_inversion.clear_resize(atoms.size());
    mol.reaction_atom_inversion.zerofill();
    mol.reaction_atom_exact_change.clear_resize(atoms.size());
    mol.reaction_atom_exact_change.zerofill();

    int atom_idx;
    for (const auto& atom : atoms)
    {
        if (_pmol)
        {
            atom_idx = _pmol->addAtom(atom.element);
            _id_to_idx.emplace(atom.id, atom_idx);
            Vec3f scaled_pos(atom.pos.x * COORD_COEF, atom.pos.y * COORD_COEF, atom.pos.z * COORD_COEF);
            mol.setAtomXyz(atom_idx, scaled_pos);
            _pmol->setAtomCharge_Silent(atom_idx, atom.charge);
            if (atom.valence)
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
                bond_idx = _pmol->addBond_Silent(_id_to_idx.at(bond.be.second), _id_to_idx.at(bond.be.first), bond.order);
            else
                bond_idx = _pmol->addBond_Silent(_id_to_idx.at(bond.be.first), _id_to_idx.at(bond.be.second), bond.order);
            if (bond.dir > 0)
                _pmol->setBondDirection(bond_idx, bond.dir);
        }
    }
}

void MoleculeCdxmlLoader::_addBracket(BaseMolecule& mol, const CdxmlBracket& bracket)
{
    static const std::unordered_map<int, int> implemeted_brackets = {
        {kCDXBracketUsage_SRU, SGroup::SG_TYPE_SRU}, {kCDXBracketUsage_MultipleGroup, SGroup::SG_TYPE_MUL}, {kCDXBracketUsage_Generic, SGroup::SG_TYPE_GEN }};

    auto it = implemeted_brackets.find(bracket.usage);
    if( it != implemeted_brackets.end() )
    {
        int grp_idx = mol.sgroups.addSGroup(it->second);
        SGroup& sgroup = mol.sgroups.getSGroup(grp_idx);
        std::unordered_set<int> sgroup_atoms;
        for (auto atom_id : bracket.bracketed_list)
        {
            int atom_idx = _id_to_idx.at(atom_id);
            sgroup.atoms.push(atom_idx);
            sgroup_atoms.insert(atom_idx);
            if (bracket.usage == kCDXBracketUsage_MultipleGroup)
            {
                MultipleGroup& mg = (MultipleGroup&)sgroup;
                if (mg.multiplier)
                    mg.parent_atoms.push(atom_idx);
            }
        }

        // add brackets
        Vec2f* p = sgroup.brackets.push();
        p[0].set(0, 0);
        p[1].set(0, 0);
        p = sgroup.brackets.push();
        p[0].set(0, 0);
        p[1].set(0, 0);
        // sgroup.brk_style
        switch (bracket.usage)
        {
        case kCDXBracketUsage_SRU: {
            RepeatingUnit& ru = (RepeatingUnit&)sgroup;
            ru.connectivity = bracket.repeat_pattern;
            ru.subscript.readString(bracket.sru_label.c_str(), true);
        }
        break;
        case kCDXBracketUsage_MultipleGroup: {
            MultipleGroup& mg = (MultipleGroup&)sgroup;
            if (bracket.repeat_count)
            {
                mg.multiplier = bracket.repeat_count;
            }
        }
        break;
        case kCDXBracketUsage_Generic:
            break;
        default:
            break;
        }

        _handleSGroup(sgroup, sgroup_atoms, mol);
    }
}

void MoleculeCdxmlLoader::_handleSGroup(SGroup& sgroup, const std::unordered_set<int>& atoms, BaseMolecule& bmol)
{
    int start = -1;
    int end = -1;
    int end_bond = -1, start_bond = -1;
    QS_DEF(Array<int>, xbonds);

    for (auto j : bmol.edges())
    {
        if (!bmol.hasEdge(j))
            continue;

        const Edge& edge = bmol.getEdge(j);
        auto itbeg = atoms.find(edge.beg);
        auto itend = atoms.find(edge.end);

        if (itbeg == atoms.end() && itend == atoms.end())
            continue;
        if (itbeg != atoms.end() && itend != atoms.end())
            continue;
        else
        {
            // bond going out of the sgroup
            xbonds.push(j);
            if (start_bond == -1)
            {
                start_bond = j;
                start = itbeg != atoms.end() ? *itbeg : *itend;
            }
            else if (end_bond == -1)
            {
                end_bond = j;
                end = itbeg != atoms.end() ? *itbeg : *itend;
            }
        }
    }

    if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
    {
        QS_DEF(Array<int>, mapping);
        std::unique_ptr<BaseMolecule> rep(bmol.neu());
        rep->makeSubmolecule(bmol, sgroup.atoms, &mapping, 0);

        rep->sgroups.clear(SGroup::SG_TYPE_SRU);
        rep->sgroups.clear(SGroup::SG_TYPE_MUL);

        int rep_start = mapping[start];
        int rep_end = mapping[end];
        MultipleGroup& mg = (MultipleGroup&)sgroup;
        if (mg.multiplier > 1)
        {
            int start_order = start_bond > 0 ? bmol.getBondOrder(start_bond) : -1;
            int end_order = end_bond > 0 ? bmol.getBondOrder(end_bond) : -1;
            for (int j = 0; j < mg.multiplier - 1; j++)
            {
                bmol.mergeWithMolecule(*rep, &mapping, 0);
                int k;
                for (k = rep->vertexBegin(); k != rep->vertexEnd(); k = rep->vertexNext(k))
                    sgroup.atoms.push(mapping[k]);
                if (rep_end >= 0 && end_bond >= 0)
                {
                    int external = bmol.getEdge(end_bond).findOtherEnd(end);
                    bmol.removeBond(end_bond);
                    if (_pmol != 0)
                    {
                        _pmol->addBond(end, mapping[rep_start], start_order);
                        end_bond = _pmol->addBond(mapping[rep_end], external, end_order);
                    }
                    else
                    {
                        _pqmol->addBond(end, mapping[rep_start], new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, start_order));
                        end_bond = _pqmol->addBond(mapping[rep_end], external, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, end_order));
                    }
                    end = mapping[rep_end];
                }
            }
        }
    }
}

void MoleculeCdxmlLoader::_loadFragments(BaseMolecule& mol, const std::vector<TiXmlElement*>& fragments, const std::vector<TiXmlElement*>& brackets)
{
    std::vector<CdxmlNode> nodes;
    std::vector<CdxmlBond> bonds;

    for (auto fragment : fragments)
    {
        auto pElem = fragment->FirstChildElement();
        for (pElem; pElem; pElem = pElem->NextSiblingElement())
        {
            const char* pKey = pElem->Value();
            switch (*pKey)
            {
            case 'n': // node
            {
                CdxmlNode atom;
                auto pAttr = pElem->FirstAttribute();
                _parseNode(atom, pAttr);
                nodes.push_back(atom);
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
    }

    std::vector<CdxmlNode> atoms;
    for (auto& node : nodes)
    {
        switch (node.type)
        {
        case kCDXNodeType_Element:
        case kCDXNodeType_ElementList:
            atoms.push_back(node);
            break;
        default:
            printf("unhandled node type: %d\n", node.type);
            break;
        }
    }

    _addAtomsAndBonds(mol, nodes, bonds);

    for (auto& brk : brackets)
    {
        CdxmlBracket bracket;
        auto pAttr = brk->FirstAttribute();
        _parseBracket(bracket, pAttr);
        _addBracket(mol, bracket);
    }

    printf("atoms: %d\n", nodes.size());
    printf("bonds: %d\n", bonds.size());
    for (auto& node : nodes)
    {
        printf("id=%d, element=%d, x=%f y=%f\n ", (int)node.id, (int)node.element, node.pos.x, node.pos.y);
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

void MoleculeCdxmlLoader::_parseNode(CdxmlNode& node, TiXmlAttribute* pAttr)
{
    // Atom parsing lambdas definition
    auto id_lambda = [&node](std::string& data) { node.id = data; };
    auto hydrogens_lambda = [&node](std::string& data) { node.hydrogens = data; };
    auto charge_lambda = [&node](std::string& data) { node.charge = data; };
    auto element_lambda = [&node](std::string& data) { node.element = data; };
    auto isotope_lambda = [&node](std::string& data) { node.isotope = data; };
    auto radical_lambda = [&node](std::string& data) { node.radical = data; };
    auto label_lambda = [&node](std::string& data) { node.label = data; };

    auto pos_lambda = [&node, this](std::string& data) {
        std::vector<std::string> coords = split(data, ' ');
        if (coords.size() >= 2)
        {
            node.pos.x = std::stof(coords[0]);
            node.pos.y = std::stof(coords[1]);
            if (this->_has_bounding_box)
                node.pos.y = this->_cdxml_bbox.top() - node.pos.y;
            if (coords.size() == 3)
                node.pos.z = std::stof(coords[2]);
            else
                node.pos.z = 0;
        }
        else
            throw Error("Not enought coordinates for atom position");
    };

    auto stereo_lambda = [&node](std::string& data) {
        static const std::unordered_map<std::string, int> cip_map = {{"U", 0}, {"N", 1}, {"R", 2}, {"S", 3}, {"r", 4}, {"s", 5}, {"u", 6}};
        node.stereo = cip_map.at(data);
    };

    auto node_type_lambda = [&node](std::string& data) {
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
        node.type = node_type_map.at(data);
    };

    auto element_list_lambda = [&node](std::string& data) {
        std::vector<std::string> elements = split(data, ' ');
        if (elements.size() && elements.front().compare("NOT") == 0)
        {
            elements.erase(elements.begin());
            node.is_not_list = true;
        }
        node.element_list.assign(elements.begin(), elements.end());
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
    auto bond_order_lambda = [&bond](std::string& data) {
        static const std::unordered_map<std::string, int> order_map = {
            {"1", BOND_SINGLE}, {"2", BOND_DOUBLE}, {"3", BOND_TRIPLE}, {"1.5", BOND_AROMATIC}, {"dative", _BOND_COORDINATION}, {"hydrogen", _BOND_HYDROGEN}};
        bond.order = order_map.at(data);
    };

    auto bond_dir_lambda = [&bond](std::string& data) {
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
        catch (std::out_of_range& e)
        {
        }
    };

    static const std::unordered_map<std::string, std::function<void(std::string&)>> bond_dispatcher = {
        {"B", bond_begin_lambda}, {"E", bond_end_lambda}, {"Order", bond_order_lambda}, {"Display", bond_dir_lambda}};

    _applyDispatcher(pAttr, bond_dispatcher);
}

void MoleculeCdxmlLoader::_parseBracket(CdxmlBracket& bracket, TiXmlAttribute* pAttr)
{
    auto bracketed_ids_lambda = [&bracket](std::string& data) {
        std::vector<std::string> bracketed_ids = split(data, ' ');
        bracket.bracketed_list.assign(bracketed_ids.begin(), bracketed_ids.end());
    };

    auto bracket_usage_lambda = [&bracket](std::string& data) {
        static const std::unordered_map<std::string, int> usage_map = {{"Unspecified", kCDXBracketUsage_Unspecified},
                                                                       {"Unused1", kCDXBracketUsage_Unused1},
                                                                       {"Unused2", kCDXBracketUsage_Unused2},
                                                                       {"SRU", kCDXBracketUsage_SRU},
                                                                       {"Monomer", kCDXBracketUsage_Monomer},
                                                                       {"Mer", kCDXBracketUsage_Mer},
                                                                       {"Copolymer", kCDXBracketUsage_Copolymer},
                                                                       {"CopolymerAlternating", kCDXBracketUsage_CopolymerAlternating},
                                                                       {"CopolymerRandom", kCDXBracketUsage_CopolymerRandom},
                                                                       {"CopolymerBlock", kCDXBracketUsage_CopolymerBlock},
                                                                       {"Crosslink", kCDXBracketUsage_Crosslink},
                                                                       {"Graft", kCDXBracketUsage_Graft},
                                                                       {"Modification", kCDXBracketUsage_Modification},
                                                                       {"Component", kCDXBracketUsage_Component},
                                                                       {"MixtureUnordered", kCDXBracketUsage_MixtureUnordered},
                                                                       {"MixtureOrdered", kCDXBracketUsage_MixtureOrdered},
                                                                       {"MultipleGroup", kCDXBracketUsage_MultipleGroup},
                                                                       {"Generic", kCDXBracketUsage_Generic},
                                                                       {"Anypolymer", kCDXBracketUsage_Anypolymer}

        };
        bracket.usage = usage_map.at(data);
    };

    auto repeat_count_lambda = [&bracket](std::string& data) { bracket.repeat_count = data; };
    auto repeat_pattern_lambda = [&bracket](std::string& data) {
        static const std::unordered_map<std::string, int> rep_map = {
            {"HeadToTail", RepeatingUnit::HEAD_TO_TAIL}, {"HeadToHead", RepeatingUnit::HEAD_TO_HEAD}, {"EitherUnknown", RepeatingUnit::EITHER}};
        bracket.repeat_pattern = rep_map.at(data);
    };

    auto sru_label_lambda = [&bracket](std::string& data) { bracket.sru_label = data; };

    static const std::unordered_map<std::string, std::function<void(std::string&)>> bracket_dispatcher = {{"BracketedObjectIDs", bracketed_ids_lambda},
                                                                                                          {"BracketUsage", bracket_usage_lambda},
                                                                                                          {"RepeatCount", repeat_count_lambda},
                                                                                                          {"PolymerRepeatPattern", repeat_pattern_lambda},
                                                                                                          {"SRULabel", sru_label_lambda}};

    _applyDispatcher(pAttr, bracket_dispatcher);
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
