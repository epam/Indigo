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

#include "molecule/sequence_saver.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "layout/sequence_layout.h"
#include "molecule/elements.h"
#include "molecule/json_writer.h"
#include "molecule/ket_document.h"
#include "molecule/ket_objects.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/molecule_standardize_options.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/smiles_saver.h"
#include <algorithm>
#include <cctype>
#include <set>
#include <tuple>

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;

static const char* get_helm_class(MonomerClass monomer_class);

IMPL_ERROR(SequenceSaver, "Sequence saver");

CP_DEF(SequenceSaver);

SequenceSaver::SequenceSaver(Output& output, MonomerTemplateLibrary& library) : _output(output), _mon_lib(MonomerTemplates::_instance()), _library(library)
{
}

SequenceSaver::~SequenceSaver()
{
}

static const std::unordered_set<std::string> IDT_STANDARD_BASES = {"A", "T", "C", "G", "U", "I", "In"};
static const std::map<std::string, std::string> IDT_STANDARD_SUGARS{{"R", "r"}, {"LR", "+"}, {"mR", "m"}, {"dR", ""}};
static const std::map<std::string, std::vector<std::string>> IDT_STANDARD_MIXED_BASES = {
    {"R", {"A", "G"}},      {"Y", {"C", "T"}},      {"M", {"A", "C"}},      {"K", {"G", "T"}},      {"S", {"G", "C"}},          {"W", {"A", "T"}},
    {"H", {"A", "C", "T"}}, {"B", {"G", "C", "T"}}, {"V", {"A", "C", "G"}}, {"D", {"A", "G", "T"}}, {"N", {"A", "C", "G", "T"}}};

static inline void add_monomer_str(std::string& helm_string, const std::string& monomer_alias)
{
    if (monomer_alias.size() == 1)
        helm_string += monomer_alias;
    else
        helm_string += '[' + monomer_alias + ']';
}

std::string SequenceSaver::getMonomerAlias(BaseMolecule& mol, int atom_idx)
{
    std::string monomer_alias = "";
    std::string monomer_class = mol.getTemplateAtomClass(atom_idx);
    std::string monomer = mol.getTemplateAtom(atom_idx);
    const std::string& monomer_id = _library.getMonomerTemplateIdByAlias(MonomerTemplates::getStrToMonomerType().at(monomer_class), monomer);
    if (monomer_id.size())
    {
        auto& monomer_template = _library.getMonomerTemplateById(monomer_id);
        monomer_alias = getKetStrProp(monomer_template, alias);
    }
    return monomer_alias;
}

std::string SequenceSaver::getHelmPolymerClass(BaseMolecule& mol, int atom_idx)
{
    std::string monomer_class = mol.getTemplateAtomClass(atom_idx);
    std::string monomer = mol.getTemplateAtom(atom_idx);
    std::string helm_polymer_class = "";
    const std::string& monomer_id = _library.getMonomerTemplateIdByAlias(MonomerTemplates::getStrToMonomerType().at(monomer_class), monomer);
    if (monomer_id.size())
    {
        auto& monomer_template = _library.getMonomerTemplateById(monomer_id);
        helm_polymer_class = getKetStrProp(monomer_template, classHELM);
    }
    if (helm_polymer_class.size() == 0)
    {
        if (isNucleicClass(monomer_class))
            helm_polymer_class = kHELMPolymerTypeRNA;
        else if (isAminoAcidClass(monomer_class))
            helm_polymer_class = kHELMPolymerTypePEPTIDE;
        else
            helm_polymer_class = kHELMPolymerTypeCHEM;
    }
    return helm_polymer_class;
}

static void check_backbone_connection(BaseMolecule& mol, std::vector<std::map<int, int>> directions_map, int template_idx, int side,
                                      std::map<int, int>& side_backbone_links, std::map<int, int>& other_side_backbone_links)
{
    auto& attachments = directions_map[template_idx];
    auto side_attachments = attachments.find(side);
    if (side_attachments != attachments.end()) // has side attachment
    {
        int side_neighbor_idx = side_attachments->second;
        if (mol.isTemplateAtom(side_neighbor_idx))
        {
            auto& neighbor_attachments = directions_map[side_neighbor_idx];
            auto neighbor_other_size = neighbor_attachments.find(side == kLeftAttachmentPointIdx ? kRightAttachmentPointIdx : kLeftAttachmentPointIdx);
            if (neighbor_other_size != neighbor_attachments.end() && neighbor_other_size->second == template_idx)
            {
                side_backbone_links[template_idx] = side_neighbor_idx;
                other_side_backbone_links[side_neighbor_idx] = template_idx;
            }
        }
    }
}

void SequenceSaver::_validateSequence(BaseMolecule& bmol)
{
    std::string unresolved;
    if (bmol.getUnresolvedTemplatesList(bmol, unresolved))
        throw Error("%s cannot be written in sequence/FASTA format.", unresolved.c_str());
}

void SequenceSaver::saveMolecule(BaseMolecule& mol, SeqFormat sf)
{
    if (sf == SeqFormat::FASTA || sf == SeqFormat::Sequence)
        _validateSequence(mol);

    if (!mol.isQueryMolecule())
        mol.getTemplatesMap(_templates);

    std::string seq_text;
    if (sf == SeqFormat::HELM || sf == SeqFormat::BILN)
    {
        std::vector<std::deque<std::string>> sequences;
        KetDocument doc(mol);
        doc.parseSimplePolymers(sequences, false);
        seq_text = sf == SeqFormat::HELM ? saveHELM(doc, sequences) : saveBILN(doc, sequences);
    }
    else if (sf == SeqFormat::IDT)
    {
        std::vector<std::deque<std::string>> sequences;
        KetDocument doc(mol);
        doc.parseSimplePolymers(sequences, true);
        saveIdt(doc, sequences, seq_text);
    }
    else
    {
        auto& mol_properties = mol.properties();
        std::vector<std::deque<int>> sequences;
        SequenceLayout sl(mol);
        sl.sequenceExtract(sequences);
        auto prop_it = mol_properties.begin();
        int seq_idx = 0;
        for (auto& sequence : sequences)
        {
            std::string seq_string;
            for (auto atom_idx : sequence)
            {
                if (mol.isTemplateAtom(atom_idx))
                {
                    std::string mon_class = mol.getTemplateAtomClass(atom_idx);
                    if (isBackboneClass(mon_class))
                    {
                        std::string label;
                        if (mon_class == kMonomerClassSUGAR)
                        {
                            auto& v = mol.getVertex(atom_idx);
                            for (auto nei_idx = v.neiBegin(); nei_idx < v.neiEnd(); nei_idx = v.neiNext(nei_idx))
                            {
                                int nei_atom_idx = v.neiVertex(nei_idx);
                                if (mol.isTemplateAtom(nei_atom_idx) && std::string(mol.getTemplateAtomClass(nei_atom_idx)) == kMonomerClassBASE)
                                {
                                    mon_class = kMonomerClassBASE;
                                    atom_idx = nei_atom_idx;
                                    label = monomerAliasByName(mon_class, mol.getTemplateAtom(nei_atom_idx));
                                    break;
                                }
                            }
                        }
                        else if (isAminoAcidClass(mon_class))
                        {
                            mon_class = kMonomerClassAA;
                            label = monomerAliasByName(kMonomerClassAA, mol.getTemplateAtom(atom_idx));
                        }
                        else if (isNucleotideClass(mon_class))
                        {
                            mon_class = kMonomerClassBASE; // treat nucleotide symbol as a base
                            label = monomerAliasByName(kMonomerClassBASE, mol.getTemplateAtom(atom_idx));
                        }

                        if (label.size())
                        {
                            TGroup temp;
                            if (!_mon_lib.getMonomerTemplate(mon_class, label, temp))
                            {
                                // if symbol is not standard, check its natural analog
                                const char* natrep = nullptr;
                                int temp_idx = mol.getTemplateAtomTemplateIndex(atom_idx);
                                if (temp_idx > -1)
                                {
                                    auto& tg = mol.tgroups.getTGroup(temp_idx);
                                    natrep = tg.tgroup_natreplace.ptr();
                                }
                                else
                                {
                                    auto tg_ref = findTemplateInMap(label, mon_class, _templates);
                                    if (tg_ref.has_value())
                                    {
                                        auto& tg = tg_ref.value().get();
                                        natrep = tg.tgroup_natreplace.ptr();
                                    }
                                }
                                std::string natural_analog;
                                if (natrep)
                                    natural_analog = monomerAliasByName(mon_class, extractMonomerName(natrep));

                                if (_mon_lib.getMonomerTemplate(mon_class, natural_analog, temp))
                                    label = natural_analog;
                                else if (mon_class == kMonomerClassBASE)
                                    throw Error("'%s' nucleotide has no natural analog and cannot be saved into a sequence.", label.c_str());
                                else if (mon_class == kMonomerClassAA)
                                    label = "X";
                            }

                            if (label.size() > 1)
                                throw Error("Can't save '%s' to sequence format", label.c_str());
                            seq_string += label;
                        }
                    }
                }
            }

            if (seq_string.size())
            {
                // sequences separators are different for FASTA, IDT and Sequence
                if (sf == SeqFormat::FASTA)
                {
                    if (seq_idx)
                        seq_text += "\n";
                    std::string fasta_header = ">Sequence";
                    fasta_header += std::to_string(seq_idx + 1);
                    if (prop_it != mol_properties.end())
                    {
                        auto& props = mol_properties.value(prop_it);
                        prop_it++;
                        if (props.contains(kFASTA_HEADER))
                            fasta_header = props.at(kFASTA_HEADER);
                    }
                    fasta_header += "\n";
                    seq_text += fasta_header;
                }
                else if (seq_text.size())
                    seq_text += sf == SeqFormat::Sequence ? " " : "\n";

                if (sf == SeqFormat::IDT)
                {
                    seq_text += seq_string;
                }
                else
                {
                    seq_text += seq_string.substr(0, SEQ_LINE_LENGTH);
                    for (size_t format_ind = SEQ_LINE_LENGTH; format_ind < seq_string.size(); format_ind += SEQ_LINE_LENGTH)
                    {
                        seq_text += "\n";
                        seq_text += seq_string.substr(format_ind, SEQ_LINE_LENGTH);
                    }
                }
                seq_idx++;
            }
        }
    }
    if (seq_text.size())
        _output.write(seq_text.data(), static_cast<int>(seq_text.size()));
}

