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
    /*
     * Clean meta info
     */
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

/*
 * Begins to write a sections
 */
void BingoPgIndex::writeBegin(BingoPgBuildEngine& fp_engine, BingoPgConfig& bingo_config)
{
    /*
     * Prepare meta info for writing
     */
    _metaInfo.n_blocks_for_map = BINGO_MOLS_PER_FINGERBLOCK / BINGO_MOLS_PER_MAPBLOCK + 1;
    _metaInfo.n_blocks_for_fp = fp_engine.getFpSize();
    _metaInfo.index_type = fp_engine.getType();
    _metaInfo.n_pages = 0;
    /*
     * Prepare meta pages
     */
    _initializeMetaPages(bingo_config);
    /*
     * Prepare the first section
     */
    _metaInfo.n_sections = 0;
    _initializeNewSection();

    /*
     * Set up write strategy
     */
    _strategy = BUILDING_STRATEGY;
}

void BingoPgIndex::updateBegin()
{
    _strategy = UPDATING_STRATEGY;
    /*
     * Read meta information
     */
    readMetaInfo();
    /*
     * Jump to the last section
     */
    _currentSectionIdx = -1;
    _jumpToSection(_metaInfo.n_sections - 1);
}

/*
 * Begins to read sections
 */
int BingoPgIndex::readBegin()
{
    _strategy = READING_STRATEGY;
    /*
     * Read meta information
     */
    readMetaInfo();
    /*
     * Jump to the first section
     */
    _jumpToSection(0);

    return _currentSectionIdx;
}

int BingoPgIndex::readNext(int section_idx)
{
    return section_idx + 1;
}

/*
 * Reads meta information
 */
void BingoPgIndex::readMetaInfo()
{
    /*
     * Read meta buffer
     */
    _metaBuffer.readBuffer(_index, BINGO_METAPAGE, BINGO_PG_READ);
    /*
     * Copy meta info
     */
    BingoMetaPage meta_page = BingoPageGetMeta(BufferGetPage(_metaBuffer.getBuffer()));
    _metaInfo = *meta_page;
    /*
     * Return buffer pin
     */
    _metaBuffer.changeAccess(BINGO_PG_NOLOCK);

    /*
     * Prepare section buffers
     */
    _sectionOffsetBuffers.expand(BINGO_SECTION_OFFSET_BLOCKS_NUM);
}

void BingoPgIndex::readConfigParameters(BingoPgConfig& bingo_config)
{
    /*
     * Read configuration page
     */
    BingoPgBuffer config_buffer(_index, BINGO_CONFIG_PAGE, BINGO_PG_READ);

    /*
     * Deserialize binary stored parameters
     */
    int data_len;
    void* data = config_buffer.getIndexData(data_len);
    bingo_config.deserialize(data, data_len);
}

/*
 * Writes meta information
 */
void BingoPgIndex::writeMetaInfo()
{
    _metaBuffer.changeAccess(BINGO_PG_WRITE);
    BingoMetaPage meta_page = BingoPageGetMeta(BufferGetPage(_metaBuffer.getBuffer()));
    *meta_page = _metaInfo;
    _metaBuffer.changeAccess(BINGO_PG_NOLOCK);
}

