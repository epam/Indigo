#ifndef __monomer_commons__
#define __monomer_commons__

#include <string>

#include "molecule/parse_utils.h"

namespace indigo
{
    const int kStdMonomerDef = 3;

    inline bool isNucleicClass(const std::string& monomer_class)
    {
        return monomer_class == "RNA" || monomer_class == "DNA" || monomer_class == "BASE" || monomer_class == "PHOSPHATE" || monomer_class == "SUGAR";
    }

    inline bool isAminoAcidClass(const std::string& monomer_class)
    {
        return monomer_class == "AA" || monomer_class == "dAA" || monomer_class == "AminoAcid";
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