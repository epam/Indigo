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

#include "indigo_savers.h"

#include <ctime>

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/cml_saver.h"
#include "molecule/molecule_cdxml_saver.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/smiles_saver.h"
#include "reaction/canonical_rsmiles_saver.h"
#include "reaction/reaction_cdxml_saver.h"
#include "reaction/reaction_cml_saver.h"
#include "reaction/reaction_json_saver.h"
#include "reaction/rsmiles_saver.h"
#include "reaction/rxnfile_saver.h"
#include <memory>

#include "indigo_io.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"

//
// IndigoSaver
//

IndigoSaver::IndigoSaver(Output& output) : IndigoObject(IndigoObject::SAVER), _output(output)
{
    _closed = false;
    _own_output = 0;
}

IndigoSaver::~IndigoSaver()
{
    close();
}

void IndigoSaver::acquireOutput(Output* output)
{
    if (_own_output)
        delete _own_output;
    _own_output = output;
}

void IndigoSaver::close()
{
    if (_closed)
        return;
    _appendFooter();

    delete _own_output;
    _own_output = 0;

    _closed = true;
}

IndigoSaver* IndigoSaver::create(Output& output, const char* type)
{
    std::unique_ptr<IndigoSaver> saver;
    if (strcasecmp(type, "sdf") == 0)
        saver = std::make_unique<IndigoSdfSaver>(output);
    else if (strcasecmp(type, "smiles") == 0 || strcasecmp(type, "smi") == 0)
        saver = std::make_unique<IndigoSmilesSaver>(output);
    else if (strcasecmp(type, "cml") == 0)
        saver = std::make_unique<IndigoCmlSaver>(output);
    else if (strcasecmp(type, "rdf") == 0)
        saver = std::make_unique<IndigoRdfSaver>(output);
    else
        throw IndigoError("unsupported saver type: '%s'. Supported formats are sdf, smiles, cml, rdf", type);

    saver->_appendHeader();
    return saver.release();
}

void IndigoSaver::appendObject(IndigoObject& object)
{
    if (_closed)
        throw IndigoError("save %s has already been closed", debugInfo());
    _append(object);
}

//
// IndigoSDFSaver
//

void IndigoSdfSaver::appendMolfile(Output& out, IndigoObject& obj)
{
    if (IndigoBaseMolecule::is(obj))
    {
        Indigo& indigo = indigoGetInstance();

        MolfileSaver saver(out);
        indigo.initMolfileSaver(saver);
        saver.saveBaseMolecule(obj.getBaseMolecule());
    }
    else
    {
        throw IndigoError("%s can not be converted to Molfile", obj.debugInfo());
    }
}

void IndigoSdfSaver::append(Output& out, IndigoObject& obj)
{
    appendMolfile(out, obj);

    auto& props = obj.getProperties();

    for (auto i : props.elements())
    {
        out.printf(">  <%s>\n%s\n\n", props.key(i), props.value(i));
    }

    out.printfCR("$$$$");
    out.flush();
}

const char* IndigoSdfSaver::debugInfo() const
{
    return "<SDF saver>";
}

void IndigoSdfSaver::_append(IndigoObject& object)
{
    append(_output, object);
}

CEXPORT int indigoSdfAppend(int output, int molecule)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(molecule);
        Output& out = IndigoOutput::get(self.getObject(output));
        IndigoSdfSaver::append(out, obj);
        return 1;
    }
    INDIGO_END(-1);
}

//
// IndigoSmilesSaver
//

