#include "bingo_ptr.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "base_c/bitarray.h"

#include "bingo_mmf_storage.h"

using namespace indigo;
using namespace bingo;

sf::safe_shared_hide_obj<std::unordered_map<int, std::unique_ptr<BingoAllocator>>> BingoAllocator::_instances;

const BingoAddr BingoAddr::bingo_null = BingoAddr(-1, -1);

int BingoAllocator::getAllocatorDataSize()
{
    return sizeof(_BingoAllocatorData);
}

void BingoAllocator::_create(const char* filename, size_t min_size, size_t max_size, size_t alloc_off, ObjArray<MMFile>& mm_files, int index_id)
{
    MMFile file;

    std::string name;
    _genFilename(0, filename, name);

    file.open(name.c_str(), min_size, true, false);

    byte* mmf_ptr = (byte*)file.ptr();

    if ((mmf_ptr == 0) || (min_size == 0) || (min_size < sizeof(BingoAllocator)))
        throw Exception("BingoAllocator: Incorrect instance initialization");

    auto inst = new BingoAllocator();
    {
        sf::xlock_safe_ptr(_instances)->emplace(MMFStorage::getDatabaseId(), std::unique_ptr<BingoAllocator>(inst));
    }

    inst->_data_offset = alloc_off;
    _BingoAllocatorData* allocator_data = (_BingoAllocatorData*)(mmf_ptr + alloc_off);
    new (allocator_data) _BingoAllocatorData();
    inst->_mm_files = &mm_files;
    allocator_data->_free_off = alloc_off + sizeof(_BingoAllocatorData);
    allocator_data->_min_file_size = min_size;
    allocator_data->_max_file_size = max_size;
    allocator_data->_cur_file_id = 0;
    {
        inst->_mm_files->push(file);
    }
    inst->_filename.assign(filename);
    inst->_index_id = index_id;
}

void BingoAllocator::_load(const char* filename, size_t alloc_off, ObjArray<MMFile>& mm_files, int index_id, bool read_only)
{
    std::string name;
    _genFilename(0, filename, name);

    std::ifstream fstream(name.c_str(), std::ios::binary | std::ios::ate);

    size_t size = fstream.tellg();

    MMFile file;

    file.open(name.c_str(), size, false, read_only);

    byte* mmf_ptr = (byte*)file.ptr();

    if ((mmf_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
        throw Exception("BingoAllocator: Incorrect instance initialization");

    auto inst = new BingoAllocator();
    {
        sf::xlock_safe_ptr(_instances)->emplace(MMFStorage::getDatabaseId(), std::unique_ptr<BingoAllocator>(inst));
    }

    _BingoAllocatorData* allocator_data = (_BingoAllocatorData*)(mmf_ptr + alloc_off);

    inst->_data_offset = alloc_off;
    inst->_mm_files = &mm_files;
    {
        inst->_mm_files->push(file);
    }
    inst->_filename.assign(filename);
    inst->_index_id = index_id;

    for (int i = 1; i < (int)allocator_data->_cur_file_id + 1; i++)
    {
        _genFilename(i, inst->_filename.c_str(), name);

        MMFile& file = inst->_mm_files->push();

        size_t file_size = _getFileSize(i, allocator_data->_min_file_size, allocator_data->_max_file_size, allocator_data->_existing_files);

        file.open(name.c_str(), file_size, false, read_only);
    }
}

BingoAllocator* BingoAllocator::_getInstance()
{
    const auto database_id = MMFStorage::getDatabaseId();
    const auto instances = sf::slock_safe_ptr(_instances);
    if (instances->count(database_id) == 0)
    {
        throw Exception("BingoAllocator: Incorrect session id");
    }
    return instances->at(database_id).get();
}

byte* BingoAllocator::_get(size_t file_id, size_t offset) const
{
    // byte * mmf_ptr = (byte *)_mm_files->at(0).ptr();

    //_BingoAllocatorData *allocator_data = (_BingoAllocatorData *)(mmf_ptr + _data_offset);
    byte* file_ptr = (byte*)(_mm_files->at(static_cast<int>(file_id)).ptr());

    return file_ptr + offset;
}

BingoAllocator::BingoAllocator()
{
}

size_t BingoAllocator::_getFileSize(size_t idx, size_t min_size, size_t max_size, dword existing_files)
{
    int incr_f_count = (int)log(max_size / min_size);
    int i;
    size_t bit_cnt = 0;

    for (i = 0; i < incr_f_count; i++)
    {
        if (bitGetBit(&existing_files, i))
            bit_cnt++;
        if (bit_cnt == idx)
            break;
    }

    size_t file_size = (i == incr_f_count ? max_size : min_size * ((size_t)1 << idx));

    return file_size;
}

void BingoAllocator::_addFile(size_t alloc_size)
{
    byte* mmf_ptr = (byte*)_mm_files->at(0).ptr();

    _BingoAllocatorData* allocator_data = (_BingoAllocatorData*)(mmf_ptr + _data_offset);

    size_t cur_file_size =
        _getFileSize(allocator_data->_cur_file_id, allocator_data->_min_file_size, allocator_data->_max_file_size, allocator_data->_existing_files);

    size_t file_size = cur_file_size * 2;
    while (file_size <= allocator_data->_max_file_size)
    {
        if (alloc_size <= file_size)
        {
            int dgr = log(file_size / allocator_data->_min_file_size);
            bitSetBit(&allocator_data->_existing_files, dgr, 1);

            break;
        }

        file_size *= 2;
    }

    if (file_size > allocator_data->_max_file_size)
        file_size = allocator_data->_max_file_size;

    if (alloc_size > file_size)
        throw Exception("BingoAllocator: Too big allocation size");

    MMFile& file = _mm_files->push();

    std::string name;
    _genFilename(_mm_files->size() - 1, _filename.c_str(), name);
    file.open(name.c_str(), file_size, true, false);

    allocator_data->_cur_file_id++;
    allocator_data->_free_off = 0;
}

void BingoAllocator::_genFilename(int idx, const char* filename, std::string& out_name)
{
    std::ostringstream name_str;

    name_str << filename;
    name_str << idx;

    out_name.assign(name_str.str());
}
