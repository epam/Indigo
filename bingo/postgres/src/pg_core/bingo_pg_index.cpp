#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "fmgr.h"
#include "storage/bufmgr.h"
}

#include "bingo_pg_fix_post.h"

#include <algorithm>

#include "base_cpp/profiling.h"

#include "bingo_core_c.h"
#include "bingo_pg_build_engine.h"
#include "bingo_pg_config.h"
#include "bingo_pg_ext_bitset.h"
#include "bingo_pg_index.h"
#include "bingo_pg_search_engine.h"
#include "pg_bingo_context.h"

IMPL_ERROR(BingoPgIndex, "bingo index");

BingoPgIndex::BingoPgIndex(PG_OBJECT index) : _index(index), _strategy(READING_STRATEGY)
{
    _metaInfo.bingo_index_version = 0;
    _metaInfo.n_blocks_for_fp = 0;
    _metaInfo.n_blocks_for_map = 0;
    _metaInfo.n_blocks_for_dictionary = 0;
    _metaInfo.offset_dictionary = 0;
    _metaInfo.n_sections = 0;
    _metaInfo.n_molecules = 0;
    _metaInfo.index_type = 0;
    _metaInfo.n_pages = 0;
    _currentSectionIdx = -1;
}

void BingoPgIndex::writeBegin(BingoPgBuildEngine& fp_engine, BingoPgConfig& bingo_config)
{
    setStrategy(BUILDING_STRATEGY);

    _metaInfo.n_blocks_for_map = BINGO_MOLS_PER_FINGERBLOCK / BINGO_MOLS_PER_MAPBLOCK + 1;
    _metaInfo.n_blocks_for_fp = fp_engine.getFpSize();
    _metaInfo.index_type = fp_engine.getType();
    _metaInfo.n_pages = 0;

    _initializeMetaPages(bingo_config);
    _metaInfo.n_sections = 0;
    _initializeNewSection();
}

void BingoPgIndex::updateBegin()
{
    setStrategy(UPDATING_STRATEGY);
    readMetaInfo();
    _currentSectionIdx = -1;
    _jumpToSection(_metaInfo.n_sections - 1);
}

int BingoPgIndex::readBegin()
{
    setStrategy(READING_STRATEGY);
    readMetaInfo();
    _jumpToSection(0);
    return _currentSectionIdx;
}

int BingoPgIndex::readNext(int section_idx)
{
    return section_idx + 1;
}

void BingoPgIndex::readMetaInfo()
{
    _metaBuffer.readBuffer(_index, BINGO_METAPAGE, BINGO_PG_READ);
    BingoMetaPage meta_page = BingoPageGetMeta((Page)_metaBuffer.getPage());
    _metaInfo = *meta_page;
    _metaBuffer.changeAccess(BINGO_PG_NOLOCK);
    _sectionOffsetBuffers.expand(BINGO_SECTION_OFFSET_BLOCKS_NUM);
}

void BingoPgIndex::readConfigParameters(BingoPgConfig& bingo_config)
{
    BingoPgBuffer config_buffer(_index, BINGO_CONFIG_PAGE, BINGO_PG_READ);
    int data_len;
    void* data = config_buffer.getIndexData(data_len);
    bingo_config.deserialize(data, data_len);
}

void BingoPgIndex::writeMetaInfo()
{
    _metaBuffer.changeAccess(BINGO_PG_WRITE);
    BingoMetaPage meta_page = BingoPageGetMeta((Page)_metaBuffer.getPage());
    *meta_page = _metaInfo;
    _metaBuffer.changeAccess(BINGO_PG_NOLOCK);
}

