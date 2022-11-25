#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "access/itup.h"
#include "fmgr.h"
#include "storage/bufmgr.h"
}

#include "bingo_pg_fix_post.h"

#include "bingo_pg_search_engine.h"

#include "base_c/bitarray.h"
#include "base_cpp/array.h"
#include "base_cpp/profiling.h"
#include "base_cpp/tlscont.h"
#include "gzip/gzip_scanner.h"

#include "bingo_core_c.h"

#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"
#include "bingo_pg_text.h"

using namespace indigo;

void BingoPgFpData::setTidItem(PG_OBJECT item_ptr)
{
    ItemPointerData& item_p = *(ItemPointer)item_ptr;
    _mapData.tid_map = (ItemPointerData&)item_p;
}

void BingoPgFpData::setFingerPrints(const char* fp_buf, int size_bits)
{
    _fingerprintBits.clear();

    for (int bit_idx = 0; bit_idx < size_bits; ++bit_idx)
    {
        if (bitGetBit(fp_buf, bit_idx))
        {
            _fingerprintBits.push(bit_idx);
        }
    }
}

void BingoPgFpData::setCmf(const char* cmf_buf, int cmf_len)
{
    _cmfBuf.copy(cmf_buf, cmf_len);
}

void BingoPgFpData::setXyz(const char* xyz_buf, int xyz_len)
{
    _xyzBuf.copy(xyz_buf, xyz_len);
}

BingoPgSearchEngine::BingoPgSearchEngine()
    : _fetchFound(false), _currentSection(-1), _currentIdx(-1), _blockBegin(0), _blockEnd(0), _bufferIndexPtr(0), _sectionBitset(BINGO_MOLS_PER_SECTION)
{
    _bingoContext = std::make_unique<BingoContext>(0);
    _mangoContext = std::make_unique<MangoContext>(*_bingoContext.get());
    _ringoContext = std::make_unique<RingoContext>(*_bingoContext.get());

    bingoCore.bingo_context = _bingoContext.get();
    bingoCore.mango_context = _mangoContext.get();
    bingoCore.ringo_context = _ringoContext.get();
}

BingoPgSearchEngine::~BingoPgSearchEngine()
{
}

void BingoPgSearchEngine::setItemPointer(PG_OBJECT result_ptr)
{
    _bufferIndexPtr->readTidItem(_currentSection, _currentIdx, result_ptr);
}

void BingoPgSearchEngine::loadDictionary(BingoPgIndex& bingo_index)
{
    QS_DEF(Array<char>, dict);
    bingo_index.readDictionary(dict);
    bingoCore.bingoSetConfigBin("cmf_dict", dict.ptr(), dict.sizeInBytes());
}

bool BingoPgSearchEngine::matchTarget(ItemPointerData& item_data)
{
    return matchTarget(ItemPointerGetBlockNumber(&item_data), ItemPointerGetOffsetNumber(&item_data));
}

void BingoPgSearchEngine::prepareQuerySearch(BingoPgIndex& bingo_idx, PG_OBJECT)
{
    _bufferIndexPtr = &bingo_idx;
    _currentSection = -1;
    _currentIdx = -1;
    _fetchFound = false;
    _blockBegin = 0;
    _blockEnd = bingo_idx.getSectionNumber();
}

bool BingoPgSearchEngine::_searchNextCursor(PG_OBJECT result_ptr)
{
    // profTimerStart(t0, "bingo_pg.search_cursor");
    ItemPointerData cmf_item;
    /*
     * Iterate through the cursor
     */
    while (_searchCursor->next())
    {
        _searchCursor->getId(1, cmf_item);

        /*
         * If structure is removed from index then seek to the next
         */
        if (_bufferIndexPtr->isStructureRemoved(cmf_item))
            continue;
        /*
         * If structure is not match then seek to the next
         */
        if (!matchTarget(cmf_item))
            continue;
        /*
         * Return tid map
         */
        _bufferIndexPtr->readTidItem(cmf_item, result_ptr);
        return true;
    }

    return false;
}

