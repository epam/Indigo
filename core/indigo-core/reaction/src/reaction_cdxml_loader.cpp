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

#include <tinyxml2.h>

#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "molecule/meta_commons.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/parse_utils.h"
#include "reaction/reaction.h"
#include "reaction/reaction_cdxml_loader.h"

using namespace indigo;
using namespace tinyxml2;

IMPL_ERROR(ReactionCdxmlLoader, "reaction CDXML loader");

ReactionCdxmlLoader::ReactionCdxmlLoader(Scanner& scanner, bool is_binary) : _scanner(scanner), _is_binary(is_binary)
{
    ignore_bad_valence = false;
}

ReactionCdxmlLoader::~ReactionCdxmlLoader()
{
}

void ReactionCdxmlLoader::_initReaction(BaseReaction& rxn)
{
    rxn.clear();
    reactants_ids.clear();
    products_ids.clear();
    intermediates_ids.clear();
    arrows_ids.clear();
    agents_ids.clear();
    bracket_ids.clear();

    if (rxn.isQueryReaction())
        _pqrxn = &rxn.asQueryReaction();
    else
        _prxn = &rxn.asReaction();

    if (_prxn)
    {
        _pmol = &_mol;
    }
    else if (_pqrxn)
    {
        _pmol = &_qmol;
    }
    else
        throw Error("unknown reaction type: %s", typeid(rxn).name());
}

void ReactionCdxmlLoader::_parseStep(BaseCDXProperty& prop)
{
    auto reactants_lambda = [this](const std::string& data) {
        std::vector<std::string> frag_ids = split(data, ' ');
        for (const auto& ids : frag_ids)
        {
            auto id = std::stoi(ids);
            if (this->products_ids.find(id) != this->products_ids.end())
            {
                this->products_ids.erase(id);
                this->intermediates_ids.insert(id);
            }
            else
            {
                this->reactants_ids.insert(id);
            }
        }
    };
    auto products_lambda = [this](const std::string& data) {
        std::vector<std::string> frag_ids = split(data, ' ');
        for (const auto& ids : frag_ids)
        {
            auto id = std::stoi(ids);
            if (this->reactants_ids.find(id) != this->reactants_ids.end())
            {
                this->reactants_ids.erase(id);
                this->intermediates_ids.insert(id);
            }
            else
            {
                this->products_ids.insert(id);
            }
        }
    };
    auto arrows_lambda = [this](const std::string& data) {
        std::vector<std::string> frag_ids = split(data, ' ');
        for (const auto& ids : frag_ids)
            this->arrows_ids.insert(std::stoi(ids));
    };
    auto agents_lambda = [this](const std::string& data) {
        std::vector<std::string> frag_ids = split(data, ' ');
        for (const auto& ids : frag_ids)
            this->agents_ids.insert(std::stoi(ids));
    };

    std::unordered_map<std::string, std::function<void(const std::string&)>> cdxml_dispatcher = {{"ReactionStepReactants", reactants_lambda},
                                                                                                 {"ReactionStepProducts", products_lambda},
                                                                                                 {"ReactionStepArrows", arrows_lambda},
                                                                                                 {"ReactionStepObjectsAboveArrow", agents_lambda},
                                                                                                 {"ReactionStepObjectsBelowArrow", agents_lambda}};
    MoleculeCdxmlLoader::applyDispatcher(prop, cdxml_dispatcher);
}

