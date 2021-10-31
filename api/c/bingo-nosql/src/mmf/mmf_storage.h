#ifndef __bingo_mmf_storage__
#define __bingo_mmf_storage__

#include <memory>
#include <vector>

#include "mmfile.h"

namespace bingo
{
    class MMFAllocator;

    class MMFStorage
    {
    public:
        static constexpr const int MAX_HEADER_LEN = 128;

        MMFStorage() = default;
        ~MMFStorage();

        void create(const char* header, MMFAllocator& allocator);

        void load();

        void close();

        std::vector<MMFile> _mm_files;
    };
}; // namespace bingo

#endif // __bingo_mmf_storage__
