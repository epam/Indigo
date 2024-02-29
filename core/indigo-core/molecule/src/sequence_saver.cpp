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

SequenceSaver::SequenceSaver(Output& output) : _output(output)
{
}

SequenceSaver::~SequenceSaver()
{
}

void SequenceSaver::saveMolecule(BaseMolecule& mol, SeqFormat sf)
{
    for (int idx = 0; idx < mol.countComponents(); ++idx)
    {
        Filter filt(mol.getDecomposition().ptr(), Filter::EQ, idx);
        std::unique_ptr<BaseMolecule> component(mol.neu());
        component->makeSubmolecule(mol, filt, NULL, NULL);

        if (sf == SeqFormat::FASTA)
        {
            std::string fasta_header = ">";
            auto& mol_properties = component->properties();
            for (auto it = mol_properties.begin(); it != mol_properties.end(); ++it)
            {
                auto& props = component->properties().value(it);
                if (props.contains(kFASTA_HEADER))
                    fasta_header = props.at(kFASTA_HEADER);
            }
            fasta_header += "\n";
            _output.write(fasta_header.data(), static_cast<int>(fasta_header.size()));
        }

        std::map<int, std::map<int, int>> layout_sequence;
        SequenceLayout sl(*component);
        sl.calculateLayout(layout_sequence);
        std::string seq_text;
        for (auto& row : layout_sequence)
        {
            std::string seq_string;
            for (auto& col : row.second)
            {
                int atom_idx = col.second;
                if (component->isTemplateAtom(atom_idx))
                {
                    std::string mon_class = component->getTemplateAtomClass(atom_idx);
                    if (isBackboneClass(mon_class))
                    {
                        std::string label;
                        if (mon_class == kMonomerClassSUGAR)
                        {
                            auto& v = component->getVertex(atom_idx);
                            for (auto nei_idx = v.neiBegin(); nei_idx < v.neiEnd(); nei_idx = v.neiNext(nei_idx))
                            {
                                int nei_atom_idx = v.neiVertex(nei_idx);
                                if (component->isTemplateAtom(nei_atom_idx) && std::string(component->getTemplateAtomClass(nei_atom_idx)) == kMonomerClassBASE)
                                {
                                    label = monomerAliasByName(kMonomerClassBASE, component->getTemplateAtom(nei_atom_idx));
                                    break;
                                }
                            }
                        }
                        else if (isAminoAcidClass(mon_class))
                            label = monomerAliasByName(kMonomerClassAA, component->getTemplateAtom(atom_idx));
                        else if (isNucleotideClass(mon_class))
                            label = monomerAliasByName(kMonomerClassBASE, component->getTemplateAtom(atom_idx));
                        if (label.size())
                        { // unknown symbol
                            switch (sf)
                            {
                            case SeqFormat::FASTA:
                                if (label.size() > 1)
                                    throw Error("Can't save '%s' to FASTA", label.c_str());
                                seq_string += label;
                                break;
                            default:
                                seq_string += label.size() > 1 ? "*" : label;
                            }
                        }
                    }
                }
            }
            if (seq_string.size())
            {
                if (seq_text.size())
                    seq_text += "\n";
                seq_text += seq_string;
            }
        }
        if (seq_text.size())
            _output.write(seq_text.data(), static_cast<int>(seq_text.size()));
    }
}
