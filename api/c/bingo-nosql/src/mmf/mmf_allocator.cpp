#include "mmf_allocator.h"

#include <fstream>
#include <cmath>
#include <sstream>

#include "base_c/bitarray.h"
#include "base_cpp/exception.h"

using namespace bingo;
using namespace indigo;

int MMFAllocator::getAllocatorDataSize()
{
    return sizeof(MMFAllocatorData);
}

std::unique_ptr<MMFAllocator> MMFAllocator::create(const char* filename, size_t min_size, size_t max_size, size_t alloc_off, std::vector<MMFile>& mm_files,
                                                       int index_id)
{
    MMFile file;

    std::string name;
    _genFilename(0, filename, name);

    file.open(name.c_str(), min_size, true, false);

    const auto* mmf_ptr = file.ptr();

    if ((mmf_ptr == nullptr) || (min_size == 0) || (min_size < sizeof(MMFAllocator)))
        throw Exception("BingoAllocator: Incorrect instance initialization");

    auto inst = std::make_unique<MMFAllocator>();
    inst->_data_offset = alloc_off;
    MMFAllocatorData* allocator_data = (MMFAllocatorData*)(mmf_ptr + alloc_off);
    new (allocator_data) MMFAllocatorData();
    inst->_mm_files = &mm_files;
    allocator_data->_free_off = alloc_off + sizeof(MMFAllocatorData);
    allocator_data->_min_file_size = min_size;
    allocator_data->_max_file_size = max_size;
    allocator_data->_cur_file_id = 0;
    inst->_mm_files->push_back(file);
    inst->_filename.assign(filename);
    inst->_index_id = index_id;
    return inst;
    //    {
    //        auto instances = sf::xlock_safe_ptr(_instances);
    //        instances->emplace(MMFStorage::getDatabaseId(), std::move(inst));
    //    }
}

std::unique_ptr<MMFAllocator> MMFAllocator::load(const char* filename, size_t alloc_off, std::vector<MMFile>& mm_files, int index_id, bool read_only)
{
    std::string name;
    _genFilename(0, filename, name);

    std::ifstream fstream(name.c_str(), std::ios::binary | std::ios::ate);

    size_t size = fstream.tellg();

    MMFile file;

    file.open(name.c_str(), size, false, read_only);

    const auto* mmf_ptr = file.ptr();

    if ((mmf_ptr == nullptr) || (size == 0) || (size < sizeof(MMFAllocator)))
        throw Exception("BingoAllocator: Incorrect instance initialization");

    auto inst = std::make_unique<MMFAllocator>();
    MMFAllocatorData* allocator_data = (MMFAllocatorData*)(mmf_ptr + alloc_off);
    inst->_data_offset = alloc_off;
    inst->_mm_files = &mm_files;
    inst->_mm_files->push_back(file);
    inst->_filename.assign(filename);
    inst->_index_id = index_id;
    for (int i = 1; i < (int)allocator_data->_cur_file_id + 1; i++)
    {
        _genFilename(i, inst->_filename.c_str(), name);
        inst->_mm_files->emplace_back();
        size_t file_size = _getFileSize(i, allocator_data->_min_file_size, allocator_data->_max_file_size, allocator_data->_existing_files);
        inst->_mm_files->at(inst->_mm_files->size() - 1).open(name.c_str(), file_size, false, read_only);
    }
    return inst;
    //    sf::xlock_safe_ptr(_instances)->emplace(MMFStorage::getDatabaseId(), std::move(inst));
}

// BingoAllocator* BingoAllocator::_getInstance()
//{
//     const auto database_id = MMFStorage::getDatabaseId();
//     const auto instances = sf::slock_safe_ptr(_instances);
//     if (instances->count(database_id) == 0)
//     {
//         throw Exception("BingoAllocator: Incorrect session id");
//     }
//     auto* allocator = instances->at(database_id).get();
//     return allocator;
// }

const byte* MMFAllocator::get(int file_id, ptrdiff_t offset) const
{
    const auto* file_ptr = _mm_files->at(static_cast<int>(file_id)).ptr();
    return file_ptr + offset;
}

byte* MMFAllocator::get(int file_id, ptrdiff_t offset)
{
    auto* file_ptr = _mm_files->at(static_cast<int>(file_id)).ptr();
    return file_ptr + offset;
}

size_t MMFAllocator::_getFileSize(size_t idx, size_t min_size, size_t max_size, dword existing_files)
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

void MMFAllocator::_addFile(size_t alloc_size)
{
    auto* mmf_ptr = _mm_files->at(0).ptr();
    auto* allocator_data = reinterpret_cast<MMFAllocatorData*>(mmf_ptr + _data_offset);

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

    _mm_files->emplace_back();

    std::string name;
    _genFilename(_mm_files->size() - 1, _filename.c_str(), name);
    _mm_files->at(_mm_files->size() - 1).open(name.c_str(), file_size, true, false);

    allocator_data->_cur_file_id++;
    allocator_data->_free_off = 0;
}

void MMFAllocator::_genFilename(int idx, const char* filename, std::string& out_name)
{
    std::ostringstream name_str;

    name_str << filename;
    name_str << idx;

    out_name.assign(name_str.str());
}

// void BingoAllocator::removeInstance()
//{
//     // const auto database_id = MMFStorage::getDatabaseId();
//     auto instances = sf::xlock_safe_ptr(_instances);
//     if (instances->count(database_id) > 0)
//     {
//         instances->erase(database_id);
//     }
// }
