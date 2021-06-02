#ifndef _RINGO_PG_SEARCH_ENGINE_H__
#define _RINGO_PG_SEARCH_ENGINE_H__

#include "bingo_pg_search_engine.h"

#include "base_cpp/array.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/exception.h"
#include "base_cpp/red_black.h"

#include "bingo_pg_cursor.h"
#include "bingo_postgres.h"
#include "pg_bingo_context.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;

class RingoPgFpData : public BingoPgFpData
{
public:
    RingoPgFpData() : _hash(0)
    {
    }
    virtual ~RingoPgFpData()
    {
    }

    void setHash(dword hash)
    {
        _hash = hash;
    }
    dword getHash() const
    {
        return _hash;
    }

private:
    RingoPgFpData(const RingoPgFpData&); // no implicit copy

    dword _hash;
};

/*
 * Class for procession reaction fingerprint data
 */
class RingoPgSearchEngine : public BingoPgSearchEngine
{
public:
    RingoPgSearchEngine(BingoPgConfig& bingo_config, const char* rel_name);
    virtual ~RingoPgSearchEngine();

    virtual bool matchTarget(int section_idx, int structure_idx);
    virtual bool matchTarget(ItemPointerData& item_data)
    {
        return BingoPgSearchEngine::matchTarget(item_data);
    }

    virtual int getType() const
    {
        return BINGO_INDEX_TYPE_REACTION;
    }

    virtual void prepareQuerySearch(BingoPgIndex&, PG_OBJECT scan_desc);
    virtual bool searchNext(PG_OBJECT result_ptr);

    DECL_ERROR;

private:
    RingoPgSearchEngine(const RingoPgSearchEngine&); // no implicit copy

    void _prepareExactQueryStrings(indigo::ArrayChar& what_clause, indigo::ArrayChar& from_clause, indigo::ArrayChar& where_clause);

    void _prepareSubSearch(PG_OBJECT scan_desc);
    void _prepareExactSearch(PG_OBJECT scan_desc);
    void _prepareSmartsSearch(PG_OBJECT scan_desc);
    void _getScanQueries(uintptr_t arg_datum, indigo::ArrayChar& str1, indigo::ArrayChar& str2);

    static void _errorHandler(const char* message, void* context);

    indigo::ArrayChar _relName;
    indigo::ArrayChar _shadowRelName;

    int _searchType;
};

#endif /* RINGO_PG_SEARCH_ENGINE_H */
