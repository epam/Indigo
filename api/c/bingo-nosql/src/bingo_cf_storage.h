#ifndef __cf_storage__
#define __cf_storage__

#include <fstream>
#include <string>
#include <vector>

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/tlscont.h"

#include "mmf/mmf_address.h"
#include "mmf/mmf_array.h"
#include "mmf/mmf_ptr.h"

namespace bingo
{
    class ByteBufferStorage
    {
    public:
        ByteBufferStorage(int block_size);

        static MMFAddress create(MMFPtr<ByteBufferStorage>& cf_ptr, int block_size);

        static void load(MMFPtr<ByteBufferStorage>& cf_ptr, MMFAddress offset);

        const byte* get(int idx, int& len);
        bool is_record_ok(int idx);
        void add(const byte* data, int len, int idx);
        void remove(int idx);
        ~ByteBufferStorage();

    private:
        struct _Addr
        {
            uint64_t block_idx;
            uint64_t offset;
            int64_t len;
        };

        int _block_size;
        int _free_pos;
        MMFArray<MMFPtr<byte>> _blocks;
        MMFArray<_Addr> _addresses;
    };

    // This class used to backward compatibility
    // https://github.com/epam/Indigo/issues/3528
    class ByteBufferStorageShort
    {
    public:
        ByteBufferStorageShort(int block_size);

        static MMFAddress create(MMFPtr<ByteBufferStorageShort>& cf_ptr, int block_size);

        static void load(MMFPtr<ByteBufferStorageShort>& cf_ptr, MMFAddress offset);

        const byte* get(int idx, int& len);
        bool is_record_ok(int idx);
        void add(const byte* data, int len, int idx);
        void remove(int idx);
        ~ByteBufferStorageShort();

    private:
        struct _Addr
        {
            uint32_t block_idx;
            uint32_t offset;
            int32_t len;
        };

        int _block_size;
        int _free_pos;
        MMFArray<MMFPtr<byte>> _blocks;
        MMFArray<_Addr> _addresses;
    };
}; // namespace bingo

#endif /* __cf_storage__ */
