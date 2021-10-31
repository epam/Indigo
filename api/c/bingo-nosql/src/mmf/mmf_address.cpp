#include "mmf_address.h"

using namespace bingo;

const MMFAddress MMFAddress::null = MMFAddress(-1, -1);

MMFAddress::MMFAddress(int f_id, ptrdiff_t off) noexcept : file_id(f_id), offset(off)
{
}

bool MMFAddress::operator==(const MMFAddress& other) const
{
    return (file_id == other.file_id) && (offset == other.offset);
}

bool MMFAddress::operator!=(const MMFAddress& other) const
{
    return !operator==(other);
}
