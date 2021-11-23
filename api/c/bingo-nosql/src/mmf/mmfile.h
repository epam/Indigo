#pragma once

#include <cstddef>
#include <cstdio>
#include <new>
#include <string>

#include <common/base_c/defs.h>

namespace bingo
{
    class MMFile
    {
    public:
        MMFile(std::string filename, size_t buf_size, bool create_flag, bool read_only);
        ~MMFile();

        MMFile& operator=(const MMFile&) = delete;
        MMFile(const MMFile&) = delete;
        MMFile& operator=(MMFile&&) = delete;
        MMFile(MMFile&&) = default;
        MMFile() = delete;

        void* ptr(ptrdiff_t offset = 0);
        const void* ptr(ptrdiff_t offset = 0) const;

        const char* name() const;

        size_t size() const;

    private:
#ifdef _WIN32
        void* _h_map_file;
        void* _h_file;
#elif (defined __GNUC__ || defined __APPLE__)
        int _fd;
#endif
        void* _ptr;
        std::string _filename;
        size_t _len;

        static char* _getSystemErrorMsg();
    };
}