void SequenceSaver::saveKetDocument(KetDocument& doc, SeqFormat sf)
{
    std::vector<std::deque<std::string>> sequences;
    int seq_idx = 0;
    std::string seq_text;
    if (sf == SeqFormat::HELM || sf == SeqFormat::BILN)
    {
        doc.parseSimplePolymers(sequences, false);
        seq_text = sf == SeqFormat::HELM ? saveHELM(doc, sequences) : saveBILN(doc, sequences);
    }
    else if (sf == SeqFormat::IDT)
    {
        doc.parseSimplePolymers(sequences, true);
        saveIdt(doc, sequences, seq_text);
    }
    else if (sf == SeqFormat::AxoLabs)
    {
        doc.parseSimplePolymers(sequences, true);
        saveAxoLabs(doc, sequences, seq_text);
    }
    else if (sf == SeqFormat::FASTA || sf == SeqFormat::Sequence || sf == SeqFormat::Sequence3)
    {
        if (doc.moleculesRefs().size() > 0)
            throw Error("Can't save micro-molecules to sequence format");
        auto& monomers = doc.monomers();
        doc.parseSimplePolymers(sequences, false);
        auto prop_it = doc.fastaProps().begin();
        for (auto& sequence : sequences)
        {
            std::string seq_string;
            for (auto monomer_id : sequence)
            {
                MonomerClass monomer_class = doc.getMonomerClass(monomer_id);
                const auto& monomer = monomers.at(monomer_id);
                auto monomer_alias = monomer->alias();
                if (sf == SeqFormat::Sequence3 && monomer_class != MonomerClass::AminoAcid)
                    throw Error("Only amino acids can be saved as three letter amino acid codes.");
                if (monomer_class == MonomerClass::CHEM)
                    throw Error("Can't save chem '%s' to sequence format", monomer_alias.c_str());
                if (monomer_class == MonomerClass::Sugar || monomer_class == MonomerClass::Phosphate ||
                    (monomer_class == MonomerClass::Base && sequence.size() == 1))
                    continue;

                if (monomer_alias.size() > 1 ||
                    (monomer_class == MonomerClass::AminoAcid && STANDARD_NUCLEOTIDES.count(monomer_alias) == 0 &&
                     STANDARD_MIXED_PEPTIDES.count(monomer_alias) == 0) ||
                    (monomer_class == MonomerClass::Base && STANDARD_NUCLEOTIDES.count(monomer_alias) == 0 && STANDARD_MIXED_BASES.count(monomer_alias) == 0))
                {
                    std::string short_analog;
                    auto get_analog = [&short_analog, &monomer_class](const KetBaseMonomerTemplate& monomer_template) {
                        const auto& analog_idx = monomer_template.getStringPropIdx("naturalAnalog");
                        if (analog_idx.first && monomer_template.hasStringProp(analog_idx.second))
                        {
                            std::string analog = monomer_template.getStringProp(analog_idx.second);
                            short_analog = monomerAliasByName(MonomerTemplate::MonomerClassToStr(monomer_class), analog);
                            if (short_analog == analog && analog.size() > 1)
                                short_analog = "";
                        }
                        const auto& short_idx = monomer_template.getStringPropIdx("naturalAnalogShort");
                        if (short_analog.size() == 0 && short_idx.first && monomer_template.hasStringProp(short_idx.second))
                            short_analog = monomer_template.getStringProp(short_idx.second);
                    };
                    if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                        get_analog(doc.ambiguousTemplates().at(monomer->templateId()));
                    else if (monomer->monomerType() == KetBaseMonomer::MonomerType::Monomer)
                        get_analog(doc.templates().at(monomer->templateId()));
                    else
                    {
                        throw Error("Unknown monomer type.");
                    }

                    if (short_analog.size() == 1)
                        monomer_alias = short_analog;
                    else if (monomer_class == MonomerClass::AminoAcid && sf != SeqFormat::Sequence3)
                        monomer_alias = "X";
                    else
                        throw Error("Can't save '%s' to sequence format", monomer_alias.c_str());
                }
                if (sf == SeqFormat::Sequence3)
                {
                    if (STANDARD_PEPTIDES.count(monomer_alias) > 0)
                        monomer_alias = monomerNameByAlias(kMonomerClassAminoAcid, monomer_alias);
                    else if (STANDARD_MIXED_PEPTIDES_ALIAS_TO_NAME.count(monomer_alias) > 0)
                        monomer_alias = STANDARD_MIXED_PEPTIDES_ALIAS_TO_NAME.at(monomer_alias);
                    else
                        throw Error("Unknown amino acid '%s'.", monomer_alias.c_str());
                }
                seq_string += monomer_alias;
            }

            if (seq_string.size())
            {
                // sequences separators are different for FASTA, IDT and Sequence
                if (sf == SeqFormat::FASTA)
                {
                    if (seq_idx)
                        seq_text += "\n";
                    std::string fasta_header;
                    if (prop_it != doc.fastaProps().end())
                    {
                        fasta_header = *prop_it;
                        prop_it++;
                    }
                    else
                    {
                        fasta_header = ">Sequence";
                        fasta_header += std::to_string(seq_idx + 1);
                    }
                    fasta_header += "\n";
                    seq_text += fasta_header;
                }
                else if (seq_text.size())
                    seq_text += sf == SeqFormat::Sequence || sf == SeqFormat::Sequence3 ? " " : "\n";

                seq_text += seq_string.substr(0, SEQ_LINE_LENGTH);
                for (size_t format_ind = SEQ_LINE_LENGTH; format_ind < seq_string.size(); format_ind += SEQ_LINE_LENGTH)
                {
                    seq_text += "\n";
                    seq_text += seq_string.substr(format_ind, SEQ_LINE_LENGTH);
                }
                seq_idx++;
            }
        }
    }
    if (seq_text.size())
        _output.writeString(seq_text.c_str());
}

