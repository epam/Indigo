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

#include <cctype>
#include <memory>
#include <regex>
#include <unordered_set>

#include "base_cpp/scanner.h"
#include "layout/sequence_layout.h"
#include "molecule/molecule.h"
#include "molecule/monomer_commons.h"
#include "molecule/sequence_loader.h"

using namespace indigo;

IMPL_ERROR(SequenceLoader, "SEQUENCE loader");

SequenceLoader::SequenceLoader(Scanner& scanner) : _scanner(scanner), _mon_lib(MonomerTemplates::_instance()), _seq_id(0), _last_sugar_idx(-1)
{
    _left_apid.readString(kLeftAttachmentPoint, true);
    _right_apid.readString(kRightAttachmentPoint, true);
    _xlink_apid.readString(kBranchAttachmentPoint, true);
}

SequenceLoader::~SequenceLoader()
{
}

void SequenceLoader::loadSequence(BaseMolecule& mol, SeqType sqtype)
{
    _seq_id = 0;
    _last_sugar_idx = -1;
    mol.clear();
    while (!_scanner.isEOF())
    {
        auto ch = _scanner.readChar();
        addMonomer(mol, ch, sqtype);
    }
    SequenceLayout sl(mol);
    sl.make();
}

void SequenceLoader::addTemplate(BaseMolecule& mol, char ch, SeqType sqtype)
{
    int tg_idx = mol.tgroups.addTGroup();
    auto& tg = mol.tgroups.getTGroup(tg_idx);

    if (_mon_lib.getMonomerTemplate(sqtype == SeqType::PEPTIDESeq ? MonomerType::AminoAcid : MonomerType::Base, std::string(1, ch), tg))
    {
        tg.tgroup_id = tg_idx;
        _added_templates.emplace(sqtype, ch);
    }
    else
        throw Error("Unknown sequence element: %c", ch);
}

void SequenceLoader::addMonomer(BaseMolecule& mol, char ch, SeqType sqtype)
{
    if (_added_templates.find(std::make_pair(sqtype, ch)) == _added_templates.end())
        addTemplate(mol, ch, sqtype);

    // add phosphate template
    if (_seq_id == 1 && sqtype != SeqType::PEPTIDESeq)
        addMonomerTemplate(mol, MonomerType::Phosphate, "P");

    _seq_id++;
    switch (sqtype)
    {
    case SeqType::PEPTIDESeq:
        addAminoAcid(mol, ch);
        break;
    case SeqType::RNASeq:
        addNucleotide(mol, ch, "R");
        break;
    case SeqType::DNASeq:
        addNucleotide(mol, ch, "dR");
        break;
    }
}

void SequenceLoader::addAminoAcid(BaseMolecule& mol, char ch)
{
    std::string aa(1, ch);
    int amino_idx = mol.asMolecule().addAtom(-1);
    mol.asMolecule().setTemplateAtom(amino_idx, monomerNameByAlias(kMonomerClassAA, aa).c_str());
    mol.asMolecule().setTemplateAtomClass(amino_idx, kMonomerClassAA);
    mol.asMolecule().setTemplateAtomSeqid(amino_idx, _seq_id);
    if (_seq_id > 1)
    {
        mol.asMolecule().addBond_Silent(amino_idx - 1, amino_idx, BOND_SINGLE);
        mol.setTemplateAtomAttachmentDestination(amino_idx - 1, amino_idx, _left_apid);
        mol.setTemplateAtomAttachmentDestination(amino_idx, amino_idx - 1, _right_apid);
    }
}

void SequenceLoader::addNucleotide(BaseMolecule& mol, char ch, const std::string& sugar_alias)
{
    // add ribose template
    if (_seq_id == 1)
        addMonomerTemplate(mol, MonomerType::Sugar, sugar_alias);

    // add base
    std::string nuc_base(1, ch);
    int nuc_base_idx = mol.asMolecule().addAtom(-1);
    mol.asMolecule().setTemplateAtom(nuc_base_idx, nuc_base.c_str());
    mol.asMolecule().setTemplateAtomClass(nuc_base_idx, kMonomerClassBASE);
    mol.asMolecule().setTemplateAtomSeqid(nuc_base_idx, _seq_id);

    // add sugar
    int sugar_idx = mol.asMolecule().addAtom(-1);
    mol.asMolecule().setTemplateAtom(sugar_idx, sugar_alias.c_str());
    mol.asMolecule().setTemplateAtomClass(sugar_idx, kMonomerClassSUGAR);
    mol.asMolecule().setTemplateAtomSeqid(sugar_idx, _seq_id);

    // connect sugar to nucleobase
    mol.asMolecule().addBond_Silent(sugar_idx, nuc_base_idx, BOND_SINGLE);
    mol.asMolecule().setTemplateAtomAttachmentDestination(sugar_idx, nuc_base_idx, _xlink_apid);
    mol.asMolecule().setTemplateAtomAttachmentDestination(nuc_base_idx, sugar_idx, _left_apid);

    if (_seq_id > 1)
    {
        // add phosphate
        int phosphate_idx = mol.asMolecule().addAtom(-1);
        mol.asMolecule().setTemplateAtom(phosphate_idx, "P");
        mol.asMolecule().setTemplateAtomClass(phosphate_idx, kMonomerClassPHOSPHATE);
        mol.asMolecule().setTemplateAtomSeqid(phosphate_idx, _seq_id - 1);

        // connect phosphate to the last sugar
        mol.asMolecule().addBond_Silent(_last_sugar_idx, phosphate_idx, BOND_SINGLE);
        mol.asMolecule().setTemplateAtomAttachmentDestination(phosphate_idx, _last_sugar_idx, _left_apid);
        mol.asMolecule().setTemplateAtomAttachmentDestination(_last_sugar_idx, phosphate_idx, _right_apid);

        // connect phoshpate to the current sugar
        mol.asMolecule().addBond_Silent(phosphate_idx, sugar_idx, BOND_SINGLE);
        mol.asMolecule().setTemplateAtomAttachmentDestination(phosphate_idx, sugar_idx, _right_apid);
        mol.asMolecule().setTemplateAtomAttachmentDestination(sugar_idx, phosphate_idx, _left_apid);
    }
    _last_sugar_idx = sugar_idx;
}

bool SequenceLoader::addMonomerTemplate(BaseMolecule& mol, MonomerType mt, const std::string& alias)
{
    int tg_idx = mol.tgroups.addTGroup();
    auto& tg = mol.tgroups.getTGroup(tg_idx);
    if (_mon_lib.getMonomerTemplate(mt, alias, tg))
    {
        tg.tgroup_id = tg_idx;
        return true;
    }
    else
        mol.tgroups.remove(tg_idx);
    return false;
}
