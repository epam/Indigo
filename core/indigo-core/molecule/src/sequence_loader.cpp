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
#include "layout/molecule_layout.h"
#include "layout/sequence_layout.h"
#include "molecule/molecule.h"
#include "molecule/monomer_commons.h"
#include "molecule/sequence_loader.h"

using namespace indigo;

IMPL_ERROR(SequenceLoader, "SEQUENCE loader");

SequenceLoader::SequenceLoader(Scanner& scanner)
    : _scanner(scanner), _mon_lib(MonomerTemplates::_instance()), _seq_id(0), _last_monomer_idx(-1), _row(-1), _col(0)
{
    _left_apid.readString(kLeftAttachmentPoint, true);
    _right_apid.readString(kRightAttachmentPoint, true);
    _xlink_apid.readString(kBranchAttachmentPoint, true);
}

SequenceLoader::~SequenceLoader()
{
}

void SequenceLoader::loadFasta(BaseMolecule& mol, const std::string& seq_type_str)
{
    if (seq_type_str == kMonomerClassDNA)
        loadFasta(mol, SeqType::DNASeq);
    else if (seq_type_str == kMonomerClassRNA)
        loadFasta(mol, SeqType::RNASeq);
    else if (seq_type_str == kMonomerClassPEPTIDE)
        loadFasta(mol, SeqType::PEPTIDESeq);
    else
        throw Error("Bad sequence type: %s", seq_type_str.c_str());
}

void SequenceLoader::loadFasta(BaseMolecule& mol, SeqType seq_type)
{
    _seq_id = 0;
    _last_monomer_idx = -1;
    _row = 0;
    _col = 0;
    int frag_idx = 0;
    std::string invalid_symbols;
    Array<int> mapping;
    PropertiesMap properties;

    while (!_scanner.isEOF())
    {
        Array<char> str;
        _scanner.readLine(str, true);
        if (str.size())
        {
            std::string fasta_str = str.ptr();
            switch (fasta_str.front())
            {
            case ';':
                // handle comment
                continue;
                break;
            case '>':
                // handle header
                if (_seq_id)
                {
                    _seq_id = 0;
                    _col = 0;
                    _row++;
                }
                properties.insert(kFASTA_HEADER, fasta_str);
                if (mol.vertexCount() > 0) // do not increment fragment id if first fragment
                    frag_idx++;
                continue;
                break;
            default:
                break;
            }

            for (auto ch : fasta_str)
            {
                if (ch == '-')
                    continue;
                else if (ch == '*' && mol.vertexCount())
                {
                    _seq_id = 0;
                    _col = 0;
                    _row++;
                    continue;
                }
                else if (!addMonomer(mol, ch, seq_type))
                {
                    if (invalid_symbols.size())
                        invalid_symbols += ',';
                    invalid_symbols += ch;
                }
            }

            if (invalid_symbols.size())
                throw Error("Invalid symbols in the sequence: %s", invalid_symbols.c_str());

            if (!properties.is_empty())
            {
                mol.properties().insert(frag_idx).copy(properties);
                properties.clear();
            }
        }
    }

    if (!mol.properties().size())
        throw Error("Invalid FASTA: no '>' headers");
}

void SequenceLoader::loadSequence(BaseMolecule& mol, const std::string& seq_type_str)
{
    if (seq_type_str == kMonomerClassDNA)
        loadSequence(mol, SeqType::DNASeq);
    else if (seq_type_str == kMonomerClassRNA)
        loadSequence(mol, SeqType::RNASeq);
    else if (seq_type_str == kMonomerClassPEPTIDE)
        loadSequence(mol, SeqType::PEPTIDESeq);
    else
        throw Error("Bad sequence type: %s", seq_type_str.c_str());
}

void SequenceLoader::loadSequence(BaseMolecule& mol, SeqType seq_type)
{
    _seq_id = 0;
    _last_monomer_idx = -1;
    _row = 0;
    _col = 0;
    mol.clear();
    std::string invalid_symbols;
    while (!_scanner.isEOF())
    {
        auto ch = _scanner.readChar();
        if (ch == '\n' || ch == '\r')
            continue;
        if (ch == ' ')
        {
            _seq_id = 0;
            _col = 0;
            _row++;
            continue;
        }
        if (!addMonomer(mol, ch, seq_type))
        {
            if (invalid_symbols.size())
                invalid_symbols += ',';
            invalid_symbols += ch;
        }
    }

    if (invalid_symbols.size())
        throw Error("Invalid symbols in the sequence: %s", invalid_symbols.c_str());
}