void SequenceSaver::saveIdt(KetDocument& doc, std::vector<std::deque<std::string>> sequences, std::string& seq_text)
{
    auto& monomers = doc.monomers();
    if (doc.nonSequenceConnections().size() > 0)
        throw Error("Cannot save in IDT format - nonstandard connection found.");
    for (auto& sequence : sequences)
    {
        std::string seq_string;
        IdtModification modification = IdtModification::FIVE_PRIME_END;
        std::map<std::string, std::string> custom_variants;
        char custom_amb_monomers = 0;
        while (sequence.size() > 0)
        {
            auto monomer_id = sequence.front();
            sequence.pop_front();
            MonomerClass monomer_class = doc.getMonomerClass(monomer_id);
            auto& monomer = monomers.at(monomer_id)->alias();
            bool standard_sugar = true;
            bool standard_base = true;
            bool standard_phosphate = true;
            std::string sugar;
            std::string base;
            std::string phosphate;
            IdtModification possible_modification = modification;
            if (sequence.size() == 0) // last monomer
            {
                possible_modification = IdtModification::THREE_PRIME_END;
                if (seq_string.size() != 0) // for corner case - only one monomer - modification will be FIVE_PRIME_END
                    modification = IdtModification::THREE_PRIME_END;
            }

            if (monomer_class == MonomerClass::Phosphate || monomer_class == MonomerClass::CHEM || monomer_class == MonomerClass::DNA ||
                monomer_class == MonomerClass::RNA)
            {
                auto write_name = [&](const IdtAlias& idtAlias) -> bool {
                    bool has_modification = idtAlias.hasModification(modification);
                    if (has_modification || (possible_modification != modification && idtAlias.hasModification(possible_modification)))
                    {
                        const std::string& idt_alias =
                            has_modification ? idtAlias.getModification(modification) : idtAlias.getModification(possible_modification);
                        seq_string += idt_alias;
                        return true;
                    }
                    return false;
                };
                // Try to find in library
                const std::string& lib_monomer_id = _library.getMonomerTemplateIdByAlias(monomer_class, monomer);
                if (lib_monomer_id.size()) // Monomer in library
                {
                    if (write_name(_library.getMonomerTemplateById(lib_monomer_id).idtAlias()))
                    {
                        if (modification == IdtModification::FIVE_PRIME_END)
                            modification = IdtModification::INTERNAL;
                        continue;
                    }
                }

                // Check template for IdtAlias
                auto& monomer_template = doc.getMonomerTemplate(monomers.at(monomer_id)->templateId());
                if (write_name(monomer_template.idtAlias()))
                {
                    modification = IdtModification::INTERNAL;
                    continue;
                }
                else
                {
                    if (monomer_template.templateType() == KetBaseMonomerTemplate::TemplateType::MonomerTemplate &&
                        static_cast<const MonomerTemplate&>(monomer_template).unresolved())
                        throw Error("Unresolved monomer '%s' has no '%s' IDT alias.", monomer.c_str(), IdtAlias::IdtModificationToString(modification).c_str());
                    else if (monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA)
                        throw Error("Nucleotide '%s' has no '%s' IDT alias.", monomer.c_str(), IdtAlias::IdtModificationToString(modification).c_str());
                    else if (monomer_class == MonomerClass::Phosphate)
                        throw Error("Phosphate '%s' has no '%s' IDT alias.", monomer.c_str(), IdtAlias::IdtModificationToString(modification).c_str());
                    else // CHEM
                        throw Error("Chem '%s' has no '%s' IDT alias.", monomer.c_str(), IdtAlias::IdtModificationToString(modification).c_str());
                }
            }
            else if (monomer_class != MonomerClass::Sugar)
            {
                throw Error("Cannot save molecule in IDT format - expected sugar but found %s monomer %s.",
                            MonomerTemplate::MonomerClassToStr(monomer_class).c_str(), monomer.c_str());
            }

            sugar = monomer;
            if (IDT_STANDARD_SUGARS.count(monomer) == 0)
                standard_sugar = false;

            bool variant_base = false;
            if (sequence.size() > 0)
            { // process base
                auto base_id = sequence.front();
                if (doc.getMonomerClass(base_id) == MonomerClass::Base)
                {
                    const auto& base_monomer = *monomers.at(base_id);
                    base = base_monomer.alias();
                    sequence.pop_front();
                    if (IDT_STANDARD_BASES.count(base) == 0 && STANDARD_MIXED_BASES.count(base) == 0)
                        standard_base = false;
                    if (base_monomer.monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                    {
                        variant_base = true;
                        std::string template_id = monomers.at(base_id)->templateId();
                        if (custom_variants.count(template_id) > 0)
                        {
                            base = custom_variants.at(template_id);
                        }
                        else
                        {
                            std::array<float, 4> ratios{0, 0, 0, 0};
                            bool has_ratio = false;
                            std::set<std::string> aliases;
                            std::string s_aliases;
                            const auto& ambiguous_template = doc.ambiguousTemplates().at(template_id);
                            if (ambiguous_template.subtype() != "mixture")
                                throw Error("Cannot save IDT - only mixture supported but found %s.", ambiguous_template.subtype().c_str());
                            for (auto& option : ambiguous_template.options())
                            {
                                auto& opt_alias = getKetStrProp(doc.templates().at(option.templateId()), alias);
                                aliases.emplace(opt_alias);
                                if (s_aliases.size() > 0)
                                    s_aliases += ", ";
                                s_aliases += opt_alias;
                                const auto& it = IDT_BASE_TO_RATIO_IDX.find(opt_alias);
                                if (it == IDT_BASE_TO_RATIO_IDX.end())
                                    throw Error("Cannot save IDT - unknown monomer template %s", opt_alias.c_str());
                                auto ratio = option.ratio();

                                if (ratio.has_value())
                                {
                                    ratios[it->second] = ratio.value();
                                    has_ratio = true;
                                }
                                else if (has_ratio)
                                    throw Error("Cannot save IDT - ambiguous monomer template '%s' use template '%s' without ratio.", base.c_str(),
                                                opt_alias.c_str());
                            }
                            if (STANDARD_MIXED_BASES_TO_ALIAS.count(aliases) == 0)
                                throw Error("Cannot save IDT - unknown mixture of monomers %s", s_aliases.c_str());
                            base = STANDARD_MIXED_BASES_TO_ALIAS.at(aliases);
                            if (RNA_DNA_MIXED_BASES.count(base) == 0)
                            {
                                if (base[0] == 'r')
                                {
                                    base.erase(base.begin());
                                    if (sugar != "R")
                                        throw Error("Cannot save IDT - RNA ambigous base connected to DNA sugar.");
                                }
                                else if (sugar == "R")
                                    throw Error("Cannot save IDT - DNA ambigous base connected to RNA sugar.");
                            }
                            if (has_ratio)
                            {
                                std::string base_short = '(' + base;
                                base_short += '1' + custom_amb_monomers++;
                                base = base_short;
                                base += ':';
                                // add ratios
                                for (auto r : ratios)
                                {
                                    int ir = static_cast<int>(std::round(r));
                                    std::string sr = std::to_string(ir);
                                    if (sr.size() < 2)
                                        sr = '0' + sr;
                                    base += sr;
                                }
                                base += ')';
                                base_short += ')';
                                custom_variants.emplace(template_id, base_short);
                            }
                            else
                                custom_variants.emplace(template_id, base);
                        }
                    }
                }
            }

            if (sequence.size() > 0)
            { // process phosphate
                auto phosphate_id = sequence.front();
                sequence.pop_front();
                MonomerClass phosphate_class = doc.getMonomerClass(phosphate_id);
                if (phosphate_class != MonomerClass::Phosphate)
                    throw Error("Cannot save molecule in IDT format - phosphate expected between sugars but %s monomer %s found.",
                                MonomerTemplate::MonomerClassToStr(phosphate_class).c_str(), monomer.c_str());
                phosphate = monomers.at(phosphate_id)->alias();
                if (phosphate != "P" && phosphate != "sP")
                    standard_phosphate = false;
            }
            else
            {
                modification = IdtModification::THREE_PRIME_END;
                phosphate = "";
                standard_phosphate = true;
            }

            bool add_asterisk = false;
            if (phosphate == "sP")
            {
                phosphate = "P"; // Assume that modified monomers always contains P and modified to sP with *. TODO: confirm it with BA
                add_asterisk = true;
            }
            // Try to find sugar,base,phosphate group template
            const std::string& sugar_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Sugar, sugar);
            const std::string& phosphate_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, phosphate);
            std::string base_id;
            if (base.size())
                base_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Base, base);
            const std::string& idt_alias = _library.getIdtAliasByModification(modification, sugar_id, base_id, phosphate_id);
            if (idt_alias.size())
            {
                seq_string += idt_alias;
            }
            else if ((standard_base || variant_base) && standard_phosphate && standard_sugar)
            {
                sugar = IDT_STANDARD_SUGARS.at(sugar);
                if (sugar.size())
                    seq_string += sugar;
                seq_string += base == "In" ? "I" : base; // Inosine coded as I in IDT
            }
            else
            {
                if (base.size())
                {
                    if (phosphate.size())
                        throw Error("IDT alias for group sugar:%s base:%s phosphate:%s not found.", sugar.c_str(), base.c_str(), phosphate.c_str());
                    else
                        throw Error("IDT alias for group sugar:%s base:%s not found.", sugar.c_str(), base.c_str());
                }
                else
                {
                    if (phosphate.size())

                        throw Error("IDT alias for group sugar:%s phosphate:%s not found.", sugar.c_str(), phosphate.c_str());
                    else
                        throw Error("IDT alias for sugar:%s not found.", sugar.c_str());
                }
            }

            bool modified = idt_alias.size() > 0 && idt_alias.front() == '/';
            if (add_asterisk && (modification != IdtModification::THREE_PRIME_END || modified))
            {
                seq_string += "*";
                phosphate = "sP";
            }

            if (sequence.size() == 0 && phosphate.size() && !modified)
            {
                if (phosphate != "P")
                    throw Error("Cannot save molecule in IDT format - phosphate %s cannot be last monomer in sequence.", phosphate.c_str());

                seq_string += _library.getMonomerTemplateById(phosphate_id).idtAlias().getThreePrimeEnd();
            }
            if (modification == IdtModification::FIVE_PRIME_END)
                modification = IdtModification::INTERNAL;
        }
        if (seq_text.size() > 0)
            seq_text += "\n";
        seq_text += seq_string;
    }
}

