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
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <tinyxml2.h>

#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/ket_commons.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/molecule_scaffold_detection.h"

using namespace indigo;
using namespace tinyxml2;
using namespace rapidjson;

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

void MoleculeCdxmlLoader::_initMolecule(BaseMolecule& mol)
{
    mol.clear();
    nodes.clear();
    bonds.clear();
    _id_to_atom_idx.clear();
    _id_to_node_index.clear();
    _id_to_bond_index.clear();
    _fragment_nodes.clear();
    text_objects.clear();
    _pluses.clear();
    brackets.clear();
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
}

void MoleculeCdxmlLoader::loadMolecule(BaseMolecule& mol)
{
    _initMolecule(mol);
    if (_scanner != 0)
    {
        QS_DEF(Array<char>, buf);
        _scanner->readAll(buf);
        buf.push(0);
        XMLDocument xml;
        xml.Parse(buf.ptr());

        if (xml.Error())
            throw Error("XML parsing error: %s", xml.ErrorStr());

        parseCDXMLAttributes(xml.RootElement()->FirstAttribute());
        _parseCDXMLPage(xml.RootElement());

        if (!nodes.size())
            throw Error("CDXML has no data");

        _parseCollections(mol);
    }
}

void MoleculeCdxmlLoader::_parseCollections(BaseMolecule& mol)
{
    std::vector<int> atoms;
    for (auto& node : nodes)
    {
        int node_idx = _id_to_node_index.at(node.id);
        switch (node.type)
        {
        case kCDXNodeType_Element:
        case kCDXNodeType_ElementList:
            atoms.push_back(node_idx);
            break;
        case kCDXNodeType_ExternalConnectionPoint:
            break;
        case kCDXNodeType_Fragment:
            _fragment_nodes.push_back(node_idx);
            break;
        default:
            break;
        }
    }

    _addAtomsAndBonds(mol, atoms, bonds);

    _processEnhancedStereo(mol);

    for (auto& brk : brackets)
        _addBracket(mol, brk);

    for (const auto& to : text_objects)
        mol.meta().addMetaObject(new KETTextObject(to.first, to.second));

    for (const auto& plus : _pluses)
        mol.meta().addMetaObject(new KETReactionPlus(plus));

    for (const auto& arrow : _arrows)
    {
        const auto& arr_info = arrow.first;
        Vec2f v1(arr_info.first.x, arr_info.first.y);
        Vec2f v2(arr_info.second.x, arr_info.second.y);
        mol.meta().addMetaObject(new KETReactionArrow(arrow.second, v1, v2));
    }
}

