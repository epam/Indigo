#ifndef _BINGO_PG_INDEX_H__
#define _BINGO_PG_INDEX_H__

#include <memory>
#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"

#include "bingo_pg_buffer.h"
#include "bingo_pg_section.h"
#include "bingo_postgres.h"
#include "pg_bingo_context.h"

/*
 * Class for handling bingo meta info and sections
 * Bingo index block are the following:
 * meta info(1 block) | config (1 block) | section mapping (10 blocks) | dictionary(100 blocks) | sections
 */
class BingoPgBuildEngine;
class BingoPgConfig;
class BingoPgFpData;
class BingoPgExternalBitset;

class BingoPgIndex
{
public:
    enum INDEX_STRATEGY
    {
        BUILDING_STRATEGY,
        UPDATING_STRATEGY,
        READING_STRATEGY
    };
    BingoPgIndex(PG_OBJECT index);
    ~BingoPgIndex()
    {
    }

    /*
     * Begins write section. Create meta buffer
     */
    void writeBegin(BingoPgBuildEngine&, BingoPgConfig&);
    void updateBegin();

    /*
     * Read section iterator
     */
    int readBegin();
    int readNext(int section_idx);
    int readEnd()
    {
        return _metaInfo.n_sections;
    }

    /*
     * Handle with meta info
     */
    void readMetaInfo();
    void readConfigParameters(BingoPgConfig&);
    void writeMetaInfo();

    /*
     * Getters
     */
    int getStructuresNumber() const
    {
        return _metaInfo.n_molecules;
    }
    int getSectionNumber() const
    {
        return _metaInfo.n_sections;
    }
    int getIndexType() const
    {
        return _metaInfo.index_type;
    }
    int getPagesCount() const
    {
        return _metaInfo.n_pages;
    }
    int getFpSize() const
    {
        return _metaInfo.n_blocks_for_fp;
    }
    int getMapSize() const
    {
        return _metaInfo.n_blocks_for_map;
    }
    int getDictCount() const
    {
        return _metaInfo.n_blocks_for_dictionary;
    }

    PG_OBJECT getIndexPtr() const
    {
        return _index;
    }
    INDEX_STRATEGY getIndexStrategy() const
    {
        return _strategy;
    }

    /*
     * Strategies
     */

    void setStrategy(INDEX_STRATEGY strategy)
    {
        _strategy = strategy;
    }

    /*
     * Insert a new structure in the index
     */
    void insertStructure(BingoPgFpData&);

    /*
     * Read all the data for the index
     */
    void readTidItem(ItemPointerData&, PG_OBJECT result_ptr);
    void readTidItem(int section_idx, int mol_idx, PG_OBJECT result_ptr);

    void readCmfItem(int section_idx, int mol_idx, indigo::Array<char>& cmf_buf);
    void readXyzItem(int section_idx, int mol_idx, indigo::Array<char>& xyz_buf);

    void andWithBitset(int section_idx, int fp_idx, BingoPgExternalBitset& ext_bitset);

    int getSectionStructuresNumber(int section_idx);
    const BingoSectionInfoData& getSectionInfo(int section_idx);

    void getSectionBitset(int section_idx, BingoPgExternalBitset& section_bitset);
    void getSectionBitsCount(int section_idx, indigo::Array<int>& bits_count);

    void removeStructure(int section_idx, int mol_idx);
    bool isStructureRemoved(int section_idx, int mol_idx);
    bool isStructureRemoved(ItemPointerData&);

    void readDictionary(indigo::Array<char>& _dictionary);
    void writeDictionary(BingoPgBuildEngine&);
    /*
     * Clear all buffers
     */
    void clearAllBuffers();

    DECL_ERROR;

private:
    BingoPgIndex(const BingoPgIndex&); // no implicit copy

    void _initializeMetaPages(BingoPgConfig&);
    void _initializeNewSection();
    void _setSectionOffset(int section_idx, int section_offset);
    BingoPgBuffer& _getOffsetBuffer(int section_idx);

    BingoPgSection& _jumpToSection(int section_idx);
    int _getSectionOffset(int section_idx);

    PG_OBJECT _index;
    INDEX_STRATEGY _strategy;

    BingoMetaPageData _metaInfo;
    BingoPgBuffer _metaBuffer;
    indigo::PtrArray<BingoPgBuffer> _sectionOffsetBuffers;
    indigo::std::unique_ptr<BingoPgSection> _currentSection;
    int _currentSectionIdx;
};

#endif /* BINGO_PG_SECTION_H */
