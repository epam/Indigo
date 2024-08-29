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
#include "molecule/elements.h"
#include "molecule/ket_commons.h"
#include "molecule/ket_document.h"
#include "molecule/molecule.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/sequence_loader.h"
#include "molecule/smiles_loader.h"

using namespace indigo;

IMPL_ERROR(SequenceLoader, "SEQUENCE loader");

SequenceLoader::SequenceLoader(Scanner& scanner, MonomerTemplateLibrary& library)
    : _scanner(scanner), _mon_lib(MonomerTemplates::_instance()), _seq_id(0), _last_monomer_idx(-1), _row(-1), _col(0), _library(library),
      _unknown_variants_count(0)
{
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
    const int row_size = seq_type == SeqType::PEPTIDESeq ? 1 : 2;
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
                    _row += row_size;
                }
                _last_monomer_idx = -1;
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
                else if (ch == '*' && seq_type == SeqType::PEPTIDESeq && mol.vertexCount())
                {
                    _seq_id = 0;
                    _col = 0;
                    _row += row_size;
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
    const int row_size = seq_type == SeqType::PEPTIDESeq ? 1 : 2;
    mol.clear();
    std::string invalid_symbols;

    bool isGenBankPept = false;
    bool start_char = true;

    while (!_scanner.isEOF())
    {
        auto ch = _scanner.readChar();
        if (ch == '\n' || ch == '\r')
            continue;

        if (start_char)
        {
            if (ch >= NUM_BEGIN && ch < NUM_END)
            {
                isGenBankPept = true;
            }
            start_char = false;
        }

        if (isGenBankPept)
        {
            if (ch == ' ' || (ch >= NUM_BEGIN && ch < NUM_END))
            {
                continue;
            }
            if (ch >= CHAR_LOWERCASE_BEGIN && ch < CHAR_LOWERCASE_END)
            {
                ch -= CHAR_SHIFT_CONVERT;
            }
        }

        if (!isGenBankPept && ch == ' ')
        {
            _seq_id = 0;
            _col = 0;
            _row += row_size;
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

bool SequenceLoader::addTemplate(BaseMolecule& mol, const std::string alias, MonomerClass mon_type)
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
    MonomerClass mt = seq_type == SeqType::PEPTIDESeq ? MonomerClass::AminoAcid : MonomerClass::Base;
    if (_added_templates.count(std::make_pair(mt, std::string(1, ch))) == 0 && !addTemplate(mol, std::string(1, ch), mt))
        return false;

    if (seq_type != SeqType::PEPTIDESeq)
    {
        // add sugar template
        if (_seq_id == 0)
            addMonomerTemplate(mol, MonomerClass::Sugar, seq_type == SeqType::RNASeq ? "R" : "dR");

        // add phosphate template
        if (_seq_id == 1)
            addMonomerTemplate(mol, MonomerClass::Phosphate, "P");
    }

    _seq_id++;
    switch (seq_type)
    {
    case SeqType::PEPTIDESeq:
        addAminoAcid(mol, ch);
        break;
    case SeqType::RNASeq:
        addNucleotide(mol, std::string(1, ch), "R", "P");
        break;
    case SeqType::DNASeq:
        addNucleotide(mol, std::string(1, ch), "dR", "P");
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
        mol.setTemplateAtomAttachmentOrder(amino_idx - 1, amino_idx, kRightAttachmentPoint);
        mol.setTemplateAtomAttachmentOrder(amino_idx, amino_idx - 1, kLeftAttachmentPoint);
    }
}

int SequenceLoader::addTemplateAtom(BaseMolecule& mol, const char* alias, const char* monomer_class, int seq_id)
{
    int idx = mol.asMolecule().addAtom(-1);
    mol.asMolecule().setTemplateAtom(idx, alias);
    mol.asMolecule().setTemplateAtomClass(idx, monomer_class);
    mol.asMolecule().setTemplateAtomSeqid(idx, seq_id);
    return idx;
};

void SequenceLoader::addTemplateBond(BaseMolecule& mol, int left_idx, int right_idx, bool branch)
{
    mol.asMolecule().addBond_Silent(left_idx, right_idx, BOND_SINGLE);
    mol.asMolecule().setTemplateAtomAttachmentOrder(left_idx, right_idx, branch ? kBranchAttachmentPoint : kRightAttachmentPoint);
    mol.asMolecule().setTemplateAtomAttachmentOrder(right_idx, left_idx, kLeftAttachmentPoint);
}

void SequenceLoader::addMonomerConnection(KetDocument& document, std::size_t left_idx, std::size_t right_idx, bool branch)
{
    KetConnectionEndPoint ep1{};
    ep1.setStringProp("monomerId", document.monomers().at(std::to_string(left_idx))->ref());
    ep1.setStringProp("attachmentPointId", branch ? "R3" : "R2");
    KetConnectionEndPoint ep2{};
    ep2.setStringProp("monomerId", document.monomers().at(std::to_string(right_idx))->ref());
    ep2.setStringProp("attachmentPointId", "R1");
    document.addConnection(ep1, ep2);
}

Vec3f SequenceLoader::getBackboneMonomerPosition()
{
    return Vec3f(_col * MoleculeLayout::DEFAULT_BOND_LENGTH, -MoleculeLayout::DEFAULT_BOND_LENGTH * _row, 0);
}

void SequenceLoader::addNucleotide(BaseMolecule& mol, std::string base, const std::string& sugar_alias, const std::string& phosphate_alias,
                                   bool phosphate_at_left)
{
    Vec3f pos = getBackboneMonomerPosition();

    // add sugar
    int sugar_idx = addTemplateAtom(mol, sugar_alias.c_str(), kMonomerClassSUGAR, _seq_id);

    mol.asMolecule().setAtomXyz(sugar_idx, pos);

    // add base
    if (base.size() > 0)
    {
        int nuc_base_idx = addTemplateAtom(mol, base.c_str(), kMonomerClassBASE, _seq_id);
        Vec3f base_coord(pos.x, pos.y - MoleculeLayout::DEFAULT_BOND_LENGTH, 0);
        mol.asMolecule().setAtomXyz(nuc_base_idx, base_coord);

        // connect nucleobase to the sugar
        addTemplateBond(mol, sugar_idx, nuc_base_idx, true);
    }

    if (phosphate_alias.size())
    {
        if (phosphate_at_left)
        {
            if (_seq_id > 1)
            {
                // add phosphate
                int phosphate_idx = addTemplateAtom(mol, phosphate_alias.c_str(), kMonomerClassPHOSPHATE, _seq_id - 1);

                Vec3f phosphate_coord(pos.x - MoleculeLayout::DEFAULT_BOND_LENGTH, pos.y, 0);
                mol.asMolecule().setAtomXyz(phosphate_idx, phosphate_coord);

                addTemplateBond(mol, _last_monomer_idx, phosphate_idx); // connect phosphate to the previous monomer
                addTemplateBond(mol, phosphate_idx, sugar_idx);         // connect current sugar to the phosphate
            }
        }
        else // Phosphate at right
        {
            // add phosphate
            int phosphate_idx = addTemplateAtom(mol, phosphate_alias.c_str(), kMonomerClassPHOSPHATE, _seq_id);

            Vec3f phosphate_coord(pos.x + MoleculeLayout::DEFAULT_BOND_LENGTH, pos.y, 0);
            mol.asMolecule().setAtomXyz(phosphate_idx, phosphate_coord);

            if (_last_monomer_idx >= 0)
                addTemplateBond(mol, _last_monomer_idx, sugar_idx); // сonnect sugar to the previous monomer
            addTemplateBond(mol, sugar_idx, phosphate_idx);         // connect phosphate to the current sugar
            _last_monomer_idx = phosphate_idx;
        }
        _col++;
    }
    else if (_last_monomer_idx >= 0)
    {
        // No phosphate - connect sugar to the previous monomer
        addTemplateBond(mol, _last_monomer_idx, sugar_idx);
    }

    if (_last_monomer_idx < 0 || phosphate_at_left)
        _last_monomer_idx = sugar_idx;
}

void SequenceLoader::addMonomer(KetDocument& document, const std::string& monomer, SeqType seq_type, bool mixed)
{
    MonomerClass monomer_class = seq_type == SeqType::PEPTIDESeq ? MonomerClass::AminoAcid : MonomerClass::Base;
    if (!mixed)
        _alias_to_id.emplace(monomer, checkAddTemplate(document, monomer_class, monomer));
    else if (!document.hasVariantMonomerTemplate(monomer))
    {
        std::optional<std::reference_wrapper<const std::vector<std::string>>> mixture;
        if (seq_type == SeqType::PEPTIDESeq)
        {
            const auto& it = STANDARD_MIXED_PEPTIDES.find(monomer);
            if (it == STANDARD_MIXED_PEPTIDES.end())
                throw Error("Unknown mixed peptide '%s'", monomer.c_str());
            mixture.emplace(std::cref(it->second));
        }
        else
        {
            const auto& it = STANDARD_MIXED_BASES.find(monomer);
            if (it == STANDARD_MIXED_BASES.end())
                throw Error("Unknown mixed base '%s'", monomer.c_str());
            mixture.emplace(std::cref(it->second));
        }

        std::vector<KetVariantMonomerOption> options;
        for (auto template_alias : mixture.value().get())
        {
            auto& template_id = _library.getMonomerTemplateIdByAlias(monomer_class, template_alias);
            if (template_id.size() == 0)
                throw Error("Monomer base template '%s' not found", template_alias.c_str());
            auto& option = options.emplace_back(template_id);
            auto& monomer_template = _library.getMonomerTemplateById(template_id);
            checkAddTemplate(document, monomer_template);
            _alias_to_id.emplace(template_alias, template_id);
        }
        document.addVariantMonomerTemplate("mixture", monomer, monomer, IdtAlias(), options);
        _alias_to_id.emplace(monomer, monomer);
    }

    std::string sugar_alias = seq_type == SeqType::RNASeq ? "R" : "dR";
    std::string phosphate_alias = "P";
    if (seq_type != SeqType::PEPTIDESeq)
    {
        // add sugar template
        if (_seq_id == 0)
            _alias_to_id.emplace(sugar_alias, checkAddTemplate(document, MonomerClass::Sugar, sugar_alias));

        // add phosphate template
        if (_seq_id == 1)
            _alias_to_id.emplace(phosphate_alias, checkAddTemplate(document, MonomerClass::Phosphate, phosphate_alias));
    }

    _seq_id++;
    switch (seq_type)
    {
    case SeqType::PEPTIDESeq:
        addAminoAcid(document, monomer, mixed);
        break;
    case SeqType::RNASeq:
        addNucleotide(document, monomer, sugar_alias, phosphate_alias, true, mixed);
        break;
    case SeqType::DNASeq:
        addNucleotide(document, monomer, sugar_alias, phosphate_alias, true, mixed);
        break;
    }
    _col++;
}

void SequenceLoader::addAminoAcid(KetDocument& document, const std::string& monomer, bool variant)
{
    Vec3f pos(_col * MoleculeLayout::DEFAULT_BOND_LENGTH, -MoleculeLayout::DEFAULT_BOND_LENGTH * _row, 0);
    auto amino_idx = document.monomers().size();
    auto& amino_acid = variant ? document.addVariantMonomer(monomer, _alias_to_id.at(monomer)) : document.addMonomer(monomer, _alias_to_id.at(monomer));
    if (variant)
        amino_acid->setAttachmentPoints(document.variantTemplates().at(_alias_to_id.at(monomer)).attachmentPoints());
    else
        amino_acid->setAttachmentPoints(document.templates().at(_alias_to_id.at(monomer)).attachmentPoints());
    amino_acid->setIntProp("seqid", _seq_id);
    amino_acid->setPosition(pos);

    if (_seq_id > 1)
        addMonomerConnection(document, amino_idx - 1, amino_idx);
}

void SequenceLoader::addNucleotide(KetDocument& document, const std::string& base_alias, const std::string& sugar_alias, const std::string& phosphate_alias,
                                   bool phosphate_at_left, bool variant)
{
    Vec3f pos = getBackboneMonomerPosition();

    // add sugar
    auto sugar_idx = document.monomers().size();
    auto& sugar = document.addMonomer(sugar_alias, _alias_to_id.at(sugar_alias));
    sugar->setAttachmentPoints(document.templates().at(_alias_to_id.at(sugar_alias)).attachmentPoints());
    sugar->setIntProp("seqid", _seq_id);
    sugar->setPosition(pos);

    // add base
    if (base_alias.size() > 0)
    {
        auto nuc_base_idx = document.monomers().size();
        auto& base =
            variant ? document.addVariantMonomer(base_alias, _alias_to_id.at(base_alias)) : document.addMonomer(base_alias, _alias_to_id.at(base_alias));
        if (variant)
            base->setAttachmentPoints(document.variantTemplates().at(base_alias).attachmentPoints());
        else
            base->setAttachmentPoints(document.templates().at(_alias_to_id.at(base_alias)).attachmentPoints());
        base->setIntProp("seqid", _seq_id);
        Vec3f base_coord(pos.x, pos.y - MoleculeLayout::DEFAULT_BOND_LENGTH, 0);
        base->setPosition(base_coord);

        // connect nucleobase to the sugar
        addMonomerConnection(document, sugar_idx, nuc_base_idx, true);
    }

    if (phosphate_alias.size())
    {
        if (phosphate_at_left)
        {
            if (_seq_id > 1)
            {
                // add phosphate
                auto phosphate_idx = document.monomers().size();
                auto& phosphate = document.addMonomer(phosphate_alias, _alias_to_id.at(phosphate_alias));
                phosphate->setAttachmentPoints(document.templates().at(_alias_to_id.at(phosphate_alias)).attachmentPoints());
                phosphate->setIntProp("seqid", _seq_id - 1);
                Vec3f phosphate_coord(pos.x - MoleculeLayout::DEFAULT_BOND_LENGTH, pos.y, 0);
                phosphate->setPosition(phosphate_coord);

                addMonomerConnection(document, _last_monomer_idx, phosphate_idx); // connect phosphate to the previous monomer
                addMonomerConnection(document, phosphate_idx, sugar_idx);         // connect current sugar to the phosphate
            }
        }
        else // Phosphate at right
        {
            // add phosphate
            auto phosphate_idx = document.monomers().size();
            auto& phosphate = document.addMonomer(phosphate_alias, _alias_to_id.at(phosphate_alias));
            phosphate->setAttachmentPoints(document.templates().at(_alias_to_id.at(phosphate_alias)).attachmentPoints());
            phosphate->setIntProp("seqid", _seq_id);
            Vec3f phosphate_coord(pos.x + MoleculeLayout::DEFAULT_BOND_LENGTH, pos.y, 0);
            phosphate->setPosition(phosphate_coord);

            if (_last_monomer_idx >= 0)
                addMonomerConnection(document, _last_monomer_idx, sugar_idx); // сonnect sugar to the previous monomer
            addMonomerConnection(document, sugar_idx, phosphate_idx);         // connect phosphate to the current sugar
            _last_monomer_idx = static_cast<int>(phosphate_idx);
        }
        _col++;
    }
    else if (_last_monomer_idx >= 0)
    {
        // No phosphate - connect sugar to the previous monomer
        addMonomerConnection(document, _last_monomer_idx, sugar_idx);
    }

    if (_last_monomer_idx < 0 || phosphate_at_left)
        _last_monomer_idx = static_cast<int>(sugar_idx);
}

bool SequenceLoader::addMonomerTemplate(BaseMolecule& mol, MonomerClass mt, const std::string& alias)
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
bool SequenceLoader::checkAddTemplate(BaseMolecule& mol, MonomerClass type, const std::string monomer)
{
    if (_added_templates.count(std::make_pair(type, monomer)) == 0)
        if (!addTemplate(mol, monomer, type))
            return false;
    return true;
}

// return true if monomer already in templates or successfuly added. otherwise - false
void SequenceLoader::checkAddTemplate(BaseMolecule& mol, const MonomerTemplate& monomer_template)
{
    if (_added_templates.count(std::make_pair(monomer_template.monomerClass(), monomer_template.getStringProp("alias"))) == 0)
    {
        int tg_idx = mol.tgroups.addTGroup();
        auto& tg = mol.tgroups.getTGroup(tg_idx);
        tg.copy(*monomer_template.getTGroup());
        tg.tgroup_id = tg_idx;
        tg.idt_alias.readString(monomer_template.idtAlias().getBase().c_str(), true);
        _added_templates.emplace(monomer_template.monomerClass(), monomer_template.getStringProp("alias"));
    }
}

void SequenceLoader::checkAddTemplate(KetDocument& document, const MonomerTemplate& monomer_template)
{
    if (_added_templates.count(std::make_pair(monomer_template.monomerClass(), monomer_template.getStringProp("alias"))) == 0)
    {
        _added_templates.emplace(monomer_template.monomerClass(), monomer_template.getStringProp("alias"));
        document.addMonomerTemplate(monomer_template);
    }
}

const std::string& SequenceLoader::checkAddTemplate(KetDocument& document, MonomerClass monomer_class, const std::string& alias)
{
    auto& id = _library.getMonomerTemplateIdByAlias(monomer_class, alias);
    if (_added_templates.count(std::make_pair(monomer_class, alias)) == 0)
    {
        _added_templates.emplace(monomer_class, alias);
        if (id.size() == 0)
            throw Error("Monomer template with class '%s' and alias '%s' not found in monomer librarys",
                        MonomerTemplate::MonomerClassToStr(monomer_class).c_str(), alias.c_str());
        document.addMonomerTemplate(_library.getMonomerTemplateById(id));
    }
    if (id.size() > 0)
        return id;
    return alias;
}

void SequenceLoader::check_monomer_place(std::string& idt_alias, IdtModification mon_mod, IdtModification alias_mod, bool has_prev_mon)
{
    if (mon_mod == IdtModification::FIVE_PRIME_END && alias_mod == IdtModification::THREE_PRIME_END)
        throw Error("IDT alias '%s' cannot be used at five prime end.", idt_alias.c_str());
    else if (mon_mod == IdtModification::INTERNAL && alias_mod != IdtModification::INTERNAL) // only internal modifications can be used in internal position
        throw Error("IDT alias '%s' cannot be used at internal position.", idt_alias.c_str());
    else if (mon_mod == IdtModification::THREE_PRIME_END && alias_mod == IdtModification::FIVE_PRIME_END && has_prev_mon)
        throw Error("IDT alias '%s' cannot be used at three prime end.", idt_alias.c_str()); // 5' monomers not allowed at 3'
                                                                                             // If this is only one monomer(no prev) - it could be any mod
}

void SequenceLoader::loadIdt(BaseMolecule& mol)
{
    const auto IDT_DEF_SUGAR = "dR";
    const auto IDT_DEF_PHOSPHATE = "P";
    const auto IDT_MODIFIED_PHOSPHATE = "sP";
    constexpr int MAX_STD_TOKEN_SIZE = 2;
    _row = 0;
    mol.clear();
    std::string invalid_symbols;
    while (!_scanner.isEOF())
    {
        _seq_id = 0;
        _last_monomer_idx = -1;
        _col = 0;

        using token_t = std::pair<std::string, bool>;
        std::queue<token_t> tokens; // second=true if token folowed by *
        std::string cur_token;

        while (true)
        {
            if (_scanner.isEOL())
            {
                if (cur_token.size())
                    tokens.emplace(cur_token, false);
                break;
            }
            auto ch = _scanner.readChar();
            switch (ch)
            {
            case ' ':
                if (cur_token.size())
                    tokens.emplace(cur_token, false);
                continue;
            case '/': {
                if (cur_token.size())
                    throw Error("Sugar prefix could not be used with modified monomer.");
                // read till next '/'
                ch = 0;
                while (!_scanner.isEOL())
                {
                    ch = _scanner.readChar();
                    if (ch == '/')
                        break;
                    cur_token += ch;
                }
                if (ch != '/')
                    throw Error("Unexpected end of data");
                if (cur_token == "")
                    throw Error("Invalid modification: empty string.");
                if (cur_token.size() < 3)
                    throw Error("Invalid modification: %s.", cur_token.c_str());
                cur_token += ch;
                break;
            }
            case 'A':
            case 'T':
            case 'C':
            case 'G':
            case 'U':
            case 'I':
                cur_token += ch;
                break;
            case 'r':
            case '+':
            case 'm':
                if (cur_token.size())
                    throw Error("Sugar prefix '%s' whithout base.", cur_token.c_str());
                else
                    cur_token += ch;
                continue;
                break;
            default:
                if (invalid_symbols.size())
                    invalid_symbols += ',';
                invalid_symbols += ch;
                continue;
                break;
            }

            if (_scanner.lookNext() == '*')
            {
                tokens.emplace(cur_token, true);
                _scanner.skip(1);
            }
            else
                tokens.emplace(cur_token, false);
            cur_token = "";
        }
        while (!_scanner.isEOF() && _scanner.isEOL()) // Skip EOL characters
            _scanner.skip(1);

        IdtModification modification = IdtModification::FIVE_PRIME_END;

        token_t prev_token;
        while (tokens.size() > 0)
        {
            token_t token = tokens.front();
            tokens.pop();

            std::string phosphate = IDT_DEF_PHOSPHATE;
            std::string sugar = IDT_DEF_SUGAR;
            std::string idt_alias = "";
            std::string base = "";
            std::string single_monomer = "";
            std::string single_monomer_class;
            bool unresolved = false;

            if (token.first.back() == '/')
            {
                token.first.pop_back();
                idt_alias = token.first;
                if ((idt_alias == "5Phos" || idt_alias == "3Phos") && (token.second || prev_token.second))
                    throw Error("Symbol '*' could be placed only between two nucleotides/nucleosides.");
            }
            else
            {
                if (token.first.size() > MAX_STD_TOKEN_SIZE)
                    throw Error("Wrong IDT syntax: '%s'", token.first.c_str());
                idt_alias = token.first.back();
                if (token.first.size() > 1)
                {
                    switch (token.first[0])
                    {
                    case 'r':
                        sugar = "R";
                        break;
                    case '+':
                        sugar = "LR";
                        break;
                    case 'm':
                        sugar = "mR";
                        break;
                    default:
                        throw Error("Wrong IDT syntax: '%s'", token.first.c_str());
                    }
                }
            }

            if (idt_alias.size() == 1)
            {
                if (IDT_STANDARD_BASES.count(idt_alias) == 0)
                {
                    if (invalid_symbols.size())
                        invalid_symbols += ',';
                    invalid_symbols += idt_alias[0];
                }
                else
                {
                    base = idt_alias;

                    if (tokens.size() == 0)
                    {
                        if (token.second)
                            throw Error("Invalid IDT sequence: '*' couldn't be the last symbol.");
                        modification = IdtModification::THREE_PRIME_END;
                        phosphate = "";
                    }
                    else if (token.second)
                    {
                        phosphate = IDT_MODIFIED_PHOSPHATE;
                    }

                    if (!checkAddTemplate(mol, MonomerClass::Sugar, sugar))
                        throw Error("Unknown sugar '%s'", sugar.c_str());
                    if (idt_alias.size() > 0 && !checkAddTemplate(mol, MonomerClass::Base, base))
                        throw Error("Unknown base '%s'", idt_alias.c_str());
                    if (phosphate.size() > 0 && !checkAddTemplate(mol, MonomerClass::Phosphate, phosphate))
                        throw Error("Unknown phosphate '%s'", phosphate.c_str());
                }
            }
            else
            {
                if (tokens.size() == 0)
                {
                    modification = IdtModification::THREE_PRIME_END;
                    // Corner case: /3Phos/ after standard monomer - no additional P should be added
                    if (prev_token.first.size() > 0 && prev_token.first.size() <= MAX_STD_TOKEN_SIZE && idt_alias == "3Phos")
                        continue;
                }

                sugar = "";
                IdtModification alias_mod;
                const std::string& mgt_id = _library.getMGTidByIdtAlias(idt_alias, alias_mod);
                if (mgt_id.size())
                {
                    // Check that alias modification can be used in current position
                    check_monomer_place(idt_alias, modification, alias_mod, prev_token.first.size() > 0);
                    MonomerGroupTemplate& mgt = _library.getMonomerGroupTemplateById(mgt_id);
                    const MonomerTemplate& sugar_template = mgt.getTemplateByClass(MonomerClass::Sugar);
                    sugar = sugar_template.getStringProp("alias");
                    checkAddTemplate(mol, sugar_template);
                    if (alias_mod == IdtModification::THREE_PRIME_END)
                    {
                        if (token.second)
                            throw Error("Monomer /%s/ doesn't have phosphate, so '*' couldn't be applied.", idt_alias.c_str());
                        phosphate = "";
                    }
                    else
                    {
                        if (mgt.hasTemplateClass(MonomerClass::Phosphate))
                        {
                            if (token.second) // * means that 'sP' should be used
                            {
                                phosphate = IDT_MODIFIED_PHOSPHATE;
                                checkAddTemplate(mol, MonomerClass::Phosphate, phosphate);
                            }
                            else // use phosphate from template
                            {
                                const MonomerTemplate& phosphate_template = mgt.getTemplateByClass(MonomerClass::Phosphate);
                                phosphate = phosphate_template.getStringProp("alias");
                                checkAddTemplate(mol, phosphate_template);
                            }
                        }
                        else
                        {
                            if (token.second)
                                throw Error("Monomer /%s/ doesn't have phosphate, so '*' couldn't be applied.", idt_alias.c_str());
                            phosphate = "";
                        }
                    }
                    if (mgt.hasTemplateClass(MonomerClass::Base))
                    {
                        const MonomerTemplate& base_template = mgt.getTemplateByClass(MonomerClass::Base);
                        base = base_template.getStringProp("alias");
                        checkAddTemplate(mol, base_template);
                    }
                }
                else
                {
                    IdtModification alias_mod;
                    auto monomer_template_id = _library.getMonomerTemplateIdByIdtAlias(idt_alias, alias_mod);
                    if (monomer_template_id.size())
                    {
                        if (token.second)
                            throw Error("'*' couldn't be applied to monomer /%s/.", idt_alias.c_str());
                        check_monomer_place(idt_alias, modification, alias_mod, prev_token.first.size() > 0);
                        const MonomerTemplate& monomer_template = _library.getMonomerTemplateById(monomer_template_id);
                        checkAddTemplate(mol, monomer_template);
                        single_monomer = monomer_template.getStringProp("alias");
                        single_monomer_class = MonomerTemplates::classToStr(monomer_template.monomerClass());
                    }
                    else // IDT alias not found
                    {
                        unresolved = true;
                        single_monomer = "unknown_monomer_with_idt_alias_" + idt_alias;
                        auto monomer_class = MonomerClass::CHEM;
                        single_monomer_class = MonomerTemplates::classToStr(monomer_class);
                        // Unresoved monomer could be in any position
                        MonomerTemplate monomer_template(single_monomer, monomer_class, IdtAlias(idt_alias, idt_alias, idt_alias, idt_alias), true);
                        monomer_template.setStringProp("alias", idt_alias);
                        for (auto ap : {"R1", "R2", "R3", "R4"})
                            monomer_template.AddAttachmentPoint(ap, -1);
                        checkAddTemplate(mol, monomer_template);
                    }
                }
            }

            if (single_monomer.size())
            {
                int monomer_idx = addTemplateAtom(mol, unresolved ? idt_alias.c_str() : single_monomer.c_str(), single_monomer_class.c_str(), _seq_id);
                mol.asMolecule().setAtomXyz(monomer_idx, getBackboneMonomerPosition());
                if (_last_monomer_idx >= 0)
                    addTemplateBond(mol, _last_monomer_idx, monomer_idx);
                _last_monomer_idx = monomer_idx;
            }
            else
                addNucleotide(mol, base, sugar, phosphate, false);

            _seq_id++;
            _col++;

            prev_token = token; // save to check */3Phos/ case
            modification = IdtModification::INTERNAL;
        }
        _row += 2;
    }

    if (invalid_symbols.size())
        throw Error("Invalid symbols in the sequence: %s", invalid_symbols.c_str());
}

void SequenceLoader::loadIdt(KetDocument& document)
{
    const auto IDT_DEF_SUGAR = "dR";
    const auto IDT_DEF_PHOSPHATE = "P";
    const auto IDT_MODIFIED_PHOSPHATE = "sP";
    constexpr int MAX_STD_TOKEN_SIZE = 2;
    _row = 0;
    std::string invalid_symbols;
    while (!_scanner.isEOF())
    {
        _seq_id = 0;
        _last_monomer_idx = -1;
        _col = 0;

        using token_t = std::pair<std::string, bool>;
        std::queue<token_t> tokens; // second=true if token folowed by *
        std::string cur_token;

        while (true)
        {
            if (_scanner.isEOL())
            {
                if (cur_token.size())
                    tokens.emplace(cur_token, false);
                break;
            }
            auto ch = _scanner.readChar();

            switch (ch)
            {
            case ' ':
                if (cur_token.size())
                    tokens.emplace(cur_token, false);
                continue;
            case '/': {
                if (cur_token.size())
                    throw Error("Sugar prefix could not be used with modified monomer.");
                // read till next '/'
                ch = 0;
                while (!_scanner.isEOL())
                {
                    ch = _scanner.readChar();
                    if (ch == '/')
                        break;
                    cur_token += ch;
                }
                if (ch != '/')
                    throw Error("Unexpected end of data");
                if (cur_token == "")
                    throw Error("Invalid modification: empty string.");
                if (cur_token.size() < 3)
                    throw Error("Invalid modification: %s.", cur_token.c_str());
                cur_token += ch;
                break;
            }
            case '(': { // read till ')'
                cur_token += ch;
                ch = 0;
                while (!_scanner.isEOL())
                {
                    ch = _scanner.readChar();
                    if (ch == ')')
                        break;
                    cur_token += ch;
                }
                if (ch != ')')
                    throw Error("Unexpected end of data");
                if (cur_token == "")
                    throw Error("Invalid variant monomer: empty string.");
                cur_token += ch;
                break;
            }
            case 'A':
            case 'T':
            case 'C':
            case 'G':
            case 'U':
            case 'I':
            case 'R':
            case 'Y':
            case 'M':
            case 'K':
            case 'S':
            case 'W':
            case 'H':
            case 'B':
            case 'V':
            case 'D':
            case 'N':
                cur_token += ch;
                break;
            case 'r':
            case '+':
            case 'm':
                if (cur_token.size())
                    throw Error("Sugar prefix '%s' whithout base.", cur_token.c_str());
                else
                    cur_token += ch;
                continue;
                break;
            default:
                if (invalid_symbols.size())
                    invalid_symbols += ',';
                invalid_symbols += ch;
                continue;
                break;
            }

            if (_scanner.lookNext() == '*')
            {
                tokens.emplace(cur_token, true);
                _scanner.skip(1);
            }
            else
                tokens.emplace(cur_token, false);
            cur_token = "";
        }
        while (!_scanner.isEOF() && _scanner.isEOL()) // Skip EOL characters
            _scanner.skip(1);

        IdtModification modification = IdtModification::FIVE_PRIME_END;

        token_t prev_token;
        while (tokens.size() > 0)
        {
            token_t token = tokens.front();
            tokens.pop();

            std::string phosphate = IDT_DEF_PHOSPHATE;
            std::string sugar = IDT_DEF_SUGAR;
            std::string idt_alias = "";
            std::string base = "";
            std::string single_monomer = "";
            std::string single_monomer_alias = "";
            std::string single_monomer_class;
            bool unresolved = false;
            bool variant_monomer = false;

            if (token.first.back() == '/')
            {
                token.first.pop_back();
                idt_alias = token.first;
                if ((idt_alias == "5Phos" || idt_alias == "3Phos") && (token.second || prev_token.second))
                    throw Error("Symbol '*' could be placed only between two nucleotides/nucleosides.");
            }
            else
            {

                if (token.first.size() > MAX_STD_TOKEN_SIZE)
                    if (token.first.back() == ')')
                        idt_alias = token.first;
                    else
                        throw Error("Wrong IDT syntax: '%s'", token.first.c_str());
                else
                    idt_alias = token.first.back();
                if (token.first.size() > 1)
                {
                    auto ch = token.first[0];
                    if (ch != '(')
                    {
                        switch (ch)
                        {
                        case 'r':
                            sugar = "R";
                            break;
                        case '+':
                            sugar = "LR";
                            break;
                        case 'm':
                            sugar = "mR";
                            break;
                        default:
                            throw Error("Wrong IDT syntax: '%s'", token.first.c_str());
                        }
                        if (idt_alias.back() == ')')
                            idt_alias.erase(0, 1);
                    }
                }
            }

            if (STANDARD_MIXED_BASES.count(idt_alias) != 0 || idt_alias.back() == ')')
                variant_monomer = true;

            if (idt_alias.size() == 1 || variant_monomer)
            {
                if (IDT_STANDARD_BASES.count(idt_alias) == 0 && !variant_monomer)
                {
                    if (invalid_symbols.size())
                        invalid_symbols += ',';
                    invalid_symbols += idt_alias[0];
                    continue;
                }

                if (variant_monomer)
                {
                    auto mixed_base = idt_alias;
                    std::optional<std::array<float, 4>> ratios;
                    if (mixed_base.back() == ')')
                    {
                        mixed_base = idt_alias.substr(1, idt_alias.size() - 2);
                        if (auto pos = mixed_base.find(':'); pos != std::string::npos)
                        {
                            auto ratios_str = mixed_base.substr(pos + 1, mixed_base.size() - pos - 1);
                            mixed_base = mixed_base.substr(0, pos);
                            if (ratios_str.size() != 8)
                                throw Exception("Invalid IDT variant monomer %s", idt_alias.c_str());
                            ratios.emplace(std::array<float, 4>{std::stof(ratios_str.substr(0, 2)), std::stof(ratios_str.substr(2, 2)),
                                                                std::stof(ratios_str.substr(4, 2)), std::stof(ratios_str.substr(6, 2))});
                            idt_alias = '(' + mixed_base + ')';
                            mixed_base = mixed_base[0];
                        }
                    }
                    if (sugar == "R")
                        idt_alias = 'r' + idt_alias;
                    if (!document.hasVariantMonomerTemplate(idt_alias))
                    {
                        auto it = STANDARD_MIXED_BASES.find(mixed_base);
                        if (it == STANDARD_MIXED_BASES.end())
                            throw Error("Unknown mixed base '%s'", mixed_base.c_str());

                        std::vector<KetVariantMonomerOption> options;
                        for (auto template_alias : it->second)
                        {
                            if (sugar == "r" && template_alias == "T") // U instead of T for RNA
                                template_alias = "U";
                            auto& template_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Base, template_alias);
                            if (template_id.size() == 0)
                                throw Error("Monomer base template '%s' not found", template_alias.c_str());
                            auto& option = options.emplace_back(template_id);
                            if (ratios.has_value())
                            {
                                option.setRatio(ratios.value()[IDT_BASE_TO_RATIO_IDX.at(template_alias)]);
                            }
                            auto& monomer_template = _library.getMonomerTemplateById(template_id);
                            checkAddTemplate(document, monomer_template);
                            _alias_to_id.emplace(template_alias, template_id);
                        }
                        document.addVariantMonomerTemplate("mixture", idt_alias, idt_alias, IdtAlias(), options);
                        _alias_to_id.emplace(idt_alias, idt_alias);
                    }
                    else if (ratios.has_value())
                        throw Error("Variant monomer %s redefinion", idt_alias.c_str());
                }
                base = idt_alias;

                if (base == "I")
                    base = "In"; // use correct alias for Inosine

                if (tokens.size() == 0)
                {
                    if (token.second)
                        throw Error("Invalid IDT sequence: '*' couldn't be the last symbol.");
                    modification = IdtModification::THREE_PRIME_END;
                    phosphate = "";
                }
                else if (token.second)
                {
                    phosphate = IDT_MODIFIED_PHOSPHATE;
                }

                _alias_to_id.emplace(sugar, checkAddTemplate(document, MonomerClass::Sugar, sugar));
                if (base.size() > 0 && !variant_monomer)
                    _alias_to_id.emplace(base, checkAddTemplate(document, MonomerClass::Base, base));
                if (phosphate.size() > 0)
                    _alias_to_id.emplace(phosphate, checkAddTemplate(document, MonomerClass::Phosphate, phosphate));
            }
            else
            {
                if (tokens.size() == 0)
                {
                    modification = IdtModification::THREE_PRIME_END;
                    // Corner case: /3Phos/ after standard monomer - no additional P should be added
                    if (prev_token.first.size() > 0 && prev_token.first.size() <= MAX_STD_TOKEN_SIZE && idt_alias == "3Phos")
                        continue;
                }

                sugar = "";
                IdtModification alias_mod;
                const std::string& mgt_id = _library.getMGTidByIdtAlias(idt_alias, alias_mod);
                if (mgt_id.size())
                {
                    // Check that alias modification can be used in current position
                    check_monomer_place(idt_alias, modification, alias_mod, prev_token.first.size() > 0);
                    MonomerGroupTemplate& mgt = _library.getMonomerGroupTemplateById(mgt_id);
                    const MonomerTemplate& sugar_template = mgt.getTemplateByClass(MonomerClass::Sugar);
                    sugar = sugar_template.getStringProp("alias");
                    _alias_to_id.emplace(sugar, sugar_template.id());
                    checkAddTemplate(document, sugar_template);
                    if (alias_mod == IdtModification::THREE_PRIME_END)
                    {
                        if (token.second)
                            throw Error("Monomer /%s/ doesn't have phosphate, so '*' couldn't be applied.", idt_alias.c_str());
                        phosphate = "";
                    }
                    else
                    {
                        if (mgt.hasTemplateClass(MonomerClass::Phosphate))
                        {
                            if (token.second) // * means that 'sP' should be used
                            {
                                phosphate = IDT_MODIFIED_PHOSPHATE;
                                _alias_to_id.emplace(phosphate, checkAddTemplate(document, MonomerClass::Phosphate, phosphate));
                            }
                            else // use phosphate from template
                            {
                                const MonomerTemplate& phosphate_template = mgt.getTemplateByClass(MonomerClass::Phosphate);
                                phosphate = phosphate_template.getStringProp("alias");
                                _alias_to_id.emplace(phosphate, phosphate_template.id());
                                checkAddTemplate(document, phosphate_template);
                            }
                        }
                        else
                        {
                            if (token.second)
                                throw Error("Monomer /%s/ doesn't have phosphate, so '*' couldn't be applied.", idt_alias.c_str());
                            phosphate = "";
                        }
                    }
                    if (mgt.hasTemplateClass(MonomerClass::Base))
                    {
                        const MonomerTemplate& base_template = mgt.getTemplateByClass(MonomerClass::Base);
                        base = base_template.getStringProp("alias");
                        _alias_to_id.emplace(base, base_template.id());
                        checkAddTemplate(document, base_template);
                    }
                }
                else
                {
                    IdtModification alias_mod;
                    auto& monomer_template_id = _library.getMonomerTemplateIdByIdtAlias(idt_alias, alias_mod);
                    if (monomer_template_id.size())
                    {
                        if (token.second)
                            throw Error("'*' couldn't be applied to monomer /%s/.", idt_alias.c_str());
                        check_monomer_place(idt_alias, modification, alias_mod, prev_token.first.size() > 0);
                        const MonomerTemplate& monomer_template = _library.getMonomerTemplateById(monomer_template_id);
                        checkAddTemplate(document, monomer_template);
                        single_monomer = monomer_template_id;
                        single_monomer_alias = monomer_template.getStringProp("alias");
                        single_monomer_class = MonomerTemplates::classToStr(monomer_template.monomerClass());
                    }
                    else // IDT alias not found
                    {
                        unresolved = true;
                        single_monomer = "unknown_monomer_with_idt_alias_" + idt_alias;
                        single_monomer_alias = idt_alias;
                        auto monomer_class = MonomerClass::CHEM;
                        single_monomer_class = MonomerTemplates::classToStr(monomer_class);
                        // Unresoved monomer could be in any position
                        MonomerTemplate monomer_template(single_monomer, monomer_class, IdtAlias(idt_alias, idt_alias, idt_alias, idt_alias), true);
                        monomer_template.setStringProp("alias", idt_alias);
                        for (auto ap : {"R1", "R2", "R3", "R4"})
                            monomer_template.AddAttachmentPoint(ap, -1);
                        checkAddTemplate(document, monomer_template);
                    }
                }
            }

            if (single_monomer.size())
            {
                auto monomer_idx = document.monomers().size();
                auto& monomer = document.addMonomer(single_monomer_alias, single_monomer);
                monomer->setIntProp("seqid", _seq_id);
                monomer->setPosition(getBackboneMonomerPosition());
                if (_last_monomer_idx >= 0)
                    addMonomerConnection(document, _last_monomer_idx, monomer_idx);
                _last_monomer_idx = static_cast<int>(monomer_idx);
            }
            else
                addNucleotide(document, base, sugar, phosphate, false, variant_monomer);

            _seq_id++;
            _col++;

            prev_token = token; // save to check */3Phos/ case
            modification = IdtModification::INTERNAL;
        }
        _row += 2;
    }

    if (invalid_symbols.size())
        throw Error("Invalid symbols in the sequence: %s", invalid_symbols.c_str());
}

static std::set<std::string> polymer_types{kHELMPolymerTypePEPTIDE, kHELMPolymerTypeRNA, kHELMPolymerTypeCHEM, kHELMPolymerTypeUnknown};
static const char* reserved_helm_chars = "${}|.,-:[]()";
static const char* unexpected_eod = unexpected_eod;

std::string SequenceLoader::readHelmMonomerAlias(KetDocument& document, MonomerClass monomer_class)
{
    std::string monomer_alias;
    auto ch = _scanner.lookNext();

    if (ch == '[')
    {
        _scanner.skip(1);
        bool smiles = false;
        for (int bracket_count = 1; bracket_count != 0 && !_scanner.isEOF();)
        {
            ch = _scanner.readChar();
            switch (ch)
            {
            case '[':
                bracket_count++;
                monomer_alias += ch;
                smiles = true;
                break;
            case ']':
                bracket_count--;
                if (bracket_count > 0)
                    monomer_alias += ch;
                break;
            default:
                monomer_alias += ch;
                break;
            }
        }
        if (_scanner.isEOF())
            throw Error(unexpected_eod);
        if (ch != ']')
            throw Error("Unexpected char. Expected ']' but found '%c'.", ch);
        if (smiles)
        {
            // Convert smiles to molecule
            BufferScanner scanner(monomer_alias.c_str());
            SmilesLoader loader(scanner);
            Molecule mol{};
            loader.loadMolecule(mol);
            MoleculeLayout ml(mol, false);
            ml.layout_orientation = UNCPECIFIED;
            ml.make();
            // create template based on molecule
            monomer_alias = "Mod" + std::to_string(_unknown_variants_count++);
            auto& mon_template = document.addMonomerTemplate(monomer_alias, MonomerTemplate::MonomerClassToStr(monomer_class), IdtAlias());
            mon_template.setStringProp("alias", monomer_alias);
            std::map<int, int> rgroups;
            std::map<int, int> rg_to_attatom;
            std::vector<KetBond> bonds;
            for (auto i : mol.vertices())
            {
                if (mol.isRSite(i))
                {
                    const auto& vertex = mol.getVertex(i);
                    if (vertex.degree() != 1)
                        throw Error("Attachment point should be connected to single atom");
                    rg_to_attatom.emplace(i, vertex.neiVertex(vertex.neiBegin()));
                    rgroups.emplace(i, mol.getSingleAllowedRGroup(i));
                    mon_template.AddAtom("H", mol.getAtomXyz(i));
                }
                else
                {
                    int anum = mol.getAtomNumber(i);
                    std::string label;
                    if (anum == VALUE_UNKNOWN)
                        throw Error("Unknown element");
                    int isotope = mol.getAtomIsotope(i);
                    mon_template.AddAtom(Element::toString(anum, isotope), mol.getAtomXyz(i));
                }
            }
            for (auto i : mol.edges())
            {
                auto edge = mol.getEdge(i);
                int bond_order = mol.getBondOrder(i);
                if (bond_order == BOND_ZERO)
                {
                    bond_order = _BOND_COORDINATION;
                    const Edge& edge = mol.getEdge(i);
                    if ((mol.getAtomNumber(edge.beg) == ELEM_H) || (mol.getAtomNumber(edge.end) == ELEM_H))
                        bond_order = _BOND_HYDROGEN;
                }
                mon_template.AddBond(bond_order, edge.beg, edge.end);
            }
            for (auto& it : rgroups)
            {
                std::string label = 'R' + std::to_string(it.second);
                auto& att_point = mon_template.AddAttachmentPoint(label, rg_to_attatom.at(it.first));
                std::vector<int> lg;
                lg.emplace_back(it.first);
                att_point.setLeavingGroup(lg);
            }
            _added_templates.emplace(monomer_class, monomer_alias);
        }
    }
    else if (ch != -1)
    {
        _scanner.skip(1);
        monomer_alias = ch;
    }
    if (_scanner.isEOF())
        throw Error(unexpected_eod);
    return monomer_alias;
}

std::string SequenceLoader::readHelmRepeating()
{
    std::string repeating;
    if (_scanner.lookNext() == '\'')
    {
        // repeating
        Array<char> name;
        _scanner.skip(1);
        _scanner.readWord(name, "'");
        if (_scanner.lookNext() != '\'')
            throw Error("Unexpected char. Expected ''' but found '%c'.", _scanner.lookNext());
        _scanner.skip(1); // skip "'"
        repeating = name.ptr();
    }
    return repeating;
}

std::string SequenceLoader::readHelmAnnotation()
{
    std::string annotation;
    if (_scanner.lookNext() == '"') // inline annotation
    {
        Array<char> name;
        _scanner.skip(1);
        _scanner.readWord(name, "\"");
        if (_scanner.lookNext() != '"')
            throw Error("Unexpected char. Expected '\"' but found '%c'.", _scanner.lookNext());
        _scanner.skip(1); // skip '"'
        annotation = name.ptr();
    }
    return annotation;
}

int SequenceLoader::readCount(std::string& count, Scanner& _scanner)
{
    count.clear();
    int ch = _scanner.lookNext();
    if (ch == ':')
    {
        _scanner.skip(1);
        ch = _scanner.lookNext();
        while (std::isdigit(ch) && !_scanner.isEOF())
        {
            _scanner.skip(1);
            count += ch;
            ch = _scanner.lookNext();
        }
        if (count.size() == 0)
            throw Error("Invalid number.");
    }
    return ch;
}

SequenceLoader::MonomerInfo SequenceLoader::readHelmMonomer(KetDocument& document, MonomerClass monomer_class)
{
    std::string monomer_alias, repeating, annotation;
    variant_template_opts options;
    int ch = _scanner.lookNext();
    bool was_bracket = false;
    if (ch == '(')
    {
        _scanner.skip(1);
        was_bracket = true;
    }
    monomer_alias = readHelmMonomerAlias(document, monomer_class);
    ch = _scanner.lookNext();
    bool is_variant = false;

    if (ch == ',' || ch == '+' || ch == ':') // looks like variant
    {
        if (!was_bracket)
            throw Error("Unexpected symbol %c. Variant monomers should be defined in ().");
        std::string count;
        is_variant = true;
        ch = readCount(count, _scanner); // in ch==':' read conunt and return next char
        bool is_mixture = false;
        if (ch == '+')
            is_mixture = true;
        else if (ch != ',')
            throw Error("Unexpected symbol %c. Expected '+' or ','");
        _scanner.skip(1);

        std::set<std::string> aliases;
        std::string opt_alias = monomer_alias;
        options.first = is_mixture;
        bool no_counts = true;
        while (true)
        {
            aliases.emplace(opt_alias);
            auto& opt = options.second.emplace_back(opt_alias, std::optional<float>());
            if (count.size() > 0)
            {
                opt.second = std::stof(count);
                no_counts = false;
            }
            if (ch == ')')
                break;
            opt_alias = readHelmMonomerAlias(document, monomer_class);
            if (aliases.count(opt_alias) > 0)
                throw Error("Ivalid variant monomer. Monomer '%s' repeated more than once.", opt_alias.c_str());
            ch = readCount(count, _scanner);
            if (is_mixture && ch != '+' && ch != ')')
                throw Error("Invalid variant monomer. Expected '+' but found '%c'", ch);
            else if (!is_mixture && ch != ',' && ch != ')')
                throw Error("Invalid variant monomer. Expected ',' but found '%c'", ch);
            _scanner.skip(1); // skip delimiter or ')'
        }
        if (monomer_class == MonomerClass::AminoAcid)
        {
            if (STANDARD_MIXED_PEPTIDES_TO_ALIAS.count(aliases) > 0)
                if (is_mixture && no_counts)
                    monomer_alias = STANDARD_MIXED_PEPTIDES_TO_ALIAS.at(aliases);
                else
                    monomer_alias = STANDARD_MIXED_PEPTIDES_TO_ALIAS.at(aliases) + std::to_string(_unknown_variants_count++);
            else
                monomer_alias = "Var" + std::to_string(_unknown_variants_count++);
        }
        else if (monomer_class == MonomerClass::Base)
        {
            if (is_mixture && STANDARD_MIXED_BASES_TO_ALIAS.count(aliases) > 0)
                if (is_mixture && no_counts)
                    monomer_alias = STANDARD_MIXED_BASES_TO_ALIAS.at(aliases);
                else
                    monomer_alias = STANDARD_MIXED_BASES_TO_ALIAS.at(aliases) + std::to_string(_unknown_variants_count++);
            else
                monomer_alias = "Var" + std::to_string(_unknown_variants_count++);
        }
        else
        {
            monomer_alias = "Var" + std::to_string(_unknown_variants_count++);
        }
    }
    repeating = readHelmRepeating();
    annotation = readHelmAnnotation();
    ch = _scanner.lookNext();
    if (was_bracket && !is_variant) // for variants ')' already processed
        if (ch == ')')
            _scanner.skip(1); // single monomer in () - branch monomer
        else
            throw Error("Unmatched '('");
    return std::make_tuple(monomer_alias, repeating, annotation, options);
}

std::string SequenceLoader::readHelmSimplePolymerName(std::string& polymer_name)
{
    auto ch = _scanner.lookNext();
    while (std::isalpha(ch) && !_scanner.isEOF())
    {
        _scanner.skip(1);
        polymer_name += std::toupper(ch);
        ch = _scanner.lookNext();
    }
    std::string polymer_type = polymer_name;
    if (polymer_types.count(polymer_name) == 0)
        throw Error("Unknown polymer type '%s'.", polymer_name.c_str());
    while (std::isdigit(ch) && !_scanner.isEOF())
    {
        _scanner.skip(1);
        polymer_name += ch;
        ch = _scanner.lookNext();
    }
    return polymer_type;
}

const std::string SequenceLoader::checkAddVariantMonomerTemplate(KetDocument& document, const std::string& alias, MonomerClass monomer_class,
                                                                 variant_template_opts& options)
{
    std::string template_id;
    const auto& it = _opts_to_template_id.find(options);
    if (it != _opts_to_template_id.end())
    {
        template_id = it->second;
    }
    else
    {
        bool is_mixture = options.first;
        std::string subtype = is_mixture ? "mixture" : "alternatives";
        std::vector<KetVariantMonomerOption> opts;
        for (auto& option : options.second)
        {
            auto& opt_template_id = _library.getMonomerTemplateIdByAlias(monomer_class, option.first);
            if (opt_template_id.size() == 0)
                throw Error("Monomer base template '%s' not found", option.first.c_str());
            auto& opt = opts.emplace_back(opt_template_id);
            if (option.second.has_value())
                if (is_mixture)
                    opt.setRatio(option.second.value());
                else
                    opt.setProbability(option.second.value());
            auto& monomer_template = _library.getMonomerTemplateById(opt_template_id);
            checkAddTemplate(document, monomer_template);
        }

        auto& var_template = document.addVariantMonomerTemplate(subtype, alias, alias, IdtAlias(), opts);
        template_id = alias;
        _opts_to_template_id.emplace(options, template_id);
    }
    return template_id;
}

size_t SequenceLoader::addKetMonomer(KetDocument& document, MonomerInfo info, MonomerClass monomer_class, const Vec3f& pos)
{
    auto [alias, repeating, annotaion, options] = info;
    if (repeating.size() && monomer_class == MonomerClass::CHEM)
        throw Error("Chem cannot be repeated.");
    if (repeating.size() && (monomer_class == MonomerClass::Base || monomer_class == MonomerClass::Sugar || monomer_class == MonomerClass::Phosphate))
        throw Error("RNA parts cannot be repeated.");
    if (repeating.size() > 0)
        throw Error("Repeating not supported now.");
    auto monomer_idx = document.monomers().size();
    if (options.second.size() > 0) // variant monomer
    {
        std::string template_id = checkAddVariantMonomerTemplate(document, alias, monomer_class, options);
        auto& monomer = document.addVariantMonomer(alias, template_id);
        monomer->setAttachmentPoints(document.variantTemplates().at(template_id).attachmentPoints());
        monomer->setIntProp("seqid", _seq_id++);
        monomer->setPosition(pos);
    }
    else
    {
        const std::string& template_id = checkAddTemplate(document, monomer_class, alias);
        _alias_to_id.emplace(alias, template_id);
        auto& monomer = document.addMonomer(alias, template_id);
        monomer->setAttachmentPoints(document.templates().at(template_id).attachmentPoints());
        monomer->setIntProp("seqid", _seq_id++);
        monomer->setPosition(pos);
    }
    return monomer_idx;
}

void SequenceLoader::loadHELM(KetDocument& document)
{
    _row = 0;
    _seq_id = 1;
    std::string simple_polymer_name = "";
    std::string simple_polymer_type = "";
    int monomer_idx = 0;
    int prev_monomer_template_atom_idx = -1;
    _unknown_variants_count = 0;
    using polymer_map = std::map<std::string, std::map<int, size_t>>;
    polymer_map used_polymer_nums;
    polymer_map::iterator cur_polymer_map;
    _opts_to_template_id.clear();
    enum class helm_parts
    {
        ListOfSimplePolymers,
        ListOfConnections,
        ListOfPolymerGroups,
        ExtendedAnnotation,
        End
    };
    helm_parts helm_part = helm_parts::ListOfSimplePolymers;
    while (!_scanner.isEOF())
    {
        if (helm_part == helm_parts::ListOfSimplePolymers)
        {
            auto ch = _scanner.lookNext();
            if (simple_polymer_name.size() == 0) // Read simple polymer_name
            {
                _col = 0;
                simple_polymer_type = readHelmSimplePolymerName(simple_polymer_name);
                if (used_polymer_nums.count(simple_polymer_name))
                    throw Error("Simple polymer '%s' defined more than once.", simple_polymer_name.c_str());
                if (simple_polymer_name == simple_polymer_type)
                    throw Error("Polymer '%s' without number not allowed.", simple_polymer_name.c_str());
                ch = _scanner.lookNext();
                if (ch != '{')
                    throw Error("Unexpected symbol. Expected '{' but found '%c'.", ch);
                _scanner.skip(1); // skip '{'
                if (used_polymer_nums.count(simple_polymer_name))
                    throw Error("Simple polymer '%s' defined more than once.", simple_polymer_name.c_str());
                auto res = used_polymer_nums.emplace(std::make_pair(simple_polymer_name, std::map<int, size_t>()));
                if (res.second)
                    cur_polymer_map = res.first;
                else
                    throw Error("Internal error - cannot emplace polymer map.");
            }
            else if (ch != '}')
            {
                monomer_idx++;
                Vec3f pos(_col * MoleculeLayout::DEFAULT_BOND_LENGTH, -MoleculeLayout::DEFAULT_BOND_LENGTH * _row, 0);
                _col++;
                if (simple_polymer_type == kHELMPolymerTypeUnknown)
                {
                    std::string name;
                    _scanner.readWord(name, reserved_helm_chars);
                    // skip blob for now
                    ch = _scanner.lookNext();
                    if (ch != '}')
                        throw Error("Unexpected symbol. Expected '}' but found '%c'.", ch);
                    continue;
                }
                const auto& monomer_class = MonomerTemplates::getStrToMonomerType().at(simple_polymer_type);
                auto monomer_info = readHelmMonomer(document, monomer_class);
                if (monomer_class == MonomerClass::CHEM)
                {
                    ch = _scanner.lookNext();
                    if (ch != '}')
                        throw Error("Unexpected symbol. Expected '}' but found '%c'.", ch); // only one monomer in chem
                    cur_polymer_map->second[monomer_idx] = addKetMonomer(document, monomer_info, monomer_class, pos);
                }
                else if (monomer_class == MonomerClass::AminoAcid)
                {
                    auto amino_idx = addKetMonomer(document, monomer_info, monomer_class, pos);
                    cur_polymer_map->second[monomer_idx] = amino_idx;
                    if (monomer_idx > 1)
                        addMonomerConnection(document, amino_idx - 1, amino_idx);
                    ch = _scanner.lookNext();
                    if (ch == '.')
                        _scanner.skip(1);
                }
                else // kHELMPolymerTypeRNA
                {
                    const std::string& phosphate_lib_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, std::get<0>(monomer_info));
                    if (phosphate_lib_id.size())
                    {
                        // add phosphate
                        auto phosphate_idx = addKetMonomer(document, monomer_info, MonomerClass::Phosphate, pos);
                        cur_polymer_map->second[monomer_idx] = phosphate_idx;
                        if (monomer_idx > 1)
                            addMonomerConnection(document, phosphate_idx - 1, phosphate_idx);
                        ch = _scanner.lookNext();
                        if (ch != '.' && ch != '}')
                            throw Error("Unexpected symbol. Expected '.' or '}' but found '%c'.", ch);
                        if (ch == '.')
                            _scanner.skip(1);
                        continue;
                    }
                    auto sugar_idx = addKetMonomer(document, monomer_info, MonomerClass::Sugar, pos);
                    cur_polymer_map->second[monomer_idx] = sugar_idx;
                    if (monomer_idx > 1)
                        addMonomerConnection(document, sugar_idx - 1, sugar_idx);
                    ch = _scanner.lookNext();
                    if (ch == '(') // In RNA after sugar could be base in ()
                    {
                        monomer_idx++;
                        auto base_info = readHelmMonomer(document, MonomerClass::Base);
                        ch = _scanner.lookNext();
                        Vec3f base_pos(pos.x, pos.y - MoleculeLayout::DEFAULT_BOND_LENGTH, 0);
                        auto base_idx = addKetMonomer(document, base_info, MonomerClass::Base, base_pos);
                        cur_polymer_map->second[monomer_idx] = base_idx;
                        if (monomer_idx > 1)
                            addMonomerConnection(document, sugar_idx, base_idx, true);
                    }
                    if (ch == '.')
                    {
                        _scanner.skip(1);
                        continue;
                    }
                    if (ch == '}')
                        continue;
                    auto phosphate_info = readHelmMonomer(document, MonomerClass::Phosphate);
                    monomer_idx++;
                    Vec3f phosphate_pos(_col * MoleculeLayout::DEFAULT_BOND_LENGTH, -MoleculeLayout::DEFAULT_BOND_LENGTH * _row, 0);
                    _col++;
                    auto phosphate_idx = addKetMonomer(document, phosphate_info, MonomerClass::Phosphate, phosphate_pos);
                    cur_polymer_map->second[monomer_idx] = phosphate_idx;
                    if (monomer_idx > 1)
                        addMonomerConnection(document, sugar_idx, phosphate_idx);
                    ch = _scanner.lookNext();
                    if (ch != '.' && ch != '}')
                        throw Error("Unexpected symbol. Expected '.' or '}' but found '%c'.", ch);
                    if (ch == '.')
                        _scanner.skip(1);
                }
            }
            else // end of polymer - }
            {
                _scanner.skip(1); // skip '}'
                ch = _scanner.lookNext();
                if (ch == '"')
                {
                    Array<char> annotation;
                    _scanner.skip(1);
                    _scanner.readWord(annotation, "\"");
                    if (_scanner.lookNext() != '"')
                        throw Error("Unexpected symbol. Expected '\"' but found '%c'.", _scanner.lookNext());
                    _scanner.skip(1);
                    // skip annotation for now
                    ch = _scanner.lookNext();
                }
                _row++;
                _col = 0;
                monomer_idx = 0;
                if (simple_polymer_type == kHELMPolymerTypeRNA)
                    _row++; // additional row for bases in RNA
                if (ch == '|')
                {
                    // cleanup to go to next simple polymer
                    simple_polymer_name = "";
                    simple_polymer_type = "";
                }
                else if (ch == '$')
                {
                    helm_part = helm_parts::ListOfConnections;
                }
                else if (ch == -1)
                {
                    throw Error(unexpected_eod);
                }
                else
                {
                    throw Error("Unexpected symbol. Expected '|' or '$' but found '%c'.", ch);
                }
                _scanner.skip(1);
            }
        }
        else if (helm_part == helm_parts::ListOfConnections)
        {
            auto ch = _scanner.lookNext();
            if (ch == '$')
            {
                helm_part = helm_parts::ListOfPolymerGroups;
                _scanner.skip(1);
                continue;
            }
            // CHEM1,RNA1,32:R1-12:R2"annotation"|.....
            std::string left_polymer, right_polymer;
            std::ignore = readHelmSimplePolymerName(left_polymer);
            auto left_polymer_nums = used_polymer_nums.find(left_polymer);
            if (left_polymer_nums == used_polymer_nums.end())
                throw Error("Polymer '%s' not found.", left_polymer.c_str());
            ch = _scanner.lookNext();
            if (ch != ',')
                throw Error("Unexpected symbol. Expected ',' but found '%c'.", _scanner.lookNext());
            _scanner.skip(1);
            std::ignore = readHelmSimplePolymerName(right_polymer);
            auto right_polymer_nums = used_polymer_nums.find(right_polymer);
            if (right_polymer_nums == used_polymer_nums.end())
                throw Error("Polymer '%s' not found.", right_polymer.c_str());
            ch = _scanner.lookNext();
            if (ch != ',')
                throw Error("Unexpected symbol. Expected ',' but found '%c'.", _scanner.lookNext());
            _scanner.skip(1);
            // read monomer position
            int left_monomer_idx, right_monomer_idx;
            std::string left_ap, right_ap;
            std::string position;
            size_t error_pos;
            _scanner.readWord(position, ":");
            _scanner.skip(1);
            left_monomer_idx = std::stoi(position, &error_pos);
            if (error_pos != position.size())
                throw Error("Only direct connections supported now.");
            _scanner.readWord(left_ap, "-");
            _scanner.skip(1);
            position.clear();
            _scanner.readWord(position, ":");
            _scanner.skip(1);
            right_monomer_idx = std::stoi(position, &error_pos);
            if (error_pos != position.size())
                throw Error("Only direct connections supported now.");
            _scanner.readWord(right_ap, "\"|$");
            auto left_mon_it = left_polymer_nums->second.find(left_monomer_idx);
            if (left_mon_it == left_polymer_nums->second.end())
                throw Error("Polymer '%s' does not contains monomer with number %d.", left_polymer.c_str(), left_monomer_idx);
            auto right_mon_it = right_polymer_nums->second.find(right_monomer_idx);
            if (right_mon_it == right_polymer_nums->second.end())
                throw Error("Polymer '%s' does not contains monomer with number %d.", right_polymer.c_str(), right_monomer_idx);
            KetConnectionEndPoint ep1{};
            ep1.setStringProp("monomerId", document.monomers().at(std::to_string(left_mon_it->second))->ref());
            ep1.setStringProp("attachmentPointId", left_ap);
            KetConnectionEndPoint ep2{};
            ep2.setStringProp("monomerId", document.monomers().at(std::to_string(right_mon_it->second))->ref());
            ep2.setStringProp("attachmentPointId", right_ap);
            document.addConnection(ep1, ep2);
            if (_scanner.isEOF())
                throw Error(unexpected_eod);
            ch = _scanner.readChar();
            if (ch == '"')
            {
                std::string annotation;
                _scanner.readWord(annotation, "\"");
                if (_scanner.isEOF())
                    throw Error(unexpected_eod);
                if (_scanner.lookNext() != '"')
                    throw Error("Unexpected char. Expected '\"' but found '%c'.", _scanner.lookNext());
                _scanner.skip(1); // skip '"'
                if (_scanner.isEOF())
                    throw Error(unexpected_eod);
                ch = _scanner.readChar();
            }
            if (ch != '|' && ch != '$')
                throw Error("Unexpected symbol. Expected '|' or '$' but found '%c'.", _scanner.lookNext());
        }
        else if (helm_part == helm_parts::ListOfPolymerGroups)
        {
            std::string groups;
            _scanner.readWord(groups, "$");
            // skip groups for now
            helm_part = helm_parts::ExtendedAnnotation;
        }
        else // helm_parts::ExtendedAnnotation
        {
            // read rest of data
            std::string rest_of_helm;
            _scanner.readAll(rest_of_helm);
            auto it = rest_of_helm.find_last_of('$');
            if (it == rest_of_helm.npos)
                throw Error("Incorrect format. Last '$' not found.");
            std::string signature = rest_of_helm.substr(it + 1);
            // split by last '$' and check if right part eq “V2.0”
            // if (signature != "v2.0")
            //     throw Error("Expected HELM V2.0 but got '%s'.", signature.c_str());
            // check that left part is valid json - TODO
            helm_part = helm_parts::End;
        }
    }
    if (helm_part != helm_parts::End)
        throw Error(unexpected_eod);
}