void BingoPgIndex::_initializeMetaPages(BingoPgConfig& bingo_config)
{
    _metaBuffer.setWalEnabled(false);
    _metaBuffer.writeNewBuffer(_index, BINGO_METAPAGE);
    _metaBuffer.formIndexTuple(&_metaInfo, sizeof(_metaInfo));
    _metaBuffer.changeAccess(BINGO_PG_NOLOCK);
    ++_metaInfo.n_pages;

    indigo::Array<char> config_data;
    bingo_config.serialize(config_data);
    BingoPgBuffer config_buffer;
    config_buffer.setWalEnabled(false);
    config_buffer.writeNewBuffer(_index, BINGO_CONFIG_PAGE);
    config_buffer.formIndexTuple(config_data.ptr(), config_data.sizeInBytes());
    config_buffer.clear();
    ++_metaInfo.n_pages;

    for (int block_idx = 0; block_idx < BINGO_SECTION_OFFSET_BLOCKS_NUM; ++block_idx)
    {
        BingoPgBuffer buffer;
        buffer.setWalEnabled(false);
        buffer.writeNewBuffer(_index, _metaInfo.n_pages);
        buffer.formEmptyIndexTuple(BINGO_SECTION_OFFSET_PER_BLOCK * sizeof(int));
        buffer.clear();
        ++_metaInfo.n_pages;
    }
    _sectionOffsetBuffers.expand(BINGO_SECTION_OFFSET_BLOCKS_NUM);

    _metaInfo.offset_dictionary = _metaInfo.n_pages;
    for (int block_idx = 0; block_idx < BINGO_DICTIONARY_BLOCKS_NUM; ++block_idx)
    {
        BingoPgBuffer buffer;
        buffer.setWalEnabled(false);
        buffer.writeNewBuffer(_index, _metaInfo.n_pages);
        buffer.formEmptyIndexTuple(BingoPgBufferCacheBin::BUFFER_SIZE);
        buffer.clear();
        ++_metaInfo.n_pages;
    }
}

void BingoPgIndex::writeDictionary(BingoPgBuildEngine& fp_engine)
{
    if (_strategy == READING_STRATEGY)
        throw Error("can not write dictionary while there is no building stage");

    int dict_size;
    const char* dict_buf = fp_engine.getDictionary(dict_size);

    _metaInfo.n_blocks_for_dictionary = 0;
    elog(DEBUG1, "bingo: index: update dictionary with size = %d", dict_size);

    if (dict_size == 0)
        return;

    if (dict_size > BingoPgBufferCacheBin::MAX_SIZE * BINGO_DICTIONARY_BLOCKS_NUM)
        throw Error("can not insert a dictionary with size = %d", dict_size);

    indigo::Array<char> buffer_dict;
    int dict_offset = 0;
    int dict_buf_size = std::min(static_cast<int>(BingoPgBufferCacheBin::MAX_SIZE), dict_size - dict_offset);
    const bool wal_enabled = (_strategy != BUILDING_STRATEGY);

    while (dict_buf_size > 0)
    {
        int blck_off = _metaInfo.offset_dictionary + _metaInfo.n_blocks_for_dictionary;
        BingoPgBufferCacheBin buffer_cache(blck_off, _index, false, wal_enabled);
        buffer_dict.copy(dict_buf + dict_offset, dict_buf_size);
        buffer_cache.writeBin(buffer_dict);

        dict_offset += dict_buf_size;
        dict_buf_size = std::min(static_cast<int>(BingoPgBufferCacheBin::MAX_SIZE), dict_size - dict_offset);
        ++_metaInfo.n_blocks_for_dictionary;
    }
}

void BingoPgIndex::clearAllBuffers()
{
    _metaBuffer.clear();
    int offset_size = _sectionOffsetBuffers.size();
    _sectionOffsetBuffers.clear();
    _sectionOffsetBuffers.expand(offset_size);
    _currentSection.reset(nullptr);
    _currentSectionIdx = -1;
}

void BingoPgIndex::_initializeNewSection()
{
    if (_currentSection.get() != nullptr)
    {
        _metaInfo.n_pages += _currentSection->getPagesCount();
        _currentSection.reset(nullptr);
    }

    const int section_offset = _metaInfo.n_pages;
    const int section_idx = _metaInfo.n_sections;
    _currentSectionIdx = section_idx;

    /*
     * Create every fixed page first. For an incremental rollover those page
     * creations are individually WAL-logged. Only after the section is fully
     * initialized do we publish its offset and increment n_sections.
     */
    _currentSection = std::make_unique<BingoPgSection>(*this, _strategy, section_offset, true);
    _setSectionOffset(section_idx, section_offset);
    ++_metaInfo.n_sections;

    if (_strategy == UPDATING_STRATEGY)
        writeMetaInfo();
}

void BingoPgIndex::_setSectionOffset(int section_idx, int section_offset)
{
    int data_len;
    int section_off_idx = section_idx % BINGO_SECTION_OFFSET_PER_BLOCK;
    BingoPgBuffer& off_buffer = _getOffsetBuffer(section_idx);
    off_buffer.changeAccess(BINGO_PG_WRITE);
    int* section_offsets = (int*)off_buffer.getIndexData(data_len);
    section_offsets[section_off_idx] = section_offset;
    off_buffer.changeAccess(BINGO_PG_NOLOCK);
}

