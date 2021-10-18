#include "bingo_mmf_storage.h"
#include "base_cpp/exception.h"

using namespace bingo;

thread_local int MMFStorage::databaseId;

int MMFStorage::getDatabaseId()
{
    return databaseId;
}

void MMFStorage::setDatabaseId(int db)
{
    databaseId = db;
}

MMFStorage::~MMFStorage()
{
    close();
}

void MMFStorage::create(const char* filename, size_t min_size, size_t max_size, const char* header, int index_id)
{
    size_t header_len = strlen(header);

    _mm_files.clear();

    if (header_len >= max_header_len)
        throw Exception("MMfStorage: create(): Too long header");

    BingoAllocator::_create(filename, min_size, max_size, max_header_len, _mm_files, index_id);

    BingoPtr<char> header_ptr(0, 0);
    strcpy(header_ptr.ptr(), header);
}

void MMFStorage::load(const char* filename, int index_id, bool read_only)
{
    _mm_files.clear();
    BingoAllocator::_load(filename, max_header_len, _mm_files, index_id, read_only);
}

void MMFStorage::close()
{
    for (auto& mm_file : _mm_files)
    {
        mm_file.close();
    }
    _mm_files.clear();
}
