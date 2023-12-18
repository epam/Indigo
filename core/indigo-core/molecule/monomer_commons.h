#ifndef __monomer_commons__
#define __monomer_commons__

#include <string>
#include <unordered_set>

#include "molecule/monomers_lib.h"
#include "molecule/parse_utils.h"

namespace indigo
{
    const int kStdMonomerDef = 3;

    // amino acids
    const std::string kMonomerClassAA = "AA";
    const std::string kMonomerClassdAA = "dAA";
    const std::string kMonomerClassMODAA = "MODAA";
    const std::string kMonomerClassMODDAA = "MODdAA";
    const std::string kMonomerClassXLINKAA = "XLINKAA";
    const std::string kMonomerClassXLINKDAA = "XLINKdAA";
    const std::string kMonomerClassAminoAcid = "AminoAcid";
    const std::string kMonomerClassDAminoAcid = "D-AminoAcid";
    const std::string kMonomerClassPEPTIDE = "PEPTIDE";

    // nucleic classes
    const std::string kMonomerClassDNA = "DNA";
    const std::string kMonomerClassRNA = "RNA";
    const std::string kMonomerClassMODDNA = "MODDNA";
    const std::string kMonomerClassMODRNA = "MODRNA";
    const std::string kMonomerClassXLINKDNA = "XLINKDNA";
    const std::string kMonomerClassXLINKRNA = "XLINKRNA";

    const std::string kMonomerClassCHEM = "CHEM";
    const std::string kMonomerClassSUGAR = "SUGAR";
    const std::string kMonomerClassBASE = "BASE";
    const std::string kMonomerClassPHOSPHATE = "PHOSPHATE";

    const std::string kMonomerClassMOD = "MOD";
    const std::string kMonomerClassXLINK = "XLINK";

    const std::string kPrefix_d("d");
    const std::string kPrefix_r("r");

    const std::unordered_set<std::string> kNucleotideClasses = {kMonomerClassDNA,    kMonomerClassRNA,      kMonomerClassMODRNA,
                                                                kMonomerClassMODDNA, kMonomerClassXLINKRNA, kMonomerClassXLINKDNA};

    const std::unordered_set<std::string> kNucleicClasses = {kMonomerClassDNA,    kMonomerClassRNA,      kMonomerClassMODRNA,
                                                             kMonomerClassMODDNA, kMonomerClassXLINKRNA, kMonomerClassXLINKDNA,
                                                             kMonomerClassSUGAR,  kMonomerClassBASE,     kMonomerClassPHOSPHATE};

    const std::unordered_set<std::string> kRNAClasses = {kMonomerClassRNA, kMonomerClassMODRNA, kMonomerClassXLINKRNA};

    const std::unordered_set<std::string> kDNAClasses = {kMonomerClassDNA, kMonomerClassMODDNA, kMonomerClassXLINKDNA};

    const std::unordered_set<std::string> kAminoClasses = {kMonomerClassAA,    kMonomerClassdAA,    kMonomerClassAminoAcid, kMonomerClassDAminoAcid,
                                                           kMonomerClassMODAA, kMonomerClassMODDAA, kMonomerClassXLINKAA,   kMonomerClassXLINKDAA};

    inline bool isNucleicClass(const std::string& monomer_class)
    {
        return kNucleicClasses.find(monomer_class) != kNucleicClasses.end();
    }

    inline bool isNucleotideClass(const std::string& monomer_class)
    {
        return kNucleotideClasses.find(monomer_class) != kNucleotideClasses.end();
    }

    inline bool isAminoAcidClass(const std::string& monomer_class)
    {
        return kAminoClasses.find(monomer_class) != kAminoClasses.end();
    }

    inline bool isRNAClass(const std::string& monomer_class)
    {
        return kRNAClasses.find(monomer_class) != kRNAClasses.end();
    }

    inline bool isDNAClass(const std::string& monomer_class)
    {
        return kDNAClasses.find(monomer_class) != kDNAClasses.end();
    }

    std::string classToPrefix(const std::string& monomer_class);

    std::string monomerNameByAlias(const std::string& monomer_class, const std::string& alias);

    bool isBasicAminoAcid(const std::string& monomer_class, const std::string& alias);

    std::string monomerAliasByName(const std::string& monomer_class, const std::string& name);

    std::string normalizeMonomerName(const std::string& monomer_class, const std::string& name);

    std::string normalizeMonomerAlias(const std::string& monomer_class, const std::string& alias);

    bool isAttachmentPointsInOrder(int order, const std::string& label);
    int getAttachmentOrder(const std::string& label);
    std::string getAttachmentLabel(int order);

    const auto kLeftAttachmentPoint = "Al";
    const auto kRightAttachmentPoint = "Br";
    const auto kBranchAttachmentPoint = "Cx";

    struct MonomerAttachmentPoint
    {
        int attachment_atom;
        int leaving_group;
        std::string id;
    };
}
#endif