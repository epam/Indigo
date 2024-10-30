#ifndef __monomer_commons__
#define __monomer_commons__

#include <optional>
#include <string>
#include <unordered_set>

#include "molecule/monomers_lib.h"
#include "molecule/parse_utils.h"

namespace indigo
{
    class TGroup;
    const int kStdMonomerDef = 3;

    // amino acids
    const auto kMonomerClassAA = "AA";
    const auto kMonomerClassdAA = "dAA";
    const auto kMonomerClassMODAA = "MODAA";
    const auto kMonomerClassMODDAA = "MODdAA";
    const auto kMonomerClassXLINKAA = "XLINKAA";
    const auto kMonomerClassXLINKDAA = "XLINKdAA";
    const auto kMonomerClassAminoAcid = "AminoAcid";
    const auto kMonomerClassDAminoAcid = "D-AminoAcid";
    const auto kMonomerClassPEPTIDE = "PEPTIDE";
    const auto kMonomerClassPEPTIDE_3_LETTER = "PEPTIDE-3-LETTER";

    // nucleic classes
    const auto kMonomerClassDNA = "DNA";
    const auto kMonomerClassRNA = "RNA";
    const auto kMonomerClassMODDNA = "MODDNA";
    const auto kMonomerClassMODRNA = "MODRNA";
    const auto kMonomerClassXLINKDNA = "XLINKDNA";
    const auto kMonomerClassXLINKRNA = "XLINKRNA";

    const auto kMonomerClassCHEM = "CHEM";
    const auto kMonomerClassSUGAR = "SUGAR";
    const auto kMonomerClassBASE = "BASE";
    const auto kMonomerClassPHOSPHATE = "PHOSPHATE";

    const auto kMonomerClassMOD = "MOD";
    const auto kMonomerClassXLINK = "XLINK";
    const auto kMonomerClassLINKER = "LINKER";

    const auto kFASTA_HEADER = "FASTA_HEADER";

    const auto kPrefix_d("d");
    const auto kPrefix_r("r");

    std::string extractMonomerName(const std::string& str);

    bool isNucleicClass(const std::string& monomer_class);

    bool isNucleotideClass(const std::string& monomer_class);

    bool isAminoAcidClass(const std::string& monomer_class);

    bool isBackboneClass(const std::string& monomer_class);

    bool isRNAClass(const std::string& monomer_class);

    bool isDNAClass(const std::string& monomer_class);

    std::string classToPrefix(const std::string& monomer_class);

    std::string monomerNameByAlias(const std::string& monomer_class, const std::string& alias);

    bool isBasicAminoAcid(const std::string& monomer_class, const std::string& alias);

    std::string monomerAliasByName(const std::string& monomer_class, const std::string& name);

    std::string normalizeMonomerName(const std::string& monomer_class, const std::string& name);

    std::string normalizeMonomerAlias(const std::string& monomer_class, const std::string& alias);

    bool isAttachmentPointsInOrder(int order, const std::string& label);
    int getAttachmentOrder(const std::string& label);
    std::string getAttachmentLabel(int order);
    std::string monomerAlias(const TGroup& tg);
    std::optional<std::reference_wrapper<TGroup>> findTemplateInMap(
        const std::string& name, const std::string& class_name,
        std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash>& templates_map);

    const auto kLeftAttachmentPoint = "Al";
    const auto kRightAttachmentPoint = "Br";
    const auto kBranchAttachmentPoint = "Cx";
    const auto kAttachmentPointR1 = "R1";
    const auto kAttachmentPointR2 = "R2";

    const int kLeftAttachmentPointIdx = 0;
    const int kRightAttachmentPointIdx = 1;
    const int kBranchAttachmentPointIdx = 2; // branch without a direction

    struct MonomerAttachmentPoint
    {
        int attachment_atom;
        int leaving_group;
        std::string id;
    };