void SequenceSaver::saveAxoLabs(KetDocument& doc, std::vector<std::deque<std::string>> sequences, std::string& seq_text)
{
    auto& monomers = doc.monomers();
    if (doc.nonSequenceConnections().size() > 0)
        throw Error("Cannot save in AxoLabs format - non-standard connection found.");
    for (auto& sequence : sequences)
    {
        std::string seq_string{AXOLABS_PREFIX};
        while (sequence.size() > 0)
        {
            auto monomer_id = sequence.front();
            sequence.pop_front();
            MonomerClass monomer_class = doc.getMonomerClass(monomer_id);
            auto& monomer = monomers.at(monomer_id)->alias();
            std::string sugar;
            std::string base;
            std::string phosphate;
            if (seq_string.size() == sizeof(AXOLABS_PREFIX) - 1 && monomer_class == MonomerClass::Phosphate && monomer != "P")
                throw Error("Cannot save molecule in AxoLabs format - phosphate %s cannot be first monomer in sequence.", monomer.c_str());

            if (monomer_class == MonomerClass::Phosphate || monomer_class == MonomerClass::CHEM || monomer_class == MonomerClass::DNA ||
                monomer_class == MonomerClass::RNA)
            {
                const std::string& lib_monomer_id = _library.getMonomerTemplateIdByAlias(monomer_class, monomer);
                auto& mon_template =
                    lib_monomer_id.size() > 0 ? _library.getMonomerTemplateById(lib_monomer_id) : doc.getMonomerTemplate(monomers.at(monomer_id)->templateId());
                if (mon_template.templateType() == KetBaseMonomerTemplate::TemplateType::AmbiguousMonomerTemplate)
                    throw Error("Cannot save in AxoLabs format - ambiguous monomer '%s' found.",
                                static_cast<const KetAmbiguousMonomerTemplate&>(mon_template).alias().c_str());
                auto& monomer_template = static_cast<const MonomerTemplate&>(mon_template);
                if (hasKetStrProp(monomer_template, aliasAxoLabs))
                {
                    seq_string += getKetStrProp(monomer_template, aliasAxoLabs);
                    continue;
                }
                else if (monomer_class == MonomerClass::Phosphate && monomer == "P" &&
                         (seq_string.size() == sizeof(AXOLABS_PREFIX) - 1 || sequence.size() == 0))
                {
                    seq_string += "p";
                    continue;
                }
                else
                {
                    if (monomer_template.templateType() == KetBaseMonomerTemplate::TemplateType::MonomerTemplate &&
                        static_cast<const MonomerTemplate&>(monomer_template).unresolved())
                        throw Error("Unresolved monomer '%s' has no AxoLabs alias.", monomer.c_str());
                    else if (monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA)
                        throw Error("Nucleotide '%s' has no AxoLabs alias.", monomer.c_str());
                    else if (monomer_class == MonomerClass::Phosphate)
                        throw Error("Phosphate '%s' has no AxoLabs alias.", monomer.c_str());
                    else // CHEM
                        throw Error("Chem '%s' has no AxoLabs alias.", monomer.c_str());
                }
            }
            else if (monomer_class != MonomerClass::Sugar)
            {
                throw Error("Cannot save molecule in AxoLabs format - expected sugar but found %s monomer %s.",
                            MonomerTemplate::MonomerClassToStr(monomer_class).c_str(), monomer.c_str());
            }

            sugar = monomer;

            if (sequence.size() > 0)
            { // process base
                auto base_id = sequence.front();
                if (doc.getMonomerClass(base_id) == MonomerClass::Base)
                {
                    const auto& base_monomer = *monomers.at(base_id);
                    base = base_monomer.alias();
                    sequence.pop_front();
                    if (base_monomer.monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                    {
                        throw Error("Cannot save in AxoLabs format - ambiguous base '%s' found.", base.c_str());
                    }
                }
            }

            bool has_phosphate = false;
            if (sequence.size() > 0)
            { // process phosphate
                auto phosphate_id = sequence.front();
                sequence.pop_front();
                MonomerClass phosphate_class = doc.getMonomerClass(phosphate_id);
                phosphate = monomers.at(phosphate_id)->alias();
                if (phosphate_class != MonomerClass::Phosphate)
                    throw Error("Cannot save molecule in AxoLabs format - phosphate expected between sugars but %s monomer %s found.",
                                MonomerTemplate::MonomerClassToStr(phosphate_class).c_str(), phosphate.c_str());
                if (phosphate != "P" && phosphate != "sP")
                    throw Error("Cannot save molecule in AxoLabs format - non-standard phosphate '%s' found", phosphate.c_str());
                has_phosphate = true;
            }

            bool add_s = false;
            if (phosphate == "sP")
            {
                if (sequence.size() == 0)
                    throw Error("Cannot save molecule in AxoLabs format - phosphate %s cannot be last monomer in sequence.", phosphate.c_str());
                phosphate = "P";
                add_s = true;
            }
            if (sequence.size() == 0 && phosphate.size() == 0 && base.size() > 0)
                phosphate = "P";

            // Try to find sugar,base,phosphate group template
            const std::string& sugar_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Sugar, sugar);
            const std::string& phosphate_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, phosphate);
            std::string base_id;
            if (base.size())
                base_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Base, base);
            std::string mgt_id;
            if (base.size() || phosphate.size())
                mgt_id = _library.getMGTidByComponents(sugar_id, base_id, phosphate_id);
            if (mgt_id.size())
            {
                auto& alias_axolabs = _library.getMonomerGroupTemplateById(mgt_id).aliasAxoLabs();
                if (!alias_axolabs.has_value())
                    throw Error("Monomer group '%s' has no AxoLabs alias.", mgt_id.c_str());
                seq_string += *alias_axolabs;
                if (sequence.size() == 0 && has_phosphate)
                    seq_string += "p";
            }
            else
            {
                if (base.size())
                {
                    if (phosphate.size() && has_phosphate)
                        throw Error("Group sugar:%s base:%s phosphate:%s not found.", sugar.c_str(), base.c_str(), phosphate.c_str());
                    else
                        throw Error("Group sugar:%s base:%s not found.", sugar.c_str(), base.c_str());
                }
                else
                {
                    if (phosphate.size() && has_phosphate)
                        throw Error("Group sugar:%s phosphate:%s not found.", sugar.c_str(), phosphate.c_str());
                    else
                        throw Error("Sugar:%s has no AxoLabs alias.", sugar.c_str());
                }
            }

            if (add_s)
            {
                seq_string += "s";
            }
        }
        if (seq_text.size() > 0)
            seq_text += "\n";
        seq_string += AXOLABS_SUFFIX;
        seq_text += seq_string;
    }
}

static std::string get_biln_attachment_idx(const KetConnectionEndPoint& ep)
{
    if (!hasKetStrProp(ep, monomerId))
        throw SequenceSaver::Error("Cannot save in BILN format - only monomer connections are supported.");
    if (!hasKetStrProp(ep, attachmentPointId))
        throw SequenceSaver::Error("Cannot save in BILN format - attachment point is required.");
    const auto& ap = getKetStrProp(ep, attachmentPointId);
    if (ap.size() < 2 || ap[0] != 'R')
        throw SequenceSaver::Error("Cannot save in BILN format - unsupported attachment point '%s'.", ap.c_str());
    for (size_t i = 1; i < ap.size(); i++)
    {
        if (!std::isdigit(static_cast<unsigned char>(ap[i])))
            throw SequenceSaver::Error("Cannot save in BILN format - unsupported attachment point '%s'.", ap.c_str());
    }
    if (ap == "R0")
        throw SequenceSaver::Error("Cannot save in BILN format - unsupported attachment point '%s'.", ap.c_str());
    return ap.substr(1);
}

static std::string format_biln_alias(const std::string& monomer_alias, bool strip_terminal_cap = false)
{
    if (monomer_alias.empty())
        throw SequenceSaver::Error("Cannot save empty monomer alias in BILN format.");
    auto biln_alias = monomer_alias;
    if (strip_terminal_cap && biln_alias.size() > 1)
    {
        if (biln_alias.back() == '-')
            biln_alias.pop_back();
        else if (biln_alias.front() == '-')
            biln_alias.erase(biln_alias.begin());
    }
    bool needs_brackets = false;
    for (auto ch : biln_alias)
    {
        if (ch == '-')
        {
            needs_brackets = true;
            continue;
        }
        if (ch == '.' || ch == '(' || ch == ')' || ch == ',' || ch == '[' || ch == ']' || std::isspace(static_cast<unsigned char>(ch)))
            throw SequenceSaver::Error("Cannot save monomer alias '%s' in BILN format.", monomer_alias.c_str());
    }
    return needs_brackets ? "[" + biln_alias + "]" : biln_alias;
}

