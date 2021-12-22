#ifndef __bingo_fp_storage__
#define __bingo_fp_storage__

#include <fstream>
#include <vector>

#include "mmf/mmf_array.h"
#include "mmf/mmf_ptr.h"

namespace bingo
{
    class TranspFpStorage
    {
    public:
        TranspFpStorage(int fp_size, int block_size, int small_base_size);

        static MMFAddress create(MMFPtr<TranspFpStorage>& ptr, int fp_size, int block_size, int small_base_size);

        static void load(MMFPtr<TranspFpStorage>& ptr, MMFAddress offset);

        void add(const byte* fp);

        int getBlockSize(void) const;

        const byte* getBlock(int idx);

        int getBlockCount() const;

        const byte* getIncrement() const;

        int getIncrementSize(void) const;

        int getIncrementCapacity(void) const;

        int getPackCount() const;

        virtual ~TranspFpStorage();

        MMFArray<int>& getFpBitUsageCounts();

    protected:
        int _fp_size;
        int _block_count;
        int _block_size;
        int _pack_count;
        bool _small_flag;
        MMFArray<MMFPtr<byte>> _storage;

        MMFPtr<byte> _inc_buffer;
        int _inc_size;
        int _inc_fp_count;
        int _small_inc_size;

        MMFArray<int> _fp_bit_usage_counts;

        void _createFpStorage(int fp_size, int inc_fp_capacity, const char* inc_filename);

        void _addIncToStorage();
    };
}; // namespace bingo

#endif /* __bingo_fp_storage__ */
