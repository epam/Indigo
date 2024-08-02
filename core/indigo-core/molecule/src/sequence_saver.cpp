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
#include "../molecule/monomer_commons.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "layout/sequence_layout.h"
#include "molecule/molecule.h"
#include "molecule/monomers_template_library.h"

using namespace indigo;

IMPL_ERROR(SequenceSaver, "Sequence saver");

CP_DEF(SequenceSaver);

SequenceSaver::SequenceSaver(Output& output, MonomerTemplateLibrary& library) : _output(output), _mon_lib(MonomerTemplates::_instance()), _library(library)
{
}

SequenceSaver::~SequenceSaver()
{
}

std::string SequenceSaver::saveIdt(BaseMolecule& mol, std::deque<int>& sequence)
{
    static const std::unordered_set<std::string> IDT_STANDARD_BASES = {"A", "T", "C", "G", "U", "I", "In"};
    static const std::map<std::string, std::string> IDT_STANDARD_SUGARS{{"R", "r"}, {"LR", "+"}, {"mR", "m"}, {"dR", ""}};
    std::string seq_string;
    std::unordered_set<int> used_atoms;
    IdtModification modification = IdtModification::FIVE_PRIME_END;
    while (sequence.size() > 0)
    {
        int atom_idx = sequence.front();
        used_atoms.emplace(atom_idx);
        sequence.pop_front();
        if (!mol.isTemplateAtom(atom_idx))
            throw Error("Cannot save regular atom %s in IDT format.", mol.getAtomDescription(atom_idx).c_str());
        std::string monomer_class = mol.getTemplateAtomClass(atom_idx);
        std::string monomer = mol.getTemplateAtom(atom_idx);
        bool standard_sugar = true;
        bool standard_base = true;
        bool standard_phosphate = true;
        std::string sugar;
        std::string base;
        std::string phosphate;
        if (monomer_class == kMonomerClassPHOSPHATE)
        {
            if (used_atoms.size() > 1 && sequence.size()) // Inside the sequence
                throw Error("Cannot save molecule in IDT format - expected sugar but found phosphate %s.", monomer.c_str());
            // first and last monomer can be phosphate "P" only
            if (monomer != "P")
            {
                if (used_atoms.size() > 1)
                    throw Error("Cannot save molecule in IDT format - phosphate %s cannot be last monomer in sequence.", monomer.c_str());
                throw Error("Cannot save molecule in IDT format - phosphate %s cannot be first monomer in sequence.", monomer.c_str());
            }
            // This is 'P' at one of the end
            if (used_atoms.size() == 1) // First monomer
            {
                seq_string += "/5Phos/";
                modification = IdtModification::INTERNAL;
            }
            else
            {
                seq_string += "/3Phos/";
                modification = IdtModification::THREE_PRIME_END;
            }
            continue;
        }
        else if (monomer_class == kMonomerClassCHEM || monomer_class == kMonomerClassDNA || monomer_class == kMonomerClassRNA)
        {
            // Try to find in library
            const std::string& monomer_id = _library.getMonomerTemplateIdByAlias(MonomerTemplate::StrToMonomerClass(monomer_class), monomer);
            if (monomer_id.size()) // Monomer in library
            {
                const MonomerTemplate& templ = _library.getMonomerTemplateById(monomer_id);
                if (templ.idtAlias().hasModification(modification))
                {
                    const std::string& idt_alias = templ.idtAlias().getModification(modification);
                    seq_string += '/';
                    seq_string += idt_alias;
                    seq_string += '/';
                    continue;
                }
            }

            // Check TGroup for IdtAlias
            std::optional<std::reference_wrapper<TGroup>> tg_ref = std::nullopt;
            int temp_idx = mol.getTemplateAtomTemplateIndex(atom_idx);
            if (temp_idx > -1)
                tg_ref = std::optional<std::reference_wrapper<TGroup>>(std::ref(mol.tgroups.getTGroup(temp_idx)));
            else
                auto tg_ref = findTemplateInMap(monomer, monomer_class, _templates);
            if (tg_ref.has_value())
            {
                auto& tg = tg_ref.value().get();
                if (tg.idt_alias.size())
                {
                    seq_string.push_back('/');
                    seq_string.append(tg.idt_alias.ptr());
                    seq_string.push_back('/');
                    modification = IdtModification::INTERNAL;
                    continue;
                }
                else
                {
                    if (tg.unresolved)
                        throw Error("Unresolved monomer '%s' has no IDT alias.", tg.tgroup_text_id.ptr());
                    else if (monomer_class == kMonomerClassDNA || monomer_class == kMonomerClassRNA)
                        throw Error("Nucleotide '%s' has no IDT alias.", monomer.c_str());
                    else // CHEM
                        throw Error("Chem '%s' has no IDT alias.", tg.tgroup_text_id.ptr());
                }
            }
            else
            {
                throw Error("Internal error - monomer %s with class %s not found.", monomer.c_str(), monomer_class.c_str());
            }
        }
        else if (monomer_class != kMonomerClassSUGAR)
        {
            throw Error("Cannot save molecule in IDT format - expected sugar but found %s monomer %s.", monomer_class.c_str(), monomer.c_str());
        }

        sugar = monomer;
        if (IDT_STANDARD_SUGARS.count(monomer) == 0)
            standard_sugar = false;
        auto& v = mol.getVertex(atom_idx);
        for (auto nei_idx = v.neiBegin(); nei_idx < v.neiEnd(); nei_idx = v.neiNext(nei_idx))
        {
            int nei_atom_idx = v.neiVertex(nei_idx);
            if (used_atoms.count(nei_atom_idx) > 0)
                continue;
            used_atoms.emplace(nei_atom_idx);
            if (mol.isTemplateAtom(nei_atom_idx))
            {
                monomer_class = std::string(mol.getTemplateAtomClass(nei_atom_idx));
                if (monomer_class == kMonomerClassBASE)
                {
                    if (base.size() > 0)
                        throw Error("Cannot save molecule in IDT format - sugar %s with two base connected %s and %s.", monomer.c_str(), base.c_str(),
                                    mol.getTemplateAtom(nei_atom_idx));
                    base = mol.getTemplateAtom(nei_atom_idx);
                    if (IDT_STANDARD_BASES.count(base) == 0)
                        standard_base = false;
                }
                else if (monomer_class == kMonomerClassPHOSPHATE)
                {
                    if (phosphate.size() > 0) // left phosphate should be in used_atoms and skiped
                        throw Error("Cannot save molecule in IDT format - sugar %s with too much phosphates connected %s and %s.", monomer.c_str(),
                                    phosphate.c_str(), mol.getTemplateAtom(nei_atom_idx));
                    phosphate = mol.getTemplateAtom(nei_atom_idx);
                }
                else if (monomer_class != kMonomerClassCHEM) // chem is ok in any place
                {
                    throw Error("Cannot save molecule in IDT format - sugar %s connected to monomer %s with class %s (only base or phosphate expected).",
                                monomer.c_str(), mol.getTemplateAtom(nei_atom_idx), monomer_class.c_str());
                }
            }
            else
            {
                throw Error("Cannot save regular atom %s in IDT format.", mol.getAtomDescription(atom_idx).c_str());
            }
        }

        if (sequence.size() > 0)
        { // process phosphate
            atom_idx = sequence.front();
            sequence.pop_front();
            if (!mol.isTemplateAtom(atom_idx))
                throw Error("Cannot save regular atom %s in IDT format.", mol.getAtomDescription(atom_idx).c_str());
            monomer_class = mol.getTemplateAtomClass(atom_idx);
            monomer = mol.getTemplateAtom(atom_idx);
            if (monomer_class != kMonomerClassPHOSPHATE)
                throw Error("Cannot save molecule in IDT format - phosphate expected between sugars but %s monomer %s found.", monomer_class.c_str(),
                            monomer.c_str());
            if (used_atoms.count(atom_idx) == 0) // phosphate should be already processed at sugar neighbours check
                throw Error("Cannot save molecule in IDT format - phosphate %s not connected to previous sugar.", phosphate.c_str());
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
        if (standard_base && standard_phosphate && standard_sugar)
        {
            sugar = IDT_STANDARD_SUGARS.at(sugar);
            if (sugar.size())
                seq_string += sugar;
            seq_string += base == "In" ? "I" : base; // Inosine coded as I in IDT
            if (sequence.size() == 0 && phosphate.size())
            {
                if (phosphate != "P" || add_asterisk)
                    throw Error("Cannot save molecule in IDT format - phosphate %s cannot be last monomer in sequence.", monomer.c_str());
                seq_string += "/3Phos/";
            }
        }
        else
        {
            // Try to find sugar,base,phosphate group template
            const std::string& sugar_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Sugar, sugar);
            const std::string& phosphate_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Phosphate, phosphate);
            std::string base_id;
            if (base.size())
                base_id = _library.getMonomerTemplateIdByAlias(MonomerClass::Base, base);
            const std::string& idt_alias = _library.getIdtAliasByModification(modification, sugar_id, base_id, phosphate_id);
            if (idt_alias.size())
            {
                seq_string += '/';
                seq_string += idt_alias;
                seq_string += '/';
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
        }

        if (add_asterisk)
        {
            seq_string += "*";
            phosphate = "sP";
        }

        if (modification == IdtModification::FIVE_PRIME_END)
            modification = IdtModification::INTERNAL;
    }
    return seq_string;
}