void IndigoSmilesSaver::generateSmiles(IndigoObject& obj, Array<char>& out_buffer, SmilesSaver::SMILES_MODE smiles_format)
{
    ArrayOutput output(out_buffer);
    if (IndigoBaseMolecule::is(obj))
    {
        BaseMolecule& mol = obj.getBaseMolecule();
        SmilesSaver saver(output);
        saver.chemaxon = (smiles_format == SmilesSaver::SMILES_MODE::SMILES_CHEMAXON);

        if (mol.isQueryMolecule())
            saver.saveQueryMolecule(mol.asQueryMolecule());
        else
            saver.saveMolecule(mol.asMolecule());
    }
    else if (IndigoBaseReaction::is(obj))
    {
        BaseReaction& rxn = obj.getBaseReaction();
        RSmilesSaver saver(output);
        saver.chemaxon = (smiles_format == SmilesSaver::SMILES_MODE::SMILES_CHEMAXON);

        if (rxn.isQueryReaction())
            saver.saveQueryReaction(rxn.asQueryReaction());
        else
            saver.saveReaction(rxn.asReaction());
    }
    else
        throw IndigoError("%s can not be converted to SMILES", obj.debugInfo());
    out_buffer.push(0);
}

void IndigoSmilesSaver::generateSmarts(IndigoObject& obj, Array<char>& out_buffer)
{
    ArrayOutput output(out_buffer);

    if (IndigoBaseMolecule::is(obj))
    {
        BaseMolecule& mol = obj.getBaseMolecule();

        SmilesSaver saver(output);
        saver.smarts_mode = true;
        if (mol.isQueryMolecule())
        {
            saver.saveQueryMolecule(mol.asQueryMolecule());
        }
        else
        {
            Array<char> mol_out_buffer;
            ArrayOutput mol_output(mol_out_buffer);
            MolfileSaver saver_tmp(mol_output);
            saver_tmp.saveMolecule(mol.asMolecule());
            mol_out_buffer.push(0);
            BufferScanner sc(mol_out_buffer);
            MolfileLoader loader_tmp(sc);
            QueryMolecule qmol;
            loader_tmp.loadQueryMolecule(qmol);
            saver.saveQueryMolecule(qmol);
        }
    }
    else if (IndigoBaseReaction::is(obj))
    {
        BaseReaction& rxn = obj.getBaseReaction();

        RSmilesSaver saver(output);
        saver.smarts_mode = true;

        if (rxn.isQueryReaction())
            saver.saveQueryReaction(rxn.asQueryReaction());
        else
            saver.saveReaction(rxn.asReaction());
    }
    else
        throw IndigoError("%s can not be converted to SMARTS", obj.debugInfo());
    out_buffer.push(0);
}

void IndigoSmilesSaver::append(Output& output, IndigoObject& object)
{
    QS_DEF(Array<char>, tmp_buffer);
    IndigoSmilesSaver::generateSmiles(object, tmp_buffer);
    output.writeString(tmp_buffer.ptr());

    Indigo& indigo = indigoGetInstance();
    if (indigo.smiles_saving_write_name)
    {
        output.writeString(" ");
        output.writeString(object.getName());
    }
    output.writeCR();
    output.flush();
}

const char* IndigoSmilesSaver::debugInfo() const
{
    return "<smiles saver>";
}

void IndigoSmilesSaver::_append(IndigoObject& object)
{
    append(_output, object);
}

CEXPORT int indigoSmilesAppend(int output, int item)
{
    INDIGO_BEGIN
    {
        Output& out = IndigoOutput::get(self.getObject(output));
        IndigoObject& obj = self.getObject(item);
        IndigoSmilesSaver::append(out, obj);
        out.flush();
        return 1;
    }
    INDIGO_END(-1);
}

//
// IndigoCanonicalSmilesSaver
//

void IndigoCanonicalSmilesSaver::generateSmiles(IndigoObject& obj, Array<char>& out_buffer)
{
    ArrayOutput output(out_buffer);

    if (IndigoBaseMolecule::is(obj))
    {
        BaseMolecule& mol = obj.getBaseMolecule();

        CanonicalSmilesSaver saver(output);

        if (mol.isQueryMolecule())
            saver.saveQueryMolecule(mol.asQueryMolecule());
        else
            saver.saveMolecule(mol.asMolecule());
    }
    else if (IndigoBaseReaction::is(obj))
    {
        BaseReaction& rxn = obj.getBaseReaction();

        CanonicalRSmilesSaver saver(output);

        if (rxn.isQueryReaction())
            saver.saveQueryReaction(rxn.asQueryReaction());
        else
            saver.saveReaction(rxn.asReaction());
    }
    else
        throw IndigoError("%s can not be converted to SMILES", obj.debugInfo());
    out_buffer.push(0);
}

