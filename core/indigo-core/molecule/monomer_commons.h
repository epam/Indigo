#ifndef __monomer_commons__
#define __monomer_commons__

#include <string>

namespace indigo
{
    const std::unordered_map<std::string, std::string> kAminoAcidToAlias = {
        {"Ala", "A"}, {"Arg", "R"}, {"Asn", "N"}, {"Asp", "D"}, {"Cys", "C"}, {"Gln", "Q"}, {"Glu", "E"}, {"Gly", "G"}, {"His", "H"}, {"Ile", "I"},
        {"Leu", "L"}, {"Lys", "K"}, {"Met", "M"}, {"Phe", "F"}, {"Pro", "P"}, {"Ser", "S"}, {"Thr", "T"}, {"Trp", "W"}, {"Tyr", "Y"}, {"Val", "V"}};

    const std::unordered_map<std::string, std::string> kAliasToAminoAcid = {
        {"A", "Ala"}, {"R", "Arg"}, {"N", "Asn"}, {"D", "Asp"}, {"C", "Cys"}, {"Q", "Gln"}, {"E", "Glu"}, {"G", "Gly"}, {"H", "His"}, {"I", "Ile"},
        {"L", "Leu"}, {"K", "Lys"}, {"M", "Met"}, {"F", "Phe"}, {"P", "Pro"}, {"S", "Ser"}, {"T", "Thr"}, {"W", "Trp"}, {"Y", "Tyr"}, {"V", "Val"}};

    const std::unordered_map<std::string, std::string> kNucleicToAlias = {{"Ade", "A"}, {"Cyt", "C"},  {"Gua", "G"},  {"Ura", "U"}, {"Thy", "T"},
                                                                          {"Rib", "r"}, {"dRib", "d"}, {"mRib", "m"}, {"Pi", "p"}};

    const std::unordered_map<std::string, std::string> kAliasToNucleic = {{"A", "Ade"}, {"C", "Cyt"},  {"G", "Gua"},  {"U", "Ura"}, {"T", "Thy"},
                                                                          {"r", "Rib"}, {"d", "dRib"}, {"m", "mRib"}, {"p", "Pi"}};
    const int kStdMonomerDef = 3;

    inline bool isNucleicClass(const std::string& monomer_class)
    {
        return monomer_class == "RNA" || monomer_class == "DNA" || monomer_class == "BASE";
    }

    inline bool isAminoAcidClass(const std::string& monomer_class)
    {
        return monomer_class == "AA" || monomer_class == "dAA";
    }

    inline std::string classToPrefix(const std::string& monomer_class)
    {
        if (monomer_class == "dAA" || monomer_class == "DNA")
            return "d";
        else if (monomer_class == "RNA")
            return "r";
        return "";
    }

    inline std::string monomerNameByAlias(const std::string& monomer_class, const std::string& alias)
    {
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

    inline std::string normalizeMonomerName(const std::string& monomer_class, const std::string& name)
    {
        auto res = name;
        if (name.size() == 1)
            res = monomerNameByAlias(monomer_class, name);
        else if (name.size() <= kStdMonomerDef)
            for (auto it = res.begin(); it < res.end(); ++it)
                *it = it > res.begin() ? std::tolower(*it) : std::toupper(*it);
        auto prefix = classToPrefix(monomer_class);
        if (prefix.size())
            res = prefix + res;
        return res;
    }

    inline std::string normalizeMonomerAlias(const std::string& monomer_class, const std::string& alias)
    {
        auto res = alias;
        if (res.size() == 1)
            res = std::toupper(res[0]);
        if (monomer_class == "dAA")
            res = std::string("d") + res;
        return res;
    }

    inline bool isAttachmentPointsInOrder(int order, const std::string& label)
    {
        switch (order)
        {
        case 0:
            if (label == "Al" || label == "R1")
                return true;
            break;
        case 1:
            if (label == "Br" || label == "R2")
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
}
#endif