    const auto kHELMPolymerTypePEPTIDE = kMonomerClassPEPTIDE;
    const auto kHELMPolymerTypeRNA = kMonomerClassRNA;
    const auto kHELMPolymerTypeCHEM = kMonomerClassCHEM;
    const auto kHELMPolymerTypeUnknown = "BLOB";

    enum class HELMType
    {
        Peptide,
        RNA,
        Chem,
        Unknown
    };

    HELMType getHELMTypeFromString(const std::string& helm_type);
    const std::string& getStringFromHELMType(HELMType helm_type);

    static const std::unordered_set<std::string> STANDARD_PEPTIDES = {"A", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M",
                                                                      "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "Y"};
    static const std::unordered_set<std::string> STANDARD_NUCLEOTIDES = {"A", "C", "G", "T", "U"};

    static const std::unordered_set<std::string> IDT_STANDARD_BASES = {"A", "T", "C", "G", "U", "I", "In"};
    static const std::map<std::string, int> IDT_BASE_TO_RATIO_IDX = {{"A", 0}, {"C", 1}, {"G", 2}, {"T", 3}, {"U", 3}};
    static const std::map<std::string, std::string> IDT_STANDARD_SUGARS{{"R", "r"}, {"LR", "+"}, {"mR", "m"}, {"dR", ""}};

    static const std::map<std::string, std::vector<std::string>> STANDARD_MIXED_PEPTIDES = {
        {"B", {"D", "N"}}, {"J", {"L", "I"}}, {"Z", {"E", "Q"}}, {"X", {"A", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M",
                                                                        "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "Y"}}};
    static const std::map<std::string, std::vector<std::string>> STANDARD_MIXED_BASES = {
        {"R", {"A", "G"}},      {"Y", {"C", "T"}},      {"M", {"A", "C"}},      {"K", {"G", "T"}},      {"S", {"G", "C"}},          {"W", {"A", "T"}},
        {"H", {"A", "C", "T"}}, {"B", {"C", "G", "T"}}, {"V", {"A", "C", "G"}}, {"D", {"A", "G", "T"}}, {"N", {"A", "C", "G", "T"}}};
    static const std::map<std::set<std::string>, std::string> STANDARD_MIXED_PEPTIDES_TO_ALIAS = {
        {{"D", "N"}, "B"},
        {{"L", "I"}, "J"},
        {{"E", "Q"}, "Z"},
        {{"A", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "Y"}, "X"}};
    static const std::map<std::set<std::string>, std::string> STANDARD_MIXED_BASES_TO_ALIAS = {
        {{"A", "G"}, "R"},       {{"C", "T"}, "Y"},           {{"C", "U"}, "rY"},          {{"A", "C"}, "M"},      {{"G", "T"}, "K"},
        {{"G", "U"}, "rK"},      {{"G", "C"}, "S"},           {{"A", "T"}, "W"},           {{"A", "U"}, "rW"},     {{"A", "C", "T"}, "H"},
        {{"A", "C", "U"}, "rH"}, {{"C", "G", "T"}, "B"},      {{"C", "G", "U"}, "rB"},     {{"A", "C", "G"}, "V"}, {{"A", "G", "T"}, "D"},
        {{"A", "G", "U"}, "rD"}, {{"A", "C", "G", "T"}, "N"}, {{"A", "C", "G", "U"}, "rN"}};
    static const std::set<std::string> RNA_DNA_MIXED_BASES = {"R", "M", "S", "V"};
    static const std::map<std::string, std::string> STANDARD_MIXED_PEPTIDES_ALIAS_TO_NAME = {{"B", "Asx"}, {"J", "Xle"}, {"X", "Xaa"}, {"Z", "Glx"}};
    static const std::map<std::string, std::string> STANDARD_MIXED_PEPTIDES_NAME_TO_ALIAS = {{"Asx", "B"}, {"Xle", "J"}, {"Xaa", "X"}, {"Glx", "Z"}};
}
#endif