#include "mmf_storage.h"

#include "base_cpp/exception.h"

#include "mmf_allocator.h"
#include "mmf_ptr.h"

using namespace bingo;

MMFStorage::~MMFStorage()
{
    close();
}

void MMFStorage::create(const char* filename, size_t min_size, size_t max_size, const char* header, int index_id)
{
    size_t header_len = strlen(header);

    _mm_files.clear();

    if (header_len >= max_header_len)
        throw indigo::Exception("MMfStorage: create(): Too long header");

    _allocator = MMFAllocator::create(filename, min_size, max_size, max_header_len, _mm_files, index_id);

    MMFPtr<char> header_ptr(0, 0);
    strcpy(header_ptr.ptr(), header);
}

void MMFStorage::load(const char* filename, int index_id, bool read_only)
{
    _mm_files.clear();
    _allocator = MMFAllocator::load(filename, max_header_len, _mm_files, index_id, read_only);
}

void MMFStorage::close()
{
    for (auto& mm_file : _mm_files)
    {
        mm_file.close();
    }
    _mm_files.clear();
}
