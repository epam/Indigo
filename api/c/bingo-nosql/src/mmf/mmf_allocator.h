#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "base_c/defs.h"

#include "mmf_address.h"
#include "mmfile.h"

namespace bingo
{
    class MMFAllocator
    {
    public:
        friend std::unique_ptr<MMFAllocator> std::make_unique<MMFAllocator>();

        static int getAllocatorDataSize();

        static std::unique_ptr<MMFAllocator> create(const char* filename, size_t min_size, size_t max_size, size_t alloc_off, std::vector<MMFile>& mm_files,
                                                    int index_id);
        static std::unique_ptr<MMFAllocator> load(const char* filename, size_t alloc_off, std::vector<MMFile>& mm_files, int index_id, bool read_only);


        const byte* get(int file_id, ptrdiff_t offset) const;
        byte* get(int file_id, ptrdiff_t offset);

        template <typename T> MMFAddress allocate(int count = 1)
        {
            size_t alloc_size, file_idx, file_off, file_size;

            auto* mmf_ptr = _mm_files->at(0).ptr();
            auto* allocator_data = reinterpret_cast<MMFAllocatorData*>(mmf_ptr + _data_offset);

            alloc_size = sizeof(T) * count;

            file_idx = allocator_data->_cur_file_id;
            file_off = allocator_data->_free_off;
            file_size = _mm_files->at((int)file_idx).size();

            if (alloc_size > file_size - file_off)
                _addFile(alloc_size);

            file_idx = allocator_data->_cur_file_id;

            file_size = _mm_files->at((int)file_idx).size();

            size_t res_off = allocator_data->_free_off;
            size_t res_id = allocator_data->_cur_file_id;
            allocator_data->_free_off += alloc_size;

            if (allocator_data->_free_off == file_size)
                _addFile(0);

            return MMFAddress(res_id, res_off);
        }

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

        std::vector<MMFile>* _mm_files;

        ptrdiff_t _data_offset;

        std::string _filename;
        int _index_id;

        MMFAllocator() = default;

        void _addFile(size_t alloc_size);

        static size_t _getFileSize(size_t idx, size_t min_size, size_t max_size, dword sizes);

        static void _genFilename(int idx, const char* filename, std::string& out_name);
    };
}