bool BingoPgSearchEngine::_searchNextSub(PG_OBJECT result_ptr)
{

    // profTimerStart(t0, "bingo_pg.search_sub");
    BingoPgFpData& query_data = *_queryFpData;
    BingoPgIndex& bingo_index = *_bufferIndexPtr;
    /*
     * If there are matches found on the previous steps
     */
    if (_fetchFound)
    {
        if (_fetchForNext())
        {
            setItemPointer(result_ptr);
            return true;
        }
        else
        {
            _fetchFound = false;
            ++_currentSection;
        }
    }
    // profTimerStart(t1, "bingo_pg.search_fp");

    if (_currentSection < 0)
        _currentSection = _blockBegin;
    /*
     * Iterate through the sections bingo_index.readEnd()
     */
    for (; _currentSection < _blockEnd; ++_currentSection)
    {
        /*
         * Get section existing structures
         */
        bingo_index.getSectionBitset(_currentSection, _sectionBitset);
        _currentIdx = -1;
        /*
         * If there is no fingerprints then check every molecule
         */
        if (query_data.bitEnd() != 0)
        {
            /*
             * Iterate through the query bits
             */
            for (int fp_idx = query_data.bitBegin(); fp_idx != query_data.bitEnd() && _sectionBitset.hasBits(); fp_idx = query_data.bitNext(fp_idx))
            {
                int fp_block = query_data.getBit(fp_idx);
                /*
                 * Get fingerprint buffer in the current section
                 */
                bingo_index.andWithBitset(_currentSection, fp_block, _sectionBitset);
            }
        }
        /*
         * If bitset is not null then matches are found
         */
        if (_sectionBitset.hasBits())
        {
            /*
             * Set first match as an answer
             */
            if (_fetchForNext())
            {
                // profTimerStop(t1);
                setItemPointer(result_ptr);
                /*
                 * Set fetch found to return on the next steps
                 */
                _fetchFound = true;
                return true;
            }
        }
    }

    /*
     * No matches or section ends
     */
    return false;
}

using namespace indigo;

void BingoPgSearchEngine::_setBingoContext()
{
}

bool BingoPgSearchEngine::_fetchForNext()
{
    /*
     * Seek for next target matched by fp engine
     */
    if (_currentIdx == -1)
        _currentIdx = _sectionBitset.begin();
    else
        _currentIdx = _sectionBitset.next(_currentIdx);

    for (; _currentIdx != _sectionBitset.end(); _currentIdx = _sectionBitset.next(_currentIdx))
    {
        /*
         * Match the next target
         */
        if (matchTarget(_currentSection, _currentIdx))
            return true;
    }

    return false;
}

void BingoPgSearchEngine::_getBlockParameters(Array<char>& params)
{
    QS_DEF(Array<char>, block_params);
    QS_DEF(Array<char>, tmp);
    block_params.clear();

    for (int i = 0; i < params.size(); ++i)
    {
        if (params[i] == ';')
        {
            block_params.copy(params.ptr(), i);
            block_params.push(0);
            tmp.copy(params);
            params.clear();
            for (int j = i + 1; j < params.size(); ++j)
            {
                params.push(tmp[j]);
            }
            params.push(0);
        }
    }
    bool verify_split = false;

    if (block_params.size() == 0)
    {
        block_params.copy(params);
        verify_split = true;
    }

    BufferScanner scanner(block_params);
    QS_DEF(Array<char>, word);

    int block_id = 0, block_count = 0;

    while (!scanner.isEOF())
    {
        scanner.skipSpace();
        scanner.readWord(word, 0);

        if (strcasecmp(word.ptr(), "B_ID") == 0)
        {
            scanner.skipSpace();
            block_id = scanner.readInt();
            if (block_id < 1)
                throw BingoPgError("B_ID should be a positive value: %d", block_id);
        }
        else if (strcasecmp(word.ptr(), "B_COUNT") == 0)
        {
            scanner.skipSpace();
            block_count = scanner.readInt();
            if (block_count < 1)
                throw BingoPgError("B_COUNT should be a positive value: %d", block_count);
        }
        else if (strcasecmp(word.ptr(), "") == 0)
        {
            break;
        }
        else
        {
            if (verify_split)
                return;
            else
                throw BingoPgError("unknown block type: %s", word.ptr());
        }
    }

    if (verify_split)
    {
        params.clear();
        params.push(0);
    }
    /*
     * Return if block id was not specified
     */
    if (block_id == 0)
        return;

    int max_blocks = _bufferIndexPtr->getSectionNumber();

    if (block_count > 0)
    {
        if (block_count > max_blocks)
            throw BingoPgError("B_COUNT %d can not be greater then maximum block count %d", block_count, max_blocks);
        if (block_id > block_count)
            throw BingoPgError("B_ID %d can not be greater then B_COUNT %d", block_id, block_count);

        double b = block_id - 1;
        b = (double)(b / block_count) * max_blocks;
        double e = block_id;
        e = (double)(e / block_count) * max_blocks;
        _blockBegin = (int)b;
        _blockEnd = (int)e;
    }
    else
    {
        if (block_id > max_blocks)
            throw BingoPgError("B_ID %d can not be greater then maximum block count %d", block_id, max_blocks);

        _blockBegin = block_id - 1;
        _blockEnd = block_id;
    }
}
