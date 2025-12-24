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
#include "molecule/ket_document.h"
#include "molecule/meta_commons.h"
#include "molecule/molecule.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/sequence_loader.h"
#include "molecule/smiles_loader.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;

void SequenceLoader::addMonomer(KetDocument& document, const std::string& monomer, SeqType seq_type, bool mixed)
{
    MonomerClass monomer_class = seq_type == SeqType::PEPTIDESeq ? MonomerClass::AminoAcid : MonomerClass::Base;
    if (!mixed)
        _alias_to_id.emplace(make_pair(monomer_class, monomer), checkAddTemplate(document, monomer_class, monomer));
    else if (!document.hasAmbiguousMonomerTemplate(monomer))
    {
        std::optional<std::reference_wrapper<const std::vector<std::string>>> alternatives;
        if (seq_type == SeqType::PEPTIDESeq)
        {
            const auto& it = STANDARD_MIXED_PEPTIDES.find(monomer);
            if (it == STANDARD_MIXED_PEPTIDES.end())
                throw Error("Unknown mixed peptide '%s'", monomer.c_str());
            alternatives.emplace(std::cref(it->second));
        }
        else
        {
            const auto& it = STANDARD_MIXED_BASES.find(monomer);
            if (it == STANDARD_MIXED_BASES.end())
                throw Error("Unknown mixed base '%s'", monomer.c_str());
            alternatives.emplace(std::cref(it->second));
        }

        std::vector<KetAmbiguousMonomerOption> options;
        for (auto template_alias : alternatives.value().get())
        {
            if (seq_type == SeqType::RNASeq && template_alias == "T")
                template_alias = "U";
            auto& template_id = _library.getMonomerTemplateIdByAlias(monomer_class, template_alias);
            if (template_id.size() == 0)
                throw Error("Monomer base template '%s' not found", template_alias.c_str());
            std::ignore = options.emplace_back(template_id);
            auto& monomer_template = _library.getMonomerTemplateById(template_id);
            checkAddTemplate(document, monomer_template);
            _alias_to_id.emplace(make_pair(monomer_class, template_alias), template_id);
        }
        auto& templ = document.addAmbiguousMonomerTemplate("alternatives", monomer, monomer, IdtAlias(), options);
        static const std::map<std::string, KetAttachmentPoint> aa_aps{{"R1", -1}, {"R2", -1}};
        static const std::map<std::string, KetAttachmentPoint> base_aps{{"R1", -1}};
        if (seq_type == SeqType::PEPTIDESeq)
            templ.setAttachmentPoints(aa_aps);
        else
            templ.setAttachmentPoints(base_aps);
        _var_alias_to_id.emplace(monomer, monomer);
    }

    std::string sugar_alias = seq_type == SeqType::RNASeq ? "R" : "dR";
    std::string phosphate_alias = "P";
    if (seq_type != SeqType::PEPTIDESeq)
    {
        // add sugar template
        if (_seq_id == 0)
            _alias_to_id.emplace(make_pair(MonomerClass::Sugar, sugar_alias), checkAddTemplate(document, MonomerClass::Sugar, sugar_alias));

        // add phosphate template
        if (_seq_id == 1)
            _alias_to_id.emplace(make_pair(MonomerClass::Phosphate, phosphate_alias), checkAddTemplate(document, MonomerClass::Phosphate, phosphate_alias));
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

void SequenceLoader::addAminoAcid(KetDocument& document, const std::string& monomer, bool ambiguous)
{
    Vec3f pos(_col * LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH, -LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH * _row, 0);
    auto amino_idx = document.monomers().size();
    auto& aa_id = ambiguous ? _var_alias_to_id.at(monomer) : _alias_to_id.at(make_pair(MonomerClass::AminoAcid, monomer));
    auto& amino_acid = ambiguous ? document.addAmbiguousMonomer(monomer, aa_id) : document.addMonomer(monomer, aa_id);
    auto& att_points = ambiguous ? document.ambiguousTemplates().at(aa_id).attachmentPoints() : document.templates().at(aa_id).attachmentPoints();
    amino_acid->setAttachmentPoints(att_points);
    setKetIntProp(*amino_acid, seqid, _seq_id);
    amino_acid->setPosition(pos);

    if (_seq_id > 1)
        addMonomerConnection(document, amino_idx - 1, amino_idx);
}

void SequenceLoader::addNucleotide(KetDocument& document, const std::string& base_alias, const std::string& sugar_alias, const std::string& phosphate_alias,
                                   bool phosphate_at_left, bool ambiguous)
{
    Vec3f pos = getBackboneMonomerPosition();

    // add sugar
    auto sugar_idx = document.monomers().size();
    auto& sugar_id = _alias_to_id.at(make_pair(MonomerClass::Sugar, sugar_alias));
    auto& sugar = document.addMonomer(sugar_alias, sugar_id);
    sugar->setAttachmentPoints(document.templates().at(sugar_id).attachmentPoints());
    setKetIntProp(*sugar, seqid, _seq_id);
    sugar->setPosition(pos);

    // add base
    if (base_alias.size() > 0)
    {
        auto nuc_base_idx = document.monomers().size();
        auto& base_id = ambiguous ? _var_alias_to_id.at(base_alias) : _alias_to_id.at(make_pair(MonomerClass::Base, base_alias));
        auto& base = ambiguous ? document.addAmbiguousMonomer(base_alias, base_id) : document.addMonomer(base_alias, base_id);
        auto& base_att_points = ambiguous ? document.ambiguousTemplates().at(base_id).attachmentPoints() : document.templates().at(base_id).attachmentPoints();
        base->setAttachmentPoints(base_att_points);
        setKetIntProp(*base, seqid, _seq_id);
        Vec3f base_coord(pos.x, pos.y - LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH, 0);
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
                auto& phosphate_id = _alias_to_id.at(make_pair(MonomerClass::Phosphate, phosphate_alias));
                auto& phosphate = document.addMonomer(phosphate_alias, phosphate_id);
                phosphate->setAttachmentPoints(document.templates().at(phosphate_id).attachmentPoints());
                setKetIntProp(*phosphate, seqid, _seq_id - 1);
                Vec3f phosphate_coord(pos.x - LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH, pos.y, 0);
                phosphate->setPosition(phosphate_coord);

                addMonomerConnection(document, _last_monomer_idx, phosphate_idx); // connect phosphate to the previous monomer
                addMonomerConnection(document, phosphate_idx, sugar_idx);         // connect current sugar to the phosphate
            }
        }
        else // Phosphate at right
        {
            // add phosphate
            auto phosphate_idx = document.monomers().size();
            auto& phosphate_id = _alias_to_id.at(make_pair(MonomerClass::Phosphate, phosphate_alias));
            auto& phosphate = document.addMonomer(phosphate_alias, phosphate_id);
            phosphate->setAttachmentPoints(document.templates().at(phosphate_id).attachmentPoints());
            setKetIntProp(*phosphate, seqid, _seq_id);
            Vec3f phosphate_coord(pos.x + LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH, pos.y, 0);
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
        _last_monomer_idx = static_cast<int>(sugar_idx);
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
        tg.tgroup_id = tg_idx + 1;
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
    if (_added_templates.count(std::make_pair(monomer_template.monomerClass(), getKetStrProp(monomer_template, alias))) == 0)
    {
        int tg_idx = mol.tgroups.addTGroup();
        auto& tg = mol.tgroups.getTGroup(tg_idx);
        tg.copy(*monomer_template.getTGroup());
        tg.tgroup_id = tg_idx + 1;
        tg.idt_alias.readString(monomer_template.idtAlias().getBase().c_str(), true);
        _added_templates.emplace(monomer_template.monomerClass(), getKetStrProp(monomer_template, alias));
    }
}

void SequenceLoader::checkAddTemplate(KetDocument& document, const MonomerTemplate& monomer_template)
{
    if (_added_templates.count(std::make_pair(monomer_template.monomerClass(), getKetStrProp(monomer_template, alias))) == 0)
    {
        _added_templates.emplace(monomer_template.monomerClass(), getKetStrProp(monomer_template, alias));
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
            throw Error("Monomer template with class '%s' and alias '%s' not found in monomer library",
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
                cur_token += ch;
                ch = 0;
                while (ch != '/' && !_scanner.isEOL())
                {
                    ch = _scanner.readChar();
                    cur_token += ch;
                }
                if (ch != '/')
                    throw Error("Unexpected end of data");
                if (cur_token == "//")
                    throw Error("Invalid modification: empty string.");
                if (cur_token.size() < 5)
                    throw Error("Invalid modification: %s.", cur_token.c_str());
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
                    throw Error("Invalid ambiguous monomer: empty string.");
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
            bool ambiguous_monomer = false;

            if (token.first.back() == '/')
            {
                idt_alias = token.first;
                if ((idt_alias == "/5Phos/" || idt_alias == "/3Phos/") && (token.second || prev_token.second))
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
                ambiguous_monomer = true;

            if (idt_alias.size() == 1 || ambiguous_monomer)
            {
                if (IDT_STANDARD_BASES.count(idt_alias) == 0 && !ambiguous_monomer)
                {
                    if (invalid_symbols.size())
                        invalid_symbols += ',';
                    invalid_symbols += idt_alias[0];
                    continue;
                }

                if (ambiguous_monomer)
                {
                    auto mixed_base = idt_alias;
                    std::optional<std::array<float, 4>> ratios;
                    if (mixed_base.back() == ')')
                    {
                        mixed_base = idt_alias.substr(1, idt_alias.size() - 2);
                        auto check_mixed_base = [](const std::string& base) {
                            if (base.size() < 2)
                                return;
                            auto count = base.substr(1, base.size() - 1);
                            for (auto ch : count)
                            {
                                if (!std::isdigit(ch))
                                    throw Error("Invalid mixed base - only numerical index allowed.");
                            }
                        };
                        if (auto pos = mixed_base.find(':'); pos != std::string::npos)
                        {
                            auto ratios_str = mixed_base.substr(pos + 1, mixed_base.size() - pos - 1);
                            mixed_base = mixed_base.substr(0, pos);
                            check_mixed_base(mixed_base);
                            if (ratios_str.size() != 8)
                                throw Exception("Invalid IDT ambiguous monomer %s", idt_alias.c_str());
                            auto stof = [](const std::string& arg) -> float {
                                try
                                {
                                    return std::stof(arg);
                                }
                                catch (...)
                                {
                                    throw Error("Invalid number '%s'", arg.c_str());
                                }
                            };
                            ratios.emplace(std::array<float, 4>{stof(ratios_str.substr(0, 2)), stof(ratios_str.substr(2, 2)), stof(ratios_str.substr(4, 2)),
                                                                stof(ratios_str.substr(6, 2))});
                            idt_alias = '(' + mixed_base + ')';
                            mixed_base = mixed_base[0];
                        }
                        else
                        {
                            check_mixed_base(mixed_base);
                        }
                    }
                    if (sugar == "R" && RNA_DNA_MIXED_BASES.count(mixed_base) == 0)
                        idt_alias = 'r' + idt_alias;
                    if (!document.hasAmbiguousMonomerTemplate(idt_alias))
                    {
                        auto it = STANDARD_MIXED_BASES.find(mixed_base);
                        if (it == STANDARD_MIXED_BASES.end())
                            throw Error("Unknown mixed base '%s'", mixed_base.c_str());

                        std::vector<KetAmbiguousMonomerOption> options;
                        for (auto template_alias : it->second)
                        {
                            if (sugar == "R" && template_alias == "T") // U instead of T for RNA
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
                            _alias_to_id.emplace(make_pair(MonomerClass::Base, template_alias), template_id);
                        }
                        auto& templ = document.addAmbiguousMonomerTemplate("mixture", idt_alias, idt_alias, IdtAlias(), options);
                        static const std::map<std::string, KetAttachmentPoint> aps{{"R1", -1}};
                        templ.setAttachmentPoints(aps);
                        _var_alias_to_id.emplace(idt_alias, idt_alias);
                    }
                    else
                    {
                        if (ratios.has_value())
                            throw Error("Ambiguous monomer %s redefinion", idt_alias.c_str());
                    }
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

                _alias_to_id.emplace(make_pair(MonomerClass::Sugar, sugar), checkAddTemplate(document, MonomerClass::Sugar, sugar));
                if (base.size() > 0 && !ambiguous_monomer)
                    _alias_to_id.emplace(make_pair(MonomerClass::Base, base), checkAddTemplate(document, MonomerClass::Base, base));
                if (phosphate.size() > 0)
                    _alias_to_id.emplace(make_pair(MonomerClass::Phosphate, phosphate), checkAddTemplate(document, MonomerClass::Phosphate, phosphate));
            }
            else
            {
                if (tokens.size() == 0)
                {
                    modification = IdtModification::THREE_PRIME_END;
                    // Corner case: /3Phos/ after standard monomer - no additional P should be added
                    if (prev_token.first.size() > 0 && prev_token.first.size() <= MAX_STD_TOKEN_SIZE && idt_alias == "/3Phos/")
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
                    sugar = getKetStrProp(sugar_template, alias);
                    _alias_to_id.emplace(make_pair(MonomerClass::Sugar, sugar), sugar_template.id());
                    checkAddTemplate(document, sugar_template);
                    if (alias_mod == IdtModification::THREE_PRIME_END)
                    {
                        if (token.second)
                            throw Error("Monomer %s doesn't have phosphate, so '*' couldn't be applied.", idt_alias.c_str());
                        phosphate = "";
                    }
                    else
                    {
                        if (mgt.hasTemplateClass(MonomerClass::Phosphate))
                        {
                            if (token.second) // * means that 'sP' should be used
                            {
                                phosphate = IDT_MODIFIED_PHOSPHATE;
                                _alias_to_id.emplace(make_pair(MonomerClass::Phosphate, phosphate),
                                                     checkAddTemplate(document, MonomerClass::Phosphate, phosphate));
                            }
                            else // use phosphate from template
                            {
                                const MonomerTemplate& phosphate_template = mgt.getTemplateByClass(MonomerClass::Phosphate);
                                phosphate = getKetStrProp(phosphate_template, alias);
                                _alias_to_id.emplace(make_pair(MonomerClass::Phosphate, phosphate), phosphate_template.id());
                                checkAddTemplate(document, phosphate_template);
                            }
                        }
                        else
                        {
                            if (token.second)
                                throw Error("Monomer %s doesn't have phosphate, so '*' couldn't be applied.", idt_alias.c_str());
                            phosphate = "";
                        }
                    }
                    if (mgt.hasTemplateClass(MonomerClass::Base))
                    {
                        const MonomerTemplate& base_template = mgt.getTemplateByClass(MonomerClass::Base);
                        base = getKetStrProp(base_template, alias);
                        _alias_to_id.emplace(make_pair(MonomerClass::Base, base), base_template.id());
                        checkAddTemplate(document, base_template);
                    }
                }
                else
                {
                    auto& monomer_template_id = _library.getMonomerTemplateIdByIdtAlias(idt_alias, alias_mod);
                    if (monomer_template_id.size())
                    {
                        if (token.second)
                            throw Error("'*' couldn't be applied to monomer %s.", idt_alias.c_str());
                        check_monomer_place(idt_alias, modification, alias_mod, prev_token.first.size() > 0);
                        const MonomerTemplate& monomer_template = _library.getMonomerTemplateById(monomer_template_id);
                        checkAddTemplate(document, monomer_template);
                        single_monomer = monomer_template_id;
                        single_monomer_alias = getKetStrProp(monomer_template, alias);
                    }
                    else // IDT alias not found
                    {
                        single_monomer_alias = idt_alias.substr(1, idt_alias.size() - 2); // remove slashes
                        single_monomer = "unknown_monomer_with_idt_alias_" + single_monomer_alias;
                        auto monomer_class = MonomerClass::CHEM;
                        // Unresoved monomer could be in any position
                        MonomerTemplate monomer_template(single_monomer, monomer_class, IdtAlias(single_monomer_alias, idt_alias, idt_alias, idt_alias), true);
                        setKetStrProp(monomer_template, alias, single_monomer_alias);
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
                setKetIntProp(*monomer, seqid, _seq_id);
                monomer->setPosition(getBackboneMonomerPosition());
                if (_last_monomer_idx >= 0)
                    addMonomerConnection(document, _last_monomer_idx, monomer_idx);
                _last_monomer_idx = static_cast<int>(monomer_idx);
            }
            else
                addNucleotide(document, base, sugar, phosphate, false, ambiguous_monomer);

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

void SequenceLoader::loadAxoLabs(KetDocument& document)
{
    _row = 0;
    std::string data;
    _scanner.readAll(data);
    auto data_size = data.size();

    constexpr char* CRLF = "\r\n";
    constexpr char* LF = "\n";
    const auto MIN_AXO_SIZE = sizeof(AXOLABS_PREFIX) + sizeof(AXOLABS_SUFFIX) - 1;

    size_t start = 0, end = 0;
    auto search_line_end = [&data, &CRLF, &LF, data_size](size_t start, size_t& pos) -> size_t {
        auto crlf = data.find(CRLF, start);
        auto lf = data.find(LF, start);
        if (lf == std::string::npos && crlf == std::string::npos)
        {
            pos = data_size;
            return std::string::npos;
        }

        if (lf < crlf)
        {
            pos = lf;
            return 1;
        }
        else
        {
            pos = crlf;
            return 2;
        };
    };
    auto search_data = [&data_size, &search_line_end](size_t& start, size_t& end) {
        auto count = search_line_end(start, end);
        while (end - start == 0 && start != data_size) // skip empty strings
        {
            start += count;
            count = search_line_end(start, end);
        }
    };

    search_data(start, end);
    if (start == end)
        throw Error("Empty string");

    class link
    {
    public:
        link(const std::string& monomer) : mon_id(monomer), base_id(){};
        link(const std::string& monomer, const std::string& base) : mon_id(monomer), base_id(base){};
        std::string mon_id;
        std::string base_id;
    };

    std::string sense;
    std::vector<link> sense_chain;
    std::string antisense;
    std::vector<link> antisense_chain;
    auto constexpr non_pairing = 'X';
    bool is_sense = true;

    auto get_analog_short = [non_pairing](const MonomerTemplate& templ) {
        if (hasKetStrProp(templ, naturalAnalogShort))
            return getKetStrProp(templ, naturalAnalogShort)[0];
        if (hasKetStrProp(templ, alias))
        {
            auto alias = getKetStrProp(templ, alias);
            if (alias.size() == 1)
                return alias[0];
        }
        return non_pairing;
    };

    auto add_link = [&](const MonomerTemplate& monomer_template) {
        if (is_sense)
        {
            sense.append(1, get_analog_short(monomer_template));
            sense_chain.emplace_back(monomer_template.id());
        }
        else
        {
            antisense.append(1, get_analog_short(monomer_template));
            antisense_chain.emplace_back(monomer_template.id());
        }
    };

    while (start != end)
    {
        std::string sequence = data.substr(start, end - start);
        if (sequence.size() < MIN_AXO_SIZE)
            throw Error("Sequence too short: '%s'", data.substr(start, end - start).c_str());
        std::string affix = sequence.substr(0, sizeof(AXOLABS_PREFIX) - 1);
        if (affix != AXOLABS_PREFIX)
            throw Error("Invalid AxoLabs sequence: expected %s got %s", AXOLABS_PREFIX, affix.c_str());
        affix = sequence.substr(sequence.size() - sizeof(AXOLABS_SUFFIX) + 1, sizeof(AXOLABS_SUFFIX) - 1);
        if (affix != AXOLABS_SUFFIX)
            throw Error("Invalid AxoLabs sequence: expected %s got %s", AXOLABS_SUFFIX, affix.c_str());
        // remove prefix and suffix
        sequence.erase(sequence.size() - sizeof(AXOLABS_SUFFIX) + 1, sizeof(AXOLABS_SUFFIX) - 1);
        sequence.erase(0, sizeof(AXOLABS_PREFIX) - 1);

        std::size_t pos = 0;
        std::size_t length = sequence.size();
        while (pos < length)
        {
            std::string group{sequence[pos++]};
            if (pos == 1 && group == "s")
                throw Error("Invalid AxoLabs sequence: phosphate 's' can only be internal.");
            if (group == "p")
            {
                if (pos != 1 && pos != length)
                    throw Error("Invalid AxoLabs sequence: phosphate 'p' can only be terminal.");
                auto& monomer_template_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, "P");
                const MonomerTemplate& monomer_template = _library.getMonomerTemplateById(monomer_template_id);
                checkAddTemplate(document, monomer_template);
                add_link(monomer_template);
                continue;
            }
            std::string mgt_id;
            if (group.front() == '(')
            { // read till ')'
                while (pos < length)
                {
                    group += sequence[pos++];
                    if (group.back() == ')')
                        break;
                    if (pos >= length)
                        throw Error("Unexpected end of data");
                }
                if (group.back() != ')')
                    throw Error("Unexpected end of data");
            }
            else if (pos < length && STANDARD_NUCLEOTIDES.count(group) > 0 && std::islower(sequence[pos]) && sequence[pos] != 's' && sequence[pos] != 'p')
            {
                group += sequence[pos++];
            }
            else
            {
                mgt_id = _library.getMGTidByAliasAxoLabs(group);
                if (mgt_id.size() == 0) // if not a/c/t/g - read second char
                {
                    if (pos >= length)
                        throw Error("Unexpected end of data");
                    group += sequence[pos++];
                }
            }
            std::string phosphate = "P";
            std::string sugar, base;
            if (pos < length && sequence[pos] == 's')
            {
                pos++;
                if (pos >= length || (pos == length - 1 && sequence[pos] == 'p'))
                    throw Error("Invalid AxoLabs sequence: phosphate 's' can only be internal.");
                phosphate = "sP";
            }
            if (mgt_id.size() == 0)
            {
                mgt_id = _library.getMGTidByAliasAxoLabs(group);
            }
            if (mgt_id.size() > 0)
            {
                auto& mgt = _library.getMonomerGroupTemplateById(mgt_id);
                std::string base_id;
                auto base_analog = non_pairing;
                if (mgt.hasTemplate(MonomerClass::Base))
                {
                    auto& base_template = mgt.getTemplateByClass(MonomerClass::Base);
                    checkAddTemplate(document, base_template);
                    base_id = base_template.id();
                    base_analog = get_analog_short(base_template);
                }
                auto& sugar_template = mgt.getTemplateByClass(MonomerClass::Sugar);
                checkAddTemplate(document, sugar_template);
                if (is_sense)
                {
                    sense.append(1, base_analog);
                    sense_chain.emplace_back(sugar_template.id(), base_id);
                }
                else
                {
                    antisense.append(1, base_analog);
                    antisense_chain.emplace_back(sugar_template.id(), base_id);
                }
                if (!(pos >= length || (pos == length - 1 && sequence[pos] == 'p'))) // last nuleotide without phosphate, this is not last nucleotide
                {
                    auto& phosphate_template = group.front() == '('
                                                   ? mgt.getTemplateByClass(MonomerClass::Phosphate)
                                                   : _library.getMonomerTemplateById(_library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, phosphate));
                    add_link(phosphate_template);
                    checkAddTemplate(document, phosphate_template);
                }
            }
            else
            {
                std::string monomer_template_id = _library.getMonomerTemplateIdByAliasAxoLabs(group);
                if (monomer_template_id.size() > 0)
                {
                    const MonomerTemplate& monomer_template = _library.getMonomerTemplateById(monomer_template_id);
                    checkAddTemplate(document, monomer_template);
                    add_link(monomer_template);
                }
                else
                {
                    if (group[0] != '(')
                        throw Error("The following string cannot be interpreted as an AxoLabs string: %s", group.c_str()); // unresolved should be in ()
                    monomer_template_id = group;
                    MonomerTemplate monomer_template(monomer_template_id, MonomerClass::CHEM, IdtAlias(), true);
                    setKetStrProp(monomer_template, alias, group);
                    setKetStrProp(monomer_template, aliasAxoLabs, group);
                    for (auto ap : {"R1", "R2"})
                        monomer_template.AddAttachmentPoint(ap, -1);
                    checkAddTemplate(document, monomer_template);
                    add_link(monomer_template);
                }
            }
        }
        start = end;
        search_data(start, end);
        if (start == end || !is_sense) // last sequence or end of antisense
        {
            auto add_link_monomers = [&](const link& cur_link, int& last_monomer_idx, bool reverse_base = false, std::string* base_id = nullptr) {
                Vec3f pos = getBackboneMonomerPosition();
                auto monomer_idx = document.monomers().size();
                auto& monomer_template = document.templates().at(cur_link.mon_id);
                if (monomer_template.monomerClass() != MonomerClass::Phosphate)
                    _seq_id++;
                auto& monomer = document.addMonomer(getKetStrProp(monomer_template, alias), monomer_template.id());
                setKetIntProp(*monomer, seqid, _seq_id);
                monomer->setPosition(pos);
                if (last_monomer_idx >= 0)
                    addMonomerConnection(document, last_monomer_idx, monomer_idx);
                if (cur_link.base_id.size() > 0)
                {
                    auto base_idx = document.monomers().size();
                    auto& base_template = document.templates().at(cur_link.base_id);
                    auto& base = document.addMonomer(getKetStrProp(base_template, alias), base_template.id());
                    if (base_id != nullptr)
                        base_id->assign(base->id());
                    setKetIntProp(*base, seqid, _seq_id);
                    auto base_shift = LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH;
                    if (reverse_base)
                        base_shift = -base_shift;
                    Vec3f base_pos(pos.x, pos.y - base_shift, 0);
                    base->setPosition(base_pos);
                    addMonomerConnection(document, monomer_idx, base_idx, true);
                }
                last_monomer_idx = static_cast<int>(monomer_idx);
            };
            if (is_sense) // last sequence is sense - just add it
            {
                _col = 0;
                int last_monomer_idx = -1;
                for (auto it = sense_chain.begin(); it != sense_chain.end(); it++)
                {
                    add_link_monomers(*it, last_monomer_idx);
                    _col++;
                }
            }
            else // end of antisense sequence - add doube-chain
            {
                std::vector<std::pair<size_t, size_t>> pairs;
                bool shift_sense;
                std::reverse(antisense.begin(), antisense.end());
                auto offset = best_allign(sense, antisense, pairs, shift_sense);
                std::map<size_t, std::string> sense_ids;
                std::map<size_t, std::string> antisense_ids;
                for (auto it = pairs.begin(); it != pairs.end(); it++)
                {
                    sense_ids.emplace(it->first, "");
                    // fix antisense index - it is reversed
                    auto antisense_idx = antisense_chain.size() - 1 - it->second;
                    it->second = antisense_idx;
                    antisense_ids.emplace(antisense_idx, "");
                }
                // save sense
                _col = static_cast<int>(shift_sense ? offset : 0);
                int last_monomer_idx = -1;
                for (size_t idx = 0; idx < sense_chain.size(); idx++)
                {
                    if (sense_ids.count(idx) > 0) // if id in pairs - save base id
                        add_link_monomers(sense_chain.at(idx), last_monomer_idx, false, &sense_ids.at(idx));
                    else
                        add_link_monomers(sense_chain.at(idx), last_monomer_idx);
                    _col++;
                }
                // save antisense (from right to left)
                _row += 3;
                _col = static_cast<int>(shift_sense ? antisense_chain.size() - 1 : offset + antisense_chain.size() - 1);
                last_monomer_idx = -1;
                for (size_t idx = 0; idx < antisense_chain.size(); idx++)
                {

                    if (antisense_ids.count(idx) > 0) // if id in pairs - save base id
                        add_link_monomers(antisense_chain.at(idx), last_monomer_idx, true, &antisense_ids.at(idx));
                    else
                        add_link_monomers(antisense_chain.at(idx), last_monomer_idx, true);
                    _col--;
                }
                // add hydrogen bonds
                for (auto it = pairs.begin(); it != pairs.end(); it++)
                {
                    document.addConnection(document.monomers().at(sense_ids.at(it->first))->ref(), HelmHydrogenPair,
                                           document.monomers().at(antisense_ids.at(it->second))->ref(), HelmHydrogenPair);
                }
                sense.clear();
                antisense.clear();
                sense_ids.clear();
                antisense_ids.clear();
                sense_chain.clear();
                antisense_chain.clear();
                is_sense = !is_sense;
                _row += 1;
            }
        }
        else // not last sequence and not antisense - next will be antisense
        {
            is_sense = !is_sense;
        }
    }
}

void SequenceLoader::loadSequence(KetDocument& document, const std::string& seq_type_str)
{
    if (seq_type_str == kMonomerClassDNA)
        loadSequence(document, SeqType::DNASeq);
    else if (seq_type_str == kMonomerClassRNA)
        loadSequence(document, SeqType::RNASeq);
    else if (seq_type_str == kMonomerClassPEPTIDE)
        loadSequence(document, SeqType::PEPTIDESeq);
    else if (seq_type_str == kMonomerClassPEPTIDE_3_LETTER)
        load3LetterSequence(document);
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
    bool before_text = true;

    while (!_scanner.isEOF())
    {
        auto ch = _scanner.readChar();
        if (ch == '\n' || ch == '\r')
        {
            before_text = true;
            continue;
        }

        if (start_char)
        {
            if (ch == ' ' || ch == '\t')
                continue; // skip leading whitespaces
            if (ch >= NUM_BEGIN && ch < NUM_END)
                isGenBankPept = true;
            start_char = false;
        }

        if (isGenBankPept)
        {
            if (ch == ' ' || (before_text && ch >= NUM_BEGIN && ch < NUM_END))
                continue;
            before_text = false;
        }
        if (islower(ch))
            ch -= CHAR_SHIFT_CONVERT;

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

// Load 3 letter amino acid sequence like AlaCys
void SequenceLoader::load3LetterSequence(KetDocument& document)
{
    _seq_id = 0;
    _last_monomer_idx = -1;
    _row = 0;
    _col = 0;
    static const char* wrong_format = "Given string cannot be interpreted as a valid three letter sequence because of incorrect formatting.";
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

        // monomer name is uppercase then two lowercase letter
        if (!std::isalpha(ch) || !std::isupper(ch))
            throw Error(wrong_format);
        std::string monomer(1, ch);
        for (auto i = 0; i < 2;) // read two chars
        {
            if (_scanner.isEOF())
                throw Error(wrong_format);
            ch = _scanner.readChar();
            if (ch == '\n' || ch == '\r')
                continue;
            i++;
            if (!std::isalpha(ch) || !std::islower(ch))
                throw Error(wrong_format);
            monomer += ch;
        }
        if (STANDARD_MIXED_PEPTIDES_NAME_TO_ALIAS.count(monomer) > 0)
        {
            addMonomer(document, STANDARD_MIXED_PEPTIDES_NAME_TO_ALIAS.at(monomer), SeqType::PEPTIDESeq, true);
        }
        else
        {
            std::string alias = monomerAliasByName(kMonomerClassAminoAcid, monomer);
            if (alias == monomer) // alias not found
                throw Error("Unknown monomer name '%s'.", monomer.c_str());
            addMonomer(document, alias, SeqType::PEPTIDESeq);
        }
    }
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif
