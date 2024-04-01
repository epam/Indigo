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
    for (auto& sequence : sequences)
    {
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
        if (seq_string.size())
        {
            if (seq_text.size() && sf == SeqFormat::Sequence)
                seq_text += " ";
            seq_text += seq_string;
        }
        seq_idx++;
    }
    if (seq_text.size())
        _output.write(seq_text.data(), static_cast<int>(seq_text.size()));
}
