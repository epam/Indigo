#ifndef _BINGO_PG_SEARCH_ENGINE_H__
#define _BINGO_PG_SEARCH_ENGINE_H__

/*
 * Interface class for procession fingerprint data
 */

#include "base_cpp/array.h"
#include <memory>

#include "bingo_pg_buffer_cache.h"
#include "bingo_pg_cursor.h"
#include "bingo_pg_ext_bitset.h"
#include "bingo_postgres.h"
#include "pg_bingo_context.h"
#include "bingo_core_c_internal.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;

class BingoPgFpData
{
public:
    BingoPgFpData()
    {
    }
    virtual ~BingoPgFpData()
    {
    }

    virtual int bitBegin() const
    {
        return 0;
    }
    virtual int bitEnd() const
    {
        return _fingerprintBits.size();
    }
    virtual int bitNext(int bit_idx) const
    {
        return bit_idx + 1;
    }
    virtual int getBit(int bit_idx) const
    {
        return _fingerprintBits[bit_idx];
    }

    void setTidItem(PG_OBJECT item_ptr);

    void setSectionIdx(int section_idx)
    {
        _sectionIdx = section_idx;
    }
    void setStructureIdx(int structure_idx)
    {
        _structureIdx = structure_idx;
    }

    void setFingerPrints(const char* fp_buf, int fp_len);
    void setCmf(const char* cmf_buf, int cmf_len);
    void setXyz(const char* xyz_buf, int xyz_len);

    ItemPointerData& getTidItem()
    {
        return _mapData.tid_map;
    }
    ItemPointerData& getCmfItem()
    {
        return _mapData.cmf_map;
    }
    ItemPointerData& getXyzItem()
    {
        return _mapData.xyz_map;
    }

    int getSectionIdx() const
    {
        return _sectionIdx;
    }
    int getStructureIdx() const
    {
        return _structureIdx;
    }

    indigo::Array<char>& getCmfBuf()
    {
        return _cmfBuf;
    }
    indigo::Array<char>& getXyzBuf()
    {
        return _xyzBuf;
    }

    void setBitsCount(unsigned short bits_count)
    {
        _bitsCount = bits_count;
    }
    unsigned short getBitsCount() const
    {
        return _bitsCount;
    }

private:
    BingoPgFpData(const BingoPgFpData&); // no implicit copy

protected:
    BingoPgBufferCacheMap::BingoMapData _mapData;

    int _sectionIdx;
    int _structureIdx;
    unsigned short _bitsCount;

    indigo::Array<char> _cmfBuf;
    indigo::Array<char> _xyzBuf;

    indigo::Array<int> _fingerprintBits;
};

class BingoPgSearchEngine
{
public:
    BingoPgSearchEngine();
    virtual ~BingoPgSearchEngine();

    virtual bool matchTarget(int section_idx, int structure_idx)
    {
        return false;
    }
    virtual bool matchTarget(ItemPointerData& item_data);

    virtual int getType() const
    {
        return 0;
    }

    virtual void prepareQuerySearch(BingoPgIndex&, PG_OBJECT scan_desc);
    virtual bool searchNext(PG_OBJECT result_ptr)
    {
        return false;
    }

    void setItemPointer(PG_OBJECT result_ptr);

    void loadDictionary(BingoPgIndex&);
    indigo::bingo_core::BingoCore bingoCore;
    //   const char* getDictionary(int& size);

private:
    BingoPgSearchEngine(const BingoPgSearchEngine&); // no implicit copy
protected:
    bool _searchNextCursor(PG_OBJECT result_ptr);
    bool _searchNextSub(PG_OBJECT result_ptr);

    void _setBingoContext();
    bool _fetchForNext();

    void _getBlockParameters(indigo::Array<char>& params);

    bool _fetchFound;
    int _currentSection;
    int _currentIdx;

    int _blockBegin;
    int _blockEnd;

    BingoPgIndex* _bufferIndexPtr;

    BingoPgExternalBitset _sectionBitset;
    std::unique_ptr<BingoPgFpData> _queryFpData;
    std::unique_ptr<BingoPgCursor> _searchCursor;
    std::unique_ptr<indigo::BingoContext> _bingoContext;
    std::unique_ptr<indigo::MangoContext> _mangoContext;
    std::unique_ptr<indigo::RingoContext> _ringoContext;
};

#endif /* BINGO_PG_SEARCH_ENGINE_H */
