#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"
#include "storage/block.h"
#include "storage/itemptr.h"
}

#include "bingo_pg_fix_post.h"

#include "base_cpp/profiling.h"
#include "base_cpp/tlscont.h"
#include "bingo_pg_common.h"
#include "bingo_pg_index.h"
#include "bingo_pg_search_engine.h"
#include "bingo_pg_section.h"

using namespace indigo;

IMPL_ERROR(BingoPgSection, "bingo postgres section");

BingoPgSection::BingoPgSection(BingoPgIndex& bingo_idx, int idx_strategy, int offset)
    : _index(bingo_idx.getIndexPtr()), _offset(offset), _idxStrategy(idx_strategy)
{

    /*
     * Clear section metainfo
     */
    clear();

    /*
     * Prepare section strategy
     */
    bool write = (_idxStrategy == BingoPgIndex::BUILDING_STRATEGY);
    if (write)
    {
        /*
         * If write then create new buffers
         */
        _sectionInfoBuffer.writeNewBuffer(_index, offset);
        _sectionInfoBuffer.changeAccess(BINGO_PG_NOLOCK);
        _sectionInfo.n_blocks_for_map = bingo_idx.getMapSize();
        _sectionInfo.n_blocks_for_fp = bingo_idx.getFpSize();
        /*
         * Initialize existing structures fingerprint
         */
        _existStructures.reset(new BingoPgBufferCacheFp(offset + 1, _index, true));

        /*
         * Initialize bits number buffers
         */
        for (int idx = 0; idx < SECTION_BITSNUMBER_PAGES; ++idx)
        {
            BingoPgBuffer& bits_buffer = _bitsCountBuffers.push();
            bits_buffer.writeNewBuffer(_index, offset + idx + SECTION_META_PAGES);
            bits_buffer.formEmptyIndexTuple(SECTION_BITS_PER_BLOCK * sizeof(unsigned short));
            bits_buffer.changeAccess(BINGO_PG_NOLOCK);
        }
    }
    else
    {
        /*
         * Read section meta info
         */
        _sectionInfoBuffer.readBuffer(_index, _offset, BINGO_PG_READ);
        int data_len;
        BingoSectionInfoData* data = (BingoSectionInfoData*)_sectionInfoBuffer.getIndexData(data_len);
        _sectionInfo = *data;
        _sectionInfoBuffer.changeAccess(BINGO_PG_NOLOCK);

        _existStructures.reset(new BingoPgBufferCacheFp(offset + 1, _index, false));
    }
    int map_count = _sectionInfo.n_blocks_for_map;
    int fp_count = _sectionInfo.n_blocks_for_fp;
    int bin_count = _sectionInfo.n_blocks_for_bin;
    /*
     * Prepare cache arrays
     */
    _buffersMap.expand(map_count);
    _buffersFp.expand(fp_count);
    _buffersBin.expand(bin_count);
    /*
     * Prepare offset arrays
     */
    _offsetMap.expand(map_count);
    _offsetFp.expand(fp_count);
    _offsetBin.expand(bin_count);
    /*
     * Prepare for reading or writing all the data buffers
     */
    int block_offset = offset + SECTION_META_PAGES + SECTION_BITSNUMBER_PAGES;
    for (int i = 0; i < map_count; ++i)
    {
        _offsetMap[i] = block_offset;
        ++block_offset;
    }
    for (int i = 0; i < fp_count; ++i)
    {
        _offsetFp[i] = block_offset;
        ++block_offset;
    }
    for (int i = 0; i < bin_count; ++i)
    {
        _offsetBin[i] = block_offset;
        ++block_offset;
    }
    if (_idxStrategy != BingoPgIndex::READING_STRATEGY)
    {
        for (int i = 0; i < map_count; ++i)
        {
            getMapBufferCache(i);
        }
        for (int i = 0; i < fp_count; ++i)
        {
            getFpBufferCache(i);
        }
        for (int i = 0; i < bin_count; ++i)
        {
            getBinBufferCache(i);
        }
    }
}

