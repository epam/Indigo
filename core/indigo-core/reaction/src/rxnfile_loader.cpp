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

#include "reaction/rxnfile_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/molfile_loader.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(RxnfileLoader, "RXN loader");

RxnfileLoader::RxnfileLoader(Scanner& scanner) : _scanner(scanner)
{
    _v3000 = false;
    ignore_noncritical_query_features = false;
    ignore_no_chiral_flag = false;
    ignore_bad_valence = false;
}

RxnfileLoader::~RxnfileLoader()
{
}

void RxnfileLoader::loadReaction(Reaction& reaction, MonomerTemplateLibrary* monomer_lib)
{
    _rxn = &reaction;
    _brxn = &reaction;
    _qrxn = 0;
    _loadReaction(monomer_lib);
}

void RxnfileLoader::loadQueryReaction(QueryReaction& rxn, MonomerTemplateLibrary* monomer_lib)
{
    _rxn = 0;
    _brxn = &rxn;
    _qrxn = &rxn;
    _loadReaction();
}

void indigo::RxnfileLoader::loadReaction(Reaction& reaction, PropertiesMap& props, MonomerTemplateLibrary* monomer_lib)
{
    loadReaction(reaction, monomer_lib);
    reaction.properties().copy(props);
}

void indigo::RxnfileLoader::loadQueryReaction(QueryReaction& reaction, PropertiesMap& props, MonomerTemplateLibrary* monomer_lib)
{
    loadQueryReaction(reaction, monomer_lib);
    reaction.properties().copy(props);
}

void RxnfileLoader::_loadReaction(MonomerTemplateLibrary* monomer_lib)
{
    _brxn->clear();

    MolfileLoader molfileLoader(_scanner, monomer_lib);

    molfileLoader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
    molfileLoader.stereochemistry_options = stereochemistry_options;
    molfileLoader.ignore_noncritical_query_features = ignore_noncritical_query_features;
    molfileLoader.ignore_no_chiral_flag = ignore_no_chiral_flag;
    molfileLoader.ignore_bad_valence = ignore_bad_valence;
    _readRxnHeader();

    if (_v3000)
        _readReactantsHeaderV3000();
    for (int i = 0; i < _n_reactants; i++)
    {
        int index = _brxn->addReactant();

        if (!_v3000)
            _readMol2000Header();
        _readMol(molfileLoader, index);
    }
    if (_v3000)
        _readReactantsFooterV3000();

    if (_v3000)
        _readProductsHeaderV3000();
    for (int i = 0; i < _n_products; i++)
    {
        int index = _brxn->addProduct();

        if (!_v3000)
            _readMol2000Header();
        _readMol(molfileLoader, index);
    }
    if (_v3000)
        _readProductsFooterV3000();

    if (_n_catalysts > 0)
    {
        if (_v3000)
            _readCatalystsHeaderV3000();
        for (int i = 0; i < _n_catalysts; i++)
        {
            int index = _brxn->addCatalyst();

            if (!_v3000)
                _readMol2000Header();
            _readMol(molfileLoader, index);
        }
        if (_v3000)
            _readCatalystsFooterV3000();
    }
}

void RxnfileLoader::_readRxnHeader()
{

    QS_DEF(Array<char>, header);

    _scanner.readLine(header, true);

    if (strcmp(header.ptr(), "$RXN") == 0)
        _v3000 = false;
    else if (strcmp(header.ptr(), "$RXN V3000") == 0)
        _v3000 = true;
    else
        throw Error("bad header %s", header.ptr());

    _scanner.readLine(_brxn->name, true);
    _scanner.skipLine();
    _scanner.skipLine();

    if (_v3000)
    {
        _scanner.skip(14); // "M  V30 COUNTS "
        _scanner.readLine(header, true);
        int n = sscanf(header.ptr(), "%d %d %d", &_n_reactants, &_n_products, &_n_catalysts);
        if (n < 2)
            throw Error("error reading counts: %s", header.ptr());
        if (n == 2)
            _n_catalysts = 0;
    }
    else
    {
        _scanner.readLine(header, false);
        BufferScanner strscan(header);

        _n_reactants = strscan.readIntFix(3);
        _n_products = strscan.readIntFix(3);
        try
        {
            _n_catalysts = strscan.readIntFix(3);
        }
        catch (Scanner::Error)
        {
            _n_catalysts = 0;
        }
    }
}

void RxnfileLoader::_readProductsHeaderV3000()
{
    QS_DEF(Array<char>, header);

    _scanner.readLine(header, true);
    if (strcmp(header.ptr(), "M  V30 BEGIN PRODUCT") != 0)
    {
        throw Error("bad products header: %s", header.ptr());
    }
}

void RxnfileLoader::_readReactantsHeaderV3000()
{
    QS_DEF(Array<char>, header);

    _scanner.readLine(header, true);
    if (strcmp(header.ptr(), "M  V30 BEGIN REACTANT") != 0)
    {
        throw Error("bad reactants header: %s", header.ptr());
    }
}

void RxnfileLoader::_readCatalystsHeaderV3000()
{
    QS_DEF(Array<char>, header);

    _scanner.readLine(header, true);
    if (strcmp(header.ptr(), "M  V30 BEGIN AGENT") != 0)
    {
        throw Error("bad catalysts header: %s", header.ptr());
    }
}

void RxnfileLoader::_readMol2000Header()
{

    QS_DEF(Array<char>, header);

    _scanner.readLine(header, true);
    if (strcmp(header.ptr(), "$MOL") != 0)
        throw Error("bad molecule header: %s", header.ptr());
}

void RxnfileLoader::_readReactantsFooterV3000()
{
    QS_DEF(Array<char>, footer);

    _scanner.readLine(footer, true);

    if (strcmp(footer.ptr(), "M  V30 END REACTANT") != 0)
        throw Error("bad reactants footer: %s", footer.ptr());
}

void RxnfileLoader::_readProductsFooterV3000()
{
    QS_DEF(Array<char>, footer);

    _scanner.readLine(footer, true);

    if (strcmp(footer.ptr(), "M  V30 END PRODUCT") != 0)
        throw Error("bad products footer: %s", footer.ptr());
}

void RxnfileLoader::_readCatalystsFooterV3000()
{
    QS_DEF(Array<char>, footer);

    _scanner.readLine(footer, true);

    if (strcmp(footer.ptr(), "M  V30 END AGENT") != 0)
        throw Error("bad agents footer: %s", footer.ptr());
}

void RxnfileLoader::_readMol(MolfileLoader& loader, int index)
{

    if (_qrxn != 0)
    {
        if (_v3000)
            loader.loadQueryMolBlock3000(_qrxn->getQueryMolecule(index));
        else
            loader.loadQueryMolecule(_qrxn->getQueryMolecule(index));
    }
    else
    {
        if (_v3000)
            loader.loadMolBlock3000(_rxn->getMolecule(index));
        else
            loader.loadMolecule(_rxn->getMolecule(index));
    }
}