bool SequenceLoader::addTemplate(BaseMolecule& mol, const std::string alias, MonomerType mon_type)
{
    int tg_idx = mol.tgroups.addTGroup();
    auto& tg = mol.tgroups.getTGroup(tg_idx);

    if (_mon_lib.getMonomerTemplate(mon_type, alias, tg))
    {
        tg.tgroup_id = tg_idx;
        _added_templates.emplace(mon_type, alias);
        return true;
    }
    return false;
}

bool SequenceLoader::addMonomer(BaseMolecule& mol, char ch, SeqType seq_type)
{
    MonomerType mt = seq_type == SeqType::PEPTIDESeq ? MonomerType::AminoAcid : MonomerType::Base;
    if (_added_templates.count(std::make_pair(mt, std::string(1, ch))) == 0 && !addTemplate(mol, std::string(1, ch), mt))
        return false;

    // add phosphate template
    if (_seq_id == 1 && seq_type != SeqType::PEPTIDESeq)
        addMonomerTemplate(mol, MonomerType::Phosphate, "P");

    _seq_id++;
    switch (seq_type)
    {
    case SeqType::PEPTIDESeq:
        addAminoAcid(mol, ch);
        break;
    case SeqType::RNASeq:
        addNucleotide(mol, ch, "R", "P");
        break;
    case SeqType::DNASeq:
        addNucleotide(mol, ch, "dR", "P");
        break;
    }
    _col++;
    return true;
}

void SequenceLoader::addAminoAcid(BaseMolecule& mol, char ch)
{
    Vec3f pos(_col * MoleculeLayout::DEFAULT_BOND_LENGTH, -MoleculeLayout::DEFAULT_BOND_LENGTH * _row, 0);
    std::string aa(1, ch);
    int amino_idx = mol.asMolecule().addAtom(-1);
    mol.asMolecule().setTemplateAtom(amino_idx, monomerNameByAlias(kMonomerClassAA, aa).c_str());
    mol.asMolecule().setTemplateAtomClass(amino_idx, kMonomerClassAA);
    mol.asMolecule().setTemplateAtomSeqid(amino_idx, _seq_id);
    mol.asMolecule().setAtomXyz(amino_idx, pos);

    if (_seq_id > 1)
    {
        mol.asMolecule().addBond_Silent(amino_idx - 1, amino_idx, BOND_SINGLE);
        mol.setTemplateAtomAttachmentDestination(amino_idx - 1, amino_idx, _right_apid);
        mol.setTemplateAtomAttachmentDestination(amino_idx, amino_idx - 1, _left_apid);
    }
}