void BingoPgIndex::_initializeMetaPages(BingoPgConfig& bingo_config)
{
    /*
     * Initialize meta buffer
     */
    _metaBuffer.writeNewBuffer(_index, BINGO_METAPAGE);
    _metaBuffer.formIndexTuple(&_metaInfo, sizeof(_metaInfo));
    _metaBuffer.changeAccess(BINGO_PG_NOLOCK);
    ++_metaInfo.n_pages;
    /*
     * Initialize config buffer
     */
    indigo::Array<char> config_data;
    bingo_config.serialize(config_data);
    BingoPgBuffer config_buffer;
    config_buffer.writeNewBuffer(_index, BINGO_CONFIG_PAGE);
    config_buffer.formIndexTuple(config_data.ptr(), config_data.sizeInBytes());
    config_buffer.clear();
    ++_metaInfo.n_pages;
    /*
     * Write section mapping buffers
     * Fulfil by max size
     */
    for (int block_idx = 0; block_idx < BINGO_SECTION_OFFSET_BLOCKS_NUM; ++block_idx)
    {
        BingoPgBuffer buffer;
        buffer.writeNewBuffer(_index, _metaInfo.n_pages);
        buffer.formEmptyIndexTuple(BINGO_SECTION_OFFSET_PER_BLOCK * sizeof(int));
        buffer.clear();
        ++_metaInfo.n_pages;
    }
    _sectionOffsetBuffers.expand(BINGO_SECTION_OFFSET_BLOCKS_NUM);

    /*
     * Write dictionary buffers
     * Fulfil by max size
     */
    _metaInfo.offset_dictionary = _metaInfo.n_pages;
    for (int block_idx = 0; block_idx < BINGO_DICTIONARY_BLOCKS_NUM; ++block_idx)
    {
        BingoPgBuffer buffer;
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

    /*
     * Fulfil dictionary buffers
     */
    indigo::Array<char> buffer_dict;
    int dict_offset = 0;
    int dict_buf_size;

    /*
     * Set offset
     */
    dict_buf_size = std::min(static_cast<int>(BingoPgBufferCacheBin::MAX_SIZE), dict_size - dict_offset);

    while (dict_buf_size > 0)
    {
        /*
         * Write buffers immediately
         */
        int blck_off = _metaInfo.offset_dictionary + _metaInfo.n_blocks_for_dictionary;
        BingoPgBufferCacheBin buffer_cache(blck_off, _index, false);
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

/*
 * Initializes and fulfils a new section
 */
void BingoPgIndex::_initializeNewSection()
{
    if (_currentSection.get() != 0)
    {
        _metaInfo.n_pages += _currentSection->getPagesCount();
    }
    int section_offset = _metaInfo.n_pages;
    /*
     * Set up section offset mapping
     */
    _currentSectionIdx = _metaInfo.n_sections;
    _setSectionOffset(_currentSectionIdx, section_offset);

    /*
     * Prepare a new section
     */
    _currentSection = std::make_unique<BingoPgSection>(*this, BUILDING_STRATEGY, section_offset);
    ++_metaInfo.n_sections;
}

void BingoPgIndex::_setSectionOffset(int section_idx, int section_offset)
{
    int data_len;
    int section_off_idx = section_idx % BINGO_SECTION_OFFSET_PER_BLOCK;
    BingoPgBuffer& off_buffer = _getOffsetBuffer(section_idx);
    /*
     * Update section offset mapping buffer
     */
    off_buffer.changeAccess(BINGO_PG_WRITE);
    int* section_offsets = (int*)off_buffer.getIndexData(data_len);
    section_offsets[section_off_idx] = section_offset;
    off_buffer.changeAccess(BINGO_PG_NOLOCK);
}

BingoPgBuffer& BingoPgIndex::_getOffsetBuffer(int section_idx)
{
    int section_buf_idx = section_idx / BINGO_SECTION_OFFSET_PER_BLOCK;
    /*
     * There is the maximum limit of sections
     */
    if (section_buf_idx >= BINGO_SECTION_OFFSET_BLOCKS_NUM)
        throw Error("internal error: can not add new section, max limit reached: %d", section_idx * BINGO_MOLS_PER_SECTION);
    BingoPgBuffer* buffer = 0;
    if (_sectionOffsetBuffers.at(section_buf_idx) == 0)
    {
        _sectionOffsetBuffers.set(section_buf_idx, new BingoPgBuffer());
        buffer = _sectionOffsetBuffers.at(section_buf_idx);
        buffer->readBuffer(_index, section_buf_idx + BINGO_METABLOCKS_NUM, BINGO_PG_NOLOCK);
    }
    buffer = _sectionOffsetBuffers.at(section_buf_idx);

    return *buffer;
}

BingoPgSection& BingoPgIndex::_jumpToSection(int section_idx)
{

    /*
     * Return if current section is already set
     */
    if (_currentSectionIdx == section_idx)
        return *_currentSection;
    if (section_idx >= getSectionNumber())
    {
        if (_strategy == READING_STRATEGY)
        {
            throw Error("could not get the buffer: section %d is out of bounds %d", section_idx, getSectionNumber());
        }
        else
        {
            /*
             * If strategy is writing or updating then append new sections
             */
            while (section_idx >= getSectionNumber())
            {
                _initializeNewSection();
            }
            return *_currentSection;
        }
    }
    // profTimerStart(t0, "bingo_pg.read_section");
    /*
     * Read the section using offset mapping
     */
    _currentSectionIdx = section_idx;

    // profTimerStart(t1, "bingo_pg.get_offset");
    int offset = _getSectionOffset(section_idx);
    // profTimerStop(t1);
    _currentSection = std::make_unique<BingoPgSection>(*this, _strategy, offset);

    return *_currentSection;
}

int BingoPgIndex::_getSectionOffset(int section_idx)
{
    /*
     * Prepare section offset mapping
     */
    int section_off_idx = section_idx % BINGO_SECTION_OFFSET_PER_BLOCK;
    int data_len;
    int result;
    BingoPgBuffer& off_buffer = _getOffsetBuffer(section_idx);
    /*
     * Read mapping
     */
    off_buffer.changeAccess(BINGO_PG_READ);
    int* section_offsets = (int*)off_buffer.getIndexData(data_len);
    result = section_offsets[section_off_idx];
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
    /*
     * Read all buffers for dictionary
     */
    for (int block_idx = _metaInfo.offset_dictionary; block_idx < block_size; ++block_idx)
    {
        BingoPgBufferCacheBin buffer_cache(block_idx, _index, false);
        /*
         * Read and concat buffers
         */
        buffer_cache.readBin(0, buffer_dict);
        dictionary.concat(buffer_dict);
    }
}

void BingoPgIndex::insertStructure(BingoPgFpData& data_item)
{

    /*
     * Assuming that insert only for a write strategy
     */
    if (_strategy == READING_STRATEGY)
        throw Error("can not insert a structure while reading");
    /*
     * Jump to the last section
     */
    _jumpToSection(getSectionNumber() - 1);

    /*
     * If a structure can not be added to the current section then initialize the new
     */
    if (!_currentSection->isExtended())
    {
        _initializeNewSection();
    }
    elog(DEBUG1, "bingo: index: start adding a structure to the section %d", _currentSectionIdx);
    // #ifdef BINGO_PG_INTEGRITY_DIR
    //    Relation index = (Relation) _index;
    //
    //    BingoPgWrapper rel_wr;
    //    const char* rel_name = rel_wr.getRelName(index->rd_id);
    //
    //    indigo::FileOutput fout(false, "%s/insert/%s_%d_%d_%d", BINGO_PG_INTEGRITY_DIR, rel_name, _currentSectionIdx, );
    // #endif
    /*
     * Add a structure
     */
    _currentSection->addStructure(data_item);

    elog(DEBUG1, "bingo: index: finish adding a structure to the section %d", _currentSectionIdx);

    /*
     * Return cmf map to the output
     */
    data_item.setSectionIdx(_currentSectionIdx);
    /*
     * Increment structures common number
     */
    ++_metaInfo.n_molecules;
    if (_metaInfo.n_molecules % 1000 == 0)
    {
        elog(NOTICE, "bingo.index: %d structures processed", _metaInfo.n_molecules);
    }
}

void BingoPgIndex::readTidItem(ItemPointerData& cmf_item, PG_OBJECT result_ptr)
{
    readTidItem(ItemPointerGetBlockNumber(&cmf_item), ItemPointerGetOffsetNumber(&cmf_item), result_ptr);
}

void BingoPgIndex::readTidItem(int section_idx, int mol_idx, PG_OBJECT result_ptr)
{
    // profTimerStart(t0, "bingo_pg.read_tid");
    /*
     * Prepare info for reading
     */
    BingoPgSection& current_section = _jumpToSection(section_idx);
    int map_block_idx = mol_idx / BINGO_MOLS_PER_MAPBLOCK;
    int map_mol_idx = mol_idx % BINGO_MOLS_PER_MAPBLOCK;

    /*
     * Prepare result item
     */
    ItemPointerData& result_item = (ItemPointerData&)(*(ItemPointer)result_ptr);
    BingoPgBufferCacheMap& map_cache = current_section.getMapBufferCache(map_block_idx);
    /*
     * Read tid map
     */
    map_cache.getTidItem(map_mol_idx, result_item);
}

void BingoPgIndex::andWithBitset(int section_idx, int fp_idx, BingoPgExternalBitset& ext_bitset)
{
    // profTimerStart(t0, "bingo_pg.read_fp_and_with");
    /*
     * Prepare info for reading
     */
    BingoPgSection& current_section = _jumpToSection(section_idx);
    BingoPgBufferCacheFp& fp_buffer = current_section.getFpBufferCache(fp_idx);
    /*
     * And with a bitset
     */
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
    // profTimerStart(t0, "bingo_pg.read_cmf");
    /*
     * Prepare info for reading
     */
    BingoPgSection& current_section = _jumpToSection(section_idx);
    int map_block_idx = mol_idx / BINGO_MOLS_PER_MAPBLOCK;
    int map_mol_idx = mol_idx % BINGO_MOLS_PER_MAPBLOCK;
    BingoPgBufferCacheMap& map_cache = current_section.getMapBufferCache(map_block_idx);

    elog(DEBUG1, "bingo: index: read cmf: start read structure %d for section = %d", mol_idx, section_idx);

    /*
     * Get cmf item
     */
    ItemPointerData cmf_item;
    map_cache.getCmfItem(map_mol_idx, cmf_item);
    /*
     * Check for correct block num
     */
    dword block_num = ItemPointerGetBlockNumber(&cmf_item);
    if (block_num == InvalidBlockNumber)
    {
        cmf_buf.clear();
        elog(DEBUG1, "bingo: index: read cmf: cmf is empty for structure %d for section = %d", mol_idx, section_idx);
        return;
    }
    unsigned short block_offset = ItemPointerGetOffsetNumber(&cmf_item);
    /*
     * Read cmf buffer for a given offset
     */
    BingoPgBufferCacheBin& bin_cache = current_section.getBinBufferCache(block_num);
    bin_cache.readBin(block_offset, cmf_buf);
    elog(DEBUG1, "bingo: index: read cmf: successfully read cmf of size %d for block %d offset %d", cmf_buf.size(), block_num, block_offset);
}

void BingoPgIndex::readXyzItem(int section_idx, int mol_idx, indigo::Array<char>& xyz_buf)
{
    /*
     * Prepare info for reading
     */
    BingoPgSection& current_section = _jumpToSection(section_idx);
    int map_block_idx = mol_idx / BINGO_MOLS_PER_MAPBLOCK;
    int map_mol_idx = mol_idx % BINGO_MOLS_PER_MAPBLOCK;
    BingoPgBufferCacheMap& map_cache = current_section.getMapBufferCache(map_block_idx);

    elog(DEBUG1, "bingo: index: read xyz: start read structure %d for section = %d", mol_idx, section_idx);

    /*
     * Get xyz item
     */
    ItemPointerData xyz_item;
    map_cache.getXyzItem(map_mol_idx, xyz_item);
    /*
     * Check for correct block num
     */
    dword block_num = ItemPointerGetBlockNumber(&xyz_item);
    if (block_num == InvalidBlockNumber)
    {
        xyz_buf.clear();
        elog(DEBUG1, "bingo: index: read xyz: xyz is empty for structure %d for section = %d", mol_idx, section_idx);
        return;
    }
    unsigned short block_offset = ItemPointerGetOffsetNumber(&xyz_item);
    /*
     * Read xyz buffer for a given offset
     */
    BingoPgBufferCacheBin& bin_cache = current_section.getBinBufferCache(block_num);
    bin_cache.readBin(block_offset, xyz_buf);
    elog(DEBUG1, "bingo: index: read xyz: successfully read xyz of size %d for block %d offset %d", xyz_buf.size(), block_num, block_offset);
}
