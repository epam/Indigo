#include "mmf_storage.h"

#include "base_cpp/exception.h"

#include "mmf_allocator.h"
#include "mmf_ptr.h"

using namespace bingo;

MMFStorage::~MMFStorage()
{
    close();
}

void MMFStorage::create(const char* header, MMFAllocator& allocator)
{
    const auto header_len = std::strlen(header);
    MMFPtr<char> header_ptr(0, 0);
    std::strcpy(header_ptr.ptr(allocator), header);
    if (header_len >= MAX_HEADER_LEN)
        throw indigo::Exception("MMFStorage: create(): Too long header");
}

void MMFStorage::load()
{
    _mm_files.clear();
}

void MMFStorage::close()
{
    for (auto& mm_file : _mm_files)
    {
        mm_file.close();
    }
    _mm_files.clear();
}