void SequenceLoader::loadSequence(KetDocument& document, const std::string& seq_type_str)
{
    if (seq_type_str == kMonomerClassDNA)
        loadSequence(document, SeqType::DNASeq);
    else if (seq_type_str == kMonomerClassRNA)
        loadSequence(document, SeqType::RNASeq);
    else if (seq_type_str == kMonomerClassPEPTIDE)
        loadSequence(document, SeqType::PEPTIDESeq);
    else
        throw Error("Bad sequence type: %s", seq_type_str.c_str());
}

void SequenceLoader::loadSequence(KetDocument& document, SeqType seq_type)
{
    _seq_id = 0;
    _last_monomer_idx = -1;
    _row = 0;
    _col = 0;
    const int row_size = seq_type == SeqType::PEPTIDESeq ? 1 : 2;
    std::string invalid_symbols;

    bool isGenBankPept = false;
    bool start_char = true;

    while (!_scanner.isEOF())
    {
        auto ch = _scanner.readChar();
        if (ch == '\n' || ch == '\r')
            continue;

        if (start_char)
        {
            if (isdigit(ch))
                isGenBankPept = true;
            start_char = false;
        }

        if (isGenBankPept)
        {
            if (ch == ' ' || isdigit(ch))
                continue;
            if (islower(ch))
                ch -= CHAR_SHIFT_CONVERT;
        }

        if (!isGenBankPept && ch == ' ')
        {
            _seq_id = 0;
            _col = 0;
            _row += row_size;
            continue;
        }
        std::string monomer(1, ch);
        if ((seq_type == SeqType::PEPTIDESeq && STANDARD_PEPTIDES.count(monomer) > 0) ||
            (seq_type != SeqType::PEPTIDESeq && STANDARD_NUCLEOTIDES.count(monomer) > 0))
        {
            addMonomer(document, monomer, seq_type);
        }
        else if ((seq_type == SeqType::PEPTIDESeq && STANDARD_MIXED_PEPTIDES.count(monomer) > 0) ||
                 (seq_type != SeqType::PEPTIDESeq && STANDARD_MIXED_BASES.count(monomer) > 0))
        {
            addMonomer(document, monomer, seq_type, true);
        }
        else
        {
            if (invalid_symbols.size())
                invalid_symbols += ',';
            invalid_symbols += ch;
        }
    }

    if (invalid_symbols.size())
        throw Error("Invalid symbols in the sequence: %s", invalid_symbols.c_str());
}

