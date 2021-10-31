#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <safe_ptr.h>

#include "base_c/defs.h"

#include "mmf_address.h"
#include "mmf_storage.h"
#include "mmfile.h"

namespace bingo
{
    class MMFAllocator
    {
    public:
        friend std::unique_ptr<MMFAllocator> std::make_unique<MMFAllocator>();

        static int getAllocatorDataSize();

        static void create(const char* filename, size_t min_size, size_t max_size, const char* index_type, int index_id);
        static void load(const char* filename, int index_id, bool read_only);
        void close();

        static MMFAllocator& getAllocator();

        const byte* get(int file_id, ptrdiff_t offset) const;
        byte* get(int file_id, ptrdiff_t offset);

        template <typename T> MMFAddress allocate(int count = 1)
        {
            size_t alloc_size, file_idx, file_off, file_size;

            auto* mmf_ptr = _mmf_storage._mm_files.at(0).ptr();
            auto* allocator_data = reinterpret_cast<MMFAllocatorData*>(mmf_ptr + _data_offset);

            alloc_size = sizeof(T) * count;

            file_idx = allocator_data->_cur_file_id;
            file_off = allocator_data->_free_off;
            file_size = _mmf_storage._mm_files.at(file_idx).size();

            if (alloc_size > file_size - file_off)
                _addFile(alloc_size);

            file_idx = allocator_data->_cur_file_id;

            file_size = _mmf_storage._mm_files.at(file_idx).size();

            size_t res_off = allocator_data->_free_off;
            size_t res_id = allocator_data->_cur_file_id;
            allocator_data->_free_off += alloc_size;

            if (allocator_data->_free_off == file_size)
                _addFile(0);

            return MMFAddress(res_id, res_off);
        }

        static void setDatabaseId(int db_id);

    private:
        struct MMFAllocatorData
        {
            MMFAllocatorData() : _min_file_size(0), _max_file_size(0), _cur_file_id(0), _existing_files(0), _free_off(0)
            {
            }

            size_t _min_file_size;
            size_t _max_file_size;
            size_t _cur_file_id;
            dword _existing_files;
            size_t _free_off;
        };

        MMFAllocator() = default;

        void _addFile(size_t alloc_size);

        static size_t _getFileSize(size_t idx, size_t min_size, size_t max_size, dword sizes);

        static void _genFilename(int idx, const char* filename, std::string& out_name);

        ptrdiff_t _data_offset;
        std::string _filename;
        int _index_id;
        MMFStorage _mmf_storage;

        static sf::safe_shared_hide_obj<std::unordered_map<int, std::unique_ptr<MMFAllocator>>> _allocators;
        static thread_local MMFAllocator* _current_allocator;
        static thread_local int _current_db_id;
    };
}
