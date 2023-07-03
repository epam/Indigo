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

#include <time.h>

#include "base_cpp/output.h"
#include "molecule/molfile_saver.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/rxnfile_saver.h"

using namespace indigo;

IMPL_ERROR(RxnfileSaver, "Rxnfile saver");

RxnfileSaver::RxnfileSaver(Output& output) : _output(output)
{
    molfile_saving_mode = MolfileSaver::MODE_AUTO;
    skip_date = false;
    add_stereo_desc = false;
    add_implicit_h = true;
}

RxnfileSaver::~RxnfileSaver()
{
}

void RxnfileSaver::saveBaseReaction(BaseReaction& reaction)
{
    if (reaction.isQueryReaction())
        saveQueryReaction(reaction.asQueryReaction());
    else
        saveReaction(reaction.asReaction());
}

void RxnfileSaver::saveReaction(Reaction& reaction)
{
    _rxn = &reaction;
    _brxn = &reaction;
    _qrxn = 0;
    _saveReaction();
}

void RxnfileSaver::saveQueryReaction(QueryReaction& reaction)
{
    _qrxn = &reaction;
    _brxn = &reaction;
    _rxn = 0;
    _saveReaction();
}

void RxnfileSaver::_saveReaction()
{
    int i;

    if (molfile_saving_mode == MolfileSaver::MODE_3000)
        _v2000 = false;
    else if (molfile_saving_mode == MolfileSaver::MODE_2000)
        _v2000 = true;
    else
    {
        _v2000 = true;

        for (i = _brxn->begin(); i != _brxn->end(); i = _brxn->next(i))
        {
            if (_brxn->getBaseMolecule(i).hasHighlighting())
            {
                _v2000 = false;
                break;
            }
            if (_brxn->getBaseMolecule(i).stereocenters.haveEnhancedStereocenter())
            {
                _v2000 = false;
                break;
            }
        }
    }

    MolfileSaver molfileSaver(_output);
    molfileSaver.mode = _v2000 ? MolfileSaver::MODE_2000 : MolfileSaver::MODE_3000;
    molfileSaver.add_stereo_desc = add_stereo_desc;
    molfileSaver.add_implicit_h = add_implicit_h;

    _writeRxnHeader(*_brxn);

    _writeReactantsHeader();

    for (i = _brxn->reactantBegin(); i < _brxn->reactantEnd(); i = _brxn->reactantNext(i))
    {
        _writeMolHeader();
        _writeMol(molfileSaver, i);
    }

    _writeReactantsFooter();
    _writeProductsHeader();

    for (int i = _brxn->productBegin(); i < _brxn->productEnd(); i = _brxn->productNext(i))
    {
        _writeMolHeader();
        _writeMol(molfileSaver, i);
    }

    for (int i = _brxn->intermediateBegin(); i < _brxn->intermediateEnd(); i = _brxn->intermediateNext(i))
    {
        _writeMolHeader();
        _writeMol(molfileSaver, i);
    }

    _writeProductsFooter();

    if (_brxn->catalystCount() > 0)
    {
        _writeCatalystsHeader();

        for (int i = _brxn->catalystBegin(); i < _brxn->catalystEnd(); i = _brxn->catalystNext(i))
        {
            _writeMolHeader();
            _writeMol(molfileSaver, i);
        }

        _writeCatalystsFooter();
    }

    _writeRxnFooter();
}

void RxnfileSaver::_writeRxnHeader(BaseReaction& reaction)
{
    if (_v2000)
        _output.writeStringCR("$RXN");
    else
        _output.writeStringCR("$RXN V3000");

    struct tm lt;
    if (skip_date)
        memset(&lt, 0, sizeof(lt));
    else
    {
        time_t tm = time(NULL);
        lt = *localtime(&tm);
    }

    if (reaction.name.ptr() != 0)
        _output.printfCR("%s", reaction.name.ptr());
    else
        _output.writeCR();
    _output.printfCR(" -INDIGO- %02d%02d%02d%02d%02d", lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min);
    _output.writeCR();

    if (_v2000)
    {
        if (reaction.catalystCount() > 0)
            _output.printf("%3d%3d%3d\n", reaction.reactantsCount(), reaction.productsCount() + reaction.intermediateCount(), reaction.catalystCount());
        else
            _output.printf("%3d%3d\n", reaction.reactantsCount(), reaction.productsCount() + reaction.intermediateCount());
    }
    else
    {
        if (reaction.catalystCount() > 0)
            _output.printf("M  V30 COUNTS %d %d %d\n", reaction.reactantsCount(), reaction.productsCount() + reaction.intermediateCount(),
                           reaction.catalystCount());
        else
            _output.printf("M  V30 COUNTS %d %d\n", reaction.reactantsCount(), reaction.productsCount() + reaction.intermediateCount());
    }
}

void RxnfileSaver::_writeMolHeader()
{
    if (_v2000)
        _output.writeStringCR("$MOL");
}

void RxnfileSaver::_writeReactantsHeader()
{
    if (!_v2000)
        _output.writeStringCR("M  V30 BEGIN REACTANT");
}

void RxnfileSaver::_writeProductsHeader()
{
    if (!_v2000)
        _output.writeStringCR("M  V30 BEGIN PRODUCT");
}

void RxnfileSaver::_writeCatalystsHeader()
{
    if (!_v2000)
        _output.writeStringCR("M  V30 BEGIN AGENT");
}

void RxnfileSaver::_writeMol(MolfileSaver& saver, int index)
{

    saver.skip_date = skip_date;

    if (_qrxn != 0)
    {
        if (_v2000)
            saver.saveQueryMolecule(_qrxn->getQueryMolecule(index));
        else
            saver.saveQueryCtab3000(_qrxn->getQueryMolecule(index));
    }
    else
    {
        if (_v2000)
            saver.saveMolecule(_rxn->getMolecule(index));
        else
            saver.saveCtab3000(_rxn->getMolecule(index));
    }
}

void RxnfileSaver::_writeReactantsFooter()
{
    if (!_v2000)
        _output.writeStringCR("M  V30 END REACTANT");
}

void RxnfileSaver::_writeProductsFooter()
{
    if (!_v2000)
        _output.writeStringCR("M  V30 END PRODUCT");
}

void RxnfileSaver::_writeCatalystsFooter()
{
    if (!_v2000)
        _output.writeStringCR("M  V30 END AGENT");
}

void RxnfileSaver::_writeRxnFooter()
{
    if (!_v2000)
        _output.writeStringCR("M  END");
}