std::string SequenceSaver::saveBILN(KetDocument& doc, const std::vector<std::deque<std::string>>& sequences)
{
    (void)sequences;
    static const char* biln_export_error = "Only amino acids and CHEMs with BILN codes can get exported to BILN.";

    if (doc.moleculesRefs().size() > 0)
        throw Error(biln_export_error);

    struct BilnNode
    {
        std::string monomer_id;
        std::string monomer_ref;
        std::string alias;
        std::string biln_alias;
        std::vector<std::string> biln_template_ids;
        MonomerClass monomer_class;
    };
    struct BilnAlias
    {
        std::string alias;
        std::vector<std::string> template_ids;
    };
    struct BilnConnection
    {
        int node1;
        std::string ap1;
        int node2;
        std::string ap2;
        int bond_idx = 0;
    };
    struct BilnChain
    {
        std::vector<int> nodes;
        std::string sort_key;
        std::string topology_sort_key;
        int amino_acid_count = 0;
        int effective_amino_acid_count = 0;
        int monomer_count = 0;
    };

    std::vector<BilnNode> nodes;
    std::map<std::string, int> monomer_ref_to_node;
    const auto& monomers = doc.monomers();
    const auto& monomer_ids = doc.monomersIds();
    auto get_biln_alias = [&](MonomerClass monomer_class, const std::string& monomer_alias) {
        auto make_biln_alias = [&](const std::vector<std::string>& template_ids) {
            if (template_ids.empty())
                throw Error(biln_export_error);
            const auto& monomer_template = _library.getMonomerTemplateById(template_ids.front());
            const auto& template_alias = getKetStrProp(monomer_template, alias);
            const bool strip_terminal_cap = monomer_class == MonomerClass::AminoAcid && template_alias.size() > 1 &&
                                            (template_alias.back() == '-' || template_alias.front() == '-') &&
                                            monomer_template.attachmentPoints().size() == 1;
            return BilnAlias{format_biln_alias(template_alias, strip_terminal_cap), template_ids};
        };

        std::string template_id = _library.getMonomerTemplateIdByAlias(monomer_class, monomer_alias);
        if (template_id.empty())
            template_id = _library.getMonomerTemplateIdByAliasHELM(monomer_class, monomer_alias);
        if (!template_id.empty())
            return make_biln_alias({template_id});

        if (monomer_class == MonomerClass::AminoAcid)
        {
            std::vector<std::string> cap_template_ids;
            template_id = _library.getMonomerTemplateIdByAlias(monomer_class, monomer_alias + "-");
            if (!template_id.empty())
                cap_template_ids.push_back(template_id);
            template_id = _library.getMonomerTemplateIdByAlias(monomer_class, "-" + monomer_alias);
            if (!template_id.empty() && std::find(cap_template_ids.begin(), cap_template_ids.end(), template_id) == cap_template_ids.end())
                cap_template_ids.push_back(template_id);
            if (!cap_template_ids.empty())
                return make_biln_alias(cap_template_ids);
        }

        throw Error(biln_export_error);
        return BilnAlias{};
    };
    for (const auto& monomer_id : monomer_ids)
    {
        auto monomer_class = doc.getMonomerClass(monomer_id);
        const auto& monomer = monomers.at(monomer_id);
        if (monomer_class != MonomerClass::AminoAcid && monomer_class != MonomerClass::CHEM)
            throw Error(biln_export_error);
        auto biln_alias = get_biln_alias(monomer_class, monomer->alias());
        const int node_idx = static_cast<int>(nodes.size());
        nodes.push_back({monomer_id, monomer->ref(), monomer->alias(), biln_alias.alias, biln_alias.template_ids, monomer_class});
        monomer_ref_to_node.emplace(monomer->ref(), node_idx);
    }

    std::vector<int> next(nodes.size(), -1);
    std::vector<int> prev(nodes.size(), -1);
    std::vector<BilnConnection> explicit_connections;
    std::set<std::pair<int, std::string>> used_connection_endpoints;
    std::map<int, std::set<std::string>> node_used_attachment_points;

    auto is_terminal_cap_alias = [&](const std::string& monomer_alias) {
        auto template_id = _library.getMonomerTemplateIdByAlias(MonomerClass::AminoAcid, monomer_alias);
        if (template_id.empty())
            template_id = _library.getMonomerTemplateIdByAlias(MonomerClass::AminoAcid, monomer_alias + "-");
        if (template_id.empty())
            return false;
        const auto& monomer_template = _library.getMonomerTemplateById(template_id);
        const auto& template_alias = getKetStrProp(monomer_template, alias);
        return template_alias.size() > 1 && (template_alias.back() == '-' || template_alias.front() == '-') &&
               monomer_template.attachmentPoints().size() == 1;
    };
    auto is_terminal_cap_template = [&](const std::string& template_id) {
        const auto& monomer_template = _library.getMonomerTemplateById(template_id);
        const auto& template_alias = getKetStrProp(monomer_template, alias);
        return template_alias.size() > 1 && (template_alias.back() == '-' || template_alias.front() == '-') &&
               monomer_template.attachmentPoints().size() == 1;
    };
    auto is_terminal_cap_node = [&](int node_idx) {
        const auto& node = nodes.at(node_idx);
        if (node.monomer_class != MonomerClass::AminoAcid)
            return false;
        for (const auto& template_id : node.biln_template_ids)
            if (is_terminal_cap_template(template_id))
                return true;
        return is_terminal_cap_alias(node.alias) || is_terminal_cap_alias(node.biln_alias);
    };
    auto is_biln_backbone_connection = [](const std::string& ap1, const std::string& ap2) {
        return (ap1 == kAttachmentPointR1 && ap2 == kAttachmentPointR2) || (ap1 == kAttachmentPointR2 && ap2 == kAttachmentPointR1);
    };
    auto read_endpoint = [&](const KetConnectionEndPoint& ep) -> std::pair<int, std::string> {
        if (!hasKetStrProp(ep, monomerId) || !hasKetStrProp(ep, attachmentPointId))
            throw Error(biln_export_error);
        if (hasKetStrProp(ep, moleculeId) || hasKetStrProp(ep, atomId))
            throw Error(biln_export_error);
        const auto& monomer_ref = getKetStrProp(ep, monomerId);
        const auto node_it = monomer_ref_to_node.find(monomer_ref);
        if (node_it == monomer_ref_to_node.end())
            throw Error(biln_export_error);
        const auto& ap = getKetStrProp(ep, attachmentPointId);
        std::ignore = get_biln_attachment_idx(ep);
        const auto& node = nodes.at(node_it->second);
        bool supported_by_biln_template = false;
        for (const auto& template_id : node.biln_template_ids)
        {
            if (_library.getMonomerTemplateById(template_id).attachmentPoints().count(ap) > 0)
            {
                supported_by_biln_template = true;
                break;
            }
        }
        if (monomers.at(node.monomer_id)->attachmentPoints().count(ap) == 0 || !supported_by_biln_template)
            throw Error("Cannot save in BILN format - unsupported attachment point '%s'.", ap.c_str());
        if (!used_connection_endpoints.emplace(node_it->second, ap).second)
            throw Error("Cannot save in BILN format - attachment point '%s' of monomer '%s' is used more than once.", ap.c_str(),
                        node.alias.c_str());
        node_used_attachment_points[node_it->second].emplace(ap);
        return {node_it->second, ap};
    };

    for (const auto& connection : doc.connections())
    {
        if (connection.connType() != KetConnection::TYPE::SINGLE)
            throw Error(biln_export_error);
        const auto& ep1 = connection.ep1();
        const auto& ep2 = connection.ep2();
        auto [node1, ap1] = read_endpoint(ep1);
        auto [node2, ap2] = read_endpoint(ep2);
        if (is_biln_backbone_connection(ap1, ap2) && !is_terminal_cap_node(node1) && !is_terminal_cap_node(node2))
        {
            int left = ap1 == kAttachmentPointR2 ? node1 : node2;
            int right = ap1 == kAttachmentPointR2 ? node2 : node1;
            if (next.at(left) != -1 || prev.at(right) != -1)
                throw Error("Cannot save in BILN format - branched backbones are not supported.");
            next.at(left) = right;
            prev.at(right) = left;
        }
        else
        {
            explicit_connections.push_back({node1, ap1, node2, ap2});
        }
    }

    for (const auto& [node_idx, attachment_points] : node_used_attachment_points)
    {
        const auto& node = nodes.at(node_idx);
        bool supported_by_single_biln_template = false;
        for (const auto& template_id : node.biln_template_ids)
        {
            const auto& template_attachment_points = _library.getMonomerTemplateById(template_id).attachmentPoints();
            bool template_supports_all_aps = true;
            for (const auto& ap : attachment_points)
            {
                if (template_attachment_points.count(ap) == 0)
                {
                    template_supports_all_aps = false;
                    break;
                }
            }
            if (template_supports_all_aps)
            {
                supported_by_single_biln_template = true;
                break;
            }
        }
        if (!supported_by_single_biln_template)
            throw Error("Cannot save in BILN format - attachment points of monomer '%s' do not match a BILN monomer template.", node.alias.c_str());
    }

    auto make_sort_key = [&](const std::vector<int>& chain_nodes) {
        std::string key;
        for (size_t idx = 0; idx < chain_nodes.size(); idx++)
        {
            if (idx > 0)
                key += '-';
            key += nodes.at(chain_nodes[idx]).biln_alias;
        }
        return key;
    };
    auto finish_chain = [&](std::vector<int> chain_nodes) {
        BilnChain chain;
        chain.nodes = std::move(chain_nodes);
        chain.monomer_count = static_cast<int>(chain.nodes.size());
        for (int node_idx : chain.nodes)
            if (nodes.at(node_idx).monomer_class == MonomerClass::AminoAcid)
                chain.amino_acid_count++;
        chain.effective_amino_acid_count = chain.monomer_count <= 5 ? chain.monomer_count : chain.amino_acid_count;
        chain.sort_key = make_sort_key(chain.nodes);
        return chain;
    };
    auto make_cycle_key = [&](const std::vector<int>& chain_nodes, bool reverse) {
        std::map<int, int> node_to_pos;
        std::set<int> chain_node_set;
        for (int idx = 0; idx < static_cast<int>(chain_nodes.size()); idx++)
        {
            node_to_pos.emplace(chain_nodes[idx], idx);
            chain_node_set.emplace(chain_nodes[idx]);
        }

        std::vector<BilnConnection> candidate_connections;
        for (const auto& connection : explicit_connections)
        {
            if (chain_node_set.count(connection.node1) || chain_node_set.count(connection.node2))
                candidate_connections.push_back(connection);
        }
        candidate_connections.push_back(
            {chain_nodes.front(), reverse ? kAttachmentPointR2 : kAttachmentPointR1, chain_nodes.back(), reverse ? kAttachmentPointR1 : kAttachmentPointR2});

        std::vector<std::vector<int>> node_to_bonds(chain_nodes.size());
        for (int bond_idx = 0; bond_idx < static_cast<int>(candidate_connections.size()); bond_idx++)
        {
            auto pos1 = node_to_pos.find(candidate_connections[bond_idx].node1);
            if (pos1 != node_to_pos.end())
                node_to_bonds.at(pos1->second).push_back(bond_idx);
            auto pos2 = node_to_pos.find(candidate_connections[bond_idx].node2);
            if (pos2 != node_to_pos.end() && candidate_connections[bond_idx].node2 != candidate_connections[bond_idx].node1)
            {
                node_to_bonds.at(pos2->second).push_back(bond_idx);
            }
        }

        int next_bond_idx = 1;
        auto append_bond_endpoint = [&](std::string& monomer_text, BilnConnection& bond, int node_idx) {
            if (bond.bond_idx == 0)
                bond.bond_idx = next_bond_idx++;
            if (bond.node1 == node_idx)
                monomer_text += "(" + std::to_string(bond.bond_idx) + "," + bond.ap1.substr(1) + ")";
            if (bond.node2 == node_idx)
                monomer_text += "(" + std::to_string(bond.bond_idx) + "," + bond.ap2.substr(1) + ")";
        };

        std::string key;
        for (int monomer_idx = 0; monomer_idx < static_cast<int>(chain_nodes.size()); monomer_idx++)
        {
            if (monomer_idx > 0)
                key += '-';
            const int node_idx = chain_nodes[monomer_idx];
            std::string monomer_text = nodes.at(node_idx).biln_alias;
            auto& incident_bonds = node_to_bonds.at(monomer_idx);
            std::sort(incident_bonds.begin(), incident_bonds.end(), [&](int left_idx, int right_idx) {
                auto other_key = [&](const BilnConnection& bond, int current_node) {
                    int other_node = bond.node1 == current_node && bond.node2 != current_node ? bond.node2 : bond.node1;
                    auto pos_it = node_to_pos.find(other_node);
                    if (pos_it != node_to_pos.end())
                        return std::make_tuple(0, pos_it->second, nodes.at(other_node).biln_alias, other_node);
                    return std::make_tuple(1, 0, nodes.at(other_node).biln_alias, other_node);
                };
                const auto& left_bond = candidate_connections.at(left_idx);
                const auto& right_bond = candidate_connections.at(right_idx);
                if ((left_bond.bond_idx == 0) != (right_bond.bond_idx == 0))
                    return left_bond.bond_idx != 0;
                if (left_bond.bond_idx != 0 && right_bond.bond_idx != 0)
                    return left_bond.bond_idx < right_bond.bond_idx;
                auto left_key = other_key(left_bond, node_idx);
                auto right_key = other_key(right_bond, node_idx);
                if (left_key != right_key)
                    return left_key < right_key;
                return std::tie(left_bond.node1, left_bond.ap1, left_bond.node2, left_bond.ap2) <
                       std::tie(right_bond.node1, right_bond.ap1, right_bond.node2, right_bond.ap2);
            });
            for (int bond_idx : incident_bonds)
                append_bond_endpoint(monomer_text, candidate_connections.at(bond_idx), node_idx);
            key += monomer_text;
        }
        return key;
    };

    std::vector<BilnChain> chains;
    std::vector<bool> visited(nodes.size(), false);
    for (int start_node = 0; start_node < static_cast<int>(nodes.size()); start_node++)
    {
        if (visited.at(start_node))
            continue;
        std::vector<int> component;
        std::vector<int> stack = {start_node};
        visited.at(start_node) = true;
        while (!stack.empty())
        {
            int node = stack.back();
            stack.pop_back();
            component.push_back(node);
            for (int adjacent : {next.at(node), prev.at(node)})
            {
                if (adjacent != -1 && !visited.at(adjacent))
                {
                    visited.at(adjacent) = true;
                    stack.push_back(adjacent);
                }
            }
        }

        bool is_cycle = component.size() > 1;
        for (int node : component)
        {
            if (next.at(node) == -1 || prev.at(node) == -1)
            {
                is_cycle = false;
                break;
            }
        }

        if (is_cycle)
        {
            std::vector<int> directed_cycle;
            int node = component.front();
            do
            {
                if (std::find(directed_cycle.begin(), directed_cycle.end(), node) != directed_cycle.end())
                    throw Error("Cannot save in BILN format - invalid cyclic backbone.");
                directed_cycle.push_back(node);
                node = next.at(node);
                if (node == -1)
                    throw Error("Cannot save in BILN format - invalid cyclic backbone.");
            } while (node != component.front());
            if (directed_cycle.size() != component.size())
                throw Error("Cannot save in BILN format - invalid cyclic backbone.");

            std::string best_key;
            std::vector<int> best_nodes;
            bool best_reverse = false;
            const int cycle_size = static_cast<int>(directed_cycle.size());
            for (int offset = 0; offset < cycle_size; offset++)
            {
                std::vector<int> candidate;
                for (int idx = 0; idx < cycle_size; idx++)
                    candidate.push_back(directed_cycle.at((offset + idx) % cycle_size));
                auto candidate_key = make_cycle_key(candidate, false);
                if (best_nodes.empty() || candidate_key < best_key)
                {
                    best_key = candidate_key;
                    best_nodes = candidate;
                    best_reverse = false;
                }

                candidate.clear();
                for (int idx = 0; idx < cycle_size; idx++)
                    candidate.push_back(directed_cycle.at((offset - idx + cycle_size) % cycle_size));
                candidate_key = make_cycle_key(candidate, true);
                if (candidate_key < best_key)
                {
                    best_key = candidate_key;
                    best_nodes = candidate;
                    best_reverse = true;
                }
            }
            explicit_connections.push_back({best_nodes.front(), best_reverse ? kAttachmentPointR2 : kAttachmentPointR1, best_nodes.back(),
                                            best_reverse ? kAttachmentPointR1 : kAttachmentPointR2});
            chains.push_back(finish_chain(best_nodes));
        }
        else
        {
            int start = -1;
            for (int node : component)
            {
                if (prev.at(node) == -1)
                {
                    if (start != -1)
                        throw Error("Cannot save in BILN format - invalid backbone.");
                    start = node;
                }
            }
            if (start == -1)
                start = component.front();
            std::vector<int> chain_nodes;
            int node = start;
            while (node != -1)
            {
                if (std::find(chain_nodes.begin(), chain_nodes.end(), node) != chain_nodes.end())
                    throw Error("Cannot save in BILN format - invalid backbone.");
                chain_nodes.push_back(node);
                node = next.at(node);
            }
            if (chain_nodes.size() != component.size())
                throw Error("Cannot save in BILN format - invalid backbone.");
            chains.push_back(finish_chain(chain_nodes));
        }
    }

    auto make_chain_topology_key = [&](const BilnChain& chain) {
        std::map<int, int> node_to_pos;
        for (int idx = 0; idx < static_cast<int>(chain.nodes.size()); idx++)
            node_to_pos.emplace(chain.nodes[idx], idx);

        std::vector<std::string> endpoint_keys;
        auto add_endpoint_key = [&](int local_node, const std::string& local_ap, int other_node, const std::string& other_ap) {
            std::string key = std::to_string(node_to_pos.at(local_node)) + ":" + nodes.at(local_node).biln_alias + ":" + local_ap + ">";
            const auto other_pos = node_to_pos.find(other_node);
            if (other_pos != node_to_pos.end())
                key += "I:" + std::to_string(other_pos->second) + ":" + nodes.at(other_node).biln_alias + ":" + other_ap;
            else
                key += "E:" + nodes.at(other_node).biln_alias + ":" + other_ap;
            endpoint_keys.push_back(key);
        };

        for (const auto& connection : explicit_connections)
        {
            const auto node1_pos = node_to_pos.find(connection.node1);
            const auto node2_pos = node_to_pos.find(connection.node2);
            if (node1_pos == node_to_pos.end() && node2_pos == node_to_pos.end())
                continue;
            if (node1_pos != node_to_pos.end())
                add_endpoint_key(connection.node1, connection.ap1, connection.node2, connection.ap2);
            if (node2_pos != node_to_pos.end() && connection.node2 != connection.node1)
                add_endpoint_key(connection.node2, connection.ap2, connection.node1, connection.ap1);
        }

        std::sort(endpoint_keys.begin(), endpoint_keys.end());
        std::string key;
        for (const auto& endpoint_key : endpoint_keys)
        {
            if (!key.empty())
                key += "|";
            key += endpoint_key;
        }
        return key;
    };

    for (auto& chain : chains)
        chain.topology_sort_key = make_chain_topology_key(chain);

    std::sort(chains.begin(), chains.end(), [&](const BilnChain& left, const BilnChain& right) {
        auto terminal_cap_rank = [&](const BilnChain& chain) {
            if (chain.monomer_count != 1 || chain.nodes.empty())
                return 0;
            int node_idx = chain.nodes.front();
            if (!is_terminal_cap_node(node_idx))
                return 0;
            for (const auto& connection : explicit_connections)
            {
                if (connection.node1 == node_idx)
                {
                    if (connection.ap1 == kAttachmentPointR2)
                        return -1;
                    if (connection.ap1 == kAttachmentPointR1)
                        return 1;
                }
                if (connection.node2 == node_idx)
                {
                    if (connection.ap2 == kAttachmentPointR2)
                        return -1;
                    if (connection.ap2 == kAttachmentPointR1)
                        return 1;
                }
            }
            return 0;
        };
        auto left_cap_rank = terminal_cap_rank(left);
        auto right_cap_rank = terminal_cap_rank(right);
        if (left_cap_rank != right_cap_rank)
            return left_cap_rank < right_cap_rank;
        if (left.effective_amino_acid_count != right.effective_amino_acid_count)
            return left.effective_amino_acid_count > right.effective_amino_acid_count;
        if (left.monomer_count != right.monomer_count)
            return left.monomer_count > right.monomer_count;
        if (left.sort_key != right.sort_key)
            return left.sort_key < right.sort_key;
        return left.topology_sort_key < right.topology_sort_key;
    });

    std::map<int, std::pair<int, int>> node_to_chain_pos;
    for (int chain_idx = 0; chain_idx < static_cast<int>(chains.size()); chain_idx++)
        for (int monomer_idx = 0; monomer_idx < static_cast<int>(chains[chain_idx].nodes.size()); monomer_idx++)
            node_to_chain_pos.emplace(chains[chain_idx].nodes[monomer_idx], std::make_pair(chain_idx, monomer_idx));

    auto endpoint_position = [&](int node_idx) { return node_to_chain_pos.at(node_idx); };
    std::vector<std::vector<std::vector<int>>> node_to_bonds(chains.size());
    for (size_t chain_idx = 0; chain_idx < chains.size(); chain_idx++)
        node_to_bonds.at(chain_idx).resize(chains[chain_idx].nodes.size());
    for (int bond_idx = 0; bond_idx < static_cast<int>(explicit_connections.size()); bond_idx++)
    {
        auto pos1 = endpoint_position(explicit_connections[bond_idx].node1);
        node_to_bonds.at(pos1.first).at(pos1.second).push_back(bond_idx);
        if (explicit_connections[bond_idx].node2 != explicit_connections[bond_idx].node1)
        {
            auto pos2 = endpoint_position(explicit_connections[bond_idx].node2);
            node_to_bonds.at(pos2.first).at(pos2.second).push_back(bond_idx);
        }
    }

    int next_bond_idx = 1;
    auto append_bond_endpoint = [&](std::string& monomer_text, BilnConnection& bond, int node_idx) {
        if (bond.bond_idx == 0)
            bond.bond_idx = next_bond_idx++;
        if (bond.node1 == node_idx)
            monomer_text += "(" + std::to_string(bond.bond_idx) + "," + bond.ap1.substr(1) + ")";
        if (bond.node2 == node_idx)
            monomer_text += "(" + std::to_string(bond.bond_idx) + "," + bond.ap2.substr(1) + ")";
    };

    std::string biln_string;
    for (int chain_idx = 0; chain_idx < static_cast<int>(chains.size()); chain_idx++)
    {
        if (chain_idx > 0)
            biln_string += '.';
        const auto& chain = chains[chain_idx];
        for (int monomer_idx = 0; monomer_idx < static_cast<int>(chain.nodes.size()); monomer_idx++)
        {
            if (monomer_idx > 0)
                biln_string += '-';
            const int node_idx = chain.nodes[monomer_idx];
            std::string monomer_text = nodes.at(node_idx).biln_alias;
            auto& incident_bonds = node_to_bonds.at(chain_idx).at(monomer_idx);
            std::sort(incident_bonds.begin(), incident_bonds.end(), [&](int left_idx, int right_idx) {
                auto other_pos = [&](const BilnConnection& bond, int current_node) {
                    if (bond.node1 == current_node && bond.node2 != current_node)
                        return endpoint_position(bond.node2);
                    return endpoint_position(bond.node1);
                };
                const auto& left_bond = explicit_connections.at(left_idx);
                const auto& right_bond = explicit_connections.at(right_idx);
                if ((left_bond.bond_idx == 0) != (right_bond.bond_idx == 0))
                    return left_bond.bond_idx != 0;
                if (left_bond.bond_idx != 0 && right_bond.bond_idx != 0)
                    return left_bond.bond_idx < right_bond.bond_idx;
                auto left_pos = other_pos(left_bond, node_idx);
                auto right_pos = other_pos(right_bond, node_idx);
                if (left_pos != right_pos)
                    return left_pos < right_pos;
                return std::tie(left_bond.node1, left_bond.ap1, left_bond.node2, left_bond.ap2) <
                       std::tie(right_bond.node1, right_bond.ap1, right_bond.node2, right_bond.ap2);
            });
            for (int bond_idx : incident_bonds)
                append_bond_endpoint(monomer_text, explicit_connections.at(bond_idx), node_idx);
            biln_string += monomer_text;
        }
    }
    return biln_string;
}

