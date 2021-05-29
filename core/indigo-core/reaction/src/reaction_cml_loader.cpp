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

#include "reaction/reaction_cml_loader.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "molecule/cml_loader.h"
#include "reaction/reaction.h"
#include "tinyxml.h"

using namespace indigo;

IMPL_ERROR(ReactionCmlLoader, "reaction CML loader");

ReactionCmlLoader::ReactionCmlLoader(Scanner& scanner) : _scanner(scanner)
{
    ignore_bad_valence = false;
}

ReactionCmlLoader::~ReactionCmlLoader()
{
}

void ReactionCmlLoader::loadReaction(Reaction& rxn)
{
    rxn.clear();

    QS_DEF(std::string, buf);
    _scanner.readAll(buf);
    buf.push(0);

    TiXmlDocument xml;

    xml.Parse(buf.ptr());

    if (xml.Error())
        throw Error("XML parsing error: %s", xml.ErrorDesc());

    TiXmlHandle hxml(&xml);
    TiXmlElement* elem;
    TiXmlHandle hroot(0);

    elem = hxml.FirstChild("reaction").Element();
    if (elem == 0)
        elem = hxml.FirstChild("cml").FirstChild("reaction").Element();
    if (elem == 0)
        throw Error("no <reaction>?");
    hroot = TiXmlHandle(elem);

    const char* title = elem->Attribute("title");

    if (title != 0)
        rxn.name.readString(title, true);

    QS_DEF(Molecule, mol);

    elem = hroot.FirstChild("reactantList").FirstChild().Element();
    for (; elem; elem = elem->NextSiblingElement())
    {
        if (strcasecmp(elem->Value(), "molecule") != 0)
            continue;
        TiXmlHandle handle(elem);
        CmlLoader loader(handle);
        loader.stereochemistry_options = stereochemistry_options;
        loader.ignore_bad_valence = ignore_bad_valence;
        loader.loadMolecule(mol);
        rxn.addReactantCopy(mol, 0, 0);
    }

    elem = hroot.FirstChild("productList").FirstChild().Element();
    for (; elem; elem = elem->NextSiblingElement())
    {
        if (strcasecmp(elem->Value(), "molecule") != 0)
            continue;
        TiXmlHandle handle(elem);
        CmlLoader loader(handle);
        loader.stereochemistry_options = stereochemistry_options;
        loader.ignore_bad_valence = ignore_bad_valence;
        loader.loadMolecule(mol);
        rxn.addProductCopy(mol, 0, 0);
    }

    elem = hroot.FirstChild("spectatorList").FirstChild().Element();
    for (; elem; elem = elem->NextSiblingElement())
    {
        if (strcasecmp(elem->Value(), "molecule") != 0)
            continue;
        TiXmlHandle handle(elem);
        CmlLoader loader(handle);
        loader.stereochemistry_options = stereochemistry_options;
        loader.ignore_bad_valence = ignore_bad_valence;
        loader.loadMolecule(mol);
        rxn.addCatalystCopy(mol, 0, 0);
    }
}
