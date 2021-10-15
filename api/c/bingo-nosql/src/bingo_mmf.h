#ifndef __bingo_mmf__
#define __bingo_mmf__

#include <new>
#include <stdio.h>
#include <string>

namespace bingo
{
    class MMFile
    {
    public:
        MMFile();

        ~MMFile();

        void open(const char* filename, size_t buf_size, bool create_flag, bool read_only);

        void resize(size_t new_size);

        void* ptr();
        const void* ptr() const;

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
}; // namespace bingo

#endif // __bingo_mmf__
