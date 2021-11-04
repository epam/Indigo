#include "mmf_allocator.h"

#include <cmath>
#include <fstream>

#include "base_c/bitarray.h"
#include "base_cpp/exception.h"

#include "mmf_ptr.h"

using namespace bingo;
using namespace indigo;

thread_local MMFAllocator* MMFAllocator::_current_allocator = nullptr;
thread_local int MMFAllocator::_current_db_id = -1;

int MMFAllocator::getAllocatorDataSize()
{
    return sizeof(MMFAllocatorData);
}

void MMFAllocator::create(const char* filename, size_t min_size, size_t max_size, const char* index_type, int index_id)
{
    auto inst = std::make_unique<MMFAllocator>();

    inst->_mm_files.emplace_back(std::make_unique<MMFile>(_genFilename(0, filename), min_size, true, false));
    MMFile& file = *inst->_mm_files.at(0);
    const auto* mmf_ptr = file.ptr();
    if ((mmf_ptr == nullptr) || (min_size == 0) || (min_size < sizeof(MMFAllocator)))
        throw Exception("MMFAllocator: Incorrect instance initialization");

    MMFAllocatorData* allocator_data = static_cast<MMFAllocatorData*>(file.ptr(MAX_HEADER_LEN));
    new (allocator_data) MMFAllocatorData();
    allocator_data->_free_off = MAX_HEADER_LEN + sizeof(MMFAllocatorData);
    allocator_data->_min_file_size = min_size;
    allocator_data->_max_file_size = max_size;
    allocator_data->_cur_file_id = 0;
    inst->_filename.assign(filename);
    inst->_addHeader(index_type);
    {
        auto allocators = sf::xlock_safe_ptr(_allocators());
        allocators->emplace(index_id, std::move(inst));
    }

    setDatabaseId(index_id);
}

void MMFAllocator::load(const char* filename, int index_id, bool read_only)
{
    auto name = _genFilename(0, filename);
    std::ifstream fstream(name.c_str(), std::ios::binary | std::ios::ate);

    size_t size = fstream.tellg();

    auto inst = std::make_unique<MMFAllocator>();

    inst->_mm_files.emplace_back(std::make_unique<MMFile>(name, size, false, read_only));
    MMFile& file = *inst->_mm_files.at(0);
    const auto* mmf_ptr = file.ptr();
    if ((mmf_ptr == nullptr) || (size == 0) || (size < sizeof(MMFAllocator)))
    {
        throw Exception("MMFAllocator: Incorrect instance initialization");
    }

    auto* allocator_data = static_cast<MMFAllocatorData*>(file.ptr(MAX_HEADER_LEN));
    inst->_filename.assign(filename);
    for (auto i = 1; i < allocator_data->_cur_file_id + 1; i++)
    {
        size_t file_size = _getFileSize(i, allocator_data->_min_file_size, allocator_data->_max_file_size, allocator_data->_existing_files);
        inst->_mm_files.emplace_back(std::make_unique<MMFile>(_genFilename(i, inst->_filename.c_str()), file_size, false, read_only));
    }

    {
        auto allocators = sf::xlock_safe_ptr(_allocators());
        allocators->emplace(index_id, std::move(inst));
    }
    setDatabaseId(index_id);
}

const void* MMFAllocator::get(int file_id, ptrdiff_t offset) const
{
    return _mm_files.at(static_cast<int>(file_id))->ptr(offset);
}

void* MMFAllocator::get(int file_id, ptrdiff_t offset)
{
    return _mm_files.at(static_cast<int>(file_id))->ptr(offset);
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
    auto* allocator_data = static_cast<MMFAllocatorData*>(_mm_files.at(0)->ptr(MAX_HEADER_LEN));

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
        throw Exception("MMFAllocator: Too big allocation size");

    _mm_files.emplace_back(std::make_unique<MMFile>(_genFilename(_mm_files.size(), _filename.c_str()), file_size, true, false));

    allocator_data->_cur_file_id++;
    allocator_data->_free_off = 0;
}

std::string MMFAllocator::_genFilename(int idx, const char* filename)
{
    std::ostringstream name_str;
    name_str << filename;
    name_str << idx;
    return name_str.str();
}

void MMFAllocator::close()
{
    auto allocators = sf::xlock_safe_ptr(_allocators());
    allocators->erase(_current_db_id);
}

MMFAllocator& MMFAllocator::getAllocator()
{
    return *_current_allocator;
}

void MMFAllocator::setDatabaseId(int db_id)
{
    if (_current_db_id != db_id)
    {
        _current_db_id = db_id;
        auto allocators = sf::xlock_safe_ptr(_allocators());
        _current_allocator = allocators->at(db_id).get();
    }
}

void MMFAllocator::_addHeader(const char* header)
{
    const auto header_len = std::strlen(header);
    MMFPtr<char> header_ptr(0, 0);
    std::strcpy(header_ptr.ptr(*this), header);
    if (header_len >= MAX_HEADER_LEN)
    {
        throw indigo::Exception("MMFStorage: create(): Too long header");
    }
}

sf::safe_shared_hide_obj<std::unordered_map<int, std::unique_ptr<MMFAllocator>>>& MMFAllocator::_allocators()
{
    static sf::safe_shared_hide_obj<std::unordered_map<int, std::unique_ptr<MMFAllocator>>> allocators;
    return allocators;
}
