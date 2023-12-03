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

    struct NucleotidePartDescriptor
    {
        NucleotideComponentType component_type;
        std::string template_id;
        std::vector<std::string> aliases;
        std::string natreplace;
    };

    struct NucleotideDescriptor
    {
        NucleotideType nucleo_type;
        std::vector<std::string> aliases;
        std::unordered_map<NucleotideComponentType, std::string> components;
    };

    const std::vector<NucleotidePartDescriptor> monomer_descriptors = {
        {NucleotideComponentType::Phosphate, "P", {"P", "p"}, "P"},       {NucleotideComponentType::Sugar, "Rib", {"R", "Rib", "r"}, "R"},
        {NucleotideComponentType::Sugar, "dRib", {"d", "dR", "dRib"}, "R"}, {NucleotideComponentType::Base, "Ade", {"A", "Ade"}, "A"},
        {NucleotideComponentType::Base, "Cyt", {"C", "Cyt"}, "C"},          {NucleotideComponentType::Base, "Gua", {"G", "Gua"}, "G"},
        {NucleotideComponentType::Base, "Thy", {"T", "Thy"}, "T"},          {NucleotideComponentType::Base, "Ura", {"U", "Ura"}, "U"}};

    const std::vector<NucleotideDescriptor> nucleotide_descriptors = {
        {NucleotideType::RNA,
         {"A", "AMP"},
         {{NucleotideComponentType::Base, "A"}, {NucleotideComponentType::Sugar, "R"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::RNA,
         {"C", "CMP"},
         {{NucleotideComponentType::Base, "C"}, {NucleotideComponentType::Sugar, "R"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::RNA,
         {"G", "GMP"},
         {{NucleotideComponentType::Base, "G"}, {NucleotideComponentType::Sugar, "R"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::RNA,
         {"T", "TMP"},
         {{NucleotideComponentType::Base, "T"}, {NucleotideComponentType::Sugar, "R"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::RNA,
         {"U", "UMP"},
         {{NucleotideComponentType::Base, "U"}, {NucleotideComponentType::Sugar, "R"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"A", "dA", "dAMP"},
         {{NucleotideComponentType::Base, "A"}, {NucleotideComponentType::Sugar, "dR"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"C", "dC", "dCMP"},
         {{NucleotideComponentType::Base, "C"}, {NucleotideComponentType::Sugar, "dR"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"G", "dG", "dGMP"},
         {{NucleotideComponentType::Base, "G"}, {NucleotideComponentType::Sugar, "dR"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"T", "dT", "dTMP"},
         {{NucleotideComponentType::Base, "T"}, {NucleotideComponentType::Sugar, "dR"}, {NucleotideComponentType::Phosphate, "P"}}},
        {NucleotideType::DNA,
         {"U", "dU", "dUMP"},
         {{NucleotideComponentType::Base, "U"}, {NucleotideComponentType::Sugar, "dR"}, {NucleotideComponentType::Phosphate, "P"}}}};

    const std::unordered_map<NucleotideComponentType, std::string> MonomerTemplates::kNucleotideComponentTypeStr = {
        {NucleotideComponentType::Phosphate, kMonomerClassPHOSPHATE},
        {NucleotideComponentType::Sugar, kMonomerClassSUGAR},
        {NucleotideComponentType::Base, kMonomerClassBASE},
    };

    const MonomerTemplates& MonomerTemplates::_instance()
    {
        static MonomerTemplates instance;
        return instance;
    }

    const std::string& MonomerTemplates::classToStr(NucleotideComponentType comp_type)
    {
        return kNucleotideComponentTypeStr.at(comp_type);
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

    bool MonomerTemplates::getNucleotideMonomer(NucleotideComponentType comp_type, std::string alias, BaseMolecule& bmol)
    {
        auto it = _instance()._monomers_lib.find(std::make_pair(comp_type, alias));
        if (it != _instance()._monomers_lib.end())
        {
            bmol.clone(*it->second.monomer);
            return true;
        }
        return false;
    }

    bool MonomerTemplates::getNucleotideMonomer(std::string comp_type, std::string alias, BaseMolecule& bmol)
    {
        auto ct_it = _instance()._component_types.find(comp_type);
        if (ct_it != _instance()._component_types.end())
            return getNucleotideMonomer(ct_it->first, alias, bmol);
        return false;
    }

    MonomerTemplates::MonomerTemplates()
        : _component_types{{kMonomerClassSUGAR, NucleotideComponentType::Sugar},
                           {kMonomerClassPHOSPHATE, NucleotideComponentType::Phosphate},
                           {kMonomerClassBASE, NucleotideComponentType::Base}}
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
                mol_map.emplace(tg.tgroup_text_id.ptr(), mon_mol);
            }
        }

        for (const auto& desc : monomer_descriptors)
        {
            for (const auto& alias : desc.aliases)
                _monomers_lib.emplace(std::make_pair(desc.component_type, alias),
                                      NucleotideComponent{desc.component_type, desc.natreplace, mol_map.at(desc.template_id)});
        }

        // create nucleotides' mappings
        for (const auto& desc : nucleotide_descriptors)
        {
            std::unordered_map<NucleotideComponentType, std::reference_wrapper<MonomersLib::value_type>> nucleotide_triplet;
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
