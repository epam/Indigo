#ifndef __monomers_lib__
#define __monomers_lib__

#include <memory>
#include <map>
#include <unordered_map>
#include <string>

#include "base_cpp/exception.h"

namespace indigo
{
    class BaseMolecule;

    enum class NucleotideComponentType
    {
        Sugar,
        Phosphate,
        Base
    };

    enum class NucleotideType
    {
        RNA,
        DNA
    };

    struct NucleotideComponent
    {
        NucleotideComponentType comp_type;
        std::string natreplace;
        std::shared_ptr<BaseMolecule> monomer;
    };

    using MonomersLib = std::unordered_map<std::string, NucleotideComponent>;
    using GranularNucleotide = std::unordered_map<NucleotideComponentType, std::reference_wrapper<MonomersLib::value_type>>;

    // singleton MonomerTemplates class

    class MonomerTemplates
    {
        DECL_ERROR;
    public:
        MonomerTemplates(const MonomerTemplates&) = delete;
        MonomerTemplates(MonomerTemplates&&) = delete;
        MonomerTemplates& operator=(const MonomerTemplates&) = delete;
        MonomerTemplates& operator=(MonomerTemplates&&) = delete;
        static const MonomerTemplates& _instance();
        static bool getNucleotideMonomer(NucleotideComponentType comp_type, std::string alias, BaseMolecule& bmol);
        static bool getNucleotideMonomer(std::string comp_type, std::string alias, BaseMolecule& bmol);
        static bool splitNucleotide(NucleotideType nucleo_type, std::string alias, GranularNucleotide& splitted_nucleotide);
        static bool splitNucleotide(std::string nucleo_type, std::string alias, GranularNucleotide& splitted_nucleotide);

    private:
        MonomerTemplates();
        void initializeMonomers();
        ~MonomerTemplates() = default;
        static std::string _getNucleotideMonomerId(NucleotideComponentType comp_type, std::string alias);
        static std::string _getNucleotideId(NucleotideType nucleo_type, std::string alias);

        MonomersLib _monomers_lib;
        std::unordered_map<std::string, NucleotideComponentType> _component_types;
        std::unordered_map<std::string, std::unordered_map<NucleotideComponentType, std::reference_wrapper<MonomersLib::value_type> >> _nucleotides_lib;
    };
}

#endif