void MoleculeCdxmlLoader::_processEnhancedStereo(BaseMolecule& mol)
{
    std::vector<int> ignore_cistrans(mol.edgeCount());
    std::vector<int> sensible_bond_directions(mol.edgeCount());
    for (int i = 0; i < mol.edgeCount(); i++)
        if (mol.getBondDirection(i) == BOND_EITHER)
        {
            if (MoleculeCisTrans::isGeomStereoBond(mol, i, 0, true))
            {
                ignore_cistrans[i] = 1;
                sensible_bond_directions[i] = 1;
            }
            else
            {
                int k;
                const Vertex& v = mol.getVertex(mol.getEdge(i).beg);

                for (k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                {
                    if (MoleculeCisTrans::isGeomStereoBond(mol, v.neiEdge(k), 0, true))
                    {
                        ignore_cistrans[v.neiEdge(k)] = 1;
                        sensible_bond_directions[i] = 1;
                        break;
                    }
                }
            }
        }

    mol.buildFromBondsStereocenters(stereochemistry_options, sensible_bond_directions.data());
    mol.buildFromBondsAlleneStereo(stereochemistry_options.ignore_errors, sensible_bond_directions.data());

    if (!mol.getChiralFlag())
        for (int i : mol.vertices())
        {
            int type = mol.stereocenters.getType(i);
            if (type == MoleculeStereocenters::ATOM_ABS)
                mol.stereocenters.setType(i, MoleculeStereocenters::ATOM_AND, 1);
        }

    mol.buildCisTrans(ignore_cistrans.data());
    mol.have_xyz = true;
    if (mol.stereocenters.size() == 0)
    {
        mol.buildFrom3dCoordinatesStereocenters(stereochemistry_options);
    }

    for (const auto& sc : _stereo_centers)
    {
        if (mol.stereocenters.getType(sc.atom_idx) == 0)
        {
            if (!stereochemistry_options.ignore_errors)
                throw Error("stereo type specified for atom #%d, but the bond "
                            "directions does not say that it is a stereocenter",
                            sc.atom_idx);
            mol.addStereocentersIgnoreBad(sc.atom_idx, sc.type, sc.group, false); // add non-valid stereocenters
        }
        else
            mol.stereocenters.setType(sc.atom_idx, sc.type, sc.group);
    }
}

void MoleculeCdxmlLoader::loadMoleculeFromFragment(BaseMolecule& mol, tinyxml2::XMLElement* pElem)
{
    _initMolecule(mol);
    _parseCDXMLElements(pElem, true);
    _parseCollections(mol);
}

void MoleculeCdxmlLoader::parseCDXMLAttributes(const XMLAttribute* pAttr)
{
    auto cdxml_bbox_lambda = [this](const std::string& data) {
        std::vector<std::string> coords = split(data, ' ');
        if (coords.size() == 4)
        {
            this->_has_bounding_box = true;
            this->cdxml_bbox = Rect2f(Vec2f(std::stof(coords[0]), std::stof(coords[1])), Vec2f(std::stof(coords[2]), std::stof(coords[3])));
        }
        else
            throw Error("Not enought coordinates for atom position");
    };

    auto& bond_length = cdxml_bond_length;
    auto cdxml_bond_length_lambda = [&bond_length](const std::string& data) { bond_length = data; };
    std::unordered_map<std::string, std::function<void(const std::string&)>> cdxml_dispatcher = {{"BoundingBox", cdxml_bbox_lambda},
                                                                                                 {"BondLength", cdxml_bond_length_lambda}};
    applyDispatcher(pAttr, cdxml_dispatcher);
}

void MoleculeCdxmlLoader::_parseCDXMLPage(XMLElement* pElem)
{
    for (auto pPageElem = pElem->FirstChildElement(); pPageElem; pPageElem = pPageElem->NextSiblingElement())
    {
        if (std::string(pPageElem->Value()) == "page")
        {
            _parseCDXMLElements(pPageElem->FirstChildElement());
        }
    }
}

void MoleculeCdxmlLoader::_parseCDXMLElements(XMLElement* pElem, bool no_siblings)
{
    auto node_lambda = [this](XMLElement* pElem) {
        CdxmlNode node;
        this->_parseNode(node, pElem);
        _addNode(node);
        if (node.type == kCDXNodeType_Fragment)
            this->_parseCDXMLElements(pElem->FirstChildElement());
    };

    auto bond_lambda = [this](XMLElement* pElem) {
        CdxmlBond bond;
        this->_parseBond(bond, pElem->FirstAttribute());
        this->_addBond(bond);
    };

    auto fragment_lambda = [this](XMLElement* pElem) {
        this->_parseFragmentAttributes(pElem->FirstAttribute());
        this->_parseCDXMLElements(pElem->FirstChildElement());
    };

    auto group_lambda = [this](XMLElement* pElem) { this->_parseCDXMLElements(pElem->FirstChildElement()); };

    auto bracketed_lambda = [this](XMLElement* pElem) {
        CdxmlBracket bracket;
        this->_parseBracket(bracket, pElem->FirstAttribute());
        this->brackets.push_back(bracket);
    };

    auto text_lambda = [this](XMLElement* pElem) { this->_parseText(pElem, this->text_objects); };

    auto graphic_lambda = [this](XMLElement* pElem) { this->_parseGraphic(pElem); };

    auto arrow_lambda = [this](XMLElement* pElem) { this->_parseArrow(pElem); };

    std::unordered_map<std::string, std::function<void(XMLElement * pElem)>> cdxml_dispatcher = {
        {"n", node_lambda}, {"b", bond_lambda},          {"fragment", fragment_lambda}, {"group", group_lambda}, {"bracketedgroup", bracketed_lambda},
        {"t", text_lambda}, {"graphic", graphic_lambda}, {"arrow", arrow_lambda}};

    for (pElem; pElem; pElem = pElem->NextSiblingElement())
    {
        auto it = cdxml_dispatcher.find(pElem->Value());
        if (it != cdxml_dispatcher.end())
        {
            it->second(pElem);
        }
        else
        {
        }
        if (no_siblings)
            break;
    }
}

void MoleculeCdxmlLoader::_updateConnection(const CdxmlNode& node, int atom_idx)
{
    for (auto fidx : _fragment_nodes)
    {
        auto& frag_node = nodes[fidx];
        auto fit = frag_node.node_id_to_connection_idx.find(node.id);
        if (fit != frag_node.node_id_to_connection_idx.end())
        {
            auto& conn = frag_node.connections[fit->second];
            conn.atom_idx = atom_idx;
        }
    }
}

void MoleculeCdxmlLoader::_addAtomsAndBonds(BaseMolecule& mol, const std::vector<int>& atoms, const std::vector<CdxmlBond>& bonds)
{
    _id_to_atom_idx.clear();
    mol.reaction_atom_mapping.clear_resize(atoms.size());
    mol.reaction_atom_mapping.zerofill();
    mol.reaction_atom_inversion.clear_resize(atoms.size());
    mol.reaction_atom_inversion.zerofill();
    mol.reaction_atom_exact_change.clear_resize(atoms.size());
    mol.reaction_atom_exact_change.zerofill();

    int atom_idx;
    for (auto atom_idx : atoms)
    {
        auto& atom = nodes[atom_idx];
        if (_pmol)
        {
            atom_idx = _pmol->addAtom(atom.element);
            _id_to_atom_idx.emplace(atom.id, atom_idx);
            mol.setAtomXyz(atom_idx, atom.pos);
            _pmol->setAtomCharge_Silent(atom_idx, atom.charge);
            if (atom.valence)
                _pmol->setExplicitValence(atom_idx, atom.valence);
            _pmol->setAtomRadical(atom_idx, atom.radical);
            _pmol->setAtomIsotope(atom_idx, atom.isotope);
            // _pmol->setPseudoAtom(atom_idx, label.c_str());
            switch (atom.enchanced_stereo)
            {
            case CdxmlNode::EnhancedStereoType::ABSOLUTE:
                _stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_ABS, 1);
                break;
            case CdxmlNode::EnhancedStereoType::AND:
                if (atom.enhanced_stereo_group)
                    _stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_AND, atom.enhanced_stereo_group);
                break;
            case CdxmlNode::EnhancedStereoType::OR:
                if (atom.enhanced_stereo_group)
                    _stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_OR, atom.enhanced_stereo_group);
                break;
            default:
                break;
            }
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
            auto bond_first_it = _id_to_atom_idx.find(bond.be.first);
            auto bond_second_it = _id_to_atom_idx.find(bond.be.second);
            auto& fn = nodes[_id_to_node_index.at(bond.be.first)];
            auto& sn = nodes[_id_to_node_index.at(bond.be.second)];

            if (bond_first_it != _id_to_atom_idx.end() && bond_second_it != _id_to_atom_idx.end())
            {
                if (bond.swap_bond)
                    bond_idx = _pmol->addBond_Silent(bond_second_it->second, bond_first_it->second, bond.order);
                else
                    bond_idx = _pmol->addBond_Silent(bond_first_it->second, bond_second_it->second, bond.order);
                if (bond.dir > 0)
                    _pmol->setBondDirection(bond_idx, bond.dir);
            }
            else if (fn.type == kCDXNodeType_ExternalConnectionPoint && bond_second_it != _id_to_atom_idx.end())
            {
                _updateConnection(fn, bond_second_it->second);
            }
            else if (sn.type == kCDXNodeType_ExternalConnectionPoint && bond_first_it != _id_to_atom_idx.end())
            {
                _updateConnection(sn, bond_first_it->second);
            }
            else if (fn.type == kCDXNodeType_Fragment && bond_second_it != _id_to_atom_idx.end())
            {
                auto bit_beg = fn.bond_id_to_connection_idx.find(bond.id);
                if (bit_beg != fn.bond_id_to_connection_idx.end())
                {
                    int a1 = fn.connections[bit_beg->second].atom_idx;
                    int a2 = bond_second_it->second;
                    if (a1 >= 0 && a2 >= 0)
                    {
                        auto bi = _pmol->addBond_Silent(a1, a2, bond.order);
                        if (bond.dir > 0)
                            _pmol->setBondDirection(bi, bond.dir);
                    }
                }
            }
            else if (sn.type == kCDXNodeType_Fragment && bond_first_it != _id_to_atom_idx.end())
            {
                auto bit_beg = sn.bond_id_to_connection_idx.find(bond.id);
                if (bit_beg != sn.bond_id_to_connection_idx.end())
                {
                    int a1 = bond_first_it->second;
                    int a2 = sn.connections[bit_beg->second].atom_idx;
                    if (a1 >= 0 && a2 >= 0)
                    {
                        auto bi = _pmol->addBond_Silent(a1, a2, bond.order);
                        if (bond.dir > 0)
                            _pmol->setBondDirection(bi, bond.dir);
                    }
                }
            }
        }
    }
}

