#ifndef __monomers_defs__
#define __monomers_defs__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

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

    enum class MonomerClass
    {
        AminoAcid,
        Sugar,
        Phosphate,
        Base,
        Terminator,
        Linker,
        Unknown,
        CHEM,
        DNA,
        RNA
    };

    enum class NucleotideType
    {
        RNA,
        DNA
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
