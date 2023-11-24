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
#include <string>
#include <unordered_map>
#include <vector>

#include "common/base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "molecule/smiles_loader.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_lib.h"

namespace indigo
{
    const auto KPhosphate = "OP([OH:1])([OH:2])=O";
    const auto KSugarRibose = "O[C@H]1[C@H]([OH:3])O[C@H](CO[H:1])[C@H]1O[H:2]";
    const auto KSugarDeoxyribose = "[H:1]OC[C@H]1O[C@@H]([OH:3])C[C@@H]1O[H:2]";
    const auto KBaseAdenine = "Nc1ncnc2n([H:1])cnc12";
    const auto KBaseCytosine = "Nc1ccn([H:1])c(=O)n1";
    const auto KBaseGuanine = "Nc1nc2n([H:1])cnc2c(=O)[nH]1";
    const auto KBaseThymine = "Cc1cn([H:1])c(=O)[nH]c1=O";
    const auto kBaseUracil = "[H:1]n1ccc(=O)[nH]c1=O";

    struct NucleotidePartDescriptor
    {
        NucleotideComponentType component_type;
        std::string molecule_str;
        std::vector<std::string> aliases;
    };

    struct NucleotideDescriptor
    {
        NucleotideType nucleo_type;
        std::vector<std::string> aliases;
        std::unordered_map<NucleotideComponentType, std::string> components;
    };

    const std::vector<NucleotidePartDescriptor> monomer_descriptors = {{NucleotideComponentType::Phosphate, KPhosphate, {"P", "p"}},
                                                                       {NucleotideComponentType::Sugar, KSugarRibose, {"R", "Rib", "r"}},
                                                                       {NucleotideComponentType::Sugar, KSugarDeoxyribose, {"d", "dR", "dRib"}},
                                                                       {NucleotideComponentType::Base, KBaseAdenine, {"A", "Ade"}},
                                                                       {NucleotideComponentType::Base, KBaseCytosine, {"C", "Cyt"}},
                                                                       {NucleotideComponentType::Base, KBaseGuanine, {"G", "Gua"}},
                                                                       {NucleotideComponentType::Base, KBaseThymine, {"T", "Thy"}},
                                                                       {NucleotideComponentType::Base, kBaseUracil, {"U", "Ura"}}};

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

    const MonomerTemplates& MonomerTemplates::_instance()
    {
        static MonomerTemplates instance;
        return instance;
    }

    std::string MonomerTemplates::_getNucleotideId(NucleotideType nucleo_type, std::string alias)
    {
        std::string res;
        switch (nucleo_type)
        {
        case NucleotideType::RNA:
            res = kMonomerClassRNA;
            break;
        case NucleotideType::DNA:
            res = kMonomerClassDNA;
            break;
        }
        return res + ":" + alias;
    }

    std::string MonomerTemplates::_getNucleotideMonomerId(NucleotideComponentType comp_type, std::string alias)
    {
        std::string res;
        switch (comp_type)
        {
        case NucleotideComponentType::Sugar:
            res = kMonomerClassSUGAR;
            break;
        case NucleotideComponentType::Phosphate:
            res = kMonomerClassPHOSPHATE;
            break;
        case NucleotideComponentType::Base:
            res = kMonomerClassBASE;
            break;
        }
        return res + ":" + alias;
    }

    bool MonomerTemplates::splitNucleotide(NucleotideType nucleo_type, std::string alias,
                                           std::unordered_map<NucleotideComponentType, std::shared_ptr<BaseMolecule>>& splitted_nucleotide)
    {
        auto it = _instance()._nucleotides_lib.find(_getNucleotideId(nucleo_type, alias));
        if (it != _instance()._nucleotides_lib.end())
            splitted_nucleotide = it->second;
        return false;
    }

    bool MonomerTemplates::getNucleotideMonomer(NucleotideComponentType comp_type, std::string alias, BaseMolecule& bmol)
    {
        auto it = _instance()._monomers_lib.find(_getNucleotideMonomerId(comp_type, alias));
        if (it != _instance()._monomers_lib.end())
        {
            bmol.clone(*it->second.second);
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
        for (const auto& desc : monomer_descriptors)
        {
            BufferScanner scan(desc.molecule_str.c_str());
            SmilesLoader loader(scan);
            std::shared_ptr<BaseMolecule> bmol_ptr(new Molecule());
            loader.loadMolecule(bmol_ptr->asMolecule());
            for (const auto& alias : desc.aliases)
                _monomers_lib.emplace(_getNucleotideMonomerId(desc.component_type, alias), std::make_pair(desc.component_type, bmol_ptr));
        }

        // create nucleotides' mappings
        for (const auto& desc : nucleotide_descriptors)
        {
            std::unordered_map<NucleotideComponentType, std::shared_ptr<BaseMolecule>> nucleotide_triplet;
            // iterate nucleotide components
            for (auto& kvp : desc.components)
            {
                // find nucleotide component
                auto comp_it = _monomers_lib.find(_getNucleotideMonomerId(kvp.first, kvp.second));
                if (comp_it != _monomers_lib.end())
                    nucleotide_triplet.insert(comp_it->second);
            }

            for (const auto& alias : desc.aliases)
                _nucleotides_lib.emplace(_getNucleotideId(desc.nucleo_type, alias), nucleotide_triplet);
        }
    }
}
