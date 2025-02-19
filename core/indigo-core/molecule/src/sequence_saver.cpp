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
#include "molecule/ket_document.h"
#include "molecule/ket_objects.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/smiles_saver.h"

#ifdef _MSC_VER
#pragma warning(push)
#endif

using namespace indigo;

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

std::string SequenceSaver::saveIdt(BaseMolecule& mol, std::deque<int>& sequence)
{
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
            add_monomer_str(helm_string, monomer_alias);
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
                            add_monomer_str(helm_string, monomerAliasByName(mon_class, mol.getTemplateAtom(nei_atom_idx)));
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
                    add_monomer_str(helm_string, phosphate);
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

void SequenceSaver::saveKetDocument(KetDocument& doc, SeqFormat sf)
{
    std::vector<std::deque<std::string>> sequences;
    int seq_idx = 0;
    std::string seq_text;
    if (sf == SeqFormat::HELM)
    {
        doc.parseSimplePolymers(sequences, false);
        seq_text = saveHELM(doc, sequences);
    }
    else if (sf == SeqFormat::IDT)
    {
        doc.parseSimplePolymers(sequences, true);
        saveIdt(doc, sequences, seq_text);
    }
    else if (sf == SeqFormat::FASTA || sf == SeqFormat::Sequence || sf == SeqFormat::Sequence3)
    {
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
                        if (monomer_template.hasStringProp("naturalAnalog"))
                        {
                            std::string analog = monomer_template.getStringProp("naturalAnalog");
                            short_analog = monomerAliasByName(MonomerTemplate::MonomerClassToStr(monomer_class), analog);
                            if (short_analog == analog && analog.size() > 1)
                                short_analog = "";
                        }
                        if (short_analog.size() == 0 && monomer_template.hasStringProp("naturalAnalogShort"))
                        {
                            short_analog = monomer_template.getStringProp("naturalAnalogShort");
                        }
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
                    if (monomer_class != MonomerClass::AminoAcid)
                        throw Error("Only amino acids can be saved as three letter amino acid codes.");
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
    auto& monomer_templates = doc.templates();
    auto& variant_monomer_templates = doc.ambiguousTemplates();
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
                if (seq_string.size() > 0)
                    modification = IdtModification::THREE_PRIME_END;
                else // corner case - only one monomer
                    possible_modification = IdtModification::THREE_PRIME_END;

            if (monomer_class == MonomerClass::Phosphate)
            {
                if (seq_string.size() > 0 && sequence.size()) // Inside the sequence
                    throw Error("Cannot save molecule in IDT format - expected sugar but found phosphate %s.", monomer.c_str());
                // first and last monomer can be phosphate "P" only
                if (monomer != "P")
                {
                    if (seq_string.size() > 0)
                        throw Error("Cannot save molecule in IDT format - phosphate %s cannot be last monomer in sequence.", monomer.c_str());
                    throw Error("Cannot save molecule in IDT format - phosphate %s cannot be first monomer in sequence.", monomer.c_str());
                }
                // This is 'P' at one of the end
                if (seq_string.size() == 0) // First monomer
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
            else if (monomer_class == MonomerClass::CHEM || monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA)
            {
                auto write_name = [&](const IdtAlias& idtAlias) -> bool {
                    bool has_modification = idtAlias.hasModification(modification);
                    if (has_modification || (possible_modification != modification && idtAlias.hasModification(possible_modification)))
                    {
                        const std::string& idt_alias =
                            has_modification ? idtAlias.getModification(modification) : idtAlias.getModification(possible_modification);
                        seq_string += '/';
                        seq_string += idt_alias;
                        seq_string += '/';
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
                        throw Error("Unresolved monomer '%s' has no IDT alias.", monomer.c_str());
                    else if (monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA)
                        throw Error("Nucleotide '%s' has no IDT alias.", monomer.c_str());
                    else // CHEM
                        throw Error("Chem '%s' has no IDT alias.", monomer.c_str());
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
                                auto& opt_alias = doc.templates().at(option.templateId()).getStringProp("alias");
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
            else
            {
                throw Error("Cannot save molecule in IDT format - sugar whithout base.");
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
            if ((standard_base || variant_base) && standard_phosphate && standard_sugar)
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
        if (seq_text.size() > 0)
            seq_text += "\n";
        seq_text += seq_string;
    }
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
    const auto& template_id = _library.getMonomerTemplateIdByAlias(mon_templ.monomerClass(), mon_templ.getStringProp("alias"));
    if (template_id.size() > 0)
    {
        monomer_str = _library.monomerTemplates().at(template_id).getStringProp("alias");
    }
    else
    {
        // monomer not in library - generate smiles
        auto tgroup = mon_templ.getTGroup();
        auto* pmol = static_cast<Molecule*>(tgroup->fragment.get());

        // convert Sup sgroup without name attachment points to rg-labels
        auto& sgroups = pmol->sgroups;
        for (int i = sgroups.begin(); i != sgroups.end(); i = sgroups.next(i))
        {
            auto& sgroup = sgroups.getSGroup(i);
            if (sgroup.sgroup_type != SGroup::SG_TYPE_SUP)
                continue;
            Superatom& sa = static_cast<Superatom&>(sgroup);
            for (int ap_id = sa.attachment_points.begin(); ap_id != sa.attachment_points.end(); ap_id = sa.attachment_points.next(ap_id))
            {
                auto& ap = sa.attachment_points.at(ap_id);
                int leaving_atom = ap.lvidx;
                int ap_idx = getAttachmentOrder(ap.apid.ptr()) + 1;
                if (leaving_atom >= 0)
                {
                    pmol->resetAtom(leaving_atom, ELEM_RSITE);
                }
                else
                {
                    leaving_atom = pmol->addAtom(ELEM_RSITE);
                    pmol->addBond(ap.aidx, leaving_atom, BOND_SINGLE);
                }
                pmol->allowRGroupOnRSite(leaving_atom, ap_idx);
            }
            sgroups.remove(i);
        }
        std::string smiles;
        StringOutput s_out(monomer_str);
        SmilesSaver saver(s_out);
        saver.separate_rsites = false;
        saver.saveMolecule(*pmol);
    }
    if (monomer_str.size() == 1)
        helm_string += monomer_str;
    else
        helm_string += '[' + monomer_str + ']';
}

std::string SequenceSaver::saveHELM(KetDocument& document, std::vector<std::deque<std::string>> sequences)
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
                std::string helm_polymer_class;
                if (monomer->monomerType() == KetBaseMonomer::MonomerType::Monomer && templates.at(monomer->templateId()).hasStringProp("classHELM"))
                    helm_polymer_class = templates.at(monomer->templateId()).getStringProp("classHELM");
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
            if (monomer_idx && monomer_class != MonomerClass::Base &&
                !((prev_monomer_class == MonomerClass::Base || prev_monomer_class == MonomerClass::Sugar) && monomer_class == MonomerClass::Phosphate))
                helm_string += '.'; // separator between sugar and base and between sugar or base and phosphate
            if (monomer_class == MonomerClass::Base && prev_monomer_class != MonomerClass::Sugar)
                throw Error("Wrong monomer sequence: base monomer %s after %s monomer.", monomer->alias().c_str(),
                            MonomerTemplate::MonomerClassToStr(prev_monomer_class).c_str());
            if (monomer_class == MonomerClass::Base)
                helm_string += '(';
            if (monomer->monomerType() == KetBaseMonomer::MonomerType::Monomer)
            {
                if (templates.at(monomer->templateId()).unresolved())
                    helm_string += '*';
                else
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
                    auto alias = templates.at(option.templateId()).getStringProp("alias");
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
            if (ep.hasStringProp("moleculeId"))
            {
                const auto& mol_id = ep.getStringProp("moleculeId");
                if (molecules_connections.count(mol_id) == 0)
                    molecules_connections.try_emplace(mol_id);
                if (ep.hasStringProp("atomId"))
                    molecules_connections.at(mol_id).push_back(std::stoi(ep.getStringProp("atomId")));
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
        for (int i = sgroups.begin(); i != sgroups.end(); i = sgroups.next(i))
        {
            auto& sgroup = sgroups.getSGroup(i);
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
            sgroups.remove(i);
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
        if (!(ep_1.hasStringProp("monomerId") || ep_1.hasStringProp("moleculeId")) || !(ep_2.hasStringProp("monomerId") || ep_2.hasStringProp("moleculeId")))
            throw Error("Endpoint without monomer or molecule id");
        bool has_mon_id1 = ep_1.hasStringProp("monomerId");
        bool has_mon_id2 = ep_2.hasStringProp("monomerId");
        const auto& monomer_id_1 = has_mon_id1 ? ep_1.getStringProp("monomerId") : ep_1.getStringProp("moleculeId");
        const auto& monomer_id_2 = has_mon_id2 ? ep_2.getStringProp("monomerId") : ep_2.getStringProp("moleculeId");
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
        if (ep_1.hasStringProp("atomId"))
            helm_string += mol_atom_to_ap.at(id1).at(std::stoi(ep_1.getStringProp("atomId")));
        else if (ep_1.hasStringProp("attachmentPointId"))
            helm_string += ep_1.getStringProp("attachmentPointId");
        else if (connection.connType() == KetConnection::TYPE::HYDROGEN)
            helm_string += HelmHydrogenPair;
        else
            helm_string += '?';
        helm_string += '-';
        helm_string += std::to_string(mon_num_2);
        helm_string += ':';
        if (ep_2.hasStringProp("atomId"))
            helm_string += mol_atom_to_ap.at(id2).at(std::stoi(ep_2.getStringProp("atomId")));
        else if (ep_2.hasStringProp("attachmentPointId"))
            helm_string += ep_2.getStringProp("attachmentPointId");
        else if (connection.connType() == KetConnection::TYPE::HYDROGEN)
            helm_string += HelmHydrogenPair;
        else
            helm_string += '?';
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif
