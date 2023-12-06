#ifndef __monomers_lib__
#define __monomers_lib__

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "base_cpp/exception.h"

namespace indigo
{
    // composite unordered_map key for pair
    const auto kCompositeKeyHashMagicNumber = 0x9e3779b9;
    template <class T>
    inline void hash_combine(size_t& seed, T const& v)
    {
        seed ^= std::hash<T>()(v) + kCompositeKeyHashMagicNumber + (seed << 6) + (seed >> 2);
    }

    // universal fuctor for pair hash calculation
    struct pair_hash
    {
        template <class T1, class T2>
        size_t operator()(const std::pair<T1, T2>& p) const
        {
            size_t seed = 0;
            hash_combine(seed, p.first);
            hash_combine(seed, p.second);
            return seed;
        }
    };

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

    using NucleotideComponentKey = std::pair<NucleotideComponentType, std::string>;
    using NucleotideKey = std::pair<NucleotideType, std::string>;
    using MonomersLib = std::unordered_map<NucleotideComponentKey, NucleotideComponent, pair_hash>;
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
        static const std::string& classToStr(NucleotideComponentType comp_type);
        static const std::unordered_map<NucleotideComponentType, std::string> kNucleotideComponentTypeStr;

    private:
        MonomerTemplates();
        void initializeMonomers();
        ~MonomerTemplates() = default;
        MonomersLib _monomers_lib;
        std::unordered_map<std::string, NucleotideComponentType> _component_types;
        std::unordered_map<NucleotideKey, GranularNucleotide, pair_hash> _nucleotides_lib;
    };
}

#endif