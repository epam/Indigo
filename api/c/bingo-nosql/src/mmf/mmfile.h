#pragma once

#include <cstdio>
#include <new>
#include <string>

#include <common/base_c/defs.h>

namespace bingo
{
    class MMFile
    {
    public:
        MMFile();
        ~MMFile() = default;

        void open(const char* filename, size_t buf_size, bool create_flag, bool read_only);

        byte* ptr();
        const byte* ptr() const;

        const char* name() const;

        size_t size() const;

        void close();

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