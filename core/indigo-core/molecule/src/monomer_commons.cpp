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

#include "base_cpp/output.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule_inchi.h"
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

    bool isChemClass(const std::string& monomer_class)
    {
        static const std::unordered_set<std::string> kChemClasses = {kMonomerClassCHEM, kMonomerClassMOD, kMonomerClassXLINK, kMonomerClassLINKER};
        return kChemClasses.find(monomer_class) != kChemClasses.end();
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

    std::string monomerId(const TGroup& tg)
    {
        std::string name;
        std::string monomer_class;
        if (tg.tgroup_text_id.ptr())
            return tg.tgroup_text_id.ptr();
        if (tg.tgroup_name.ptr())
            name = tg.tgroup_name.ptr();
        if (tg.tgroup_class.ptr())
            monomer_class = tg.tgroup_class.ptr();
        if (name.size())
            name = monomerNameByAlias(monomer_class, name) + "_" + std::to_string(tg.tgroup_id);
        else
            name = std::string("#") + std::to_string(tg.tgroup_id);
        return name;
    }

    std::string monomerTemplateId(const TGroup& tg)
    {
        std::string name;
        std::string monomer_class;
        if (tg.tgroup_text_id.ptr())
            return tg.tgroup_text_id.ptr();
        if (tg.tgroup_name.ptr())
            name = tg.tgroup_name.ptr();
        if (tg.tgroup_class.ptr())
            monomer_class = tg.tgroup_class.ptr();

        return monomerNameByAlias(monomer_class, name);
    }

    std::string monomerInchi(const TGroup& tg)
    {
        std::string templ_inchi_str;
        StringOutput templ_inchi_output(templ_inchi_str);
        MoleculeInChI templ_inchi(templ_inchi_output);
        templ_inchi.outputInChI(tg.fragment->asMolecule());
        return templ_inchi_str;
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

    std::string monomerKETClass(const std::string& class_name)
    {
        auto mclass = class_name;
        if (class_name == kMonomerClassAA)
            return kMonomerClassAminoAcid;

        if (mclass == kMonomerClassdAA)
            return kMonomerClassDAminoAcid;

        if (mclass == kMonomerClassRNA || mclass == kMonomerClassDNA || mclass.find(kMonomerClassMOD) == 0 || mclass.find(kMonomerClassXLINK) == 0)
            return mclass;

        for (auto it = mclass.begin(); it < mclass.end(); ++it)
            *it = static_cast<char>(it > mclass.begin() ? std::tolower(*it) : std::toupper(*it));

        return mclass;
    }

    std::string monomerHELMClass(const std::string& class_name)
    {
        if (isAminoAcidClass(class_name))
            return kMonomerClassPEPTIDE;
        if (isNucleicClass(class_name))
            return kMonomerClassRNA;
        return kMonomerClassCHEM;
    }

    // Calclulate offset to maximize sense-antisense complementary pairs count
    // return offset, vector of pairs and flag set to true when antisense should be moved to left
    size_t best_allign(const std::string& sense, const std::string& antisense, std::vector<std::pair<size_t, size_t>>& pairs, bool& shift_sense)
    {
        shift_sense = false;
        pairs.clear();
        if (sense.size() == 0 || antisense.size() == 0)
            return 0;
        size_t max_count = 0;
        std::vector<std::pair<size_t, size_t>> max_pairs;
        size_t allign = 0;
        bool left = false;
        // move right
        for (size_t i = 0; i < sense.size(); i++)
        {
            size_t count = 0;
            size_t len = std::min(sense.size() - i, antisense.size());
            std::vector<std::pair<size_t, size_t>> cur_pairs;
            if (len < max_count)
                break;
            for (size_t idx = 0; idx < len; idx++)
            {
                if (complementary_bases.count(std::make_pair(sense[i + idx], antisense[idx])) > 0)
                {
                    count++;
                    cur_pairs.emplace_back(i + idx, idx);
                }
            }
            if (count > max_count)
            {
                allign = i;
                max_count = count;
                max_pairs = cur_pairs;
            }
        }
        // move left
        for (size_t i = 1; i < antisense.size(); i++)
        {
            size_t count = 0;
            size_t len = std::min(antisense.size() - i, sense.size());
            std::vector<std::pair<size_t, size_t>> cur_pairs;
            if (len < max_count)
                break;
            for (size_t idx = 0; idx < len; idx++)
            {
                if (complementary_bases.count(std::make_pair(sense[idx], antisense[idx + i])) > 0)
                {
                    count++;
                    cur_pairs.emplace_back(idx, idx + i);
                }
            }
            if (count > max_count)
            {
                allign = i;
                max_count = count;
                max_pairs = cur_pairs;
                left = true;
            }
        }
        pairs = max_pairs;
        shift_sense = left;
        return allign;
    }

    size_t needleman_wunsch(const std::string& sense, const std::string& antisense, std::vector<std::pair<size_t, size_t>>& strands,
                            std::map<std::pair<char, char>, int> similarity, int mismatch, int indel)
    {
        std::vector<std::vector<int>> H;
        // First line of scorring matrix contais sense size +1 zeroes
        H.emplace_back(std::vector<int>());
        for (size_t i = 0; i <= sense.size(); i++)
            H[0].emplace_back(static_cast<int>(i) * indel);
        // Function to get pair score from similarity or mismatch value
        auto score = [&similarity, &mismatch](char base1, char base2) {
            auto it = similarity.find(std::make_pair(base1, base2));
            int match = it == similarity.end() ? mismatch : it->second;
            return match;
        };
        // Fill scoring matrix
        for (size_t a = 0; a < antisense.size(); a++)
        {
            auto a_base = antisense[a];
            auto& vec = H.emplace_back(std::vector<int>(1, static_cast<int>(a + 1) * indel));
            auto& prev_vec = H[a];
            int prev_a = prev_vec[0];
            int prev_s = vec[0];
            for (size_t s = 0; s < sense.size(); s++)
            {
                // Calculate next vector element(cur_s) based on already calculated (prev_s, prev_a, cur_a)
                // | prev_a | cur_a |
                // +--------+-------+
                // | prev_s | cur_s +
                int cur_a = prev_vec[s + 1];
                int cur_s = std::max({prev_a + score(a_base, sense[s]), cur_a + indel, prev_s + indel});
                vec.emplace_back(cur_s);
                prev_s = cur_s;
                prev_a = cur_a;
            }
        }
        // Traceback
        size_t a = antisense.size(), s = sense.size(), match_count = 0;
        while (a > 0 && s > 0)
        {
            int current = H[a][s];
            if (current == H[a - 1][s - 1] + score(antisense[a - 1], sense[s - 1]))
            {
                strands.emplace_back(--s, --a);
                ++match_count;
            }
            else if (current == H[a - 1][s] + indel)
            {
                strands.emplace_back(SIZE_MAX, --a);
            }
            else
            {
                strands.emplace_back(--s, SIZE_MAX);
            }
        }
        while (a > 0)
        {
            strands.emplace_back(SIZE_MAX, --a);
        }
        while (s > 0)
        {
            strands.emplace_back(--s, SIZE_MAX);
        }

        // Reverse pairs
        std::reverse(strands.begin(), strands.end());
        return match_count;
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
