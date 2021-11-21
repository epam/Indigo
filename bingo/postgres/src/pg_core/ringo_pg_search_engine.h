#ifndef _RINGO_PG_SEARCH_ENGINE_H__
#define _RINGO_PG_SEARCH_ENGINE_H__

#include "bingo_pg_search_engine.h"

#include "base_cpp/array.h"
#include <memory>
#include "base_cpp/exception.h"
#include "base_cpp/red_black.h"

#include "bingo_pg_cursor.h"
#include "bingo_postgres.h"
#include "pg_bingo_context.h"

class BingoPgText;
class BingoPgIndex;

class RingoPgFpData : public BingoPgFpData
{
public:
    RingoPgFpData() : _hash(0)
    {
    }
    ~RingoPgFpData() override
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
    RingoPgSearchEngine(const char* rel_name);
    ~RingoPgSearchEngine() override;

    bool matchTarget(int section_idx, int structure_idx) override;
    bool matchTarget(ItemPointerData& item_data) override
    {
        return BingoPgSearchEngine::matchTarget(item_data);
    }

    int getType() const override
    {
        return BINGO_INDEX_TYPE_REACTION;
    }

    void prepareQuerySearch(BingoPgIndex&, PG_OBJECT scan_desc) override;
    bool searchNext(PG_OBJECT result_ptr) override;

    DECL_ERROR;

private:
    RingoPgSearchEngine(const RingoPgSearchEngine&); // no implicit copy

    void _prepareExactQueryStrings(indigo::Array<char>& what_clause, indigo::Array<char>& from_clause, indigo::Array<char>& where_clause);

    void _prepareSubSearch(PG_OBJECT scan_desc);
    void _prepareExactSearch(PG_OBJECT scan_desc);
    void _prepareSmartsSearch(PG_OBJECT scan_desc);
    void _getScanQueries(uintptr_t arg_datum, indigo::Array<char>& str1, indigo::Array<char>& str2);

    static void _errorHandler(const char* message, void* context);

    indigo::Array<char> _relName;
    indigo::Array<char> _shadowRelName;

    int _searchType;
};

#endif /* RINGO_PG_SEARCH_ENGINE_H */
