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

#include "reaction/reaction_cml_saver.h"
#include "base_cpp/output.h"
#include "molecule/cml_saver.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(ReactionCmlSaver, "reaction CML saver");

ReactionCmlSaver::ReactionCmlSaver(Output& output) : _output(output)
{
    skip_cml_tag = false;
}

ReactionCmlSaver::~ReactionCmlSaver()
{
}

void ReactionCmlSaver::saveReaction(BaseReaction& rxn)
{
    if (!skip_cml_tag)
    {
        _output.printf("<?xml version=\"1.0\" ?>\n");
        _output.printf("<cml>\n");
    }

    if (rxn.name.ptr() != 0)
    {
        if (strchr(rxn.name.ptr(), '\"') != NULL)
            throw Error("can not save reaction with '\"' in title");
        _output.printf("<reaction title=\"%s\">\n", rxn.name.ptr());
    }
    else
        _output.printf("<reaction>\n");

    int i;

    CmlSaver molsaver(_output);

    molsaver.skip_cml_tag = true;

    if (rxn.reactantsCount() > 0)
    {
        _output.printf("<reactantList>\n");
        for (i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
            molsaver.saveMolecule(rxn.getBaseMolecule(i).asMolecule());
        _output.printf("</reactantList>\n");
    }
    if (rxn.productsCount() > 0)
    {
        _output.printf("<productList>\n");
        for (i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
            molsaver.saveMolecule(rxn.getBaseMolecule(i).asMolecule());
        _output.printf("</productList>\n");
    }
    if (rxn.catalystCount() > 0)
    {
        _output.printf("<spectatorList>\n");
        for (i = rxn.catalystBegin(); i != rxn.catalystEnd(); i = rxn.catalystNext(i))
            molsaver.saveMolecule(rxn.getBaseMolecule(i).asMolecule());
        _output.printf("</spectatorList>\n");
    }
    _output.printf("</reaction>\n");
    if (!skip_cml_tag)
        _output.printf("</cml>\n");
}