static const char* get_helm_class(MonomerClass monomer_class)
{
    switch (monomer_class)
    {
    case MonomerClass::AminoAcid:
        return kHELMPolymerTypePEPTIDE;
    case MonomerClass::Base:
    case MonomerClass::Sugar:
    case MonomerClass::Phosphate:
    case MonomerClass::RNA:
    case MonomerClass::DNA:
    case MonomerClass::Linker:
    case MonomerClass::Terminator:
        return kHELMPolymerTypeRNA;
    case MonomerClass::Unknown:
    case MonomerClass::CHEM:
    default:
        return kHELMPolymerTypeCHEM;
        break;
    }
    return kHELMPolymerTypeCHEM;
}

void SequenceSaver::add_monomer(KetDocument& document, const std::unique_ptr<KetBaseMonomer>& monomer, std::string& helm_string)
{
    std::string monomer_str;
    const auto& mon_templ = document.templates().at(monomer->templateId());
    const auto& template_id = _library.getMonomerTemplateIdByAlias(mon_templ.monomerClass(), getKetStrProp(mon_templ, alias));
    if (template_id.size() > 0)
    {
        auto& mononomer_template = _library.monomerTemplates().at(template_id);
        std::string alias;
        if (hasKetStrProp(mononomer_template, aliasHELM))
            alias = getKetStrProp(mononomer_template, aliasHELM);
        if (alias.size() > 0)
            monomer_str = alias;
        else
            monomer_str = getKetStrProp(mononomer_template, alias);
    }
    else if (mon_templ.unresolved())
    {
        if (hasKetStrProp(mon_templ, aliasHELM))
            monomer_str = getKetStrProp(mon_templ, aliasHELM);
        else
            monomer_str = "*";
    }
    else
    {
        // monomer not in library - generate smiles
        auto tgroup = mon_templ.getTGroup(true);
        auto* pmol = static_cast<Molecule*>(tgroup->fragment.get());
        StringOutput s_out(monomer_str);
        SmilesSaver saver(s_out);
        saver.separate_rsites = false;
        saver.chemaxon = true;
        saver.saveMolecule(*pmol);
    }
    if (monomer_str.size() == 1)
        helm_string += monomer_str;
    else
        helm_string += '[' + monomer_str + ']';
}

