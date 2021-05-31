#ifndef _BINGO_PG_SECTION_H__
#define _BINGO_PG_SECTION_H__

#include "base_cpp/array.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_array.h"
#include "bingo_pg_buffer_cache.h"
#include "bingo_postgres.h"
#include "pg_bingo_context.h"

class BingoPgIndex;
class BingoPgFpData;
class BingoPgExternalBitset;

/*
 * Class for handling bingo postgres section
 * Section consists of:
 *    section meta info (1 block) |
 *    section removed bitset (1 block) |
 *    bits count buffers (16 blocks) |
 *    map buffers (64k / 500) |
 *    fp buffers (fp count) |
 *    binary buffers (dynamic)
 */
class BingoPgSection
{
public:
    enum
    {
        SECTION_META_PAGES = 2,
        SECTION_BITSNUMBER_PAGES = 16,
        SECTION_BITS_PER_BLOCK = 4000 /* 4000 * sizeof(unsigned short) < 8K*/
    };
    BingoPgSection(BingoPgIndex& bingo_idx, int idx_strategy, int offset);
    ~BingoPgSection();

    void clear();

    /*
     * Returns true if section can be extended
     */
    bool isExtended();
    /*
     * Add a structure to current section
     */
    void addStructure(BingoPgFpData&);

    /*
     * Getters
     */
    int getPagesCount() const;
    int getStructuresNumber() const
    {
        return _sectionInfo.n_structures;
    }

    void getSectionStructures(BingoPgExternalBitset& section_bitset);
    void removeStructure(int mol_idx);
    bool isStructureRemoved(int mol_idx);

    BingoPgBufferCacheMap& getMapBufferCache(int map_idx);
    BingoPgBufferCacheFp& getFpBufferCache(int fp_idx);
    BingoPgBufferCacheBin& getBinBufferCache(int bin_idx);

    void readSectionBitsCount(indigo::Array<int>& bits_count);

    const BingoSectionInfoData& getSectionInfo() const
    {
        return _sectionInfo;
    };

    DECL_ERROR;

private:
    BingoPgSection(const BingoPgSection&); // no implicit copy

    void _setCmfData(std::string& cmf_buf, int map_buf_idx, int map_idx);
    void _setXyzData(std::string& xyz_buf, int map_buf_idx, int map_idx);
    void _setBinData(std::string& buf, int& last_buf, ItemPointerData& item_data);
    void _setBitsCountData(unsigned short bits_count);

    BingoPgBufferCacheBin* _getBufferBin(int idx);

    PG_OBJECT _index;
    int _offset;
    int _idxStrategy;

    BingoSectionInfoData _sectionInfo;
    BingoPgBuffer _sectionInfoBuffer;
    indigo::AutoPtr<BingoPgBufferCacheFp> _existStructures;

    indigo::PtrArray<BingoPgBufferCacheFp> _buffersFp;
    indigo::PtrArray<BingoPgBufferCacheMap> _buffersMap;
    indigo::PtrArray<BingoPgBufferCacheBin> _buffersBin;

    indigo::Array<int> _offsetFp;
    indigo::Array<int> _offsetMap;
    indigo::Array<int> _offsetBin;

    indigo::ObjArray<BingoPgBuffer> _bitsCountBuffers;
};

#endif /* BINGO_PG_SECTION1_H */