static inline void add_monomer(std::string& helm_string, const std::string& monomer_alias)
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
        monomer_alias = monomer_template.getStringProp("alias");
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
        helm_polymer_class = monomer_template.getStringProp("classHELM");
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

std::string SequenceSaver::saveHELM(BaseMolecule& mol, std::vector<std::deque<int>>& sequences)
{
    std::string helm_string = "";
    int peptide_idx = 0;
    int rna_idx = 0;
    int chem_idx = 0;
    std::set<int> used_atoms;
    using MonomerInfo = std::tuple<HELMType, int, int>;
    constexpr int polymer_type = 0;
    constexpr int polymer_num = 1;
    constexpr int monomer_num = 2;
    std::map<int, MonomerInfo> atom_idx_to_monomer_info;
    std::set<std::pair<int, int>> used_connections;
    int prev_atom_idx;
    for (auto& sequence : sequences)
    {
        int monomer_idx = 0;
        int polymer_idx = -1;
        std::string helm_polymer_class = "";
        HELMType helm_type = HELMType::Unknown;
        for (auto atom_idx : sequence)
        {
            if (used_atoms.count(atom_idx) > 0) // Phosphate can be processed with rest of nucleotide
                continue;
            std::string monomer = mol.getTemplateAtom(atom_idx);
            std::string monomer_alias = getMonomerAlias(mol, atom_idx);
            std::string monomer_class = mol.getTemplateAtomClass(atom_idx);
            if (monomer_idx == 0)
            {
                // start new polymer
                const std::string& monomer_id = _library.getMonomerTemplateIdByAlias(MonomerTemplates::getStrToMonomerType().at(monomer_class), monomer);
                if (monomer_id.size())
                    helm_polymer_class = _library.getMonomerTemplateById(monomer_id).getStringProp("classHELM");
                if (helm_string.size())
                    helm_string += '|'; // separator between polymers
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
            else
            {
                used_connections.emplace(std::min(atom_idx, prev_atom_idx), std::max(atom_idx, prev_atom_idx));
            }
            if (monomer_alias.size() == 0)
            {
                if (monomer_class == kMonomerClassBASE)
                    monomer_alias = monomerAliasByName(monomer_class, monomer);
                else if (isAminoAcidClass(monomer_class))
                    monomer_alias = monomerAliasByName(kMonomerClassAA, monomer);
                else if (isNucleotideClass(monomer_class))
                    monomer_alias = monomerAliasByName(kMonomerClassBASE, monomer);
                if (monomer_alias.size() == 0) // If alias not foud - use monomer name
                    monomer_alias = monomer;
            }
            if (monomer_idx)
                helm_string += '.'; // separator between monomers
            add_monomer(helm_string, monomer_alias);
            monomer_idx++;
            atom_idx_to_monomer_info.emplace(std::make_pair(atom_idx, std::make_tuple(helm_type, polymer_idx, monomer_idx)));

            used_atoms.emplace(atom_idx);
            prev_atom_idx = atom_idx;

            if (monomer_class == kMonomerClassSUGAR)
            {
                auto& v = mol.getVertex(atom_idx);
                std::string phosphate = "";
                int phosphate_idx = -1;
                for (auto nei_idx = v.neiBegin(); nei_idx < v.neiEnd(); nei_idx = v.neiNext(nei_idx))
                {
                    int nei_atom_idx = v.neiVertex(nei_idx);
                    if (mol.isTemplateAtom(nei_atom_idx))
                    {
                        if (used_atoms.count(nei_atom_idx) > 0)
                            continue;
                        std::string mon_class = mol.getTemplateAtomClass(nei_atom_idx);
                        if (mon_class == kMonomerClassBASE)
                        {
                            helm_string += '('; // branch monomers in ()
                            add_monomer(helm_string, monomerAliasByName(mon_class, mol.getTemplateAtom(nei_atom_idx)));
                            monomer_idx++;
                            atom_idx_to_monomer_info.emplace(std::make_pair(nei_atom_idx, std::make_tuple(helm_type, polymer_idx, monomer_idx)));
                            used_atoms.emplace(nei_atom_idx);
                            used_connections.emplace(std::min(atom_idx, nei_atom_idx), std::max(atom_idx, nei_atom_idx));
                            helm_string += ')';
                        }
                        else if (mon_class == kMonomerClassPHOSPHATE)
                        {
                            phosphate = monomerAliasByName(mon_class, mol.getTemplateAtom(nei_atom_idx));
                            phosphate_idx = nei_atom_idx;
                        }
                    }
                }
                if (phosphate.size())
                {
                    add_monomer(helm_string, phosphate);
                    monomer_idx++;
                    atom_idx_to_monomer_info.emplace(std::make_pair(phosphate_idx, std::make_tuple(helm_type, polymer_idx, monomer_idx)));
                    used_atoms.emplace(phosphate_idx);
                    used_connections.emplace(std::min(atom_idx, phosphate_idx), std::max(atom_idx, phosphate_idx));
                    prev_atom_idx = phosphate_idx;
                }
            }
        }
        if (monomer_idx)
            helm_string += '}'; // Finish polymer
    }
    helm_string += '$';
    // Add connections
    int connections_count = 0;
    std::vector<std::map<int, int>> directions_map;
    mol.getTemplateAtomDirectionsMap(directions_map);
    std::set<std::pair<int, int>> processed_connections;
    for (int atom_idx = 0; atom_idx < mol.vertexCount(); atom_idx++)
    {
        if (mol.isTemplateAtom(atom_idx))
        {
            for (auto& connection : directions_map[atom_idx])
            {
                if (processed_connections.count(std::make_pair(atom_idx, connection.second)) == 0)
                {
                    auto [cur_type, cur_pol_num, cur_mon_num] = atom_idx_to_monomer_info.at(atom_idx);
                    auto [nei_type, nei_pol_num, nei_mon_num] = atom_idx_to_monomer_info.at(connection.second);
                    if (cur_type != nei_type || cur_pol_num != nei_pol_num ||
                        used_connections.find(std::make_pair(std::min(atom_idx, connection.second), std::max(atom_idx, connection.second))) ==
                            used_connections.end())
                    {
                        // add connection
                        if (connections_count)
                            helm_string += '|';
                        connections_count++;
                        helm_string += getStringFromHELMType(cur_type);
                        helm_string += std::to_string(cur_pol_num);
                        helm_string += ',';
                        helm_string += getStringFromHELMType(nei_type);
                        helm_string += std::to_string(nei_pol_num);
                        helm_string += ',';
                        helm_string += std::to_string(cur_mon_num);
                        helm_string += ":R";
                        helm_string += std::to_string(connection.first + 1);
                        helm_string += '-';
                        helm_string += std::to_string(nei_mon_num);
                        helm_string += ':';
                        int nei_ap_id = -1;
                        for (auto& nei_conn : directions_map[connection.second])
                        { // TODO: rewrite when connection will contain info about neighb ap_id
                            if (nei_conn.second == atom_idx)
                            {
                                nei_ap_id = nei_conn.first;
                                break;
                            }
                        }
                        if (nei_ap_id >= 0)
                        {
                            helm_string += 'R';
                            helm_string += std::to_string(nei_ap_id + 1);
                        }
                        else
                        {
                            helm_string += '?';
                        }
                    }
                    processed_connections.emplace(std::make_pair(atom_idx, connection.second));
                    processed_connections.emplace(std::make_pair(connection.second, atom_idx));
                }
            }
        }
    }
    helm_string += '$';
    // Add polymer groups
    helm_string += '$';
    // Add ExtendedAnnotation
    helm_string += '$';
    // Add helm version
    helm_string += "V2.0";
    return helm_string;
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
    auto& mol_properties = mol.properties();
    std::vector<std::deque<int>> sequences;
    SequenceLayout sl(mol);
    sl.sequenceExtract(sequences);
    auto prop_it = mol_properties.begin();
    int seq_idx = 0;
    if (sf == SeqFormat::HELM)
    {
        seq_text = saveHELM(mol, sequences);
    }
    else
    {
        if (sf == SeqFormat::IDT)
        {
            std::vector<std::map<int, int>> directions_map;
            mol.getTemplateAtomDirectionsMap(directions_map);
            std::map<int, int> left_backbone_links;
            std::map<int, int> right_backbone_links;
            std::map<int, size_t> seq_start;
            std::map<int, size_t> seq_end;
            for (size_t idx = 0; idx < sequences.size(); idx++)
            {
                auto& sequence = sequences[idx];
                auto template_idx = sequence.front();
                seq_start[template_idx] = idx;
                seq_end[sequence.back()] = idx;
                if (sequence.size() != 1) // CHEM sequence always only one monomer
                    continue;
                if (strcasecmp(mol.getTemplateAtomClass(template_idx), kMonomerClassCHEM))
                    continue;
                check_backbone_connection(mol, directions_map, template_idx, kLeftAttachmentPointIdx, left_backbone_links, right_backbone_links);
                check_backbone_connection(mol, directions_map, template_idx, kRightAttachmentPointIdx, right_backbone_links, left_backbone_links);
            }
            if (left_backbone_links.size())
            {
                std::vector<std::deque<int>> joined_sequences;
                while (left_backbone_links.size())
                {
                    auto left_atom_idx = left_backbone_links.begin()->second;
                    // find leftmost sequence and copy to joined sequences
                    for (auto left = left_backbone_links.find(left_atom_idx); left != left_backbone_links.end(); left = left_backbone_links.find(left_atom_idx))
                    {
                        left_atom_idx = left->second;
                    }
                    joined_sequences.push_back({});
                    for (auto idx : sequences[seq_end[left_atom_idx]])
                    {
                        joined_sequences.back().emplace_back(idx);
                    }
                    // while have sequence at right - connect it
                    for (auto right = right_backbone_links.find(left_atom_idx); right != right_backbone_links.end();)
                    {
                        auto right_atom_idx = right->second;
                        left_backbone_links.erase(right_atom_idx);
                        int right_idx;
                        for (auto idx : sequences[seq_start[right_atom_idx]])
                        {
                            joined_sequences.back().emplace_back(idx);
                            right_idx = idx;
                        }
                        right = right_backbone_links.find(right_idx);
                    }
                }
                sequences = joined_sequences;
            }
        }
        for (auto& sequence : sequences)
        {
            std::string seq_string;
            if (sf == SeqFormat::IDT)
            {
                seq_string.append(saveIdt(mol, sequence));
            }
            else
            {
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