void MoleculeCdxmlLoader::_addBracket(BaseMolecule& mol, const CdxmlBracket& bracket)
{
    static const std::unordered_map<int, int> implemeted_brackets = {
        {kCDXBracketUsage_SRU, SGroup::SG_TYPE_SRU}, {kCDXBracketUsage_MultipleGroup, SGroup::SG_TYPE_MUL}, {kCDXBracketUsage_Generic, SGroup::SG_TYPE_GEN}};

    auto it = implemeted_brackets.find(bracket.usage);
    if (it != implemeted_brackets.end())
    {
        int grp_idx = mol.sgroups.addSGroup(it->second);
        SGroup& sgroup = mol.sgroups.getSGroup(grp_idx);
        std::unordered_set<int> sgroup_atoms;
        for (auto atom_id : bracket.bracketed_list)
        {
            int atom_idx = _id_to_atom_idx.at(atom_id);
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

void MoleculeCdxmlLoader::_parseFragmentAttributes(const XMLAttribute* pAttr)
{
    for (pAttr; pAttr; pAttr = pAttr->Next())
    {
        if (std::string(pAttr->Name()) == "ConnectionOrder")
        {
            // it means that we are inside of NodeType=Fragment
            // let's check it
            if (nodes.size() && nodes.back().type == kCDXNodeType_Fragment)
            {
                auto& fn = nodes.back();
                auto vec_str = split(pAttr->Value(), ' ');
                if (fn.connections.size() == vec_str.size())
                {
                    for (int i = 0; i < vec_str.size(); ++i)
                    {
                        auto pid = std::stoi(vec_str[i]);
                        fn.connections[i].point_id = pid;
                        fn.node_id_to_connection_idx.emplace(pid, i);
                    }
                }
                else
                    throw Error("BondOrdering and ConnectionOrder sizes are not equal");
            }
            else
                throw Error("Unexpected ConnectionOrder");
        }
    }
}

void MoleculeCdxmlLoader::applyDispatcher(const XMLAttribute* pAttr, const std::unordered_map<std::string, std::function<void(const std::string&)>>& dispatcher)
{
    for (pAttr; pAttr; pAttr = pAttr->Next())
    {
        auto it = dispatcher.find(pAttr->Name());
        if (it != dispatcher.end())
        {
            std::string str_arg(pAttr->Value());
            it->second(str_arg);
        }
    }
}

void MoleculeCdxmlLoader::_parseNode(CdxmlNode& node, XMLElement* pElem)
{
    // Atom parsing lambdas definition
    auto id_lambda = [&node](const std::string& data) { node.id = data; };
    auto hydrogens_lambda = [&node](const std::string& data) { node.hydrogens = data; };
    auto charge_lambda = [&node](const std::string& data) { node.charge = data; };
    auto element_lambda = [&node](const std::string& data) { node.element = data; };
    auto isotope_lambda = [&node](const std::string& data) { node.isotope = data; };
    auto radical_lambda = [&node](const std::string& data) { node.radical = data; };
    auto label_lambda = [&node](const std::string& data) { node.label = data; };

    auto bond_ordering_lambda = [&node](const std::string& data) {
        auto vec_str = split(data, ' ');
        for (auto& str : vec_str)
        {
            auto bid = std::stoi(str);
            node.connections.push_back(_ExtConnection{bid, 0, -1});
            node.bond_id_to_connection_idx.emplace(bid, node.connections.size() - 1);
        }
    };

    auto pos_lambda = [&node, this](const std::string& data) { this->parsePos(data, node.pos); };

    auto stereo_lambda = [&node](const std::string& data) {
        static const std::unordered_map<std::string, int> cip_map = {{"U", 0}, {"N", 1}, {"R", 2}, {"S", 3}, {"r", 4}, {"s", 5}, {"u", 6}};
        node.stereo = cip_map.at(data);
    };

    auto node_type_lambda = [&node](const std::string& data) {
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

    auto element_list_lambda = [&node](const std::string& data) {
        std::vector<std::string> elements = split(data, ' ');
        if (elements.size() && elements.front().compare("NOT") == 0)
        {
            elements.erase(elements.begin());
            node.is_not_list = true;
        }
        node.element_list.assign(elements.begin(), elements.end());
    };

    auto geometry_lambda = [&node](const std::string& data) {
        static const std::unordered_map<std::string, int> geometry_map = {{"Unspecified", kCDXAtomGeometry_Unknown},
                                                                          {"1", kCDXAtomGeometry_1Ligand},
                                                                          {"Linear", kCDXAtomGeometry_Linear},
                                                                          {"Bent", kCDXAtomGeometry_Bent},
                                                                          {"TrigonalPlanar", kCDXAtomGeometry_TrigonalPlanar},
                                                                          {"TrigonalPyramidal", kCDXAtomGeometry_TrigonalPyramidal},
                                                                          {"SquarePlanar", kCDXAtomGeometry_SquarePlanar},
                                                                          {"Tetrahedral", kCDXAtomGeometry_Tetrahedral},
                                                                          {"TrigonalBipyramidal", kCDXAtomGeometry_TrigonalBipyramidal},
                                                                          {"SquarePyramidal", kCDXAtomGeometry_SquarePyramidal},
                                                                          {"5", kCDXAtomGeometry_5Ligand},
                                                                          {"Octahedral", kCDXAtomGeometry_Octahedral},
                                                                          {"6", kCDXAtomGeometry_6Ligand},
                                                                          {"7", kCDXAtomGeometry_7Ligand},
                                                                          {"8", kCDXAtomGeometry_8Ligand},
                                                                          {"9", kCDXAtomGeometry_9Ligand},
                                                                          {"10", kCDXAtomGeometry_10Ligand}};
        node.geometry = geometry_map.at(data);
    };

    auto enhanced_stereo_type_lambda = [&node](const std::string& data) {
        static const std::unordered_map<std::string, CdxmlNode::EnhancedStereoType> enhanced_stereo_map = {
            {"Unspecified", CdxmlNode::EnhancedStereoType::UNSPECIFIED},
            {"None", CdxmlNode::EnhancedStereoType::NONE},
            {"Absolute", CdxmlNode::EnhancedStereoType::ABSOLUTE},
            {"Or", CdxmlNode::EnhancedStereoType::OR},
            {"And", CdxmlNode::EnhancedStereoType::AND}};
        node.enchanced_stereo = enhanced_stereo_map.at(data);
    };

    auto enhanced_stereo_group_lambda = [&node](const std::string& data) { node.enhanced_stereo_group = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> node_dispatcher = {
        {"id", id_lambda},
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
        {"ElementList", element_list_lambda},
        {"BondOrdering", bond_ordering_lambda},
        {"Geometry", geometry_lambda},
        {"EnhancedStereoType", enhanced_stereo_type_lambda},
        {"EnhancedStereoGroupNum", enhanced_stereo_group_lambda},
    };
    applyDispatcher(pElem->FirstAttribute(), node_dispatcher);
}

void MoleculeCdxmlLoader::_addNode(CdxmlNode& node)
{
    nodes.push_back(node);
    _id_to_node_index.emplace(node.id, nodes.size() - 1);
}

void MoleculeCdxmlLoader::_addBond(CdxmlBond& bond)
{
    bonds.push_back(bond);
    _id_to_bond_index.emplace(bond.id, bonds.size() - 1);
}

void MoleculeCdxmlLoader::_parseBond(CdxmlBond& bond, const XMLAttribute* pAttr)
{
    auto id_lambda = [&bond](const std::string& data) { bond.id = data; };
    auto bond_begin_lambda = [&bond](const std::string& data) { bond.be.first = data; };
    auto bond_end_lambda = [&bond](const std::string& data) { bond.be.second = data; };
    auto bond_order_lambda = [&bond](const std::string& data) {
        static const std::unordered_map<std::string, int> order_map = {
            {"1", BOND_SINGLE}, {"2", BOND_DOUBLE}, {"3", BOND_TRIPLE}, {"1.5", BOND_AROMATIC}, {"dative", _BOND_COORDINATION}, {"hydrogen", _BOND_HYDROGEN}};
        bond.order = order_map.at(data);
    };

    auto bond_dir_lambda = [&bond](const std::string& data) {
        static const std::unordered_map<std::string, std::pair<int, bool>> dir_map = {{"WedgedHashBegin", {BOND_DOWN, false}},
                                                                                      {"WedgedHashEnd", {BOND_DOWN, true}},
                                                                                      {"WedgeBegin", {BOND_UP, false}},
                                                                                      {"WedgeEnd", {BOND_UP, true}},
                                                                                      {"Wavy", {BOND_EITHER, false}}};
        try
        {
            std::cout << data << std::endl;
            auto& dir = dir_map.at(data);
            bond.dir = dir.first;
            bond.swap_bond = dir.second;
        }
        catch (std::out_of_range& e)
        {
        }
    };

    std::unordered_map<std::string, std::function<void(const std::string&)>> bond_dispatcher = {
        {"id", id_lambda}, {"B", bond_begin_lambda}, {"E", bond_end_lambda}, {"Order", bond_order_lambda}, {"Display", bond_dir_lambda}};

    applyDispatcher(pAttr, bond_dispatcher);
}

void MoleculeCdxmlLoader::parsePos(const std::string& data, Vec3f& pos)
{
    std::vector<std::string> coords = split(data, ' ');
    if (coords.size() >= 2)
    {
        pos.x = std::stof(coords[0]);
        pos.y = std::stof(coords[1]);
        pos.z = 0;
        if (this->_has_bounding_box)
        {
            pos.x -= this->cdxml_bbox.left();
            pos.y -= this->cdxml_bbox.bottom();
        }
        pos.x /= SCALE;
        pos.y /= -SCALE;
    }
    else
        throw Error("Not enought coordinates");
}

void MoleculeCdxmlLoader::parseBBox(const std::string& data, Rect2f& bbox)
{
    std::vector<std::string> coords = split(data, ' ');
    if (coords.size() == 4)
    {
        Vec2f v1(std::stof(coords[0]), std::stof(coords[1]));
        Vec2f v2(std::stof(coords[2]), std::stof(coords[3]));
        if (this->_has_bounding_box)
        {
            v1.sub(this->cdxml_bbox.leftBottom());
            v2.sub(this->cdxml_bbox.leftBottom());
        }
        v1.x /= SCALE;
        v2.x /= SCALE;
        v1.y /= -SCALE;
        v2.y /= -SCALE;
        bbox = Rect2f(v1, v2);
    }
    else
        throw Error("Not enought coordinates for text bounding box");
}

void MoleculeCdxmlLoader::_parseGraphic(const XMLElement* pElem)
{
    AutoInt superseded_id = 0;
    auto superseded_lambda = [&superseded_id](const std::string& data) { superseded_id = data; };

    Rect2f graph_bbox;
    auto graphic_bbox_lambda = [&graph_bbox, this](const std::string& data) { this->parseBBox(data, graph_bbox); };

    std::string graphic_type;
    auto graphic_type_lambda = [&graphic_type](const std::string& data) { graphic_type = data; };

    std::string symbol_type;
    auto symbol_type_lambda = [&symbol_type](const std::string& data) { symbol_type = data; };

    std::string arrow_type;
    auto arrow_type_lambda = [&arrow_type](const std::string& data) { arrow_type = data; };

    AutoInt head_size;
    auto head_size_lambda = [&head_size](const std::string& data) { head_size = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> graphic_dispatcher = {
        {"SupersededBy", superseded_lambda}, {"BoundingBox", graphic_bbox_lambda}, {"GraphicType", graphic_type_lambda},
        {"SymbolType", symbol_type_lambda},  {"ArrowType", arrow_type_lambda},     {"HeadSize", head_size_lambda}};

    auto pAttr = pElem->FirstAttribute();
    applyDispatcher(pAttr, graphic_dispatcher);

    if (graphic_type == "Line" && arrow_type == "FullHead")
    {
    }
    else if (graphic_type == "Symbol" && symbol_type == "Plus")
    {
        _pluses.emplace_back(graph_bbox.center());
    }
}

void MoleculeCdxmlLoader::_parseArrow(const tinyxml2::XMLElement* pElem)
{
    Rect2f text_bbox;
    auto arrow_bbox_lambda = [&text_bbox, this](const std::string& data) { this->parseBBox(data, text_bbox); };
    Vec3f begin_pos;
    auto arrow_begin_lambda = [&begin_pos, this](const std::string& data) { this->parsePos(data, begin_pos); };
    Vec3f end_pos;
    auto arrow_end_lambda = [&end_pos, this](const std::string& data) { this->parsePos(data, end_pos); };
    std::string fill_type;
    auto fill_type_lambda = [&fill_type](const std::string& data) { fill_type = data; };
    std::string arrow_head;
    auto arrow_head_lambda = [&arrow_head](const std::string& data) { arrow_head = data; };
    std::string head_type;
    auto head_type_lambda = [&head_type](const std::string& data) { head_type = data; };
    std::unordered_map<std::string, std::function<void(const std::string&)>> arrow_dispatcher = {
        {"BoundingBox", arrow_bbox_lambda},  {"FillType", fill_type_lambda}, {"ArrowheadHead", arrow_head_lambda},
        {"ArrowheadType", head_type_lambda}, {"Head3D", arrow_end_lambda},   {"Tail3D", arrow_begin_lambda}};

    auto pAttr = pElem->FirstAttribute();
    applyDispatcher(pAttr, arrow_dispatcher);
    _arrows.push_back(std::make_pair(std::make_pair(begin_pos, end_pos), 2));
}

void MoleculeCdxmlLoader::_parseText(const XMLElement* pElem, std::vector<std::pair<Vec3f, std::string>>& text_parsed)
{
    Vec3f text_pos;
    auto text_coordinates_lambda = [&text_pos, this](const std::string& data) { this->parsePos(data, text_pos); };

    Rect2f text_bbox;
    auto text_bbox_lambda = [&text_bbox, this](const std::string& data) { this->parseBBox(data, text_bbox); };

    std::string label_justification, label_alignment;
    auto label_justification_lambda = [&label_justification, this](const std::string& data) { label_justification = data; };
    auto label_justification_alignment_lambda = [&label_alignment, this](const std::string& data) { label_alignment = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> text_dispatcher = {{"p", text_coordinates_lambda},
                                                                                                {"BoundingBox", text_bbox_lambda},
                                                                                                {"LabelJustification", label_justification_lambda},
                                                                                                {"LabelAlignment", label_justification_alignment_lambda}};

    AutoInt font_id, font_color_id, font_face;
    float font_size;

    auto style_font_lambda = [&font_id](const std::string& data) { font_id = data; };
    auto style_size_lambda = [&font_size](const std::string& data) { font_size = std::stof(data) * 1.5; };
    auto style_color_lambda = [&font_color_id](const std::string& data) { font_color_id = data; };
    auto style_face_lambda = [&font_face](const std::string& data) { font_face = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> style_dispatcher = {
        {"font", style_font_lambda}, {"size", style_size_lambda}, {"color", style_color_lambda}, {"face", style_face_lambda}};

    auto pAttr = pElem->FirstAttribute();
    applyDispatcher(pAttr, text_dispatcher);

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("blocks");
    writer.StartArray();
    for (auto pTextStyle = pElem->FirstChildElement(); pTextStyle; pTextStyle = pTextStyle->NextSiblingElement())
    {
        std::string text_element = pTextStyle->Value();
        if (text_element == "s")
        {
            font_face = 0;
            font_size = 0.0;
            auto pStyleAttribute = pTextStyle->FirstAttribute();
            applyDispatcher(pStyleAttribute, style_dispatcher);
            std::vector<std::string> text_vec_styles;
            CDXMLFontStyle fs(font_face);
            if (font_face == KCDXMLChemicalFontStyle)
            {
                // special case
            }
            else
            {
                if (fs.is_bold)
                    text_vec_styles.push_back(KETFontBoldStr);
                if (fs.is_italic)
                    text_vec_styles.push_back(KETFontItalicStr);
                if (fs.is_superscript)
                    text_vec_styles.push_back(KETFontSuperscriptStr);
                if (fs.is_superscript)
                    text_vec_styles.push_back(KETFontSubscriptStr);
            }
            if (font_size > 0)
                text_vec_styles.push_back(std::string(KETFontCustomSizeStr) + "_" + std::to_string((int)ceil(font_size)) + "px");

            std::string label_plain = pTextStyle->GetText();
            std::remove_if(label_plain.begin(), label_plain.end(), [](char c) { return (c == '\r'); });

            auto labels = split(label_plain, '\n');
            for (const auto& label : labels)
            {
                writer.StartObject();
                writer.Key("text");
                writer.String(label.c_str());
                writer.Key("inlineStyleRanges");
                writer.StartArray();
                for (auto style_str : text_vec_styles)
                {
                    writer.StartObject();
                    writer.Key("offset");
                    writer.Int(0);
                    writer.Key("length");
                    writer.Int(label.size());
                    writer.Key("style");
                    writer.String(style_str.c_str());
                }
                writer.EndObject();
                writer.EndArray();

                writer.Key("entityRanges");
                writer.StartArray();
                writer.EndArray();

                writer.Key("data");
                writer.StartObject();
                writer.EndObject();

                writer.EndObject();
            }
        }
        else
        {
            throw Error("unrecognized text element: %s", text_element.c_str());
        }
    }
    writer.EndArray();
    writer.Key("entityMap");
    writer.StartObject();
    writer.EndObject();

    writer.EndObject();
    text_pos.y -= text_bbox.height() / 2;
    text_parsed.emplace_back(text_pos, s.GetString());
}

void MoleculeCdxmlLoader::_parseBracket(CdxmlBracket& bracket, const XMLAttribute* pAttr)
{
    auto bracketed_ids_lambda = [&bracket](const std::string& data) {
        std::vector<std::string> vec_str = split(data, ' ');
        bracket.bracketed_list.assign(vec_str.begin(), vec_str.end());
    };
    auto bracket_usage_lambda = [&bracket](const std::string& data) {
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

    auto repeat_count_lambda = [&bracket](const std::string& data) { bracket.repeat_count = data; };
    auto repeat_pattern_lambda = [&bracket](const std::string& data) {
        static const std::unordered_map<std::string, int> rep_map = {
            {"HeadToTail", RepeatingUnit::HEAD_TO_TAIL}, {"HeadToHead", RepeatingUnit::HEAD_TO_HEAD}, {"EitherUnknown", RepeatingUnit::EITHER}};
        bracket.repeat_pattern = rep_map.at(data);
    };

    auto sru_label_lambda = [&bracket](const std::string& data) { bracket.sru_label = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> bracket_dispatcher = {{"BracketedObjectIDs", bracketed_ids_lambda},
                                                                                                   {"BracketUsage", bracket_usage_lambda},
                                                                                                   {"RepeatCount", repeat_count_lambda},
                                                                                                   {"PolymerRepeatPattern", repeat_pattern_lambda},
                                                                                                   {"SRULabel", sru_label_lambda}};

    applyDispatcher(pAttr, bracket_dispatcher);
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
