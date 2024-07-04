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

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

#include <unordered_map>

#include "molecule/base_molecule.h"
#include "molecule/monomer_commons.h"

namespace indigo
{
    std::string extractMonomerName(const std::string& str)
    {
        std::string res = str;
        if (str.size())
        {
            auto nat_replace = split(std::string(str.c_str()), '/');
            if (nat_replace.size() > 1)
                res = normalizeMonomerName(nat_replace.front(), nat_replace[1]);
        }
        return res;
    }

    std::string classToPrefix(const std::string& monomer_class)
    {
        if (monomer_class == kMonomerClassdAA || monomer_class == kMonomerClassDNA)
            return kPrefix_d;
        return "";
    }

    bool isNucleicClass(const std::string& monomer_class)
    {
        static const std::unordered_set<std::string> kNucleicClasses = {kMonomerClassDNA,    kMonomerClassRNA,      kMonomerClassMODRNA,
                                                                        kMonomerClassMODDNA, kMonomerClassXLINKRNA, kMonomerClassXLINKDNA,
                                                                        kMonomerClassSUGAR,  kMonomerClassBASE,     kMonomerClassPHOSPHATE};
        return kNucleicClasses.find(monomer_class) != kNucleicClasses.end();
    }

    bool isNucleotideClass(const std::string& monomer_class)
    {
        static const std::unordered_set<std::string> kNucleotideClasses = {kMonomerClassDNA,    kMonomerClassRNA,      kMonomerClassMODRNA,
                                                                           kMonomerClassMODDNA, kMonomerClassXLINKRNA, kMonomerClassXLINKDNA};
        return kNucleotideClasses.find(monomer_class) != kNucleotideClasses.end();
    }

    bool isRNAClass(const std::string& monomer_class)
    {
        static const std::unordered_set<std::string> kRNAClasses = {kMonomerClassRNA, kMonomerClassMODRNA, kMonomerClassXLINKRNA};
        return kRNAClasses.find(monomer_class) != kRNAClasses.end();
    }

    bool isDNAClass(const std::string& monomer_class)
    {
        static const std::unordered_set<std::string> kDNAClasses = {kMonomerClassDNA, kMonomerClassMODDNA, kMonomerClassXLINKDNA};
        return kDNAClasses.find(monomer_class) != kDNAClasses.end();
    }

    bool isAminoAcidClass(const std::string& monomer_class)
    {
        static const std::unordered_set<std::string> kAminoClasses = {kMonomerClassAA,    kMonomerClassdAA,    kMonomerClassAminoAcid, kMonomerClassDAminoAcid,
                                                                      kMonomerClassMODAA, kMonomerClassMODDAA, kMonomerClassXLINKAA,   kMonomerClassXLINKDAA};
        return kAminoClasses.find(monomer_class) != kAminoClasses.end();
    }

    bool isBackboneClass(const std::string& monomer_class)
    {
        return isAminoAcidClass(monomer_class) || monomer_class == kMonomerClassSUGAR || monomer_class == kMonomerClassPHOSPHATE ||
               monomer_class == kMonomerClassCHEM || isNucleotideClass(monomer_class);
    }

    bool isBasicAminoAcid(const std::string& monomer_class, const std::string& alias)
    {
        return isAminoAcidClass(monomer_class) && monomerNameByAlias(monomer_class, alias).size() != alias.size();
    }

    std::string monomerNameByAlias(const std::string& monomer_class, const std::string& alias)
    {
        static const std::unordered_map<std::string, std::string> kAliasToNucleic = {{"A", "Ade"}, {"C", "Cyt"},   {"G", "Gua"},  {"U", "Ura"}, {"T", "Thy"},
                                                                                     {"R", "Rib"}, {"dR", "dRib"}, {"m", "mRib"}, {"p", "Pi"}};

        static const std::unordered_map<std::string, std::string> kAliasToAminoAcid = {
            {"A", "Ala"}, {"R", "Arg"}, {"N", "Asn"}, {"D", "Asp"}, {"C", "Cys"}, {"Q", "Gln"}, {"E", "Glu"}, {"G", "Gly"},
            {"H", "His"}, {"I", "Ile"}, {"L", "Leu"}, {"K", "Lys"}, {"M", "Met"}, {"F", "Phe"}, {"P", "Pro"}, {"S", "Ser"},
            {"T", "Thr"}, {"W", "Trp"}, {"Y", "Tyr"}, {"V", "Val"}, {"U", "Sec"}, {"O", "Pyl"}};

        if (isAminoAcidClass(monomer_class))
        {
            auto it = kAliasToAminoAcid.find(alias);
            if (it != kAliasToAminoAcid.end())
                return it->second;
        }
        else if (isNucleicClass(monomer_class))
        {
            auto it = kAliasToNucleic.find(alias);
            if (it != kAliasToNucleic.end())
                return it->second;
        }
        return alias;
    }

    std::string monomerAliasByName(const std::string& monomer_class, const std::string& name)
    {
        static const std::unordered_map<std::string, std::string> kAminoAcidToAlias = {
            {"Ala", "A"}, {"Arg", "R"}, {"Asn", "N"}, {"Asp", "D"}, {"Cys", "C"}, {"Gln", "Q"}, {"Glu", "E"}, {"Gly", "G"},
            {"His", "H"}, {"Ile", "I"}, {"Leu", "L"}, {"Lys", "K"}, {"Met", "M"}, {"Phe", "F"}, {"Pro", "P"}, {"Ser", "S"},
            {"Thr", "T"}, {"Trp", "W"}, {"Tyr", "Y"}, {"Val", "V"}, {"Sec", "U"}, {"Pyl", "O"}};

        static const std::unordered_map<std::string, std::string> kNucleicToAlias = {{"Ade", "A"}, {"Cyt", "C"},   {"Gua", "G"},  {"Ura", "U"}, {"Thy", "T"},
                                                                                     {"Rib", "R"}, {"dRib", "dR"}, {"mRib", "m"}, {"Pi", "p"}};

        if (isAminoAcidClass(monomer_class))
        {
            auto it = kAminoAcidToAlias.find(name);
            if (it != kAminoAcidToAlias.end())
                return it->second;
        }
        else if (isNucleicClass(monomer_class))
        {
            auto it = kNucleicToAlias.find(name);
            if (it != kNucleicToAlias.end())
                return it->second;
        }
        return name;
    }

