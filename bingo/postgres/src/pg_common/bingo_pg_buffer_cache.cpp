#include "bingo_pg_buffer_cache.h"

#include "bingo_pg_fix_post.h"
#include "bingo_pg_fix_pre.h"

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "bingo_pg_common.h"

using namespace indigo;

BingoPgBufferCache::BingoPgBufferCache(int block_id, PG_OBJECT index_ptr, bool write) : _blockId(block_id), _index(index_ptr), _write(write)
{
    /*
     * Create a new buffer if the write strategy
     */
    if (_write)
    {
        _buffer.writeNewBuffer(_index, _blockId);
        /*
         * Release pin on buffer
         */
        _buffer.clear();
    }
}

IMPL_ERROR(BingoPgBufferCacheMap, "bingo buffer cache");

BingoPgBufferCacheMap::BingoPgBufferCacheMap(int block_id, PG_OBJECT index_ptr, bool write) : BingoPgBufferCache(block_id, index_ptr, write)
{
    /*
     * Prepare cache
     */
    if (_write)
    {
        _cache.resize(BINGO_MOLS_PER_MAPBLOCK);
        _cache.zerofill();
    }
}

BingoPgBufferCacheMap::~BingoPgBufferCacheMap()
{
    if (_write)
    {
        /*
         * Write the cache only in the end
         */
        if (!_buffer.isReady())
        {
            _buffer.readBuffer(_index, _blockId, BINGO_PG_NOLOCK);
        }
        _buffer.changeAccess(BINGO_PG_WRITE);
        _buffer.formIndexTuple(_cache.ptr(), _cache.sizeInBytes());
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheMap::setTidItem(int map_idx, ItemPointerData& tid_item)
{
    _checkMapIdx(map_idx);
    if (_write)
    {
        _cache[map_idx].tid_map = tid_item;
    }
    else
    {
        /*
         * If read strategy then it is an update, so add data directly to the buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_WRITE);
        int data_len;
        BingoMapData* map_data = (BingoMapData*)_buffer.getIndexData(data_len);
        map_data[map_idx].tid_map = tid_item;
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheMap::setCmfItem(int map_idx, ItemPointerData& cmf_item)
{
    _checkMapIdx(map_idx);
    if (_write)
    {
        _cache[map_idx].cmf_map = cmf_item;
    }
    else
    {
        /*
         * If read strategy then it is an update, so add data directly to the buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_WRITE);
        int data_len;
        BingoMapData* map_data = (BingoMapData*)_buffer.getIndexData(data_len);
        map_data[map_idx].cmf_map = cmf_item;
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheMap::setXyzItem(int map_idx, ItemPointerData& xyz_item)
{
    _checkMapIdx(map_idx);
    if (_write)
    {
        _cache[map_idx].xyz_map = xyz_item;
    }
    else
    {
        /*
         * If read strategy then it is an update, so add data directly to the buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_WRITE);
        int data_len;
        BingoMapData* map_data = (BingoMapData*)_buffer.getIndexData(data_len);
        map_data[map_idx].xyz_map = xyz_item;
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheMap::getTidItem(int map_idx, ItemPointerData& tid_item)
{
    _checkMapIdx(map_idx);
    if (_write)
        tid_item = _cache[map_idx].tid_map;
    else
    {
        /*
         * Read data for a buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_READ);
        int data_len;
        BingoMapData* map_data = (BingoMapData*)_buffer.getIndexData(data_len);
        tid_item = map_data[map_idx].tid_map;
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheMap::getCmfItem(int map_idx, ItemPointerData& cmf_item)
{
    _checkMapIdx(map_idx);
    if (_write)
        cmf_item = _cache[map_idx].cmf_map;
    else
    {
        /*
         * Read data for a buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_READ);
        int data_len;
        BingoMapData* map_data = (BingoMapData*)_buffer.getIndexData(data_len);
        cmf_item = map_data[map_idx].cmf_map;
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheMap::getXyzItem(int map_idx, ItemPointerData& xyz_item)
{
    _checkMapIdx(map_idx);
    if (_write)
        xyz_item = _cache[map_idx].xyz_map;
    else
    {
        /*
         * Read data for a buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_READ);
        int data_len;
        BingoMapData* map_data = (BingoMapData*)_buffer.getIndexData(data_len);
        xyz_item = map_data[map_idx].xyz_map;
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheMap::_checkMapIdx(int map_idx)
{
    if (map_idx >= BINGO_MOLS_PER_MAPBLOCK)
        throw Error("internal error: map index %d is out of range %d", map_idx, BINGO_MOLS_PER_MAPBLOCK);
}

// Revview IMPL_ERROR
IMPL_ERROR(BingoPgBufferCacheFp, "bingo buffer cache fingerprints");

BingoPgBufferCacheFp::BingoPgBufferCacheFp(int block_id, PG_OBJECT index_ptr, bool write)
    : BingoPgBufferCache(block_id, index_ptr, write), _cache(BINGO_MOLS_PER_FINGERBLOCK)
{
    /*
     * Cache already prepared. Clean it
     */
    if (_write)
    {
        _cache.zeroFill();
    }
}

BingoPgBufferCacheFp::~BingoPgBufferCacheFp()
{
    if (_write)
    {
        /*
         * Write the cache only in the end
         */
        if (!_buffer.isReady())
        {
            _buffer.readBuffer(_index, _blockId, BINGO_PG_NOLOCK);
        }
        _buffer.changeAccess(BINGO_PG_WRITE);
        int data_len;
        void* data = _cache.serialize(data_len);
        _buffer.formIndexTuple(data, data_len);
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheFp::setBit(int str_idx, bool value)
{
    if (_write)
    {
        _cache.set(str_idx, value);
    }
    else
    {
        /*
         * If read strategy then it is an update, so add data directly to the buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_WRITE);
        int data_len;
        void* data = _buffer.getIndexData(data_len);
        _cache.deserialize(data, data_len, true);
        _cache.set(str_idx, value);
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

bool BingoPgBufferCacheFp::getBit(int str_idx)
{
    bool result;
    if (_write)
    {
        result = _cache.get(str_idx);
    }
    else
    {
        /*
         * If read strategy then it is an update, so add data directly to the buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_WRITE);
        int data_len;
        void* data = _buffer.getIndexData(data_len);
        _cache.deserialize(data, data_len, true);
        result = _cache.get(str_idx);
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
    return result;
}

void BingoPgBufferCacheFp::andWithBitset(BingoPgExternalBitset& ext_bitset)
{
    if (_write)
    {
        ext_bitset.andWith(_cache);
    }
    else
    {
        /*
         * Read data for a buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_READ);
        int data_len;
        void* data = _buffer.getIndexData(data_len);
        _cache.deserialize(data, data_len, true);
        /*
         * And with bitset
         */
        ext_bitset.andWith(_cache);
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgBufferCacheFp::getCopy(BingoPgExternalBitset& other)
{
    if (_write)
    {
        other.copy(_cache);
    }
    else
    {
        /*
         * Read data for a buffer
         */
        _buffer.readBuffer(_index, _blockId, BINGO_PG_READ);
        int data_len;
        void* data = _buffer.getIndexData(data_len);
        _cache.deserialize(data, data_len, true);
        /*
         * And with bitset
         */
        other.copy(_cache);
        _buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

// Revview IMPL_ERROR
IMPL_ERROR(BingoPgBufferCacheBin, "bingo buffer cache binary data");

BingoPgBufferCacheBin::BingoPgBufferCacheBin(int block_id, PG_OBJECT index_ptr, bool write) : BingoPgBufferCache(block_id, index_ptr, write)
{
}

BingoPgBufferCacheBin::~BingoPgBufferCacheBin()
{
    if (_write)
    {
        /*
         * Write the cache only in the end
         */
        if (!_buffer.isReady())
        {
            _buffer.readBuffer(_index, _blockId, BINGO_PG_NOLOCK);
        }
        /*
         * Store max size always
         */
        _buffer.changeAccess(BINGO_PG_WRITE);
        _buffer.formEmptyIndexTuple(BUFFER_SIZE);
        _buffer.changeAccess(BINGO_PG_NOLOCK);
        _writeCache();
    }
}

bool BingoPgBufferCacheBin::isEnoughSpace(int size)
{
    bool result;
    if (!_write)
    {
        /*
         * Read cache size from a buffer
         */
        _readCache();
    }
    result = (size + _cache.size()) < MAX_SIZE;
    return result;
}

unsigned short BingoPgBufferCacheBin::addBin(indigo::Array<char>& bin_buf)
{

    /*
     * If read strategy then it is an update so read the buffer in this function also
     */
    if (!isEnoughSpace(bin_buf.sizeInBytes()))
        throw Error("internal error: can not add cmf to the cache because is not enough space");

    /*
     * Prepare output offset
     */
    unsigned short result = _cache.size();

    /*
     * Prepare new array = size + buf
     */
    indigo::Array<char> out_arr;
    indigo::ArrayOutput ao(out_arr);
    BingoPgCommon::DataProcessing::handleArray(bin_buf, 0, &ao);
    /*
     * Store data with it size
     */
    _cache.concat(out_arr);
    /*
     * If read strategy then it is an update so write the buffer
     */
    if (!_write)
    {
        _writeCache();
    }
    return result;
}

unsigned short BingoPgBufferCacheBin::writeBin(indigo::Array<char>& bin_buf)
{
    /*
     * If read strategy then it is an update so read the buffer in this function also
     */
    if (bin_buf.sizeInBytes() > MAX_SIZE)
        throw Error("internal error: can not add bin to the cache because is not enough space");

    /*
     * Prepare output offset
     */
    unsigned short result = 0;

    /*
     * Prepare new array = size + buf
     */
    indigo::Array<char> out_arr;
    indigo::ArrayOutput ao(out_arr);
    BingoPgCommon::DataProcessing::handleArray(bin_buf, 0, &ao);
    /*
     * Store data with it size
     */
    _cache.copy(out_arr);
    /*
     * If read strategy then it is an update so write the buffer
     */
    if (!_write)
    {
        _writeCache();
    }
    return result;
}

void BingoPgBufferCacheBin::readBin(unsigned short offset, indigo::Array<char>& result)
{
    if (!_write)
    {
        _readCache();
    }
    /*
     * Read buffer from the given offset
     */
    const char* data = _cache.ptr();
    int data_len = _cache.sizeInBytes();
    BufferScanner sc(data + offset, data_len - offset);
    BingoPgCommon::DataProcessing::handleArray(result, &sc, 0);
}

void BingoPgBufferCacheBin::_writeCache()
{
    /*
     * Write size and cache itself
     */
    if (!_buffer.isReady())
        _buffer.readBuffer(_index, _blockId, BINGO_PG_NOLOCK);

    _buffer.changeAccess(BINGO_PG_WRITE);
    int data_len;
    char* buf_data = (char*)_buffer.getIndexData(data_len);
    int cache_size = _cache.sizeInBytes();
    /*
     * Store size
     */
    memcpy(buf_data, &cache_size, sizeof(int));
    /*
     * Store cache data
     */
    memcpy(buf_data + sizeof(int), _cache.ptr(), cache_size);
    _buffer.changeAccess(BINGO_PG_NOLOCK);
}

void BingoPgBufferCacheBin::_readCache()
{
    /*
     * If buffer was readed then cache is already fulfiled
     */
    if (_buffer.isReady())
        return;
    /*
     * Read size and cache itself
     */
    _buffer.readBuffer(_index, _blockId, BINGO_PG_READ);
    int data_len;
    const char* data = (const char*)_buffer.getIndexData(data_len);
    int cache_size;
    /*
     * Read size
     */
    memcpy(&cache_size, data, sizeof(int));
    _cache.clear();
    _cache.resize(cache_size);
    /*
     * Read cache data
     */
    memcpy(_cache.ptr(), data + sizeof(int), cache_size);
    _buffer.changeAccess(BINGO_PG_NOLOCK);
}