BingoPgBuffer& BingoPgIndex::_getOffsetBuffer(int section_idx)
{
    int section_buf_idx = section_idx / BINGO_SECTION_OFFSET_PER_BLOCK;
    if (section_buf_idx >= BINGO_SECTION_OFFSET_BLOCKS_NUM)
        throw Error("internal error: can not add new section, max limit reached: %d", section_idx * BINGO_MOLS_PER_SECTION);

    BingoPgBuffer* buffer = nullptr;
    if (_sectionOffsetBuffers.at(section_buf_idx) == nullptr)
    {
        _sectionOffsetBuffers.set(section_buf_idx, new BingoPgBuffer());
        buffer = _sectionOffsetBuffers.at(section_buf_idx);
        buffer->setWalEnabled(_strategy != BUILDING_STRATEGY);
        buffer->readBuffer(_index, section_buf_idx + BINGO_METABLOCKS_NUM, BINGO_PG_NOLOCK);
    }
    buffer = _sectionOffsetBuffers.at(section_buf_idx);
    return *buffer;
}

BingoPgSection& BingoPgIndex::_jumpToSection(int section_idx)
{
    if (_currentSectionIdx == section_idx)
        return *_currentSection;

    if (section_idx >= getSectionNumber())
    {
        if (_strategy == READING_STRATEGY)
            throw Error("could not get the buffer: section %d is out of bounds %d", section_idx, getSectionNumber());

        while (section_idx >= getSectionNumber())
            _initializeNewSection();
        return *_currentSection;
    }

    _currentSectionIdx = section_idx;
    int offset = _getSectionOffset(section_idx);
    _currentSection = std::make_unique<BingoPgSection>(*this, _strategy, offset);
    return *_currentSection;
}

int BingoPgIndex::_getSectionOffset(int section_idx)
{
    int section_off_idx = section_idx % BINGO_SECTION_OFFSET_PER_BLOCK;
    int data_len;
    BingoPgBuffer& off_buffer = _getOffsetBuffer(section_idx);
    off_buffer.changeAccess(BINGO_PG_READ);
    int* section_offsets = (int*)off_buffer.getIndexData(data_len);
    int result = section_offsets[section_off_idx];
    off_buffer.changeAccess(BINGO_PG_NOLOCK);
    return result;
}

void BingoPgIndex::readDictionary(indigo::Array<char>& dictionary)
{
    dictionary.clear();
    if (_metaInfo.n_blocks_for_dictionary == 0)
        return;

    indigo::Array<char> buffer_dict;
    int block_size = _metaInfo.n_blocks_for_dictionary + _metaInfo.offset_dictionary;
    for (int block_idx = _metaInfo.offset_dictionary; block_idx < block_size; ++block_idx)
    {
        BingoPgBufferCacheBin buffer_cache(block_idx, _index, false);
        buffer_cache.readBin(0, buffer_dict);
        dictionary.concat(buffer_dict);
    }
}

void BingoPgIndex::insertStructure(BingoPgFpData& data_item)
{
    if (_strategy == READING_STRATEGY)
        throw Error("can not insert a structure while reading");

    _jumpToSection(getSectionNumber() - 1);
    if (!_currentSection->isExtended())
        _initializeNewSection();

    elog(DEBUG1, "bingo: index: start adding a structure to the section %d", _currentSectionIdx);
    _currentSection->addStructure(data_item);
    elog(DEBUG1, "bingo: index: finish adding a structure to the section %d", _currentSectionIdx);

    data_item.setSectionIdx(_currentSectionIdx);
    ++_metaInfo.n_molecules;

    /*
     * The section existence bit is already durable at this point. Persist the
     * global count promptly instead of relying on BingoPgBuild destruction.
     * A crash between the two can at worst leave this diagnostic count one
     * behind; it cannot expose partially written structure content.
     */
    if (_strategy == UPDATING_STRATEGY)
        writeMetaInfo();

    if (_metaInfo.n_molecules % 1000 == 0)
        elog(NOTICE, "bingo.index: %d structures processed", _metaInfo.n_molecules);
}

void BingoPgIndex::readTidItem(ItemPointerData& cmf_item, PG_OBJECT result_ptr)
{
    readTidItem(ItemPointerGetBlockNumber(&cmf_item), ItemPointerGetOffsetNumber(&cmf_item), result_ptr);
}

void BingoPgIndex::readTidItem(int section_idx, int mol_idx, PG_OBJECT result_ptr)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    int map_block_idx = mol_idx / BINGO_MOLS_PER_MAPBLOCK;
    int map_mol_idx = mol_idx % BINGO_MOLS_PER_MAPBLOCK;
    ItemPointerData& result_item = (ItemPointerData&)(*(ItemPointer)result_ptr);
    BingoPgBufferCacheMap& map_cache = current_section.getMapBufferCache(map_block_idx);
    map_cache.getTidItem(map_mol_idx, result_item);
}

