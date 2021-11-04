#include "mmfile.h"

#include <utility>

#include "base_cpp/exception.h"

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#elif (defined __GNUC__ || defined __APPLE__)
#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace bingo;
using namespace indigo;

void* MMFile::ptr(const ptrdiff_t offset)
{
    return static_cast<void*>(static_cast<byte*>(_ptr) + offset);
}

const void* MMFile::ptr(const ptrdiff_t offset) const
{
    return static_cast<void*>(static_cast<byte*>(_ptr) + offset);
}

const char* MMFile::name() const
{
    return _filename.c_str();
}

char* MMFile::_getSystemErrorMsg()
{
#ifdef _WIN32
    char* msg;
    DWORD dw = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL);
    return msg;
#elif (defined __GNUC__ || defined __APPLE__)
    return strerror(errno);
#endif
}

size_t MMFile::size() const
{
    return _len;
}

MMFile::MMFile(std::string filename, size_t buf_size, bool create_flag, bool read_only) : _len(buf_size), _filename(std::move(filename))
{
    if (create_flag)
    {
        std::remove(_filename.c_str());
    }

#ifdef _WIN32
    DWORD dwflags = GENERIC_READ | GENERIC_WRITE;

    if (read_only)
        dwflags = GENERIC_READ;

    _h_file = CreateFile((LPCSTR)_filename.c_str(), dwflags, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (_h_file == INVALID_HANDLE_VALUE)
        throw Exception("MMF: Could not open file. Error message: %s", _getSystemErrorMsg());

    dword access_info = PAGE_READWRITE;

    if (read_only)
        access_info = PAGE_READONLY;

    _h_map_file = CreateFileMapping(_h_file,               // use paging file
                                    NULL,                  // default security
                                    access_info,           // read/write access
                                    buf_size >> 32,        // maximum object size (high-order DWORD)
                                    buf_size & 0xFFFFFFFF, // maximum object size (low-order DWORD)
                                    0);                    // name of mapping object

    if (_h_map_file == NULL)
        throw Exception("MMF: Could not create file mapping object. Error message: %s", _getSystemErrorMsg());

    dword map_access_permission = FILE_MAP_ALL_ACCESS;
    if (read_only)
        map_access_permission = FILE_MAP_READ;

    _ptr = (char*)MapViewOfFile(_h_map_file, // handle to map object
                                map_access_permission, 0, 0, buf_size);

    if (_ptr == nullptr)
        throw Exception("MMF: Could not map view of file. Error message: %s", _getSystemErrorMsg());

#elif (defined __GNUC__ || defined __APPLE__)
    int flags;
    mode_t permissions = 0;

    if (read_only)
        flags = O_RDONLY;
    else
    {
        flags = O_RDWR | O_CREAT;
        permissions = S_IRUSR | S_IWUSR;
    }

    if ((_fd = open(_filename.c_str(), flags, permissions)) == -1)
        throw Exception("MMF: Could not open file. Error message: %s", _getSystemErrorMsg());

    auto trunc_res = ftruncate(_fd, _len);
    if (trunc_res < 0)
    {
        // TODO check result
    }

    int prot_flags = PROT_READ | PROT_WRITE;

    if (read_only)
        prot_flags = PROT_READ;

    _ptr = mmap(static_cast<caddr_t>(nullptr), _len, prot_flags, MAP_SHARED, _fd, 0);

    if (_ptr == MAP_FAILED)
    {
        _ptr = nullptr;
        throw Exception("MMF: Could not map view of file. Error message: %s", _getSystemErrorMsg());
    }
#endif
}

MMFile::~MMFile()
{
    //    std::cout << "~MMFile(" << this << ")" << std::endl;

#ifdef _WIN32
    if (_filename.size() != 0)
    {
        DeleteFile(_filename.c_str());
        _filename.clear();
    }

    if (_h_file != nullptr)
    {
        CloseHandle(_h_file);
        _h_file = nullptr;
    }

    if (_h_map_file != nullptr)
    {
        CloseHandle(_h_map_file);
        _h_map_file = nullptr;
    }

    if (_ptr != nullptr)
    {
        UnmapViewOfFile(_ptr);
        _ptr = nullptr;
    }
#elif (defined __GNUC__ || defined __APPLE__)
    if (_ptr != nullptr)
    {
        munmap(static_cast<caddr_t>(_ptr), _len);
        _ptr = nullptr;
    }

    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
#endif
}
