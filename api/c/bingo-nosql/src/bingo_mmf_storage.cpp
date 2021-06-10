#include "bingo_mmf_storage.h"
#include "base_cpp/exception.h"
#include <fstream>
#include <new>
#include <stdio.h>

#include "base_cpp/tlscont.h"

using namespace bingo;

// TODO: implement real thread local storage - not session local
static _SIDManager _database_id;

int MMFStorage::getDatabaseId()
{
    return (int)_database_id.getSessionId();
}

void MMFStorage::setDatabaseId(int db)
{
    _database_id.setSessionId((int)db);
}

MMFStorage::MMFStorage()
{
}

void MMFStorage::create(const char* filename, size_t min_size, size_t max_size, const char* header, int index_id)
{
    size_t header_len = strlen(header);
    _mm_files.clear();

    if (header_len >= max_header_len)
        throw Exception("MMfStorage: create(): Too long header");

    BingoAllocator::_create(filename, min_size, max_size, max_header_len, &_mm_files, index_id);

    BingoPtr<char> header_ptr(0, 0);
    strcpy(header_ptr.ptr(), header);
}

void MMFStorage::load(const char* filename, BingoPtr<char> header_ptr, int index_id, bool read_only)
{
    _mm_files.clear();

    BingoAllocator::_load(filename, max_header_len, &_mm_files, index_id, read_only);

    header_ptr = BingoPtr<char>(0, 0);
}

void MMFStorage::close()
{
    for (int i = 0; i < _mm_files.size(); i++)
        _mm_files[i].close();
    _mm_files.clear();
}