void SequenceLoader::loadFasta(KetDocument& document, const std::string& seq_type_str)
{
    if (seq_type_str == kMonomerClassDNA)
        loadFasta(document, SeqType::DNASeq);
    else if (seq_type_str == kMonomerClassRNA)
        loadFasta(document, SeqType::RNASeq);
    else if (seq_type_str == kMonomerClassPEPTIDE)
        loadFasta(document, SeqType::PEPTIDESeq);
    else
        throw Error("Bad sequence type: %s", seq_type_str.c_str());
}

void SequenceLoader::loadFasta(KetDocument& document, SeqType seq_type)
{
    _seq_id = 0;
    _last_monomer_idx = -1;
    _row = 0;
    _col = 0;
    const int row_size = seq_type == SeqType::PEPTIDESeq ? 1 : 2;
    std::string invalid_symbols;
    Array<int> mapping;
    std::vector<std::string> properties;

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
                    _row += row_size;
                }
                _last_monomer_idx = -1;
                properties.emplace_back(fasta_str);
                continue;
                break;
            default:
                break;
            }

            for (auto ch : fasta_str)
            {
                auto monomer = std::string(1, ch);
                if (ch == '-')
                    continue;
                else if (ch == '*' && seq_type == SeqType::PEPTIDESeq)
                {
                    _seq_id = 0;
                    _col = 0;
                    _row += row_size;
                    continue;
                }
                if ((seq_type == SeqType::PEPTIDESeq && STANDARD_PEPTIDES.count(monomer) > 0) ||
                    (seq_type != SeqType::PEPTIDESeq && STANDARD_NUCLEOTIDES.count(monomer) > 0))
                {
                    addMonomer(document, monomer, seq_type);
                }
                else if ((seq_type == SeqType::PEPTIDESeq && STANDARD_MIXED_PEPTIDES.count(monomer) > 0) ||
                         (seq_type != SeqType::PEPTIDESeq && STANDARD_MIXED_BASES.count(monomer) > 0))
                {
                    addMonomer(document, monomer, seq_type, true);
                }
                else
                {
                    if (invalid_symbols.size())
                        invalid_symbols += ',';
                    invalid_symbols += ch;
                }
            }

            if (invalid_symbols.size())
                throw Error("Invalid symbols in the sequence: %s", invalid_symbols.c_str());
        }
    }

    if (!properties.size())
        throw Error("Invalid FASTA: no '>' headers");
    else
        document.setFastaProps(properties);
}
