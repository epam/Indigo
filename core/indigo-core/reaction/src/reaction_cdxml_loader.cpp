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
#include "molecule/ket_commons.h"
#include "molecule/molecule_cdxml_loader.h"
#include "reaction/reaction.h"
#include "reaction/reaction_cdxml_loader.h"

using namespace indigo;
using namespace tinyxml2;

IMPL_ERROR(ReactionCdxmlLoader, "reaction CDXML loader");

ReactionCdxmlLoader::ReactionCdxmlLoader(Scanner& scanner) : _scanner(scanner)
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

void ReactionCdxmlLoader::_parseStep(const XMLAttribute* pAttr)
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
    MoleculeCdxmlLoader::applyDispatcher(pAttr, cdxml_dispatcher);
}

void ReactionCdxmlLoader::loadReaction(BaseReaction& rxn)
{
    _initReaction(rxn);
    QS_DEF(Array<char>, buf);
    _scanner.readAll(buf);
    buf.push(0);
    XMLDocument xml;
    xml.Parse(buf.ptr());

    if (xml.Error())
        throw Error("XML parsing error: %s", xml.ErrorStr());

    MoleculeCdxmlLoader loader(_scanner);
    loader.parseCDXMLAttributes(xml.RootElement()->FirstAttribute());

    for (auto pPageElem = xml.RootElement()->FirstChildElement(); pPageElem; pPageElem = pPageElem->NextSiblingElement())
    {
        if (std::string(pPageElem->Value()).compare("page") == 0)
        {
            for (auto pCdxmlElem = pPageElem->FirstChildElement(); pCdxmlElem; pCdxmlElem = pCdxmlElem->NextSiblingElement())
            {
                if (std::string(pCdxmlElem->Value()).compare("scheme") == 0)
                {
                    for (auto pSchemeElement = pCdxmlElem->FirstChildElement(); pSchemeElement; pSchemeElement = pSchemeElement->NextSiblingElement())
                        if (std::string(pSchemeElement->Value()).compare("step") == 0)
                            _parseStep(pSchemeElement->FirstAttribute());
                }
                else
                {
                    auto pid = pCdxmlElem->FindAttribute("id");
                    if (pid)
                        _cdxml_elements.emplace(atoi(pid->Value()), pCdxmlElem);
                }
            }
        }
    }

    for (auto id : reactants_ids)
    {
        auto elem_it = _cdxml_elements.find(id);
        if (elem_it != _cdxml_elements.end())
        {
            loader.loadMoleculeFromFragment(*_pmol, elem_it->second);
            if (_pmol->vertexCount())
                rxn.addReactantCopy(*_pmol, 0, 0);
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
            loader.loadMoleculeFromFragment(*_pmol, elem_it->second);
            if (_pmol->vertexCount())
                rxn.addProductCopy(*_pmol, 0, 0);
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
            loader.loadMoleculeFromFragment(*_pmol, elem_it->second);
            if (_pmol->vertexCount())
                rxn.addIntermediateCopy(*_pmol, 0, 0);
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
            loader.loadMoleculeFromFragment(*_pmol, elem_it->second);
            if (_pmol->vertexCount())
                rxn.addCatalystCopy(*_pmol, 0, 0);
            else
            {
                for (int i = 0; i < _pmol->meta().getMetaCount(KETTextObject::CID); ++i)
                {
                    auto& text = (KETTextObject&)_pmol->meta().getMetaObject(KETTextObject::CID, i);
                    int idx = rxn.meta().addMetaObject(text.clone());
                    rxn.addSpecialCondition(idx, Rect2f(Vec2f(text._pos.x, text._pos.y), Vec2f(text._pos.x, text._pos.y)));
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
            loader.loadMoleculeFromFragment(*_pmol, elem_it->second);
            rxn.meta().append(_pmol->meta());
            _cdxml_elements.erase(elem_it);
        }
    }

    for (const auto& kvp : _cdxml_elements)
    {
        loader.loadMoleculeFromFragment(*_pmol, kvp.second);
        rxn.meta().append(_pmol->meta());
    }
}
