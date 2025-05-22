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

// UPC is molar concentration of unipositive cations, NAC is molar concentration of the nucleotide strands, these options need to calculate melting temperature
void MacroPropertiesCalculator::CalculateMacroProps(KetDocument& document, Output& output, float upc, float nac, bool pretty_json) const
{
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
    struct polymer_t
    {
        std::set<size_t> sequences;
        std::set<std::string> molecules;
        std::map<std::string, std::map<int, int>> mol_atom_connections;
        bool is_double_chain = false;
        bool has_non_sequence_connection = false;
        bool deleted = false;
        polymer_t(size_t idx) : is_double_chain(false), has_non_sequence_connection(false), deleted(false)
        {
            sequences.emplace(idx);
        };
        polymer_t(const std::string& mol_ref) : is_double_chain(false), has_non_sequence_connection(false), deleted(false)
        {
            molecules.emplace(mol_ref);
            mol_atom_connections[mol_ref] = {};
        };
        void merge(const polymer_t& other)
        {
            sequences.insert(other.sequences.begin(), other.sequences.end());
            molecules.insert(other.molecules.begin(), other.molecules.end());
            mol_atom_connections.insert(other.mol_atom_connections.begin(), other.mol_atom_connections.end());
        };
    };
    std::vector<std::deque<std::string>> sequences;
    document.parseSimplePolymers(sequences, false);
    std::map<std::string, size_t> monomer_to_sequence_idx;
    std::map<size_t, size_t> sequence_to_polymer_idx;
    std::map<std::string, size_t> molecule_to_polymer_idx;
    const auto& monomers = document.monomers();
    const auto& molecules_refs = document.moleculesRefs();
    const auto& templates = document.templates();
    std::deque<polymer_t> polymers;
    for (size_t i = 0; i < sequences.size(); i++)
    {
        sequence_to_polymer_idx.emplace(i, polymers.size());
        polymers.emplace_back(i);
        for (auto& monomer : sequences[i])
        {
            monomer_to_sequence_idx.emplace(monomer, i);
        }
    }
    for (size_t i = 0; i < molecules_refs.size(); i++)
    {
        auto idx = polymers.size();
        molecule_to_polymer_idx.emplace(molecules_refs[i], idx);
        polymers.emplace_back(molecules_refs[i]);
    }
    auto add_connection_to_atom = [&](size_t polymer_idx, const std::string& molecule_id, int atom_idx) {
        auto& atom_connections = polymers[polymer_idx].mol_atom_connections[molecule_id];
        if (atom_connections.count(atom_idx) == 0)
            atom_connections.emplace(atom_idx, 1);
        else
            atom_connections[atom_idx] += 1;
    };
    for (auto connection : document.nonSequenceConnections())
    {
        auto& ep1 = connection.ep1();
        auto& ep2 = connection.ep2();
        if (ep1.hasStringProp("monomerId") && ep2.hasStringProp("monomerId"))
        {
            auto& left_monomer_id = document.monomerIdByRef(ep1.getStringProp("monomerId"));
            auto& right_monomer_id = document.monomerIdByRef(ep2.getStringProp("monomerId"));
            auto& left_monomer = monomers.at(left_monomer_id);
            auto& right_monomer = monomers.at(right_monomer_id);
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
            auto& left_sequence_idx = monomer_to_sequence_idx[left_monomer_id];
            size_t right_sequence_idx = monomer_to_sequence_idx[right_monomer_id];
            size_t left_polymer_idx = sequence_to_polymer_idx[left_sequence_idx];
            size_t right_polymer_idx = sequence_to_polymer_idx[right_sequence_idx];
            if (left_sequence_idx != right_sequence_idx && left_polymer_idx != right_polymer_idx)
            {
                polymers[left_polymer_idx].merge(polymers[right_polymer_idx]);
                sequence_to_polymer_idx[right_sequence_idx] = left_polymer_idx;
                polymers[right_polymer_idx].deleted = true;
            }
            if (connection.connectionType() != KetConnectionHydro)
                polymers[left_polymer_idx].has_non_sequence_connection = true;
        }
        else if ((ep1.hasStringProp("monomerId") && ep2.hasStringProp("moleculeId")) || (ep1.hasStringProp("moleculeId") && ep2.hasStringProp("monomerId")))
        {
            auto monomer_id = ep1.hasStringProp("monomerId") ? document.monomerIdByRef(ep1.getStringProp("monomerId"))
                                                             : document.monomerIdByRef(ep2.getStringProp("monomerId"));
            auto molecule_id = ep1.hasStringProp("moleculeId") ? ep1.getStringProp("moleculeId") : ep2.getStringProp("moleculeId");
            int atom_idx = std::stoi(ep1.hasStringProp("moleculeId") ? ep1.getStringProp("atomId") : ep2.getStringProp("atomId"));
            auto monomer_ap = ep1.hasStringProp("monomerId") ? ep1.getStringProp("attachmentPointId") : ep2.getStringProp("attachmentPointId");
            monomers.at(monomer_id)->connectAttachmentPointToMolecule(monomer_ap, molecule_id, atom_idx);
            size_t sequence_polymer_idx = sequence_to_polymer_idx[monomer_to_sequence_idx[monomer_id]];

            size_t molecule_polymer_idx = molecule_to_polymer_idx[molecule_id];
            if (sequence_polymer_idx != molecule_polymer_idx)
            {
                polymers[sequence_polymer_idx].merge(polymers[molecule_polymer_idx]);
                molecule_to_polymer_idx[molecule_id] = sequence_polymer_idx;
                polymers[molecule_polymer_idx].deleted = true;
            }
            add_connection_to_atom(sequence_polymer_idx, molecule_id, atom_idx);
        }
        else if (ep1.hasStringProp("moleculeId") && ep2.hasStringProp("moleculeId"))
        {
            auto first_molecule_id = ep1.getStringProp("moleculeId");
            auto second_molecule_id = ep2.getStringProp("moleculeId");
            int first_atom_idx = std::stoi(ep1.getStringProp("atomId"));
            int second_atom_idx = std::stoi(ep2.getStringProp("atomId"));
            size_t first_molecule_polymer_idx = molecule_to_polymer_idx[first_molecule_id];
            size_t second_molecule_polymer_idx = molecule_to_polymer_idx[second_molecule_id];
            if (first_molecule_polymer_idx != second_molecule_polymer_idx)
            {
                polymers[first_molecule_polymer_idx].merge(polymers[second_molecule_polymer_idx]);
                molecule_to_polymer_idx[second_molecule_id] = first_molecule_polymer_idx;
                polymers[second_molecule_polymer_idx].deleted = true;
            }
            add_connection_to_atom(first_molecule_polymer_idx, first_molecule_id, first_atom_idx);
            add_connection_to_atom(first_molecule_polymer_idx, second_molecule_id, second_atom_idx);
        }
    }
    // search for DNA/RNA double chains
    for (auto& polymer : polymers)
    {
        if (polymer.deleted)
            continue;
        if (polymer.molecules.size() > 0)
            continue;
        if (polymer.sequences.size() != 2)
            continue;
        if (polymer.has_non_sequence_connection)
            continue;

        struct nucleo_t
        {
            std::string id;
            bool nucleotide = true;
            bool rna = true;
            nucleo_t(const std::string& monomer_id) : id(monomer_id), nucleotide(true), rna(true)
            {
            }
        };
        auto collect_nucleos = [&](std::deque<std::string> sequence, std::vector<nucleo_t>& nucleos) -> bool {
            // forward order - Sugar, Base then Phosphate(opt)
            auto it = sequence.begin();
            while (it != sequence.end())
            {
                if (document.getMonomerById(*it)->monomerType() != KetBaseMonomer::MonomerType::Monomer)
                    return false;

                auto monomer_class = document.getMonomerClass(*it);
                switch (monomer_class)
                {
                case MonomerClass::RNA:
                case MonomerClass::DNA:
                    nucleos.emplace_back(*it);
                    if (monomer_class != MonomerClass::RNA)
                        nucleos.rbegin()->rna = false;
                    it++;
                    break;
                case MonomerClass::Sugar:
                    if (document.getMonomerById(*it)->hydrogenConnections().size() > 0)
                        return false;
                    it++;
                    if (it == sequence.end() || document.getMonomerClass(*it) != MonomerClass::Base)
                        return false;
                    nucleos.emplace_back(*it);
                    it++;
                    if (it != sequence.end() && document.getMonomerClass(*it) == MonomerClass::Phosphate)
                    {
                        if (document.getMonomerById(*it)->hydrogenConnections().size() > 0)
                            return false;
                        it++;
                    }
                    break;
                default:
                    return false;
                };
                monomer_class == MonomerClass::Base || monomer_class == MonomerClass::DNA || monomer_class == MonomerClass::RNA;
            }
            return true;
        };
        auto collect_nucleos_reverse = [&](std::deque<std::string> sequence, std::vector<nucleo_t>& nucleos) -> bool {
            // reverse order - Phosphate(opt), Base then Sugar
            auto it = sequence.rbegin();
            while (it != sequence.rend())
            {
                if (document.getMonomerById(*it)->monomerType() != KetBaseMonomer::MonomerType::Monomer)
                    return false;
                auto monomer_class = document.getMonomerClass(*it);
                if (monomer_class == MonomerClass::RNA || monomer_class == MonomerClass::DNA)
                {
                    nucleos.emplace_back(*it);
                    if (monomer_class != MonomerClass::RNA)
                        nucleos.rbegin()->rna = false;
                    it++;
                }
                else if (monomer_class == MonomerClass::Phosphate || monomer_class == MonomerClass::Base)
                {
                    if (monomer_class == MonomerClass::Phosphate)
                    {
                        if (document.getMonomerById(*it)->hydrogenConnections().size() > 0)
                            return false;
                        it++;
                        if (it == sequence.rend())
                            return false;
                        monomer_class = document.getMonomerClass(*it);
                        if (monomer_class != MonomerClass::Base)
                            return false;
                    }
                    nucleos.emplace_back(*it);
                    it++;
                    if (it == sequence.rend() && document.getMonomerClass(*it) != MonomerClass::Sugar)
                        return false;
                    if (document.getMonomerById(*it)->hydrogenConnections().size() > 0)
                        return false;
                    it++;
                }
                else
                {
                    return false;
                };
            }
            return true;
        };
        std::vector<nucleo_t> first_nucleos;
        auto& first_seq = sequences[*polymer.sequences.begin()];
        if (!collect_nucleos(first_seq, first_nucleos))
            continue;
        std::vector<nucleo_t> second_nucleos;
        auto& second_seq = sequences[*polymer.sequences.rbegin()];
        if (!collect_nucleos_reverse(second_seq, second_nucleos))
            continue;

        auto first_nucleos_it = first_nucleos.begin();
        auto second_nucleos_it = second_nucleos.begin();
        const std::set<std::string> possible_bases{"A", "C", "T", "G", "U"};
        while (first_nucleos_it != first_nucleos.end() && second_nucleos_it != second_nucleos.end())
        {
            auto& first_monomer = monomers.at(first_nucleos_it->id);
            auto& second_monomer = monomers.at(second_nucleos_it->id);
            // check than bases have only one connection (to sugar - it is backbone sequence)
            if (first_monomer->connections().size() != 1)
                break;
            if (second_monomer->connections().size() != 1)
                break;
            // Bases should be only connected by hydrogen connections to each other
            if (first_monomer->hydrogenConnections().size() != 1)
                break;
            if (first_monomer->hydrogenConnections().count(second_monomer->ref()) == 0)
                break;
            if (second_monomer->hydrogenConnections().size() != 1)
                break;
            if (second_monomer->hydrogenConnections().count(first_monomer->ref()) == 0)
                break;
            if (possible_bases.count(templates.at(first_monomer->templateId()).getStringProp("naturalAnalogShort")) == 0)
                break;
            if (possible_bases.count(templates.at(second_monomer->templateId()).getStringProp("naturalAnalogShort")) == 0)
                break;
            first_nucleos_it++;
            second_nucleos_it++;
        }
        if (first_nucleos_it == first_nucleos.end() && second_nucleos_it == second_nucleos.end())
        {
            polymer.is_double_chain = true;
        }
    }
    // Sequences generated. Calculate macro properties
    rapidjson::StringBuffer s;
    JsonWriter writer(pretty_json);
    writer.Reset(s);
    writer.SetMaxDecimalPlaces(6);
    writer.StartArray();
    for (auto& polymer : polymers)
    {
        if (polymer.deleted)
            continue;
        writer.StartObject();
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
        for (size_t sequence_idx : polymer.sequences)
        {
            if (!calculate_mass)
                break;
            for (auto& monomer_id : sequences[sequence_idx])
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
                auto& att_points = monomer->attachmentPoints();
                std::vector<std::string> used_attachment_points;
                for (auto& conn : monomer->connections())
                {
                    used_attachment_points.emplace_back(conn.first);
                }
                for (auto& conn : monomer->connectionsToMolecules())
                {
                    used_attachment_points.emplace_back(conn.first);
                }
                for (auto& att_point_id : used_attachment_points)
                {
                    auto it = att_points.find(att_point_id);
                    if (it == att_points.end())
                        throw Error("Internal error. Attachment point %s not found in monomer %s", att_point_id.c_str(), monomer_id.c_str());
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
        }
        if (calculate_mass)
        {
            const auto& molecules = document.jsonMolecules();
            for (auto& molecule_id : polymer.molecules)
            {
                auto& mol_json = molecules[document.moleculeIdxByRef(molecule_id)];
                rapidjson::Value marr(rapidjson::kArrayType);
                marr.PushBack(document.jsonDocument().CopyFrom(mol_json, document.jsonDocument().GetAllocator()), document.jsonDocument().GetAllocator());
                MoleculeJsonLoader loader(marr);
                BaseMolecule* pbmol;
                Molecule mol;
                // QueryMolecule qmol;
                loader.loadMolecule(mol);
                pbmol = &mol;
                // select all atoms and
                if (mol.countSelectedAtoms() == 0) //
                {
                    for (int i = 0; i < mol.vertexCount(); i++)
                    {
                        mol.selectAtom(i);
                    }
                }
                std::set<int> used_leaving_atoms;
                for (auto atom_connections : polymer.mol_atom_connections[molecule_id])
                {
                    for (int j = 0; j < atom_connections.second; j++)
                    {
                        bool not_found_in_spu_aps = true;
                        // search atom in SUP attachment points and deselect leaving atom
                        auto& sgroups = mol.sgroups;
                        for (int s_idx = sgroups.begin(); s_idx != sgroups.end(); s_idx = sgroups.next(s_idx))
                        {
                            SGroup& sg = sgroups.getSGroup(s_idx);
                            if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
                            {
                                auto& sa = static_cast<Superatom&>(sg);
                                for (int ap_idx = sa.attachment_points.begin(); ap_idx != sa.attachment_points.end();
                                     ap_idx = sa.attachment_points.next(ap_idx))
                                {
                                    auto& atp = sa.attachment_points[ap_idx];
                                    if (atp.aidx != atom_connections.first)
                                        continue;
                                    if (used_leaving_atoms.count(atp.aidx) != 0)
                                        continue;
                                    // found unused AP
                                    used_leaving_atoms.emplace(atp.aidx);
                                    mol.unselectAtom(atp.lvidx);
                                    not_found_in_spu_aps = false;
                                }
                            }
                        }
                        // if not found - add unselected H to each monomer connection to decrease implicit H
                        if (not_found_in_spu_aps)
                            std::ignore = mol.addBond(atom_connections.first, mol.addAtom(ELEM_H), BOND_SINGLE);
                    }
                }
                MoleculeMass mass;
                mass.mass_options.skip_error_on_pseudoatoms = true;
                mass_sum += mass.molecularWeight(mol);
                auto gross = MoleculeGrossFormula::collect(*pbmol, true);
                merge_gross_data(*gross);
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
        if (polymer.is_double_chain)
        {
            std::deque<std::string> bases;
            auto& sequence = sequences[*polymer.sequences.begin()];
            auto it = sequence.begin();
            if (!is_base(document.getMonomerClass(*it)))
                move_to_next_base(it, sequence.end());
            while (it != sequence.end())
            {
                auto& monomer = monomers.at(*it);
                if (monomer->monomerType() == KetBaseMonomer::MonomerType::AmbiguousMonomer)
                    continue;
                auto& monomer_template = templates.at(monomer->templateId());
                if (!monomer_template.hasStringProp("naturalAnalogShort"))
                    throw Error("Monomer template without natural analog short: %s", monomer_template.id().c_str());
                bases.emplace_back(monomer_template.getStringProp("naturalAnalogShort"));
                move_to_next_base(it, sequence.end());
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
            for (size_t sequence_idx : polymer.sequences)
            {
                for (auto& monomer_id : sequences[sequence_idx])
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
        for (size_t sequence_idx : polymer.sequences)
        {
            for (auto& monomer_id : sequences[sequence_idx])
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
        for (size_t sequence_idx : polymer.sequences)
        {
            for (auto& monomer_id : sequences[sequence_idx])
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
