#ifndef __bingo_mmf_storage__
#define __bingo_mmf_storage__

#include "base_cpp/obj_array.h"
#include "bingo_mmf.h"
#include "bingo_ptr.h"

using namespace indigo;

namespace bingo
{
    class MMFStorage
    {
    public:
        static int getDatabaseId();
        static void setDatabaseId(int db);

        static const int max_header_len = 128;

        MMFStorage();

        void create(const char* filename, size_t min_size, size_t max_size, const char* header, int index_id);

        void load(const char* filename, BingoPtr<char> header_ptr, int index_id, bool read_only);

        void close();

    private:
        ObjArray<MMFile> _mm_files;
        bool _read_only;
        static thread_local int databaseId;
    };
}; // namespace bingo

#endif // __bingo_mmf_storage__