void SequenceLoader::addNucleotide(BaseMolecule& mol, char ch, const std::string& sugar_alias, const std::string& phosphate_alias)
{
    Vec3f pos(_col * MoleculeLayout::DEFAULT_BOND_LENGTH, -MoleculeLayout::DEFAULT_BOND_LENGTH * _row, 0);

    // add sugar template
    if (_seq_id == 1)
        addMonomerTemplate(mol, MonomerType::Sugar, sugar_alias);

    // add sugar
    int sugar_idx = mol.asMolecule().addAtom(-1);
    mol.asMolecule().setTemplateAtom(sugar_idx, sugar_alias.c_str());
    mol.asMolecule().setTemplateAtomClass(sugar_idx, kMonomerClassSUGAR);
    mol.asMolecule().setTemplateAtomSeqid(sugar_idx, _seq_id);
    mol.asMolecule().setAtomXyz(sugar_idx, pos);

    // add base
    std::string nuc_base(1, ch);
    int nuc_base_idx = mol.asMolecule().addAtom(-1);
    mol.asMolecule().setTemplateAtom(nuc_base_idx, nuc_base.c_str());
    mol.asMolecule().setTemplateAtomClass(nuc_base_idx, kMonomerClassBASE);
    mol.asMolecule().setTemplateAtomSeqid(nuc_base_idx, _seq_id);
    Vec3f base_coord(pos.x, pos.y - MoleculeLayout::DEFAULT_BOND_LENGTH, 0);
    mol.asMolecule().setAtomXyz(nuc_base_idx, base_coord);

    // connect sugar to nucleobase
    mol.asMolecule().addBond_Silent(sugar_idx, nuc_base_idx, BOND_SINGLE);
    mol.asMolecule().setTemplateAtomAttachmentDestination(sugar_idx, nuc_base_idx, _xlink_apid);
    mol.asMolecule().setTemplateAtomAttachmentDestination(nuc_base_idx, sugar_idx, _left_apid);

    if (_seq_id > 1 && phosphate_alias.size())
    {
        // add phosphate
        int phosphate_idx = mol.asMolecule().addAtom(-1);
        mol.asMolecule().setTemplateAtom(phosphate_idx, phosphate_alias.c_str());
        mol.asMolecule().setTemplateAtomClass(phosphate_idx, kMonomerClassPHOSPHATE);
        mol.asMolecule().setTemplateAtomSeqid(phosphate_idx, _seq_id - 1);
        Vec3f phosphate_coord(pos.x - MoleculeLayout::DEFAULT_BOND_LENGTH, pos.y, 0);
        mol.asMolecule().setAtomXyz(phosphate_idx, phosphate_coord);

        // connect phosphate to the last sugar
        mol.asMolecule().addBond_Silent(_last_monomer_idx, phosphate_idx, BOND_SINGLE);
        mol.asMolecule().setTemplateAtomAttachmentDestination(phosphate_idx, _last_monomer_idx, _left_apid);
        mol.asMolecule().setTemplateAtomAttachmentDestination(_last_monomer_idx, phosphate_idx, _right_apid);

        // connect phosphate to the current sugar
        mol.asMolecule().addBond_Silent(phosphate_idx, sugar_idx, BOND_SINGLE);
        mol.asMolecule().setTemplateAtomAttachmentDestination(phosphate_idx, sugar_idx, _right_apid);
        mol.asMolecule().setTemplateAtomAttachmentDestination(sugar_idx, phosphate_idx, _left_apid);
    }
    else if (_last_monomer_idx >= 0)
    {
        // No phosphate - connect sugar to previous monomer
        mol.asMolecule().addBond_Silent(_last_monomer_idx, sugar_idx, BOND_SINGLE);
        mol.asMolecule().setTemplateAtomAttachmentDestination(sugar_idx, _last_monomer_idx, _left_apid);
        mol.asMolecule().setTemplateAtomAttachmentDestination(_last_monomer_idx, sugar_idx, _right_apid);
    }
    _last_monomer_idx = sugar_idx;
    _col++;
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

// return true if monomer already in templates or successfuly added. otherwise - false
bool SequenceLoader::checkAddTemplate(BaseMolecule& mol, MonomerType type, const std::string monomer)
{
    if (_added_templates.count(std::make_pair(type, monomer)) == 0)
        if (!addTemplate(mol, monomer, type))
            return false;
    return true;
}

void SequenceLoader::loadIDT(BaseMolecule& mol)
{
    const auto IDT_DEF_SUGAR = "dR";
    const auto IDT_DEF_PHOSPHATE = "P";
    static const std::unordered_set<char> IDT_STANDARD_BASES = {'A', 'T', 'C', 'G', 'U', 'I'};
    _seq_id = 0;
    _last_monomer_idx = -1;
    _row = 0;
    _col = 0;
    mol.clear();
    std::string invalid_symbols;
    std::string sugar = IDT_DEF_SUGAR;
    std::string phosphate = "";
    std::string base = "";

    while (!_scanner.isEOF())
    {
        auto ch = _scanner.readChar();
        switch (ch)
        {
        case '+':
            sugar = "LR";
            break;
        case 'm':
            sugar = "mR";
            break;
        case 'r':
            sugar = "R";
            break;
        case '*':
            phosphate = "sP";
            break;
        default:
            base = std::string(1, ch);
            break;
        };
        if (base.size() > 0)
        {
            if (_col > 0 && phosphate.size() == 0)
            {
                phosphate = IDT_DEF_PHOSPHATE;
            }
            if (IDT_STANDARD_BASES.count(ch) == 0)
            {
                if (invalid_symbols.size())
                    invalid_symbols += ',';
                invalid_symbols += ch;
            }
            else
            {
                if (!checkAddTemplate(mol, MonomerType::Base, base))
                    throw Error("Unknown base '%s'", base);
                if (!checkAddTemplate(mol, MonomerType::Sugar, sugar))
                    throw Error("Unknown sugar '%s'", sugar);
                if (phosphate.size() > 0 && !checkAddTemplate(mol, MonomerType::Phosphate, phosphate))
                    throw Error("Unknown phosphate '%s'", phosphate);

                _seq_id++;

                addNucleotide(mol, ch, sugar, phosphate);
                _col++;
            }
            sugar = IDT_DEF_SUGAR; // reset sugar to default
            phosphate = "";
            base = "";
        }
    }

    if (invalid_symbols.size())
        throw Error("Invalid symbols in the sequence: %s", invalid_symbols.c_str());
}
