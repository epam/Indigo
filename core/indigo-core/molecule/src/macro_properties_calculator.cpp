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

#include "molecule/macro_properties_calculator.h"
#include "molecule/crippen.h"
#include "molecule/json_writer.h"
#include "molecule/molecule.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/molecule_mass.h"
#include "molecule/monomer_commons.h"

using namespace indigo;

IMPL_ERROR(MacroPropertiesCalculator, "Macro Properties Calculator")

void MacroPropertiesCalculator::CalculateMacroProps(KetDocument& document, Output& output, float upc, float nac, bool pretty_json) const
{
    const auto& monomers = document.monomers();
    auto is_base = [&](MonomerClass monomer_class) -> bool {
        return monomer_class == MonomerClass::Base || monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA;
    };
    auto move_to_next_base = [&](auto& it, const auto& end) {
        while (it != end)
        {
            it++;
            if (it != end && is_base(document.getMonomerClass(*it)))
                return;
        }
    };
    struct chain
    {
        std::deque<std::string> sequence, secondary_sequence;
        chain(std::deque<std::string>& seq) : sequence(seq), secondary_sequence(){};
        chain(std::deque<std::string>& seq, std::deque<std::string>& secondary_seq) : sequence(seq), secondary_sequence(secondary_seq){};
    };
    std::vector<std::deque<std::string>> sequences;
    std::vector<chain> joined_sequences;
    std::map<std::string, std::pair<std::string, std::string>> monomer_to_molecule;
    document.parseSimplePolymers(sequences, false);
    for (auto connection : document.nonSequenceConnections())
    {
        auto& ep1 = connection.ep1();
        auto& ep2 = connection.ep2();
        if (ep1.hasStringProp("monomerId") && ep2.hasStringProp("monomerId"))
        {
            const auto& monomers = document.monomers();
            auto& left_monomer = monomers.at(document.monomerIdByRef(ep1.getStringProp("monomerId")));
            auto& right_monomer = monomers.at(document.monomerIdByRef(ep2.getStringProp("monomerId")));
            if (connection.connectionType() == KetConnectionHydro)
            {
                left_monomer->addHydrogenConnection(right_monomer->ref());
                right_monomer->addHydrogenConnection(left_monomer->ref());
            }
            else if (connection.connectionType() == KetConnectionSingle)
            {
                auto& ap_left = ep1.getStringProp("attachmentPointId");
                auto& ap_right = ep2.getStringProp("attachmentPointId");
                left_monomer->connectAttachmentPointTo(ap_left, right_monomer->ref(), ap_right);
                right_monomer->connectAttachmentPointTo(ap_right, left_monomer->ref(), ap_left);
            }
        }
        else if ((ep1.hasStringProp("monomerId") && ep2.hasStringProp("moleculeId")) || (ep1.hasStringProp("moleculeId") && ep2.hasStringProp("monomerId")))
        {
            auto monomer_id = ep1.hasStringProp("monomerId") ? document.monomerIdByRef(ep1.getStringProp("monomerId"))
                                                             : document.monomerIdByRef(ep2.getStringProp("monomerId"));
            auto molecule_id = ep1.hasStringProp("moleculeId") ? ep1.getStringProp("moleculeId") : ep2.getStringProp("moleculeId");
            auto monomer_ap = ep1.hasStringProp("monomerId") ? ep1.getStringProp("attachmentPointId") : ep2.getStringProp("attachmentPointId");
            if (monomer_ap == kAttachmentPointR1 || monomer_ap == kAttachmentPointR2)
                monomer_to_molecule.emplace(monomer_id, std::make_pair(molecule_id, monomer_ap));
        }
    }
    std::map<std::string, size_t> five_prime_monomers, three_prime_monomers;
    for (size_t i = 0; i < sequences.size(); i++)
    {
        five_prime_monomers.emplace(sequences[i].front(), i);
        three_prime_monomers.emplace(sequences[i].back(), i);
    }
    std::set<size_t> used_sequences;
    std::set<size_t> possible_double_chains;
    std::map<size_t, std::set<size_t>> bases_count_to_variants;
    while (five_prime_monomers.size() > 0)
    {
        auto idx = five_prime_monomers.begin()->second;
        auto cur_monomer_id = five_prime_monomers.begin()->first;
        five_prime_monomers.erase(five_prime_monomers.begin());
        used_sequences.emplace(idx);
        // If no chains connected to 5' and 3' ends of DNA/RNA chain - add it to possible double chain
        if (monomers.at(sequences[idx].front())->connections().count(kAttachmentPointR1) == 0 &&
            monomers.at(sequences[idx].back())->connections().count(kAttachmentPointR2) == 0)
        {
            auto& first_monomer = monomers.at(sequences[idx].front());
            auto first_monomer_class = document.getMonomerClass(*first_monomer);
            switch (first_monomer_class)
            {
            case MonomerClass::DNA:
            case MonomerClass::RNA:
            case MonomerClass::Sugar:
            case MonomerClass::Phosphate:
            case MonomerClass::Base: {
                bool found_no_hydrogen = false;
                size_t bases_count = 0;
                // check that all bases have hydrogen connections
                for (auto& monomer_id : sequences[idx])
                {
                    auto& monomer = monomers.at(monomer_id);
                    if (is_base(document.getMonomerClass(*monomer)))
                    {
                        if (monomer->hydrogenConnections().size() == 0) // no hydrogen connections
                        {
                            found_no_hydrogen = true;
                            break;
                        }
                        bases_count++;
                    }
                }
                if (found_no_hydrogen == false)
                {
                    possible_double_chains.emplace(idx);
                    if (bases_count_to_variants.count(bases_count) == 0)
                        bases_count_to_variants.emplace(bases_count, std::set<size_t>());
                    bases_count_to_variants.at(bases_count).emplace(idx);
                    continue;
                }
            }
            default:
                break;
            }
        }
        auto& sequence = joined_sequences.emplace_back(sequences[idx]).sequence;
        while (true)
        {
            auto& front_connections = monomers.at(sequence.front())->connections();
            if (front_connections.count(kAttachmentPointR1) > 0)
            {
                auto& connection = front_connections.at(kAttachmentPointR1);
                if (connection.second == kAttachmentPointR2)
                {
                    auto& monomer_id = document.monomerIdByRef(connection.first);
                    auto it = three_prime_monomers.find(monomer_id);
                    if (it == three_prime_monomers.end())
                        throw Error("Internal error. Connection to monomer %s not found", monomer_id.c_str());
                    auto letf_idx = it->second;
                    if (used_sequences.count(letf_idx) == 0)
                    {
                        sequence.insert(sequence.begin(), sequences[letf_idx].begin(), sequences[letf_idx].end());
                        five_prime_monomers.erase(sequences[letf_idx].front()); // remove from 5' monomers
                        used_sequences.emplace(letf_idx);
                        continue;
                    }
                }
            }
            auto& back_connections = monomers.at(sequence.back())->connections();
            if (back_connections.count(kAttachmentPointR2) > 0)
            {
                auto& connection = back_connections.at(kAttachmentPointR2);
                if (connection.second == kAttachmentPointR1)
                {
                    auto& monomer_id = document.monomerIdByRef(connection.first);
                    if (monomer_id != cur_monomer_id) // avoid cycle
                    {
                        auto it = five_prime_monomers.find(monomer_id);
                        if (it == five_prime_monomers.end())
                            throw Error("Internal error. Connection to monomer %s not found", monomer_id.c_str());
                        auto right_idx = it->second;
                        if (used_sequences.count(right_idx) == 0)
                        {
                            sequence.insert(sequence.end(), sequences[right_idx].begin(), sequences[right_idx].end());
                            five_prime_monomers.erase(sequences[right_idx].front()); // remove from 5' monomers
                            used_sequences.emplace(right_idx);
                            continue;
                        }
                    }
                }
            }
            break;
        }
    }
    std::set<size_t> verified_options;
    for (auto idx : possible_double_chains)
    {
        if (verified_options.count(idx) > 0)
            continue;
        auto& sequence = sequences[idx];
        verified_options.emplace(idx);
        size_t bases_count = 0;
        for (auto it = sequence.begin(); it != sequence.end(); it++)
        {
            if (is_base(document.getMonomerClass(*it)))
                bases_count++;
        }
        bases_count_to_variants.at(bases_count).erase(idx);
        bool pair_found = false;
        for (auto& variant_idx : bases_count_to_variants.at(bases_count))
        {
            auto& variant = sequences[variant_idx];
            auto variant_it = variant.rbegin();
            auto sequence_it = sequence.begin();
            if (!is_base(document.getMonomerClass(*variant_it)))
                move_to_next_base(variant_it, variant.rend());
            if (!is_base(document.getMonomerClass(*sequence_it)))
                move_to_next_base(sequence_it, sequence.end());
            while (sequence_it != sequence.end() && variant_it != variant.rend())
            {
                auto& base = monomers.at(*sequence_it);
                auto& variant_base = monomers.at(*variant_it);
                if (base->hydrogenConnections().count(variant_base->ref()) == 0)
                    break;
                move_to_next_base(variant_it, variant.rend());
                move_to_next_base(sequence_it, sequence.end());
            }
            if (sequence_it == sequence.end() && variant_it == variant.rend())
            {
                pair_found = true;
                verified_options.emplace(variant_idx);
                bases_count_to_variants.at(bases_count).erase(variant_idx);
                joined_sequences.emplace_back(sequence, variant);
                break;
            }
        }
        if (!pair_found)
        {
            joined_sequences.emplace_back(sequence);
        }
    }
    const auto& templates = document.templates();
    // Sequences generated. Calculate macro properties
    rapidjson::StringBuffer s;
    JsonWriter writer(pretty_json);
    writer.Reset(s);
    writer.StartArray();
    for (auto& sequence_arr : joined_sequences)
    {
        writer.StartObject();
        std::deque<std::string> sequence{sequence_arr.sequence};
        sequence.insert(sequence.begin(), sequence_arr.secondary_sequence.begin(), sequence_arr.secondary_sequence.end());
        std::vector<double> pKa_values;
        // in kDa(1000g/mol) (all chains)
        double mass_sum = 0;
        bool calculate_mass = true;
        std::map<char, size_t> atoms_count;
        GROSS_UNITS gross_units;
        gross_units.resize(1);
        auto merge_gross_data = [&gross_units](const GROSS_UNITS& gross) {
            for (int i = 0; i < gross.size(); i++)
            {
                for (auto it : gross.at(i).isotopes)
                {
                    if (gross_units[0].isotopes.count(it.first) == 0)
                        gross_units[0].isotopes[it.first] = it.second;
                    else
                        gross_units[0].isotopes[it.first] += it.second;
                }
            }
        };
        for (auto& monomer_id : sequence)
        {
            auto& monomer = monomers.at(monomer_id);
            if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
            {
                calculate_mass = false;
                atoms_count.clear();
                break;
            }
            auto& monomer_template = templates.at(monomer->templateId());
            if (monomer_template.unresolved())
            {
                calculate_mass = false;
                atoms_count.clear();
                break;
            }
            std::vector<int> leaved_atoms;
            auto& connections = monomer->connections();
            auto& att_points = monomer->attachmentPoints();
            for (auto& conn : connections)
            {
                auto it = att_points.find(conn.first);
                if (it == att_points.end())
                    throw Error("Internal error. Attachment point %s not found in monomer %s", conn.first.c_str(), monomer_id.c_str());
                auto& leaving_group = it->second.leavingGroup();
                if (!leaving_group.has_value())
                    continue;
                auto& leaved = leaving_group.value();
                leaved_atoms.insert(leaved_atoms.end(), leaved.begin(), leaved.end());
            }
            std::sort(leaved_atoms.rbegin(), leaved_atoms.rend());
            auto tgroup = monomer_template.getTGroup();
            auto* pmol = static_cast<Molecule*>(tgroup->fragment.get());
            Array<int> atom_filt;
            atom_filt.expandFill(pmol->vertexCount(), 1);
            for (auto& idx : leaved_atoms)
                atom_filt[idx] = 0;
            Filter atom_filter(atom_filt.ptr(), Filter::EQ, 1);
            pmol->selectAtoms(atom_filter);
            MoleculeMass mass;
            mass.mass_options.skip_error_on_pseudoatoms = true;
            mass_sum += mass.molecularWeight(*pmol);
            if (document.getMonomerClass(*monomer) == MonomerClass::AminoAcid)
            {
                pKa_values.emplace_back(Crippen::pKa(*pmol));
            }
            auto gross = MoleculeGrossFormula::collect(*pmol, true);
            merge_gross_data(*gross);
        }
        if (calculate_mass)
        {
            std::vector<std::string> sequence_ends;
            sequence_ends.emplace_back(sequence.front());
            if (sequence.back() != sequence.front())
                sequence_ends.emplace_back(sequence.back());
            const auto& molecules = document.jsonMolecules();
            for (auto& monomer_id : sequence_ends)
            {
                if (monomer_to_molecule.count(monomer_id) > 0)
                {
                    auto& molecule_id = monomer_to_molecule.at(monomer_id).first;
                    auto& mol_json = molecules[document.moleculeIdxByRef(molecule_id)];
                    rapidjson::Value marr(rapidjson::kArrayType);
                    marr.PushBack(document.jsonDocument().CopyFrom(mol_json, document.jsonDocument().GetAllocator()), document.jsonDocument().GetAllocator());
                    MoleculeJsonLoader loader(marr);
                    BaseMolecule* pbmol;
                    Molecule mol;
                    // QueryMolecule qmol;
                    try
                    {
                        loader.loadMolecule(mol);
                        pbmol = &mol;
                        MoleculeMass mass;
                        mass.mass_options.skip_error_on_pseudoatoms = true;
                        mass_sum += mass.molecularWeight(mol);
                        auto gross = MoleculeGrossFormula::collect(*pbmol, true);
                        merge_gross_data(*gross);
                    }
                    catch (...)
                    {
                        // query molecule just skipped
                    }
                }
            }
            Array<char> gross_str;
            MoleculeGrossFormula::toString(gross_units, gross_str);
            writer.Key("grossFormula");
            writer.String(gross_str.ptr());
            writer.Key("mass");
            writer.Double(mass_sum);
        }

        // pKa (only peptides)
        auto pka_count = pKa_values.size();
        if (pka_count > 0)
        {
            double pKa;
            if (pka_count > 1)
            {
                std::sort(pKa_values.begin(), pKa_values.end());
                if (pka_count & 1) // odd
                {
                    pKa = pKa_values[pka_count / 2];
                }
                else // even - get average
                {
                    pKa = (pKa_values[pka_count / 2 - 1] + pKa_values[pka_count / 2]) / 2;
                }
            }
            else // only one value
            {
                pKa = pKa_values[0];
            }
            writer.Key("pKa");
            writer.Double(pKa);
        }

        // Melting temperature (only double stranded DNA)
        if (sequence_arr.secondary_sequence.size() > 0)
        {
            std::deque<std::string> bases;
            auto it = sequence_arr.sequence.begin();
            if (!is_base(document.getMonomerClass(*it)))
                move_to_next_base(it, sequence_arr.sequence.end());
            while (it != sequence_arr.sequence.end())
            {
                auto& monomer = monomers.at(*it);
                if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                    continue;
                auto& monomer_template = templates.at(monomer->templateId());
                if (monomer_template.hasStringProp("naturalAnalogShort"))
                {
                    bases.emplace_back(monomer_template.getStringProp("naturalAnalogShort"));
                }
                move_to_next_base(it, sequence_arr.sequence.end());
            }
            if (bases.size() > 0)
            {
                std::string left = bases.front();
                bases.pop_front();
                if (left == "U")
                    left = "T";
                size_t base_count = 1;
                size_t total_strength = 0;
                static const std::map<std::pair<std::string, std::string>, size_t> STRENGTH_PARAMS{
                    {{"C", "G"}, 13}, {{"C", "C"}, 11}, {{"G", "G"}, 11}, {{"C", "G"}, 10}, {{"A", "C"}, 10}, {{"T", "C"}, 8},
                    {{"A", "G"}, 8},  {{"T", "G"}, 7},  {{"G", "T"}, 10}, {{"C", "T"}, 8},  {{"G", "A"}, 8},  {{"C", "A"}, 7},
                    {{"A", "T"}, 7},  {{"T", "T"}, 5},  {{"A", "A"}, 5},  {{"T", "A"}, 4}};
                while (bases.size() > 0)
                {
                    auto right = bases.front();
                    bases.pop_front();
                    if (right == "U")
                        right = "T";
                    auto str_it = STRENGTH_PARAMS.find({left, right});
                    if (str_it == STRENGTH_PARAMS.end())
                        throw Error("Internal error. No strength params for %s and %s", left.c_str(), right.c_str());
                    total_strength += str_it->second;
                    base_count++;
                    left = right;
                }
                double sp = static_cast<double>(total_strength) / base_count;
                double tm = 7.35 * sp + 17.34 * log(base_count) + 4.96 * log(upc) + 0.89 * log(nac) - 25.42;
                writer.Key("Tm");
                writer.Double(tm);
            }
        }

        // Extinction coefficient (only peptides)
        {
            static const std::map<std::string, size_t> extinction_coefficients{{"C", 125}, {"W", 5500}, {"Y", 1490}};
            std::map<std::string, size_t> extinction_counts;
            for (auto& it : extinction_coefficients)
                extinction_counts.emplace(it.first, 0);
            size_t peptides_count = 0;
            for (auto monomer_id : sequence)
            {
                auto& monomer = monomers.at(monomer_id);
                if (document.getMonomerClass(*monomer) != MonomerClass::AminoAcid)
                    continue;
                peptides_count++;
                if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                    continue;
                auto& monomer_template = templates.at(monomer->templateId());
                if (monomer_template.hasStringProp("naturalAnalogShort"))
                {
                    auto it = extinction_counts.find(monomer_template.getStringProp("naturalAnalogShort"));
                    if (it != extinction_counts.end())
                    {
                        it->second++;
                    }
                }
            }
            if (peptides_count > 0)
            {
                size_t e_calc = 0;
                for (auto& it : extinction_counts)
                {
                    e_calc += it.second * extinction_coefficients.at(it.first);
                }
                writer.Key("extinctionCoefficient");
                writer.Uint64(e_calc);
            }
        }

        // Hydrophobicity (only peptides)
        std::vector<double> hydrophobicity;
        static const std::map<std::string, double> hydrophobicity_coefficients{
            {"A", 0.616}, {"G", 0.501}, {"M", 0.738}, {"S", 0.359}, {"C", 0.680}, {"H", 0.165}, {"N", 0.236}, {"T", 0.450}, {"D", 0.028}, {"I", 0.943},
            {"P", 0.711}, {"V", 0.825}, {"E", 0.043}, {"K", 0.283}, {"Q", 0.251}, {"W", 0.878}, {"F", 1.000}, {"L", 0.943}, {"R", 0.000}, {"Y", 0.880}};
        for (auto& monomer_id : sequence)
        {
            if (document.getMonomerClass(monomer_id) != MonomerClass::AminoAcid)
                continue;
            auto& monomer = monomers.at(monomer_id);
            if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                continue;
            auto& monomer_template = templates.at(monomer->templateId());
            if (monomer_template.hasStringProp("naturalAnalogShort"))
            {

                auto it = hydrophobicity_coefficients.find(monomer_template.getStringProp("naturalAnalogShort"));
                if (it != hydrophobicity_coefficients.end())
                    hydrophobicity.emplace_back(it->second);
            }
        }
        if (hydrophobicity.size() > 0)
        {
            writer.Key("hydrophobicity");
            writer.StartArray();
            for (auto value : hydrophobicity)
                writer.Double(value);
            writer.EndArray();
        }

        // Monomer count
        static const std::string peptides = "ACDEFGHIKLMNPQRSTVWY";
        static const std::string nucleotides = "ACGTU";
        std::map<std::string, size_t> peptides_count;
        std::map<std::string, size_t> nucleotides_count;
        for (auto ch : peptides)
            peptides_count.emplace(std::string(1, ch), 0);
        for (auto ch : nucleotides)
            nucleotides_count.emplace(std::string(1, ch), 0);
        static const std::string OTHER = "Other";
        peptides_count.emplace(OTHER, 0);
        nucleotides_count.emplace(OTHER, 0);
        for (auto& monomer_id : sequence)
        {
            auto& monomer = monomers.at(monomer_id);
            auto monomer_class = document.getMonomerClass(*monomer);
            if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
            {
                if (monomer_class == MonomerClass::AminoAcid)
                    peptides_count[OTHER]++;
                else if (monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA || monomer_class == MonomerClass::Sugar ||
                         monomer_class == MonomerClass::Phosphate || monomer_class == MonomerClass::Base)
                    nucleotides_count[OTHER]++;
                continue;
            }
            std::string natural_analog = "";
            auto& monomer_template = templates.at(monomer->templateId());
            if (monomer_template.monomerClass() == MonomerClass::AminoAcid)
            {
                if (monomer_template.hasStringProp("naturalAnalogShort"))
                    natural_analog = monomer_template.getStringProp("naturalAnalogShort");
                auto it = peptides_count.find(natural_analog);
                if (it == peptides_count.end())
                    peptides_count[OTHER]++;
                else
                    it->second++;
            }
            else if (is_base(monomer_template.monomerClass()))
            {
                if (monomer_template.hasStringProp("naturalAnalogShort"))
                    natural_analog = monomer_template.getStringProp("naturalAnalogShort");
                auto it = nucleotides_count.find(natural_analog);
                if (it == nucleotides_count.end())
                    nucleotides_count[OTHER]++;
                else
                    it->second++;
            }
        }
        writer.Key("monomerCount");
        writer.StartObject();
        writer.Key("peptides");
        writer.StartObject();
        for (const auto& it : peptides_count)
        {
            if (it.second > 0)
            {
                writer.Key(it.first);
                writer.Uint64(it.second);
            }
        }
        writer.EndObject(); // peptides
        writer.Key("nucleotides");
        writer.StartObject();
        for (const auto& it : nucleotides_count)
        {
            if (it.second > 0)
            {
                writer.Key(it.first);
                writer.Uint64(it.second);
            }
        }
        writer.EndObject(); // nucleotides
        writer.EndObject(); // monomerCount
        writer.EndObject();
    }
    writer.EndArray();
    std::stringstream result;
    result << s.GetString();
    output.printf("%s", result.str().c_str());
}
