#ifndef __monomers_lib__
#define __monomers_lib__

#include <memory>
#include <string>

namespace indigo
{
    class BaseMolecule;

    enum NucleotideComponentType
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

    // implicit conversion to underlying_type
    template <typename NucleotideComponentType>
    constexpr typename std::underlying_type<NucleotideComponentType>::type to_underlying(NucleotideComponentType e) noexcept
    {
        return static_cast<typename std::underlying_type<NucleotideComponentType>::type>(e);
    }

    class MonomerTemplates
    {
    public:
        MonomerTemplates(const MonomerTemplates&) = delete;
        MonomerTemplates(MonomerTemplates&&) = delete;
        MonomerTemplates& operator=(const MonomerTemplates&) = delete;
        MonomerTemplates& operator=(MonomerTemplates&&) = delete;
        static const MonomerTemplates& _instance();
        static bool getNucleotideMonomer(NucleotideComponentType comp_type, std::string alias, BaseMolecule& bmol);
        static bool getNucleotideMonomer(std::string comp_type, std::string alias, BaseMolecule& bmol);
        static bool splitNucleotide(NucleotideType nucleo_type, std::string alias,
                                    std::unordered_map<NucleotideComponentType, std::shared_ptr<BaseMolecule>>& splitted_nucleotide);

    private:
        MonomerTemplates();
        void initializeMonomers();
        ~MonomerTemplates() = default;
        static std::string _getNucleotideMonomerId(NucleotideComponentType comp_type, std::string alias);
        static std::string _getNucleotideId(NucleotideType nucleo_type, std::string alias);

        std::unordered_map<std::string, std::pair<NucleotideComponentType, std::shared_ptr<BaseMolecule>>> _monomers_lib;
        std::unordered_map<std::string, NucleotideComponentType> _component_types;
        std::unordered_map<std::string, std::unordered_map<NucleotideComponentType, std::shared_ptr<BaseMolecule>>> _nucleotides_lib;
    };
}

#endif