void IndigoCanonicalSmilesSaver::generateSmarts(IndigoObject& obj, Array<char>& out_buffer)
{
    ArrayOutput output(out_buffer);

    if (IndigoBaseMolecule::is(obj))
    {
        BaseMolecule& mol = obj.getBaseMolecule();

        CanonicalSmilesSaver saver(output);
        saver.smarts_mode = true;

        if (mol.isQueryMolecule())
            saver.saveQueryMolecule(mol.asQueryMolecule());
        else
            saver.saveMolecule(mol.asMolecule());
    }
    else if (IndigoBaseReaction::is(obj))
    {
        BaseReaction& rxn = obj.getBaseReaction();

        CanonicalRSmilesSaver saver(output);
        saver.smarts_mode = true;

        if (rxn.isQueryReaction())
            saver.saveQueryReaction(rxn.asQueryReaction());
        else
            saver.saveReaction(rxn.asReaction());
    }
    else
        throw IndigoError("%s can not be converted to SMARTS", obj.debugInfo());
    out_buffer.push(0);
}

const char* IndigoCanonicalSmilesSaver::debugInfo() const
{
    return "<smiles saver>";
}

//
// IndigoCMLSaver
//

void IndigoCmlSaver::append(Output& out, IndigoObject& obj)
{
    if (IndigoBaseMolecule::is(obj))
    {
        CmlSaver saver(out);
        saver.skip_cml_tag = true;

        BaseMolecule& mol = obj.getBaseMolecule();

        if (mol.isQueryMolecule())
            saver.saveQueryMolecule(mol.asQueryMolecule());
        else
            saver.saveMolecule(mol.asMolecule());
    }
    else if (IndigoBaseReaction::is(obj))
    {
        ReactionCmlSaver saver(out);
        saver.skip_cml_tag = true;
        saver.saveReaction(obj.getReaction());
    }
    else
        throw IndigoError("%s can not be saved to CML", obj.debugInfo());
}

void IndigoCmlSaver::appendHeader(Output& out)
{
    out.printf("<?xml version=\"1.0\" ?>\n");
    out.printf("<cml>\n");
}

void IndigoCmlSaver::appendFooter(Output& out)
{
    out.printf("</cml>\n");
}

const char* IndigoCmlSaver::debugInfo() const
{
    return "<CML saver>";
}

void IndigoCmlSaver::_append(IndigoObject& object)
{
    append(_output, object);
}

void IndigoCmlSaver::_appendHeader()
{
    appendHeader(_output);
}

void IndigoCmlSaver::_appendFooter()
{
    appendFooter(_output);
}

