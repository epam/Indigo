#ifndef __bingo_fp_storage__
#define __bingo_fp_storage__

#include "bingo_ptr.h"
#include <fstream>
#include <iostream>
#include <vector>

namespace bingo
{
    class TranspFpStorage
    {
    public:
        TranspFpStorage(int fp_size, int block_size, int small_base_size);

        static BingoAddr create(BingoPtr<TranspFpStorage>& ptr, int fp_size, int block_size, int small_base_size);

        static void load(BingoPtr<TranspFpStorage>& ptr, BingoAddr offset);

        void add(const byte* fp);

        int getBlockSize(void) const;

        const byte* getBlock(int idx);

        int getBlockCount() const;

        const byte* getIncrement() const;

        int getIncrementSize(void) const;

        int getIncrementCapacity(void) const;

        int getPackCount() const;

        virtual ~TranspFpStorage();

        BingoArray<int>& getFpBitUsageCounts();

    protected:
        int _fp_size;
        int _block_count;
        int _block_size;
        int _pack_count;
        bool _small_flag;
        BingoArray<BingoPtr<byte>> _storage;

        BingoPtr<byte> _inc_buffer;
        int _inc_size;
        int _inc_fp_count;
        int _small_inc_size;

        BingoArray<int> _fp_bit_usage_counts;

        void _createFpStorage(int fp_size, int inc_fp_capacity, const char* inc_filename);

        void _addIncToStorage();
    };
}; // namespace bingo

#endif /* __bingo_fp_storage__ */
