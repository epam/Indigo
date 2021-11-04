#pragma once

#include <cstddef>

namespace bingo
{
    struct MMFAddress
    {
        MMFAddress() = default;
        MMFAddress(int f_id, ptrdiff_t off) noexcept;

        bool operator==(const MMFAddress& other) const;
        bool operator!=(const MMFAddress& other) const;

        static const MMFAddress null;

        int file_id = -1;
        ptrdiff_t offset = -1;
    };
}
