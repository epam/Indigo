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

#include <functional>
#include <rapidjson/document.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/base_cpp/scanner.h"
#include "layout/molecule_layout.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_basic_templates.h"
#include "molecule/monomers_lib.h"
#include "molecule/smiles_loader.h"

namespace indigo
{
    IMPL_ERROR(MonomerTemplates, "monomers library");

    struct MonomerDescriptor
    {
        MonomerType monomer_type;
        std::string template_id;
        std::vector<std::string> aliases;
        std::string natreplace;
    };

    struct NucleotideDescriptor
    {
        NucleotideType nucleo_type;
        std::vector<std::string> aliases;
        std::unordered_map<MonomerType, std::string> components;
    };

    const std::vector<MonomerDescriptor> monomer_descriptors = {
        {MonomerType::Phosphate, "P", {"P", "p"}, "P"},         {MonomerType::Sugar, "Rib", {"R", "Rib", "r"}, "R"},
        {MonomerType::Sugar, "dRib", {"d", "dR", "dRib"}, "R"}, {MonomerType::Base, "Ade", {"A", "Ade"}, "A"},
        {MonomerType::Base, "Cyt", {"C", "Cyt"}, "C"},          {MonomerType::Base, "Gua", {"G", "Gua"}, "G"},
        {MonomerType::Base, "Thy", {"T", "Thy"}, "T"},          {MonomerType::Base, "Ura", {"U", "Ura"}, "U"}};

    const std::vector<NucleotideDescriptor> nucleotide_descriptors = {
        {NucleotideType::RNA,
         {"A", "AMP"},
         {{MonomerType::Base, "A"}, {MonomerType::Sugar, "R"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::RNA,
         {"C", "CMP"},
         {{MonomerType::Base, "C"}, {MonomerType::Sugar, "R"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::RNA,
         {"G", "GMP"},
         {{MonomerType::Base, "G"}, {MonomerType::Sugar, "R"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::RNA,
         {"T", "TMP"},
         {{MonomerType::Base, "T"}, {MonomerType::Sugar, "R"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::RNA,
         {"U", "UMP"},
         {{MonomerType::Base, "U"}, {MonomerType::Sugar, "R"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"A", "dA", "dAMP"},
         {{MonomerType::Base, "A"}, {MonomerType::Sugar, "dR"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"C", "dC", "dCMP"},
         {{MonomerType::Base, "C"}, {MonomerType::Sugar, "dR"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"G", "dG", "dGMP"},
         {{MonomerType::Base, "G"}, {MonomerType::Sugar, "dR"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"T", "dT", "dTMP"},
         {{MonomerType::Base, "T"}, {MonomerType::Sugar, "dR"}, {MonomerType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"U", "dU", "dUMP"},
         {{MonomerType::Base, "U"}, {MonomerType::Sugar, "dR"}, {MonomerType::Phosphate, "P"}}}};

    const std::unordered_map<MonomerType, std::string> MonomerTemplates::kNucleotideComponentTypeStr = {
        {MonomerType::Phosphate, kMonomerClassPHOSPHATE},
        {MonomerType::Sugar, kMonomerClassSUGAR},
        {MonomerType::Base, kMonomerClassBASE}, 
        {MonomerType::AminoAcid, kMonomerClassAminoAcid},
        {MonomerType::CHEM, kMonomerClassCHEM}
    };

    const MonomerTemplates& MonomerTemplates::_instance()
    {
        static MonomerTemplates instance;
        return instance;
    }

    const std::string& MonomerTemplates::classToStr(MonomerType mon_type)
    {
        return kNucleotideComponentTypeStr.at(mon_type);
    }

    bool MonomerTemplates::splitNucleotide(std::string nucleo_type, std::string alias, GranularNucleotide& splitted_nucleotide)
    {
        NucleotideType nt = NucleotideType::RNA;
        if (isDNAClass(nucleo_type))
            nt = NucleotideType::DNA;
        else if (!isRNAClass(nucleo_type))
            return false;
        return splitNucleotide(nt, alias, splitted_nucleotide);
    }

    bool MonomerTemplates::splitNucleotide(NucleotideType nucleo_type, std::string alias, GranularNucleotide& splitted_nucleotide)
    {
        auto it = _instance()._nucleotides_lib.find(std::make_pair(nucleo_type, alias));
        if (it != _instance()._nucleotides_lib.end())
        {
            splitted_nucleotide = it->second;
            return true;
        }
        return false;
    }

    bool MonomerTemplates::getNucleotideMonomer(MonomerType mon_type, std::string alias, BaseMolecule& bmol)
    {
        auto it = _instance()._monomers_lib.find(std::make_pair(mon_type, alias));
        if (it != _instance()._monomers_lib.end())
        {
            bmol.clone(*it->second.monomer);
            return true;
        }
        return false;
    }

    bool MonomerTemplates::getNucleotideMonomer(std::string mon_type, std::string alias, BaseMolecule& bmol)
    {
        auto ct_it = _instance()._component_types.find(mon_type);
        if (ct_it != _instance()._component_types.end())
            return getNucleotideMonomer(ct_it->first, alias, bmol);
        return false;
    }

    MonomerTemplates::MonomerTemplates()
        : _component_types{{kMonomerClassSUGAR, MonomerType::Sugar},
                           {kMonomerClassPHOSPHATE, MonomerType::Phosphate},
                           {kMonomerClassBASE, MonomerType::Base}}
    {
        initializeMonomers();
    }

    void MonomerTemplates::initializeMonomers()
    {
        // collect basic monomers
        using namespace rapidjson;
        Document data;
        std::unordered_map<std::string, std::shared_ptr<BaseMolecule>> mol_map;
        if (!data.Parse(kMonomersBasicTemplates).HasParseError())
        {
            std::unique_ptr<BaseMolecule> monomers_mol(new Molecule());
            MoleculeJsonLoader loader(data);
            loader.loadMolecule(monomers_mol->asMolecule());
            for (auto i = monomers_mol->tgroups.begin(); i != monomers_mol->tgroups.end(); i = monomers_mol->tgroups.next(i))
            {
                std::shared_ptr<BaseMolecule> mon_mol(new Molecule());
                auto& tg = monomers_mol->tgroups.getTGroup(i);
                mon_mol->clone_KeepIndices(*tg.fragment);
                mon_mol->asMolecule().setIgnoreBadValenceFlag(true);
                //mol_map.emplace(tg.tgroup_text_id.ptr(), mon_mol);
            }
        }

        for (const auto& desc : monomer_descriptors)
        {
            for (const auto& alias : desc.aliases)
                _monomers_lib.emplace(std::make_pair(desc.monomer_type, alias),
                                      MonomerTemplate{desc.monomer_type, desc.natreplace, mol_map.at(desc.template_id)});
        }

        // create nucleotides' mappings
        for (const auto& desc : nucleotide_descriptors)
        {
            std::unordered_map<MonomerType, std::reference_wrapper<MonomersLib::value_type>> nucleotide_triplet;
            // iterate nucleotide components
            for (auto& kvp : desc.components)
            {
                // find nucleotide component
                auto comp_it = _monomers_lib.find(std::make_pair(kvp.first, kvp.second));
                if (comp_it != _monomers_lib.end())
                    nucleotide_triplet.emplace(kvp.first, std::ref(*comp_it));
            }

            for (const auto& alias : desc.aliases)
                _nucleotides_lib.emplace(std::make_pair(desc.nucleo_type, alias), nucleotide_triplet);
        }
    }
}