BingoPgSection::~BingoPgSection()
{
    /*
     * Write meta info
     */
    _sectionInfo.n_blocks_for_bin = _buffersBin.size();
    _sectionInfo.section_size = getPagesCount();
    if (_idxStrategy == BingoPgIndex::BUILDING_STRATEGY)
    {
        _sectionInfoBuffer.changeAccess(BINGO_PG_WRITE);
        _sectionInfoBuffer.formIndexTuple(&_sectionInfo, sizeof(_sectionInfo));
        _sectionInfoBuffer.changeAccess(BINGO_PG_NOLOCK);
    }
    else if (_idxStrategy == BingoPgIndex::UPDATING_STRATEGY)
    {
        _sectionInfoBuffer.changeAccess(BINGO_PG_WRITE);
        int data_len;
        BingoSectionInfoData* data = (BingoSectionInfoData*)_sectionInfoBuffer.getIndexData(data_len);
        *data = _sectionInfo;
        _sectionInfoBuffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgSection::clear()
{
    _sectionInfo.n_blocks_for_bin = 0;
    _sectionInfo.n_blocks_for_fp = 0;
    _sectionInfo.n_blocks_for_map = 0;
    _sectionInfo.n_structures = 0;
    _sectionInfo.section_size = 0;
    _sectionInfo.last_cmf = -1;
    _sectionInfo.last_xyz = -1;
    _sectionInfo.has_removed = 0;
    _sectionInfoBuffer.clear();
    _existStructures.reset(0);
    _buffersMap.clear();
    _buffersFp.clear();
    _buffersBin.clear();
    _offsetBin.clear();
    _offsetFp.clear();
    _offsetMap.clear();
}

bool BingoPgSection::isExtended()
{
    return _sectionInfo.n_structures < BINGO_MOLS_PER_SECTION;
}

void BingoPgSection::addStructure(BingoPgFpData& item_data)
{
    int current_str = _sectionInfo.n_structures;
    elog(DEBUG1, "bingo: section: insert a structure %d", current_str);
    /*
     * Set fp bits
     */
    for (int idx = item_data.bitBegin(); idx != item_data.bitEnd(); idx = item_data.bitNext(idx))
    {
        int bit_idx = item_data.getBit(idx);
        BingoPgBufferCacheFp& buffer_fp = getFpBufferCache(bit_idx);
        buffer_fp.setBit(current_str, true);
    }

    int map_buf_idx = current_str / BINGO_MOLS_PER_MAPBLOCK;
    int map_idx = current_str % BINGO_MOLS_PER_MAPBLOCK;
    /*
     * Set tid map
     */
    elog(DEBUG1, "bingo: section: set tid map: map buffer idx = %d offset = %d", map_buf_idx, map_idx);
    BingoPgBufferCacheMap& buffer_map = getMapBufferCache(map_buf_idx);
    buffer_map.setTidItem(map_idx, item_data.getTidItem());

    /*
     * Prepare and set cmf map
     */
    _setCmfData(item_data.getCmfBuf(), map_buf_idx, map_idx);
    /*
     * Prepare and set xyz map
     */
    _setXyzData(item_data.getXyzBuf(), map_buf_idx, map_idx);

    /*
     * Set bits number
     */
    _setBitsCountData(item_data.getBitsCount());
    /*
     * Set structure index
     */
    item_data.setStructureIdx(_sectionInfo.n_structures);

    /*
     * Set structure bit
     */
    _existStructures->setBit(current_str, true);

    /*
     * Increment structures number
     */
    ++_sectionInfo.n_structures;
}

int BingoPgSection::getPagesCount() const
{
    return _buffersMap.size() + _buffersFp.size() + _buffersBin.size() + SECTION_META_PAGES + SECTION_BITSNUMBER_PAGES;
}

void BingoPgSection::getSectionStructures(BingoPgExternalBitset& section_bitset)
{
    _existStructures->getCopy(section_bitset);
}

void BingoPgSection::removeStructure(int mol_idx)
{
    _existStructures->setBit(mol_idx, false);
    _sectionInfo.has_removed = 1;
}

bool BingoPgSection::isStructureRemoved(int mol_idx)
{
    if (_sectionInfo.has_removed == 0)
        return false;
    return (!_existStructures->getBit(mol_idx));
}

BingoPgBufferCacheFp& BingoPgSection::getFpBufferCache(int fp_idx)
{
    BingoPgBufferCacheFp* elem = _buffersFp.at(fp_idx);
    if (elem == 0)
    {
        bool write = (_idxStrategy == BingoPgIndex::BUILDING_STRATEGY);
        int block_offset = _offsetFp[fp_idx];
        elem = new BingoPgBufferCacheFp(block_offset, _index, write);
        _buffersFp.set(fp_idx, elem);
    }
    return *elem;
}

BingoPgBufferCacheMap& BingoPgSection::getMapBufferCache(int map_idx)
{
    BingoPgBufferCacheMap* elem = _buffersMap.at(map_idx);
    if (elem == 0)
    {
        bool write = (_idxStrategy == BingoPgIndex::BUILDING_STRATEGY);
        int block_offset = _offsetMap[map_idx];
        elem = new BingoPgBufferCacheMap(block_offset, _index, write);
        _buffersMap.set(map_idx, elem);
    }
    return *elem;
}

BingoPgBufferCacheBin& BingoPgSection::getBinBufferCache(int bin_idx)
{
    return *_getBufferBin(bin_idx);
}

void BingoPgSection::readSectionBitsCount(indigo::Array<int>& bits_number)
{
    bits_number.resize(_sectionInfo.n_structures);
    bits_number.zerofill();

    if (_bitsCountBuffers.size() == 0)
        _bitsCountBuffers.resize(SECTION_BITSNUMBER_PAGES);

    int data_len, str_idx;
    unsigned short* buffer_data;
    for (int buf_idx = 0; buf_idx < SECTION_BITSNUMBER_PAGES; ++buf_idx)
    {
        if (buf_idx * SECTION_BITS_PER_BLOCK >= _sectionInfo.n_structures)
            break;

        BingoPgBuffer& bits_buffer = _bitsCountBuffers[buf_idx];
        bits_buffer.readBuffer(_index, _offset + buf_idx + SECTION_META_PAGES, BINGO_PG_READ);
        buffer_data = (unsigned short*)bits_buffer.getIndexData(data_len);
        for (int page_str_idx = 0; page_str_idx < SECTION_BITS_PER_BLOCK; ++page_str_idx)
        {
            str_idx = buf_idx * SECTION_BITS_PER_BLOCK + page_str_idx;
            if (str_idx >= _sectionInfo.n_structures)
                break;
            bits_number[str_idx] = buffer_data[page_str_idx];
        }
        bits_buffer.changeAccess(BINGO_PG_NOLOCK);
    }
}

void BingoPgSection::_setCmfData(indigo::std::string& cmf_buf, int map_buf_idx, int map_idx)
{
    /*
     * Set binary info
     */
    ItemPointerData cmf_item;
    _setBinData(cmf_buf, _sectionInfo.last_cmf, cmf_item);
    /*
     * Set mappings
     */
    BingoPgBufferCacheMap& buffer_map = getMapBufferCache(map_buf_idx);
    buffer_map.setCmfItem(map_idx, cmf_item);

    elog(DEBUG1, "bingo: section: set cmf map: buffer = %d, offset = %d", ItemPointerGetBlockNumber(&cmf_item), ItemPointerGetOffsetNumber(&cmf_item));
}

void BingoPgSection::_setXyzData(indigo::std::string& xyz_buf, int map_buf_idx, int map_idx)
{
    /*
     * Set binary info
     */
    ItemPointerData xyz_item;
    _setBinData(xyz_buf, _sectionInfo.last_xyz, xyz_item);
    /*
     * Set mappings
     */
    BingoPgBufferCacheMap& buffer_map = getMapBufferCache(map_buf_idx);
    buffer_map.setXyzItem(map_idx, xyz_item);

    elog(DEBUG1, "bingo: section: set xyz map: buffer = %d, offset = %d", ItemPointerGetBlockNumber(&xyz_item), ItemPointerGetOffsetNumber(&xyz_item));
}

void BingoPgSection::_setBinData(indigo::std::string& buf, int& last_buf, ItemPointerData& item_data)
{
    if (buf.size() == 0)
    {
        BINGO_PG_TRY
        {
            ItemPointerSet(&item_data, InvalidBlockNumber, 0);
        }
        BINGO_PG_HANDLE(throw Error("internal error: can not set block data: %s", message));
        return;
    }
    /*
     * Dynamic binary buffers
     */

    if (last_buf == -1)
    {
        int block_off = _offset + getPagesCount();
        _buffersBin.add(new BingoPgBufferCacheBin(block_off, _index, true));
        last_buf = _buffersBin.size() - 1;
        _offsetBin.push(block_off);
    }

    BingoPgBufferCacheBin* cache_bin = _getBufferBin(last_buf);
    /*
     * If not enough space for inserting a new structure - then create and new buffer
     */
    if (!cache_bin->isEnoughSpace(buf.sizeInBytes()))
    {
        int block_off = _offset + getPagesCount();
        _buffersBin.add(new BingoPgBufferCacheBin(block_off, _index, true));
        last_buf = _buffersBin.size() - 1;
        _offsetBin.push(block_off);
    }
    cache_bin = _getBufferBin(last_buf);

    /*
     * Get cmf offset for storing cmf mapping
     */
    unsigned short bin_offset = cache_bin->addBin(buf);

    /*
     * Set mappings
     */
    BINGO_PG_TRY
    {
        ItemPointerSet(&item_data, last_buf, bin_offset);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not set block data: %s", message));
}

void BingoPgSection::_setBitsCountData(unsigned short bits_count)
{

    if (_bitsCountBuffers.size() == 0)
        _bitsCountBuffers.resize(SECTION_BITSNUMBER_PAGES);

    int data_len;
    int buf_idx = _sectionInfo.n_structures / SECTION_BITS_PER_BLOCK;
    int page_str_idx = _sectionInfo.n_structures % SECTION_BITS_PER_BLOCK;

    BingoPgBuffer& bits_buffer = _bitsCountBuffers[buf_idx];
    bits_buffer.readBuffer(_index, _offset + buf_idx + SECTION_META_PAGES, BINGO_PG_WRITE);
    unsigned short* buffer_data = (unsigned short*)bits_buffer.getIndexData(data_len);
    buffer_data[page_str_idx] = bits_count;
    bits_buffer.changeAccess(BINGO_PG_NOLOCK);
}

BingoPgBufferCacheBin* BingoPgSection::_getBufferBin(int idx)
{
    BingoPgBufferCacheBin* elem = _buffersBin.at(idx);
    if (elem == 0)
    {
        bool write = (_idxStrategy == BingoPgIndex::BUILDING_STRATEGY);
        int block_offset = _offsetBin[idx];
        elem = new BingoPgBufferCacheBin(block_offset, _index, write);
        _buffersBin.set(idx, elem);
    }
    return elem;
}