    std::string normalizeMonomerName(const std::string& monomer_class, const std::string& name)
    {
        auto res = name;
        if (name.size() == 1)
            res = monomerNameByAlias(monomer_class, name);
        else if (name.size() <= kStdMonomerDef)
        {
            if (is_lower_case(name) || is_upper_case(name))
                for (auto it = res.begin(); it < res.end(); ++it)
                    *it = it > res.begin() ? std::tolower(*it, std::locale()) : std::toupper(*it, std::locale());
        }
        // do not add prefix
        auto prefix = classToPrefix(monomer_class);
        if (prefix.size() && res.size() <= kStdMonomerDef)
            res = prefix + res;
        return res;
    }

    std::string normalizeMonomerAlias(const std::string& monomer_class, const std::string& alias)
    {
        auto res = alias;
        if (monomer_class == kMonomerClassdAA)
            res = kPrefix_d + res;
        return res;
    }

    std::string getAttachmentLabel(int order)
    {
        std::string second_chars = "lrx";
        std::string label(1, static_cast<char>('A' + order));
        if (order > static_cast<long>(second_chars.size()) - 1)
            label += second_chars.back();
        else
            label += second_chars[order];
        return label;
    }

    int getAttachmentOrder(const std::string& label)
    {
        if (label == kLeftAttachmentPoint)
            return kLeftAttachmentPointIdx;
        if (label == kRightAttachmentPoint)
            return kRightAttachmentPointIdx;
        if (label.size() > 1 || isupper(label[0]))
        {
            if (label[0] == 'R')
            {
                auto rnum = label.substr(1);
                if (std::all_of(rnum.begin(), rnum.end(), ::isdigit))
                    return std::stol(rnum) - 1;
            }
            if (label[1] == 'x')
                return label[0] - 'A';
        }
        // TODO: return right value at this point
        //       this value returned just to avoid warnings
        return kBranchAttachmentPointIdx;
    }

    bool isAttachmentPointsInOrder(int order, const std::string& label)
    {
        switch (order)
        {
        case 0:
            if (label == kLeftAttachmentPoint || label == kAttachmentPointR1)
                return true;
            break;
        case 1:
            if (label == kRightAttachmentPoint || label == kAttachmentPointR2)
                return true;
            break;
        default:
            if (label.size() > 1 || isupper(label[0]))
            {
                if (label[0] == 'R')
                {
                    auto rnum = label.substr(1);
                    if (std::all_of(rnum.begin(), rnum.end(), ::isdigit) && (std::stol(rnum) == order + 1))
                        return true;
                }
                if (label[1] == 'x' && (label[0] - 'A' == order))
                    return true;
            }
            break;
        }
        return false;
    }

    std::string monomerAlias(const TGroup& tg)
    {
        std::string monomer_class;
        std::string alias;
        std::string name;

        if (tg.tgroup_class.ptr())
            monomer_class = tg.tgroup_class.ptr();

        if (tg.tgroup_alias.ptr())
            alias = tg.tgroup_alias.ptr();

        if (tg.tgroup_name.ptr())
            name = tg.tgroup_name.ptr();

        if (alias.size())
            alias = normalizeMonomerAlias(monomer_class, alias);
        else
        {
            alias = name;
            if (name.size() == 1)
                std::transform(alias.begin(), alias.end(), alias.begin(), ::toupper);
            else if (name.empty())
                alias = std::string("#") + std::to_string(tg.tgroup_id - 1);
        }
        return alias;
    }

    std::optional<std::reference_wrapper<TGroup>> findTemplateInMap(
        const std::string& name, const std::string& class_name,
        std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash>& templates_map)
    {
        auto tg_it = templates_map.find(std::make_pair(name, class_name));
        if (tg_it == templates_map.end())
        {
            auto mname = monomerNameByAlias(class_name, name);
            tg_it = templates_map.find(std::make_pair(mname, class_name));
        }
        return tg_it == templates_map.end() ? std::nullopt : std::optional<std::reference_wrapper<TGroup>>(std::ref(tg_it->second));
    }

    HELMType getHELMTypeFromString(const std::string& helm_type)
    {
        static const std::unordered_map<std::string, HELMType> strToType = {
            {kHELMPolymerTypePEPTIDE, HELMType::Peptide},
            {kHELMPolymerTypeRNA, HELMType::RNA},
            {kHELMPolymerTypeCHEM, HELMType::Chem},
            {kHELMPolymerTypeUnknown, HELMType::Unknown},
        };
        auto it = strToType.find(helm_type);
        if (it != strToType.end())
            return it->second;
        return HELMType::Unknown;
    }

    const std::string& getStringFromHELMType(HELMType helm_type)
    {
        static const std::unordered_map<HELMType, std::string> typeToStr = {
            {HELMType::Peptide, kHELMPolymerTypePEPTIDE},
            {HELMType::RNA, kHELMPolymerTypeRNA},
            {HELMType::Chem, kHELMPolymerTypeCHEM},
            {HELMType::Unknown, kHELMPolymerTypeUnknown},
        };
        return typeToStr.at(helm_type);
    }

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
