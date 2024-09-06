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
        MonomerClass monomer_type;
        std::string template_id;
        std::vector<std::string> aliases;
        std::string natreplace;
    };

    struct NucleotideDescriptor
    {
        NucleotideType nucleo_type;
        std::vector<std::string> aliases;
        std::unordered_map<MonomerClass, std::string> components;
    };

    const MonomerTemplates& MonomerTemplates::_instance()
    {
        static MonomerTemplates instance;
        return instance;
    }

    const std::string& MonomerTemplates::classToStr(MonomerClass mon_type)
    {
        static const std::unordered_map<MonomerClass, std::string> kMonomerTypeStr = {{MonomerClass::Phosphate, kMonomerClassPHOSPHATE},
                                                                                      {MonomerClass::Sugar, kMonomerClassSUGAR},
                                                                                      {MonomerClass::Base, kMonomerClassBASE},
                                                                                      {MonomerClass::AminoAcid, kMonomerClassAA},
                                                                                      {MonomerClass::CHEM, kMonomerClassCHEM},
                                                                                      {MonomerClass::DNA, kMonomerClassDNA},
                                                                                      {MonomerClass::RNA, kMonomerClassRNA}};

        return kMonomerTypeStr.at(mon_type);
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

    const std::unordered_map<std::string, MonomerClass>& MonomerTemplates::getStrToMonomerType()
    {
        static const std::unordered_map<std::string, MonomerClass> kStrMonomerType = {
            {kMonomerClassSUGAR, MonomerClass::Sugar},  {kMonomerClassPHOSPHATE, MonomerClass::Phosphate}, {kMonomerClassBASE, MonomerClass::Base},
            {kMonomerClassAA, MonomerClass::AminoAcid}, {kMonomerClassCHEM, MonomerClass::CHEM},           {kMonomerClassDNA, MonomerClass::DNA},
            {kMonomerClassRNA, MonomerClass::RNA},      {kMonomerClassPEPTIDE, MonomerClass::AminoAcid}};
        return kStrMonomerType;
    }

    bool MonomerTemplates::getMonomerTemplate(MonomerClass mon_type, std::string alias, TGroup& tgroup)
    {
        auto it = _instance()._monomers_lib.find(std::make_pair(mon_type, alias));
        if (it != _instance()._monomers_lib.end())
        {
            tgroup.copy(it->second.get());
            return true;
        }
        return false;
    }

    bool MonomerTemplates::getMonomerTemplate(std::string mon_type, std::string alias, TGroup& tgroup)
    {
        auto ct_it = getStrToMonomerType().find(mon_type);
        if (ct_it != _instance().getStrToMonomerType().end())
            return getMonomerTemplate(ct_it->second, alias, tgroup);
        return false;
    }

    MonomerTemplates::MonomerTemplates() : _templates_mol(new Molecule())
    {
        initializeMonomers();
    }

    void MonomerTemplates::initializeMonomers()
    {
        static const std::vector<NucleotideDescriptor> nucleotide_presets = {
            {NucleotideType::RNA, {"A", "AMP"}, {{MonomerClass::Base, "A"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::RNA, {"C", "CMP"}, {{MonomerClass::Base, "C"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::RNA, {"G", "GMP"}, {{MonomerClass::Base, "G"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::RNA, {"T", "TMP"}, {{MonomerClass::Base, "T"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::RNA, {"U", "UMP"}, {{MonomerClass::Base, "U"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"A", "dA", "dAMP"}, {{MonomerClass::Base, "A"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"C", "dC", "dCMP"}, {{MonomerClass::Base, "C"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"G", "dG", "dGMP"}, {{MonomerClass::Base, "G"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"T", "dT", "dTMP"}, {{MonomerClass::Base, "T"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"U", "dU", "dUMP"}, {{MonomerClass::Base, "U"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}}};

        // collect basic monomers
        using namespace rapidjson;
        Document data;
        if (!data.Parse(kMonomersBasicTemplates).HasParseError())
        {
            MoleculeJsonLoader loader(data);
            loader.stereochemistry_options.ignore_errors = true;
            loader.loadMolecule(_templates_mol->asMolecule());
            for (auto i = _templates_mol->tgroups.begin(); i != _templates_mol->tgroups.end(); i = _templates_mol->tgroups.next(i))
            {
                auto& tg = _templates_mol->tgroups.getTGroup(i);

                _monomers_lib.emplace(MonomerKey(getStrToMonomerType().at(tg.tgroup_class.ptr()), tg.tgroup_alias.ptr()), std::ref(tg));
            }
        }

        // create nucleotides' mappings
        for (const auto& desc : nucleotide_presets)
        {
            std::unordered_map<MonomerClass, std::reference_wrapper<MonomersLib::value_type>> nucleotide_triplet;
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
