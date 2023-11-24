#ifndef __monomers_lib__
#define __monomers_lib__

#include <memory>

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

namespace std
{
    template <>
    struct hash<indigo::NucleotideComponentType>
    {
        size_t operator()(const indigo::NucleotideComponentType& t) const
        {
            return size_t(t);
        }
    };
}

#endif