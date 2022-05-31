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

#include "molecule/molecule_auto_loader.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "gzip/gzip_scanner.h"
#include "molecule/cml_loader.h"
#include "molecule/icm_loader.h"
#include "molecule/icm_saver.h"
#include "molecule/inchi_wrapper.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdx_loader.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/molecule_name_parser.h"
#include "molecule/molfile_loader.h"
#include "molecule/query_molecule.h"
#include "molecule/sdf_loader.h"
#include "molecule/smiles_loader.h"

using namespace indigo;

void MoleculeAutoLoader::_init()
{
    stereochemistry_options.reset();
    treat_x_as_pseudoatom = false;
    ignore_closing_bond_direction_mismatch = false;
    ignore_noncritical_query_features = false;
    skip_3d_chirality = false;
    ignore_cistrans_errors = false;
    ignore_no_chiral_flag = false;
    ignore_bad_valence = false;
    treat_stereo_as = 0;
}

IMPL_ERROR(MoleculeAutoLoader, "molecule auto loader");

MoleculeAutoLoader::MoleculeAutoLoader(Scanner& scanner)
{
    _scanner = &scanner;
    _own_scanner = false;
    _init();
}

MoleculeAutoLoader::MoleculeAutoLoader(const Array<char>& arr)
{
    _scanner = new BufferScanner(arr);
    _own_scanner = true;
    _init();
}

MoleculeAutoLoader::MoleculeAutoLoader(const char* str)
{
    _scanner = new BufferScanner(str);
    _own_scanner = true;
    _init();
}

MoleculeAutoLoader::~MoleculeAutoLoader()
{
    if (_own_scanner)
        delete _scanner;
}

void MoleculeAutoLoader::loadMolecule(Molecule& mol)
{
    _loadMolecule(mol, false);
    mol.setIgnoreBadValenceFlag(ignore_bad_valence);
}

void MoleculeAutoLoader::loadQueryMolecule(QueryMolecule& qmol)
{
    _loadMolecule(qmol, true);
}

bool MoleculeAutoLoader::tryMDLCT(Scanner& scanner, Array<char>& outbuf)
{
    long long pos = scanner.tell();
    bool endmark = false;
    QS_DEF(Array<char>, curline);

    outbuf.clear();
    while (!scanner.isEOF())
    {
        int len = scanner.readByte();

        if (len > 90) // Molfiles and Rxnfiles actually have 80 characters limit
        {
            scanner.seek(pos, SEEK_SET);
            // Garbage after endmark means end of data.
            // (See the note below about endmarks)
            if (endmark)
                return true;
            return false;
        }

        curline.clear();

        while (len-- > 0)
        {
            if (scanner.isEOF())
            {
                scanner.seek(pos, SEEK_SET);
                return false;
            }
            int c = scanner.readChar();
            curline.push(c);
        }

        curline.push(0);

        if (endmark)
        {
            // We can not properly read the data to the end as there
            // is often garbage after the actual MDLCT data.
            // Instead, we are doing this lousy check:
            // "M  END" or "$END MOL" can be followed only
            // by "$END CTAB" (in Markush queries), or
            // by "$MOL" (in Rxnfiles). Otherwise, this
            // is actually the end of data.
            if (strcmp(curline.ptr(), "$END CTAB") != 0 && strcmp(curline.ptr(), "$MOL") != 0)
            {
                scanner.seek(pos, SEEK_SET);
                return true;
            }
        }

        if (strcmp(curline.ptr(), "M  END") == 0)
            endmark = true;
        else if (strcmp(curline.ptr(), "$END MOL") == 0)
            endmark = true;
        else
            endmark = false;

        outbuf.appendString(curline.ptr(), false);
        outbuf.push('\n');
    }
    scanner.seek(pos, SEEK_SET);
    // It happened once that a valid Molfile had successfully
    // made its way through the above while() cycle, and thus
    // falsely recognized as MDLCT. To fight this case, we include
    // here a check that the last line was actually an endmark
    return endmark;
}

void MoleculeAutoLoader::readAllDataToString(Scanner& scanner, Array<char>& dataBuf)
{
    // check GZip format
    if (scanner.length() >= 2)
    {
        byte id[2];
        long long pos = scanner.tell();

        scanner.readCharsFix(2, (char*)id);
        scanner.seek(pos, SEEK_SET);

        if (id[0] == 0x1f && id[1] == 0x8b)
        {
            GZipScanner gzscanner(scanner);
            gzscanner.readAll(dataBuf);
            dataBuf.push('\0');

            return;
        }
    }

    scanner.readAll(dataBuf);
    dataBuf.push('\0');
}

void MoleculeAutoLoader::_loadMolecule(BaseMolecule& mol, bool query)
{
    properties.clear();

    // check for GZip format
    if (_scanner->length() >= 2LL)
    {
        byte id[2];
        long long pos = _scanner->tell();

        _scanner->readCharsFix(2, (char*)id);
        _scanner->seek(pos, SEEK_SET);

        if (id[0] == 0x1f && id[1] == 0x8b)
        {
            GZipScanner gzscanner(*_scanner);
            QS_DEF(Array<char>, buf);

            gzscanner.readAll(buf);
            MoleculeAutoLoader loader2(buf);

            loader2.stereochemistry_options = stereochemistry_options;
            loader2.ignore_noncritical_query_features = ignore_noncritical_query_features;
            loader2.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
            loader2.skip_3d_chirality = skip_3d_chirality;
            loader2.ignore_no_chiral_flag = ignore_no_chiral_flag;
            loader2.treat_stereo_as = treat_stereo_as;

            if (query)
                loader2.loadQueryMolecule((QueryMolecule&)mol);
            else
                loader2.loadMolecule((Molecule&)mol);

            return;
        }
    }

    // check for MDLCT format
    {
        QS_DEF(Array<char>, buf);
        if (tryMDLCT(*_scanner, buf))
        {
            BufferScanner scanner2(buf);
            MolfileLoader loader(scanner2);
            loader.stereochemistry_options = stereochemistry_options;
            loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
            loader.skip_3d_chirality = skip_3d_chirality;
            loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
            loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
            loader.treat_stereo_as = treat_stereo_as;

            if (query)
                loader.loadQueryMolecule((QueryMolecule&)mol);
            else
                loader.loadMolecule((Molecule&)mol);
            return;
        }
    }

    // check for ICM format
    if (!query && _scanner->length() >= 4LL)
    {
        char id[3];
        long long pos = _scanner->tell();

        _scanner->readCharsFix(3, id);
        _scanner->seek(pos, SEEK_SET);
        if (IcmSaver::checkVersion(id))
        {
            if (query)
                throw Error("cannot load query molecule from ICM format");

            IcmLoader loader(*_scanner);
            loader.loadMolecule((Molecule&)mol);
            return;
        }
    }

    // check for CML format
    {
        long long pos = _scanner->tell();
        _scanner->skipSpace();

        if (_scanner->lookNext() == '<')
        {
            if (_scanner->findWord("<molecule"))
            {
                CmlLoader loader(*_scanner);
                loader.stereochemistry_options = stereochemistry_options;
                if (query)
                    loader.loadQueryMolecule((QueryMolecule&)mol);
                else
                    loader.loadMolecule((Molecule&)mol);
                return;
            }
        }

        _scanner->seek(pos, SEEK_SET);
    }

    // check for CDXML format
    {
        long long pos = _scanner->tell();
        _scanner->skipSpace();
        if (_scanner->lookNext() == '<' && _scanner->findWord("CDXML"))
        {
            _scanner->seek(pos, SEEK_SET);
            MoleculeCdxmlLoader loader(*_scanner);
            loader.stereochemistry_options = stereochemistry_options;
            loader.loadMolecule(mol);
            return;
        }
        _scanner->seek(pos, SEEK_SET);
    }

    // check json format
    {
        long long pos = _scanner->tell();
        _scanner->skipSpace();
        if (_scanner->lookNext() == '{')
        {
            if (_scanner->findWord("root") && _scanner->findWord("nodes"))
            {
                using namespace rapidjson;
                _scanner->seek(pos, SEEK_SET);
                //                try
                {
                    Array<char> buf;
                    _scanner->readAll(buf);
                    buf.push(0);
                    Document data;

                    if (data.Parse(buf.ptr()).HasParseError())
                        throw Error("Error at parsing JSON: %s", buf.ptr());
                    if (data.HasMember("root"))
                    {
                        MoleculeJsonLoader loader(data);
                        loader.stereochemistry_options = stereochemistry_options;
                        loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
                        loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
                        loader.skip_3d_chirality = skip_3d_chirality;
                        loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
                        loader.treat_stereo_as = treat_stereo_as;
                        loader.loadMolecule(mol);
                    }
                    else
                        throw Error("Ketcher's JSON has no root node");
                    return;
                }
                //              catch (...)
                //              {
                //              }
            }
        }
        _scanner->seek(pos, SEEK_SET);
    }

    // check for single line formats
    if (Scanner::isSingleLine(*_scanner))
    {
        // check for InChI format
        {
            char prefix[6] = {'\0'};
            long long start = _scanner->tell();

            bool inchi = false;
            {
                char* ptr = prefix;
                while (!_scanner->isEOF() && ptr - prefix < 6)
                {
                    *ptr = _scanner->readChar();
                    ptr++;
                }
                inchi = (strncmp(prefix, "InChI=", 6) == 0);
                _scanner->seek(start, SEEK_SET);
            }

            if (inchi)
            {
                if (query)
                {
                    throw Error("InChI input doesn't support query molecules");
                }

                Array<char> inchi;
                _scanner->readWord(inchi, " ");

                InchiWrapper loader;
                loader.loadMoleculeFromInchi(inchi.ptr(), (Molecule&)mol);
                return;
            }
        }

        // If not InChI then SMILES or IUPAC name
        Array<char> err_buf;

        try
        {
            SmilesLoader loader(*_scanner);

            loader.ignore_closing_bond_direction_mismatch = ignore_closing_bond_direction_mismatch;
            loader.stereochemistry_options = stereochemistry_options;
            loader.ignore_cistrans_errors = ignore_cistrans_errors;

            /*
            If exception is thrown, the string is rather an IUPAC name than a SMILES string
            We catch it and pass down to IUPAC name conversion
            */
            if (query)
                loader.loadQueryMolecule((QueryMolecule&)mol);
            else
                loader.loadMolecule((Molecule&)mol);
            return;
        }
        catch (Exception& e)
        {
            err_buf.appendString(e.message(), true);
        }

        // We fall down to IUPAC name conversion if SMILES loading threw an exception
        try
        {
            Array<char> name;
            _scanner->seek(SEEK_SET, SEEK_SET);
            _scanner->readLine(name, true);
            MoleculeNameParser parser;
            parser.parseMolecule(name.ptr(), static_cast<Molecule&>(mol));
            return;
        }
        catch (Exception& e)
        {
        }

        if (err_buf.size() > 0)
        {
            throw Error(err_buf.ptr());
        }
    }

    // check for CDX format
    /*
       {
          if (_scanner->findWord("VjCD0100"))
          {
             MoleculeCdxLoader loader(*_scanner);
             loader.stereochemistry_options = stereochemistry_options;
             if (query)
                throw Error("CDX queries not supported yet");
             loader.loadMolecule(mol.asMolecule());

             properties.copy(loader.properties);

             return;
          }
       }
    */
    // default is Molfile format

    {
        SdfLoader sdf_loader(*_scanner);
        sdf_loader.readNext();

        // Copy properties
        properties.copy(sdf_loader.properties);

        BufferScanner scanner2(sdf_loader.data);

        MolfileLoader loader(scanner2);
        loader.stereochemistry_options = stereochemistry_options;
        loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
        loader.skip_3d_chirality = skip_3d_chirality;
        loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
        loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
        loader.treat_stereo_as = treat_stereo_as;

        if (query)
            loader.loadQueryMolecule((QueryMolecule&)mol);
        else
            loader.loadMolecule((Molecule&)mol);
    }
}
