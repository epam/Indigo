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
        static constexpr const int max_header_len = 128;

        MMFStorage() = default;
        ~MMFStorage();

        void create(const char* filename, size_t min_size, size_t max_size, const char* header, int index_id);

        void load(const char* filename, int index_id, bool read_only);

        void close();

    private:
        std::vector<MMFile> _mm_files;
        std::unique_ptr<MMFAllocator> _allocator = nullptr;
    };
}; // namespace bingo

#endif // __bingo_mmf_storage__