void ReactionCdxmlLoader::loadReaction(BaseReaction& rxn)
{
    _initReaction(rxn);
    std::unique_ptr<CDXReader> cdx_reader = _is_binary ? std::make_unique<CDXReader>(_scanner) : std::make_unique<CDXMLReader>(_scanner);
    cdx_reader->process();
    MoleculeCdxmlLoader loader(_scanner, _is_binary);
    loader.stereochemistry_options = stereochemistry_options;
    loader.parseCDXMLAttributes(*cdx_reader->rootElement()->firstProperty());

    for (auto page_elem = cdx_reader->rootElement()->firstChildElement(); page_elem->hasContent(); page_elem = page_elem->nextSiblingElement())
    {
        if (page_elem->value() == "page")
        {
            for (auto cdxml_elem = page_elem->firstChildElement(); cdxml_elem->hasContent(); cdxml_elem = cdxml_elem->nextSiblingElement())
            {
                if (cdxml_elem->value() == "scheme")
                {
                    for (auto scheme_element = cdxml_elem->firstChildElement(); scheme_element->hasContent();
                         scheme_element = scheme_element->nextSiblingElement())
                        if (scheme_element->value() == "step")
                            _parseStep(*scheme_element->firstProperty());
                }
                else if (cdxml_elem->value() == "bracketedgroup")
                {
                    for (auto brackets = cdxml_elem->firstChildElement(); brackets->hasContent(); brackets = brackets->nextSiblingElement())
                    {
                        auto id = brackets->findProperty("id");
                        bracket_ids.insert(std::stoi(id->value()));
                        if (id->hasContent())
                            _cdxml_elements.emplace(std::stoi(id->value()), cdxml_elem->copy());
                    }
                }
                else
                {
                    auto id = cdxml_elem->findProperty("id");
                    if (id->hasContent())
                        _cdxml_elements.emplace(std::stoi(id->value()), cdxml_elem->copy());
                    else
                        std::cout << "Unknown element: " << page_elem->value() << std::endl;
                }
            }
        }
        else if (page_elem->value() == "colortable")
            loader.parseColorTable(*page_elem);
        else if (page_elem->value() == "fonttable")
            loader.parseFontTable(*page_elem);
        else
            std::cout << "Unknown element: " << page_elem->value() << std::endl;
    }

    std::map<int, std::unordered_map<int, int>> moleculeToAtomIDs;
    for (auto id : reactants_ids)
    {
        auto elem_it = _cdxml_elements.find(id);
        if (elem_it != _cdxml_elements.end())
        {
            loader.loadMoleculeFromFragment(*_pmol, *elem_it->second);
            if (_pmol->vertexCount())
            {
                int index = rxn.addReactantCopy(*_pmol, 0, 0);
                moleculeToAtomIDs.emplace(std::pair<int, std::unordered_map<int, int>>{index, loader.idToAtomIndexMap()});
            }
            else
                throw Error("Empty reactant: %d", id);
            _cdxml_elements.erase(elem_it);
        }
    }

    for (auto id : products_ids)
    {
        auto elem_it = _cdxml_elements.find(id);
        if (elem_it != _cdxml_elements.end())
        {
            loader.loadMoleculeFromFragment(*_pmol, *elem_it->second);
            if (_pmol->vertexCount())
            {
                int index = rxn.addProductCopy(*_pmol, 0, 0);
                moleculeToAtomIDs.emplace(std::pair<int, std::unordered_map<int, int>>{index, loader.idToAtomIndexMap()});
            }
            else
                throw Error("Empty product: %d", id);
            _cdxml_elements.erase(elem_it);
        }
    }

    for (auto id : intermediates_ids)
    {
        auto elem_it = _cdxml_elements.find(id);
        if (elem_it != _cdxml_elements.end())
        {
            loader.loadMoleculeFromFragment(*_pmol, *elem_it->second);
            if (_pmol->vertexCount())
            {
                int index = rxn.addIntermediateCopy(*_pmol, 0, 0);
                moleculeToAtomIDs.emplace(std::pair<int, std::unordered_map<int, int>>{index, loader.idToAtomIndexMap()});
            }
            else
                throw Error("Empty product: %d", id);
            _cdxml_elements.erase(elem_it);
        }
    }

    for (auto id : agents_ids)
    {
        auto elem_it = _cdxml_elements.find(id);
        if (elem_it != _cdxml_elements.end())
        {
            loader.loadMoleculeFromFragment(*_pmol, *elem_it->second);
            if (_pmol->vertexCount())
                rxn.addCatalystCopy(*_pmol, 0, 0);
            else
            {
                for (int i = 0; i < _pmol->meta().getMetaCount(SimpleTextObject::CID); ++i)
                {
                    auto& text = (SimpleTextObject&)_pmol->meta().getMetaObject(SimpleTextObject::CID, i);
                    int idx = rxn.meta().addMetaObject(text.clone());
                    rxn.addSpecialCondition(
                        idx, Rect2f(Vec2f(text.boundingBox().left(), text.boundingBox().top()), Vec2f(text.boundingBox().left(), text.boundingBox().top())));
                }
            }
            _cdxml_elements.erase(elem_it);
        }
    }

    for (auto id : arrows_ids)
    {
        auto elem_it = _cdxml_elements.find(id);
        if (elem_it != _cdxml_elements.end())
        {
            loader.loadMoleculeFromFragment(*_pmol, *elem_it->second);
            rxn.meta().append(_pmol->meta());
            if (_pmol->vertexCount())
                rxn.addUndefinedCopy(*_pmol, 0, 0);
            _cdxml_elements.erase(elem_it);
        }
    }

    for (const auto& bracketID : bracket_ids)
    {
        auto bracketIt = _cdxml_elements.find(bracketID);
        if (bracketIt != _cdxml_elements.end())
        {
            for (auto bracketAttachment = bracketIt->second->firstChildElement(); bracketAttachment->hasContent();
                 bracketAttachment = bracketAttachment->nextSiblingElement())
            {
                int id = -1;
                auto idProperty = bracketAttachment->findProperty("id");
                if (idProperty->hasContent())
                    id = std::stoi(idProperty->value());

                if (id == bracketID)
                {
                    for (auto crossingBond = bracketAttachment->firstChildElement(); crossingBond->hasContent();
                         crossingBond = crossingBond->nextSiblingElement())
                    {
                        int atom = -1;
                        auto atomID = crossingBond->findProperty("InnerAtomID");
                        if (atomID->hasContent())
                            atom = std::stoi(atomID->value());

                        int bond = -1;
                        auto bondID = crossingBond->findProperty("BondID");
                        if (bondID->hasContent())
                            bond = std::stoi(bondID->value());

                        if (bond < 0 && atom < 0)
                            throw Error("No atoms or bonds within the bracket %i", id);

                        int molecule = -1;
                        for (const auto& [moleculeIndex, atomMap] : moleculeToAtomIDs)
                        {
                            for (const auto& [id, index] : atomMap)
                            {
                                if (id == atom)
                                {
                                    molecule = moleculeIndex;
                                    break;
                                }
                            }
                            if (molecule >= 0)
                                break;
                        }

                        if (molecule >= 0)
                        {
                            auto& mol = rxn.getBaseMolecule(molecule);
                            loader.loadBracket(mol, *bracketIt->second, moleculeToAtomIDs[molecule]);
                        }
                        else
                        {
                            throw Error("Couldn't find the molecule with atoms matching the bracket %i", id);
                        }
                    }
                }
            }
        }
    }

    for (const auto& bracketID : bracket_ids)
    {
        auto bracketIt = _cdxml_elements.find(bracketID);
        if (bracketIt != _cdxml_elements.end())
        {
            _cdxml_elements.erase(bracketIt);
        }
    }

    for (const auto& kvp : _cdxml_elements)
    {
        loader.loadMoleculeFromFragment(*_pmol, *kvp.second);
        if (_pmol->vertexCount())
        {
            Array<char> label;
            _pmol->getAtomSymbol(0, label);
            rxn.addUndefinedCopy(*_pmol, 0, 0);
        }
        rxn.meta().append(_pmol->meta());
    }
}
