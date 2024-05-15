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

using namespace indigo;

IMPL_ERROR(SequenceSaver, "Sequence saver");

CP_DEF(SequenceSaver);

SequenceSaver::SequenceSaver(Output& output) : _output(output), _mon_lib(MonomerTemplates::_instance())
{
}

SequenceSaver::~SequenceSaver()
{
}

void SequenceSaver::saveMolecule(BaseMolecule& mol, SeqFormat sf)
{
    if (!mol.isQueryMolecule())
        mol.getTemplatesMap(_templates);

    std::string seq_text;
    auto& mol_properties = mol.properties();
    std::vector<std::deque<int>> sequences;
    SequenceLayout sl(mol);
    sl.sequenceExtract(sequences);
    auto prop_it = mol_properties.begin();
    int seq_idx = 0;
    static const std::unordered_set<std::string> IDT_STANDARD_BASES = {"A", "T", "C", "G", "U", "I"};
    for (auto& sequence : sequences)
    {
        std::string seq_string;
        if (sf == SeqFormat::IDT)
        {
            std::unordered_set<int> used_atoms;
            while (sequence.size() > 0)
            {
                int atom_idx = sequence.front();
                used_atoms.emplace(atom_idx);
                sequence.pop_front();
                if (!mol.isTemplateAtom(atom_idx))
                    throw Error("Canot save regular atom %s in IDT format.", mol.getAtomDescription(atom_idx).c_str());
                std::string monomer_class = mol.getTemplateAtomClass(atom_idx);
                std::string monomer = mol.getTemplateAtom(atom_idx);
                if (monomer_class != kMonomerClassSUGAR)
                {
                    if (used_atoms.size() > 1)
                        throw Error("Canot save molecule in IDT format - expected shugar but found %s.", monomer.c_str());
                    if (monomer_class != kMonomerClassPHOSPHATE || monomer != "P") // first monomer can be phosphate "P"
                        throw Error("Canot save molecule in IDT format - monomer %s cannot be first.", monomer.c_str());
                    seq_string += "/5Phos/";
                    continue;
                }
                if (monomer == "R")
                    seq_string += "r";
                else if (monomer == "LR")
                    seq_string += "+";
                else if (monomer == "mR")
                    seq_string += "m";
                else if (monomer != "dR")
                    throw Error("Canot save molecule in IDT format - unknown sugar %s.", monomer.c_str());
                auto& v = mol.getVertex(atom_idx);
                std::string base;
                std::string phosphate;
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
                                throw Error("Canot save molecule in IDT format - sugar %s with two base connected %s and %s.", monomer.c_str(), base.c_str(),
                                            mol.getTemplateAtom(nei_atom_idx));
                            base = mol.getTemplateAtom(nei_atom_idx);
                            if (IDT_STANDARD_BASES.count(base) == 0)
                                throw Error("Canot save molecule in IDT format - unknown base %s.", base.c_str());
                            seq_string += base;
                        }
                        else if (monomer_class == kMonomerClassPHOSPHATE)
                        {
                            if (phosphate.size() > 0) // left phosphate should be in used_atoms and skiped
                                throw Error("Canot save molecule in IDT format - sugar %s with too much phosphates connected %s and %s.", monomer.c_str(),
                                            phosphate.c_str(), mol.getTemplateAtom(nei_atom_idx));
                            phosphate = mol.getTemplateAtom(nei_atom_idx);
                        }
                        else
                        {
                            throw Error("Canot save molecule in IDT format - sugar %s connected to unknown monomer %s with class %s.", monomer.c_str(),
                                        mol.getTemplateAtom(nei_atom_idx), monomer_class.c_str());
                        }
                    }
                    else
                    {
                        throw Error("Canot save regular atom %s in IDT format.", mol.getAtomDescription(atom_idx).c_str());
                    }
                }
                if (sequence.size() > 0)
                { // process phosphate
                    atom_idx = sequence.front();
                    sequence.pop_front();
                    if (!mol.isTemplateAtom(atom_idx))
                        throw Error("Canot save regular atom %s in IDT format.", mol.getAtomDescription(atom_idx).c_str());
                    monomer_class = mol.getTemplateAtomClass(atom_idx);
                    monomer = mol.getTemplateAtom(atom_idx);
                    if (monomer_class != kMonomerClassPHOSPHATE)
                        throw Error("Canot save molecule in IDT format - phosphate expected between sugars but %s monomer %s found.", monomer_class.c_str(),
                                    monomer.c_str());
                    if (used_atoms.count(atom_idx) == 0) // phosphate should be already processed at sugar neighbours check
                        throw Error("Canot save molecule in IDT format - phosphate %s not connected to privious sugar.", phosphate.c_str());
                    if (sequence.size() == 0)
                        if (phosphate == "P")
                            seq_string += "/3Phos/";
                        else
                            throw Error("Canot save molecule in IDT format - phosphate %s cannot be last monomer in sequence.", phosphate.c_str());
                    else if (phosphate == "sP")
                        seq_string += "*";
                    else if (phosphate != "P")
                        throw Error("Canot save molecule in IDT format - unknown phosphate %s.", phosphate.c_str());
                }
            }
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
                                    label = "N";
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

            seq_text += seq_string.substr(0, SEQ_LINE_LENGTH);
            for (size_t format_ind = SEQ_LINE_LENGTH; format_ind < seq_string.size(); format_ind += SEQ_LINE_LENGTH)
            {
                seq_text += "\n";
                seq_text += seq_string.substr(format_ind, SEQ_LINE_LENGTH);
            }
            seq_idx++;
        }
    }
    if (seq_text.size())
        _output.write(seq_text.data(), static_cast<int>(seq_text.size()));
}
