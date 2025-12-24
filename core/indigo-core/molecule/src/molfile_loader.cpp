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

#include <memory>

#include "../layout/molecule_layout.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "layout/sequence_layout.h"
#include "molecule/elements.h"
#include "molecule/inchi_wrapper.h"
#include "molecule/molecule.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/monomer_commons.h"
#include "molecule/parse_utils.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"

#define STRCMP(a, b) strncmp((a), (b), strlen(b))

using namespace indigo;

IMPL_ERROR(MolfileLoader, "molfile loader");

CP_DEF(MolfileLoader);

MolfileLoader::MolfileLoader(Scanner& scanner, MonomerTemplateLibrary* monomer_library)
    : _scanner(scanner), _monomer_templates(MonomerTemplates::_instance()), _max_template_id(0), _disable_sgroups_conversion(false), CP_INIT,
      TL_CP_GET(_stereo_care_atoms), TL_CP_GET(_stereo_care_bonds), TL_CP_GET(_stereocenter_types), TL_CP_GET(_stereocenter_groups),
      TL_CP_GET(_sensible_bond_directions), TL_CP_GET(_ignore_cistrans), TL_CP_GET(_atom_types), TL_CP_GET(_hcount), TL_CP_GET(_sgroup_types),
      TL_CP_GET(_sgroup_mapping)
{
    _rgfile = false;
    treat_x_as_pseudoatom = false;
    skip_3d_chirality = false;
    ignore_noncritical_query_features = false;
    ignore_no_chiral_flag = false;
    ignore_bad_valence = false;
    treat_stereo_as = 0;
    _monomer_library = monomer_library;
}

void MolfileLoader::loadMolecule(Molecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = &mol;
    _qmol = 0;
    _max_template_id = 0;
    _loadMolecule();
    mol.setIgnoreBadValenceFlag(ignore_bad_valence);
    if (mol.stereocenters.size() == 0 && !skip_3d_chirality)
        mol.buildFrom3dCoordinatesStereocenters(stereochemistry_options);
}

void MolfileLoader::copyProperties(const MolfileLoader& loader)
{
    stereochemistry_options = loader.stereochemistry_options;
    ignore_bad_valence = loader.ignore_bad_valence;
    ignore_no_chiral_flag = loader.ignore_no_chiral_flag;
    skip_3d_chirality = loader.skip_3d_chirality;
    treat_stereo_as = loader.treat_stereo_as;
    treat_x_as_pseudoatom = loader.treat_x_as_pseudoatom;
    _monomer_library = loader._monomer_library;
}

void MolfileLoader::loadQueryMolecule(QueryMolecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _qmol = &mol;
    _mol = 0;
    _max_template_id = 0;
    _loadMolecule();
    if (mol.stereocenters.size() == 0)
        mol.buildFrom3dCoordinatesStereocenters(stereochemistry_options);
}

void MolfileLoader::_loadMolecule()
{
    _readHeader();

    _readCtabHeader();

    if (_v2000)
    {
        _readCtab2000();

        if (_rgfile)
            _readRGroups2000();
    }
    else
    {
        _readCtab3000();
        _readRGroups3000();
        _readTGroups3000();

        long long next_block_pos = _scanner.tell();
        QS_DEF(Array<char>, str);
        _scanner.readLine(str, true);
        if (strncmp(str.ptr(), "M  END", 6) != 0)
            throw Error("unexpected string in molecule: %s", str.ptr());
        _scanner.seek(next_block_pos, SEEK_SET);
    }
    _postLoad();
}

void MolfileLoader::_checkEndOfMolBlock3000()
{
    long long next_block_pos = _scanner.tell();
    QS_DEF(Array<char>, str);
    _scanner.readLine(str, true);
    // could be end of rectant, product or catalyst, ot start of next item
    if (!(strncmp(str.ptr(), "M  V30 END ", 11) == 0 || strncmp(str.ptr(), "M  V30 BEGIN CTAB", 15) == 0))
        throw Error("unexpected string in reaction: %s", str.ptr());
    _scanner.seek(next_block_pos, SEEK_SET);
}

void MolfileLoader::loadMolBlock3000(Molecule& mol)
{
    _bmol = &mol;
    _qmol = 0;
    _mol = &mol;
    _readCtab3000();
    _readTGroups3000();
    _checkEndOfMolBlock3000();
    _postLoad();
}

void MolfileLoader::loadQueryMolBlock3000(QueryMolecule& mol)
{
    _bmol = &mol;
    _qmol = &mol;
    _mol = 0;
    _readCtab3000();
    _readTGroups3000();
    _checkEndOfMolBlock3000();
    _postLoad();
}

void MolfileLoader::_readHeader()
{
    if (_scanner.lookNext() == '$')
    {
        _rgfile = true;      // It's RGfile
        _scanner.skipLine(); // Skip $MDL REV  1   Date/Time
        _scanner.skipLine(); // Skip $MOL
        _scanner.skipLine(); // Skip $HDR
    }

    // Skip header
    _scanner.readLine(_bmol->name, true);
    // Check UTF-8 BOM mark in the name
    if (_bmol->name.size() >= 3 && (unsigned char)_bmol->name[0] == 0xEF && (unsigned char)_bmol->name[1] == 0xBB && (unsigned char)_bmol->name[2] == 0xBF)
        _bmol->name.remove(0, 3);

    _scanner.skipLine();
    _scanner.skipLine();

    if (_rgfile)
    {
        _scanner.skipLine(); // Skip $END HDR
        _scanner.skipLine(); // Skip $CTAB
    }
}

void MolfileLoader::_readCtabHeader()
{
    QS_DEF(Array<char>, str);

    _scanner.readLine(str, false);

    BufferScanner strscan(str);

    _atoms_num = strscan.readIntFix(3);
    _bonds_num = strscan.readIntFix(3);

    try
    {
        char version[6];
        int chiral_int;

        strscan.skip(6);
        chiral_int = strscan.readIntFix(3);
        strscan.skip(19);
        strscan.read(5, version);
        strscan.skipLine();

        version[5] = 0;

        if (strcasecmp(version, "V2000") == 0 || strcasecmp(version, "     ") == 0) // ISISHOST version of Molfile do not have version in the header
            _v2000 = true;
        else if (strcasecmp(version, "V3000") == 0)
            _v2000 = false;
        else
            throw Error("bad molfile version : %s", version);

        _bmol->setChiralFlag(chiral_int);
        _chiral = (chiral_int != 0);
    }
    catch (Scanner::Error&)
    {
        _chiral = false;
        _v2000 = true;
    }

    if (ignore_no_chiral_flag)
        _chiral = true;
}

int MolfileLoader::_getElement(const char* buf)
{
    char buf2[4] = {0, 0, 0, 0};

    size_t len = strlen(buf);
    if (len > 3)
        throw Error("Internal error in MolfileLoader::_getElement: len = %d > 3", len);

    for (size_t i = 0; i < len; i++)
    {
        if (isspace(buf[i]))
            break;

        if (!isalpha(buf[i]))
            return -1;

        //      buf2[i] = (i == 0) ? toupper(buf[i]) : tolower(buf[i]);
        // Removed defensive conversion of input symbols to avoid possible issues with abbreviations like
        //   No <-> NO
        //   Co <-> CO and etc.
        buf2[i] = buf[i];
    }

    return Element::fromString2(buf2);
}

char* MolfileLoader::_strtrim(char* buf)
{
    while (*buf == ' ')
        buf++;
    if (*buf != 0)
    {
        size_t len = strlen(buf);
        char* end = buf + len - 1;
        while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')
        {
            *end = 0;
            end--;
        }
    }
    return buf;
}