void BingoPgIndex::andWithBitset(int section_idx, int fp_idx, BingoPgExternalBitset& ext_bitset)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    BingoPgBufferCacheFp& fp_buffer = current_section.getFpBufferCache(fp_idx);
    fp_buffer.andWithBitset(ext_bitset);
}

int BingoPgIndex::getSectionStructuresNumber(int section_idx)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    return current_section.getStructuresNumber();
}

const BingoSectionInfoData& BingoPgIndex::getSectionInfo(int section_idx)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    return current_section.getSectionInfo();
}

void BingoPgIndex::getSectionBitset(int section_idx, BingoPgExternalBitset& section_bitset)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    current_section.getSectionStructures(section_bitset);
}

void BingoPgIndex::getSectionBitsCount(int section_idx, indigo::Array<int>& bits_count)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    current_section.readSectionBitsCount(bits_count);
}

void BingoPgIndex::removeStructure(int section_idx, int mol_idx)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    current_section.removeStructure(mol_idx);
    --_metaInfo.n_molecules;
    if (_strategy == UPDATING_STRATEGY)
        writeMetaInfo();
}

bool BingoPgIndex::isStructureRemoved(int section_idx, int mol_idx)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    return current_section.isStructureRemoved(mol_idx);
}

bool BingoPgIndex::isStructureRemoved(ItemPointerData& cmf_item)
{
    return isStructureRemoved(ItemPointerGetBlockNumber(&cmf_item), ItemPointerGetOffsetNumber(&cmf_item));
}

void BingoPgIndex::readCmfItem(int section_idx, int mol_idx, indigo::Array<char>& cmf_buf)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    int map_block_idx = mol_idx / BINGO_MOLS_PER_MAPBLOCK;
    int map_mol_idx = mol_idx % BINGO_MOLS_PER_MAPBLOCK;
    BingoPgBufferCacheMap& map_cache = current_section.getMapBufferCache(map_block_idx);

    elog(DEBUG1, "bingo: index: read cmf: start read structure %d for section = %d", mol_idx, section_idx);

    ItemPointerData cmf_item;
    map_cache.getCmfItem(map_mol_idx, cmf_item);
    dword block_num = ItemPointerGetBlockNumber(&cmf_item);
    if (block_num == InvalidBlockNumber)
    {
        cmf_buf.clear();
        elog(DEBUG1, "bingo: index: read cmf: cmf is empty for structure %d for section = %d", mol_idx, section_idx);
        return;
    }
    unsigned short block_offset = ItemPointerGetOffsetNumber(&cmf_item);
    BingoPgBufferCacheBin& bin_cache = current_section.getBinBufferCache(block_num);
    bin_cache.readBin(block_offset, cmf_buf);
    elog(DEBUG1, "bingo: index: read cmf: successfully read cmf of size %d for block %d offset %d", cmf_buf.size(), block_num, block_offset);
}

void BingoPgIndex::readXyzItem(int section_idx, int mol_idx, indigo::Array<char>& xyz_buf)
{
    BingoPgSection& current_section = _jumpToSection(section_idx);
    int map_block_idx = mol_idx / BINGO_MOLS_PER_MAPBLOCK;
    int map_mol_idx = mol_idx % BINGO_MOLS_PER_MAPBLOCK;
    BingoPgBufferCacheMap& map_cache = current_section.getMapBufferCache(map_block_idx);

    elog(DEBUG1, "bingo: index: read xyz: start read structure %d for section = %d", mol_idx, section_idx);

    ItemPointerData xyz_item;
    map_cache.getXyzItem(map_mol_idx, xyz_item);
    dword block_num = ItemPointerGetBlockNumber(&xyz_item);
    if (block_num == InvalidBlockNumber)
    {
        xyz_buf.clear();
        elog(DEBUG1, "bingo: index: read xyz: xyz is empty for structure %d for section = %d", mol_idx, section_idx);
        return;
    }
    unsigned short block_offset = ItemPointerGetOffsetNumber(&xyz_item);
    BingoPgBufferCacheBin& bin_cache = current_section.getBinBufferCache(block_num);
    bin_cache.readBin(block_offset, xyz_buf);
    elog(DEBUG1, "bingo: index: read xyz: successfully read xyz of size %d for block %d offset %d", xyz_buf.size(), block_num, block_offset);
}