CEXPORT int indigoCmlHeader(int output)
{
    INDIGO_BEGIN
    {
        Output& out = IndigoOutput::get(self.getObject(output));
        IndigoCmlSaver::appendHeader(out);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCmlFooter(int output)
{
    INDIGO_BEGIN
    {
        Output& out = IndigoOutput::get(self.getObject(output));
        IndigoCmlSaver::appendFooter(out);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCmlAppend(int output, int item)
{
    INDIGO_BEGIN
    {
        Output& out = IndigoOutput::get(self.getObject(output));
        IndigoObject& obj = self.getObject(item);
        IndigoCmlSaver::append(out, obj);
        return 1;
    }
    INDIGO_END(-1);
}

//
// IndigoRDFSaver
//

void IndigoRdfSaver::appendRXN(Output& out, IndigoObject& obj)
{
    Indigo& indigo = indigoGetInstance();

    RxnfileSaver saver(out);
    indigo.initRxnfileSaver(saver);
    saver.saveBaseReaction(obj.getBaseReaction());
}

void IndigoRdfSaver::append(Output& out, IndigoObject& obj)
{
    if (IndigoBaseMolecule::is(obj))
    {
        out.writeStringCR("$MFMT");
        IndigoSdfSaver::appendMolfile(out, obj);
    }
    else if (IndigoBaseReaction::is(obj))
    {
        out.writeStringCR("$RFMT");
        IndigoRdfSaver::appendRXN(out, obj);
    }
    else
        throw IndigoError("%s can not be saved to RDF", obj.debugInfo());

    auto& props = obj.getProperties();

    for (auto i : props.elements())
    {
        out.printf("$DTYPE %s\n$DATUM %s\n", props.key(i), props.value(i));
    }
}

void IndigoRdfSaver::appendHeader(Output& out)
{
    Indigo& indigo = indigoGetInstance();

    out.printfCR("$RDFILE 1");
    struct tm lt;
    if (indigo.molfile_saving_skip_date)
        memset(&lt, 0, sizeof(lt));
    else
    {
        time_t tm = time(NULL);
        lt = *localtime(&tm);
    }
    out.printfCR("$DATM    %02d/%02d/%02d %02d:%02d", lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min);
}

const char* IndigoRdfSaver::debugInfo() const
{
    return "<smiles saver>";
}

void IndigoRdfSaver::_append(IndigoObject& object)
{
    append(_output, object);
}

void IndigoRdfSaver::_appendHeader()
{
    appendHeader(_output);
}

CEXPORT int indigoRdfHeader(int output)
{
    INDIGO_BEGIN
    {
        Output& out = IndigoOutput::get(self.getObject(output));
        IndigoRdfSaver::appendHeader(out);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoRdfAppend(int output, int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        Output& out = IndigoOutput::get(self.getObject(output));
        IndigoRdfSaver::append(out, obj);
        return 1;
    }
    INDIGO_END(-1);
}

//
// Saving functions
//
CEXPORT int indigoCreateSaver(int output, const char* format)
{
    INDIGO_BEGIN
    {
        Output& out = IndigoOutput::get(self.getObject(output));
        return self.addObject(IndigoSaver::create(out, format));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCreateFileSaver(const char* filename, const char* format)
{
    INDIGO_BEGIN
    {
        std::unique_ptr<FileOutput> output = std::make_unique<FileOutput>(self.filename_encoding, filename);
        std::unique_ptr<IndigoSaver> saver(IndigoSaver::create(*output, format));
        saver->acquireOutput(output.release());
        return self.addObject(saver.release());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSaveMolfile(int molecule, int output)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(molecule);
        Output& out = IndigoOutput::get(self.getObject(output));
        IndigoSdfSaver::appendMolfile(out, obj);
        out.flush();
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSaveJson(int item, int output)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        Output& out = IndigoOutput::get(self.getObject(output));
        if (IndigoBaseMolecule::is(obj))
        {
            MoleculeJsonSaver saver(out);
            self.initMoleculeJsonSaver(saver);
            BaseMolecule& mol = obj.getBaseMolecule();
            saver.saveMolecule(mol);
            out.flush();
            return 1;
        }
        else if (IndigoBaseReaction::is(obj))
        {
            ReactionJsonSaver saver(out);
            self.initReactionJsonSaver(saver);
            BaseReaction& rxn = obj.getBaseReaction();
            saver.saveReaction(rxn);
            out.flush();
            return 1;
        }
        throw IndigoError("indigoSaveJson(): expected molecule or reaction, got %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSaveCml(int item, int output)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        Output& out = IndigoOutput::get(self.getObject(output));
        if (IndigoBaseMolecule::is(obj))
        {
            CmlSaver saver(out);

            BaseMolecule& mol = obj.getBaseMolecule();

            if (mol.isQueryMolecule())
                saver.saveQueryMolecule(mol.asQueryMolecule());
            else
                saver.saveMolecule(mol.asMolecule());
            out.flush();
            return 1;
        }
        if (IndigoBaseReaction::is(obj))
        {
            Reaction& rxn = obj.getReaction();
            ReactionCmlSaver saver(out);

            saver.saveReaction(rxn);
            out.flush();
            return 1;
        }
        throw IndigoError("indigoSaveCml(): expected molecule or reaction, got %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSaveMDLCT(int item, int output)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);

        if (IndigoBaseMolecule::is(obj))
            IndigoSdfSaver::appendMolfile(out, obj);
        else if (IndigoBaseReaction::is(obj))
            IndigoRdfSaver::appendRXN(out, obj);
        Output& out2 = IndigoOutput::get(self.getObject(output));
        BufferScanner scanner(buf);
        QS_DEF(Array<char>, line);
        while (!scanner.isEOF())
        {
            scanner.readLine(line, false);
            if (line.size() > 255)
                throw IndigoError("indigoSaveMDLCT: line too big (%d)", line.size());
            out2.writeChar(line.size());
            out2.writeArray(line);
        }
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSaveRxnfile(int reaction, int output)
{
    INDIGO_BEGIN
    {
        BaseReaction& rxn = self.getObject(reaction).getBaseReaction();
        Output& out = IndigoOutput::get(self.getObject(output));
        RxnfileSaver saver(out);
        self.initRxnfileSaver(saver);
        if (rxn.isQueryReaction())
            saver.saveQueryReaction(rxn.asQueryReaction());
        else
            saver.saveReaction(rxn.asReaction());
        out.flush();
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAppend(int saver_id, int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);
        IndigoObject& saver_obj = self.getObject(saver_id);
        if (saver_obj.type != IndigoObject::SAVER)
            throw IndigoError("indigoAppend() is only applicable to saver objects. %s object was passed as a saver", saver_obj.debugInfo());
        IndigoSaver& saver = (IndigoSaver&)saver_obj;
        saver.appendObject(obj);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSaveCdxml(int item, int output)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        Output& out = IndigoOutput::get(self.getObject(output));

        if (IndigoBaseMolecule::is(obj))
        {
            MoleculeCdxmlSaver saver(out);
            if (obj.type == IndigoObject::MOLECULE)
            {
                Molecule& mol = obj.getMolecule();
                saver.saveMolecule(mol);
            }
            else if (obj.type == IndigoObject::QUERY_MOLECULE)
            {
                QueryMolecule& mol = obj.getQueryMolecule();
                saver.saveMolecule(mol);
            }
            out.flush();
            return 1;
        }
        if (IndigoBaseReaction::is(obj))
        {
            ReactionCdxmlSaver saver(out);
            if (obj.type == IndigoObject::REACTION)
            {
                Reaction& rxn = obj.getReaction();
                saver.saveReaction(rxn);
            }
            else if (obj.type == IndigoObject::QUERY_REACTION)
            {
                QueryReaction& rxn = obj.getQueryReaction();
                saver.saveReaction(rxn);
            }
            out.flush();
            return 1;
        }
        throw IndigoError("indigoSaveCdxml(): expected molecule or reaction, got %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSaveCdxm(int item, int output)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        Output& out = IndigoOutput::get(self.getObject(output));

        if (IndigoBaseMolecule::is(obj))
        {
            MoleculeCdxmlSaver saver(out, true);
            if (obj.type == IndigoObject::MOLECULE)
            {
                Molecule& mol = obj.getMolecule();
                saver.saveMolecule(mol);
            }
            else if (obj.type == IndigoObject::QUERY_MOLECULE)
            {
                QueryMolecule& mol = obj.getQueryMolecule();
                saver.saveMolecule(mol);
            }
            out.flush();
            return 1;
        }
        if (IndigoBaseReaction::is(obj))
        {
            ReactionCdxmlSaver saver(out, true);
            if (obj.type == IndigoObject::REACTION)
            {
                Reaction& rxn = obj.getReaction();
                saver.saveReaction(rxn);
            }
            else if (obj.type == IndigoObject::QUERY_REACTION)
            {
                QueryReaction& rxn = obj.getQueryReaction();
                saver.saveReaction(rxn);
            }
            out.flush();
            return 1;
        }
        throw IndigoError("indigoSaveCdxml(): expected molecule or reaction, got %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}
