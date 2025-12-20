/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

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

    const std::string HelmHydrogenPair{"pair"};
    const std::string KetConnectionSingle{"single"};
    const std::string KetConnectionHydro{"hydrogen"};
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