std::string SequenceSaver::saveHELM(KetDocument& document, const std::vector<std::deque<std::string>>& sequences)
{
    std::string helm_string = "";
    int peptide_idx = 0;
    int rna_idx = 0;
    int chem_idx = 0;
    using MonomerInfo = std::tuple<HELMType, int, int>;
    std::map<std::string, MonomerInfo> monomer_id_to_monomer_info;
    const auto& monomers = document.monomers();
    const auto& templates = document.templates();
    const auto& variant_templates = document.ambiguousTemplates();
    std::map<std::string, std::map<int, std::string>> mol_atom_to_ap;
    for (auto& sequence : sequences)
    {
        int monomer_idx = 0;
        int polymer_idx = -1;
        std::string helm_polymer_class = "";
        HELMType helm_type = HELMType::Unknown;
        MonomerClass prev_monomer_class = MonomerClass::Unknown;
        for (auto monomer_id : sequence)
        {
            const auto& monomer = monomers.at(monomer_id);
            auto monomer_class = document.getMonomerClass(monomer_id);
            if (monomer_idx == 0)
            {
                if (helm_string.size() > 0)
                    helm_string += '|';
                // start new polymer
                if (monomer->monomerType() == KetBaseMonomer::MonomerType::Monomer && hasKetStrProp(templates.at(monomer->templateId()), classHELM))
                    helm_polymer_class = getKetStrProp(templates.at(monomer->templateId()), classHELM);
                else
                    helm_polymer_class = get_helm_class(monomer_class);
                helm_string += helm_polymer_class;
                helm_type = getHELMTypeFromString(helm_polymer_class);
                if (helm_polymer_class == kHELMPolymerTypePEPTIDE)
                    polymer_idx = ++peptide_idx;
                else if (helm_polymer_class == kHELMPolymerTypeRNA)
                    polymer_idx = ++rna_idx;
                else if (helm_polymer_class == kHELMPolymerTypeCHEM)
                    polymer_idx = ++chem_idx;
                helm_string += std::to_string(polymer_idx);
                helm_string += '{';
            }
            if (monomer_idx && monomer_class != MonomerClass::Base && !((prev_monomer_class == MonomerClass::Base) && monomer_class == MonomerClass::Phosphate))
                helm_string += '.'; // no separator between base and between base and phosphate
            if (monomer_idx > 0 && monomer_class == MonomerClass::Base && prev_monomer_class != MonomerClass::Sugar)
                throw Error("Wrong monomer sequence: base monomer %s after %s monomer.", monomer->alias().c_str(),
                            MonomerTemplate::MonomerClassToStr(prev_monomer_class).c_str());
            if (monomer_class == MonomerClass::Base)
                helm_string += '(';
            if (monomer->monomerType() == KetBaseMonomer::MonomerType::Monomer)
            {
                add_monomer(document, monomer, helm_string);
            }
            else if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
            {
                const auto& templ = variant_templates.at(monomer->templateId());
                if (monomer_class != MonomerClass::Base)
                    helm_string += '(';
                std::string variants;
                bool mixture = (templ.subtype() == "mixture");
                for (const auto& option : templ.options())
                {
                    if (variants.size() > 0)
                        variants += mixture ? '+' : ',';
                    std::string alias;
                    auto& mononomer_template = templates.at(option.templateId());
                    if (hasKetStrProp(mononomer_template, aliasHELM))
                        alias = getKetStrProp(mononomer_template, aliasHELM);
                    if (alias.size() == 0)
                        alias = getKetStrProp(mononomer_template, alias);
                    if (alias.size() > 1)
                        variants += '[';
                    variants += alias;
                    if (alias.size() > 1)
                        variants += ']';
                    auto num = mixture ? option.ratio() : option.probability();
                    if (num.has_value())
                    {
                        variants += ':';
                        std::ostringstream ss;
                        ss << num.value();
                        variants += ss.str();
                    }
                }
                helm_string += variants;
                if (monomer_class != MonomerClass::Base)
                    helm_string += ')';
            }
            auto& annotation = monomer->annotation();
            if (annotation.has_value())
            {
                if (hasKetStrProp(annotation.value(), text))
                {
                    helm_string += '"';
                    helm_string += getKetStrProp(annotation.value(), text);
                    helm_string += '"';
                }
            }
            if (monomer_class == MonomerClass::Base)
                helm_string += ')';
            monomer_idx++;
            monomer_id_to_monomer_info.emplace(std::make_pair(monomer_id, std::make_tuple(helm_type, polymer_idx, monomer_idx)));
            prev_monomer_class = monomer_class;
        }
        if (monomer_idx)
            helm_string += '}'; // Finish polymer
    }
    auto& molecules = document.jsonMolecules();
    int molecule_idx = 0;
    rapidjson::Document json{};
    std::map<std::string, std::vector<int>> molecules_connections;
    if (molecules.Size() > 0)
    {
        auto process_ep = [&molecules_connections](const KetConnectionEndPoint& ep) {
            if (hasKetStrProp(ep, moleculeId))
            {
                const auto& mol_id = getKetStrProp(ep, moleculeId);
                if (molecules_connections.count(mol_id) == 0)
                    molecules_connections.try_emplace(mol_id);
                if (hasKetStrProp(ep, atomId))
                    molecules_connections.at(mol_id).push_back(std::stoi(getKetStrProp(ep, atomId)));
            }
        };
        for (const auto& connection : document.nonSequenceConnections())
        {
            process_ep(connection.ep1());
            process_ep(connection.ep2());
        }
    }
    for (rapidjson::SizeType i = 0; i < molecules.Size(); i++)
    {
        const auto& molecule = molecules[i];
        std::string mol_id = "mol" + std::to_string(molecule_idx++);
        rapidjson::Value marr(rapidjson::kArrayType);
        marr.PushBack(json.CopyFrom(molecule, json.GetAllocator()), json.GetAllocator());
        MoleculeJsonLoader loader(marr);
        BaseMolecule* pbmol;
        Molecule mol;
        QueryMolecule qmol;
        try
        {
            loader.loadMolecule(mol);
            pbmol = &mol;
        }
        catch (...)
        {
            loader.loadMolecule(qmol);
            pbmol = &qmol;
        }
        // convert Sup sgroup without name attachment points to rg-labels
        auto& sgroups = pbmol->sgroups;
        int ap_count = 0;
        for (int j = sgroups.begin(); j != sgroups.end(); j = sgroups.next(j))
        {
            auto& sgroup = sgroups.getSGroup(j);
            if (sgroup.sgroup_type != SGroup::SG_TYPE_SUP)
                continue;
            Superatom& sa = static_cast<Superatom&>(sgroup);
            if (sa.subscript.size() != 0 && sa.subscript.ptr()[0] != 0)
                continue;
            // convert leaving atom H to rg-ref
            auto res = mol_atom_to_ap.try_emplace(mol_id);
            auto& atom_to_ap = res.first;
            static std::string apid_prefix{'R'};
            for (int ap_id = sa.attachment_points.begin(); ap_id != sa.attachment_points.end(); ap_id = sa.attachment_points.next(ap_id))
            {
                ap_count++;
                auto& ap = sa.attachment_points.at(ap_id);
                std::string apid = apid_prefix + ap.apid.ptr();
                atom_to_ap->second.emplace(ap.aidx, apid);
                int leaving_atom = ap.lvidx;
                int ap_idx = std::stoi(ap.apid.ptr());
                if (pbmol == &mol)
                {
                    mol.resetAtom(leaving_atom, ELEM_RSITE);
                    mol.allowRGroupOnRSite(leaving_atom, ap_idx);
                }
                else
                {
                    auto rsite = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_RSITE, 0);
                    qmol.resetAtom(leaving_atom, rsite.release());
                    qmol.allowRGroupOnRSite(leaving_atom, ap_idx);
                }
            }
            sgroups.remove(j);
        }
        // check direct monomer to molecule connections without attachment point
        if (molecules_connections.count(mol_id) > 0 && ap_count == 0)
        {
            int ap_idx = 1;
            auto res = mol_atom_to_ap.try_emplace(mol_id);
            auto& atom_to_ap = res.first;
            static std::string apid_prefix{'R'};
            for (auto atom_id : molecules_connections.at(mol_id))
            {
                std::string apid = apid_prefix + std::to_string(ap_idx);
                atom_to_ap->second.emplace(atom_id, apid);
                // add leaving atom and set it as R-site
                auto leaving_atom = pbmol->addAtom(ELEM_RSITE);
                pbmol->addBond(atom_id, leaving_atom, BOND_SINGLE);
                pbmol->allowRGroupOnRSite(leaving_atom, ap_idx++);
            }
        }
        // generate smiles
        std::string smiles;
        StringOutput s_out(smiles);
        SmilesSaver saver(s_out);
        saver.separate_rsites = false;
        saver.chemaxon = true;
        if (pbmol == &mol)
            saver.saveMolecule(mol);
        else
            saver.saveQueryMolecule(qmol);
        // save as chem
        if (helm_string.size() > 0)
            helm_string += '|';
        helm_string += "CHEM";
        int polymer_idx = ++chem_idx;
        helm_string += std::to_string(polymer_idx);
        helm_string += "{[";
        helm_string += smiles;
        helm_string += "]}";
        monomer_id_to_monomer_info.emplace(std::make_pair(mol_id, std::make_tuple(HELMType::Chem, polymer_idx, 1)));
    }
    helm_string += '$';
    // Add connections
    int connections_count = 0;
    for (const auto& connection : document.nonSequenceConnections())
    {
        // add connection
        if (connections_count)
            helm_string += '|';
        const auto& ep_1 = connection.ep1();
        const auto& ep_2 = connection.ep2();
        if (!(hasKetStrProp(ep_1, monomerId) || hasKetStrProp(ep_1, moleculeId)) || !(hasKetStrProp(ep_2, monomerId) || hasKetStrProp(ep_2, moleculeId)))
            throw Error("Endpoint without monomer or molecule id");
        bool has_mon_id1 = hasKetStrProp(ep_1, monomerId);
        bool has_mon_id2 = hasKetStrProp(ep_2, monomerId);
        const auto& monomer_id_1 = has_mon_id1 ? getKetStrProp(ep_1, monomerId) : getKetStrProp(ep_1, moleculeId);
        const auto& monomer_id_2 = has_mon_id2 ? getKetStrProp(ep_2, monomerId) : getKetStrProp(ep_2, moleculeId);
        const auto& id1 = has_mon_id1 ? document.monomerIdByRef(monomer_id_1) : monomer_id_1;
        const auto& id2 = has_mon_id2 ? document.monomerIdByRef(monomer_id_2) : monomer_id_2;
        auto [type_1, pol_num_1, mon_num_1] = monomer_id_to_monomer_info.at(id1);
        auto [type_2, pol_num_2, mon_num_2] = monomer_id_to_monomer_info.at(id2);
        connections_count++;
        helm_string += getStringFromHELMType(type_1);
        helm_string += std::to_string(pol_num_1);
        helm_string += ',';
        helm_string += getStringFromHELMType(type_2);
        helm_string += std::to_string(pol_num_2);
        helm_string += ',';
        helm_string += std::to_string(mon_num_1);
        helm_string += ":";
        if (hasKetStrProp(ep_1, atomId))
            helm_string += mol_atom_to_ap.at(id1).at(std::stoi(getKetStrProp(ep_1, atomId)));
        else if (hasKetStrProp(ep_1, attachmentPointId))
            helm_string += getKetStrProp(ep_1, attachmentPointId);
        else if (connection.connType() == KetConnection::TYPE::HYDROGEN)
            helm_string += HelmHydrogenPair;
        else
            helm_string += '?';
        helm_string += '-';
        helm_string += std::to_string(mon_num_2);
        helm_string += ':';
        if (hasKetStrProp(ep_2, atomId))
            helm_string += mol_atom_to_ap.at(id2).at(std::stoi(getKetStrProp(ep_2, atomId)));
        else if (hasKetStrProp(ep_2, attachmentPointId))
            helm_string += getKetStrProp(ep_2, attachmentPointId);
        else if (connection.connType() == KetConnection::TYPE::HYDROGEN)
            helm_string += HelmHydrogenPair;
        else
            helm_string += '?';
        auto& annotation = connection.annotation();
        if (annotation.has_value())
        {
            if (hasKetStrProp(annotation.value(), text))
            {
                helm_string += '"';
                helm_string += getKetStrProp(annotation.value(), text);
                helm_string += '"';
            }
        }
    }
    helm_string += '$';
    // Add polymer groups
    helm_string += '$';
    // Add ExtendedAnnotation
    auto& annotation = document.annotation();
    if (annotation.has_value())
    {
        auto& extended = annotation->extended();
        if (extended.has_value())
        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            extended->Accept(writer);
            helm_string += buffer.GetString();
        }
    }
    helm_string += '$';
    // Add helm version
    helm_string += "V2.0";
    return helm_string;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
