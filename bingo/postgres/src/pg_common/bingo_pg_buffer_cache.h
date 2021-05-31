#ifndef _BINGO_PG_BUFFER_CASHE__
#define _BINGO_PG_BUFFER_CASHE__

extern "C"
{
#include "c.h"
#include "storage/itemptr.h"
}

#ifdef qsort
#undef qsort
#endif

#include "base_cpp/exception.h"
#include "bingo_pg_buffer.h"
#include "bingo_pg_ext_bitset.h"
#include "bingo_postgres.h"
/*
 * Class for data buffers handling
 */
class BingoPgBufferCache
{
public:
    BingoPgBufferCache(int block_id, PG_OBJECT index_ptr, bool write);
    virtual ~BingoPgBufferCache()
    {
    }

    int getBlockIdx() const
    {
        return _blockId;
    }
    BingoPgBuffer& getBuffer()
    {
        return _buffer;
    }

private:
    BingoPgBufferCache(const BingoPgBufferCache&); // no implicit copy

protected:
    int _blockId;
    PG_OBJECT _index;
    bool _write;
    BingoPgBuffer _buffer;
};
/*
 * Map buffers handling
 */
class BingoPgBufferCacheMap : public BingoPgBufferCache
{
public:
    /*
     * Tid mapping = tid + cmf + xyz
     */
    typedef struct BingoMapData
    {
        ItemPointerData tid_map;
        ItemPointerData cmf_map;
        ItemPointerData xyz_map;
    } BingoMapData;

    BingoPgBufferCacheMap(int block_id, PG_OBJECT index_ptr, bool write);
    virtual ~BingoPgBufferCacheMap();

    /*
     * Setters
     */
    void setTidItem(int map_idx, ItemPointerData& tid_item);
    void setCmfItem(int map_idx, ItemPointerData& cmf_item);
    void setXyzItem(int map_idx, ItemPointerData& xyz_item);
    /*
     * Getters
     */
    void getTidItem(int map_idx, ItemPointerData& tid_item);
    void getCmfItem(int map_idx, ItemPointerData& cmf_item);
    void getXyzItem(int map_idx, ItemPointerData& xyz_item);

    DECL_ERROR;

private:
    BingoPgBufferCacheMap(const BingoPgBufferCacheMap&); // no implicit copy

    void _checkMapIdx(int map_idx);

    indigo::Array<BingoMapData> _cache;
};
/*
 * Finger buffers handling
 */
class BingoPgBufferCacheFp : public BingoPgBufferCache
{
public:
    BingoPgBufferCacheFp(int block_id, PG_OBJECT index_ptr, bool write);
    virtual ~BingoPgBufferCacheFp();

    void setBit(int str_idx, bool value);
    bool getBit(int str_idx);
    /*
     * Main bit processing
     */
    void andWithBitset(BingoPgExternalBitset& ext_bitset);

    void getCopy(BingoPgExternalBitset& other);

    DECL_ERROR;

private:
    BingoPgBufferCacheFp(const BingoPgBufferCacheFp&); // no implicit copy

    BingoPgExternalBitset _cache;
};
/*
 * Biniary buffers handling
 */
class BingoPgBufferCacheBin : public BingoPgBufferCache
{
public:
    /*
     * Max size is rewrite BLCKSZ because there is int for keeping data length (stored in the begining of the buffer)
     */
    enum
    {
        MAX_SIZE = 8140,
        BUFFER_SIZE = 8150
    };

    BingoPgBufferCacheBin(int block_id, PG_OBJECT index_ptr, bool write);
    virtual ~BingoPgBufferCacheBin();

    /*
     * Returns true if enough space for adding a new structure with given size
     */
    bool isEnoughSpace(int size);
    /*
     * Add cmf to the buffer. Returns offset for a added cmf
     */
    unsigned short addBin(std::string& bin_buf);
    unsigned short writeBin(std::string& bin_buf);

    /*
     * Get cmf from a buffer
     */
    void readBin(unsigned short offset, std::string& result);

    DECL_ERROR;

private:
    BingoPgBufferCacheBin(const BingoPgBufferCacheBin&); // no implicit copy

    void _writeCache();
    void _readCache();

    std::string _cache;
};

#endif /* BUFFER_CASHE_H */
