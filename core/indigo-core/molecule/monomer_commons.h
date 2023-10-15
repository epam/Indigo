#ifndef __monomer_commons__
#define __monomer_commons__

#include <string>
#include <unordered_set>

#include "molecule/parse_utils.h"

namespace indigo
{
    const int kStdMonomerDef = 3;

    const std::string kMonomerClassAA = "AA";
    const std::string kMonomerClassdAA = "dAA";
    const std::string kMonomerClassDNA = "DNA";
    const std::string kMonomerClassRNA = "RNA";
    const std::string kMonomerClassSUGAR = "SUGAR";
    const std::string kMonomerClassBASE = "BASE";
    const std::string kMonomerClassPHOSPHATE = "PHOSPHATE";
    const std::string kMonomerClassAminoAcid = "AminoAcid";
    const std::string kMonomerClassDAminoAcid = "D-AminoAcid";

    const std::string kPrefix_d("d");
    const std::string kPrefix_r("r");

    const std::unordered_set<std::string> kNucleicClasses = {kMonomerClassDNA, kMonomerClassRNA, kMonomerClassSUGAR, kMonomerClassBASE, kMonomerClassPHOSPHATE};
    const std::unordered_set<std::string> kAminoClasses = {kMonomerClassAA, kMonomerClassdAA, kMonomerClassAminoAcid, kMonomerClassDAminoAcid};

    inline bool isNucleicClass(const std::string& monomer_class)
    {
        return kNucleicClasses.find(monomer_class) != kNucleicClasses.end();
    }

    inline bool isAminoAcidClass(const std::string& monomer_class)
    {
        return kAminoClasses.find(monomer_class) != kAminoClasses.end();
    }

    std::string classToPrefix(const std::string& monomer_class);

    std::string monomerNameByAlias(const std::string& monomer_class, const std::string& alias);

    std::string monomerAliasByName(const std::string& monomer_class, const std::string& name);

    std::string normalizeMonomerName(const std::string& monomer_class, const std::string& name);

    std::string normalizeMonomerAlias(const std::string& monomer_class, const std::string& alias);

    bool isAttachmentPointsInOrder(int order, const std::string& label);

    struct MonomerAttachmentPoint
    {
        int attachment_atom;
        int leaving_group;
        std::string id;
    };

}
#endif