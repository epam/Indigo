/*
 */

#ifndef _MANGO_PG_SEARCH_ENGINE_H__
#define _MANGO_PG_SEARCH_ENGINE_H__

#include "bingo_pg_search_engine.h"

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/red_black.h"
#include <memory>

#include "bingo_pg_cursor.h"
#include "bingo_postgres.h"
#include "pg_bingo_context.h"

#include <cfloat>

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;

class MangoPgFpData : public BingoPgFpData
{
public:
    MangoPgFpData() : _mass(0), _fragments(0)
    {
    }
    ~MangoPgFpData() override
    {
    }

    void setMass(float mass)
    {
        _mass = mass;
    }
    float getMass() const
    {
        return _mass;
    }

    void insertHash(dword hash, int c_cnt);
    const indigo::RedBlackMap<dword, int>& getHashes() const
    {
        return _hashes;
    }

    void setGrossStr(const char* gross_str, const char* counter_str);
    const char* getGrossStr() const
    {
        return _gross.ptr();
    }

    int getFragmentsCount() const
    {
        return _fragments;
    }
    void setFragmentsCount(int fr)
    {
        _fragments = fr;
    }

private:
    MangoPgFpData(const MangoPgFpData&); // no implicit copy

    float _mass;
    int _fragments;
    /*
     * Map: hash - components count
     */
    indigo::RedBlackMap<dword, int> _hashes;
    indigo::Array<char> _gross;
};

/*
 * Class for procession molecule fingerprint data
 */
class MangoPgSearchEngine : public BingoPgSearchEngine
{
public:
    enum
    {
        MAX_HASH_ELEMENTS = 5
    };
    MangoPgSearchEngine(BingoPgConfig& bingo_config, const char* rel_name);
    ~MangoPgSearchEngine() override;

    bool matchTarget(int section_idx, int structure_idx) override;
    bool matchTarget(ItemPointerData& item_data) override
    {
        return BingoPgSearchEngine::matchTarget(item_data);
    }

    int getType() const override
    {
        return BINGO_INDEX_TYPE_MOLECULE;
    }

    void prepareQuerySearch(BingoPgIndex&, PG_OBJECT scan_desc) override;
    bool searchNext(PG_OBJECT result_ptr) override;

    DECL_ERROR;

private:
    MangoPgSearchEngine(const MangoPgSearchEngine&); // no implicit copy

    bool _searchNextSim(PG_OBJECT result_ptr);

    void _prepareExactQueryStrings(indigo::Array<char>& what_clause, indigo::Array<char>& from_clause, indigo::Array<char>& where_clause);
    void _prepareExactTauStrings(indigo::Array<char>& what_clause, indigo::Array<char>& from_clause, indigo::Array<char>& where_clause);

    void _prepareSubSearch(PG_OBJECT scan_desc);
    void _prepareExactSearch(PG_OBJECT scan_desc);
    void _prepareGrossSearch(PG_OBJECT scan_desc);
    void _prepareSmartsSearch(PG_OBJECT scan_desc);
    void _prepareMassSearch(PG_OBJECT scan_desc);
    void _prepareSimSearch(PG_OBJECT scan_desc);
    void _getScanQueries(uintptr_t arg_datum, indigo::Array<char>& str1, indigo::Array<char>& str2);
    void _getScanQueries(uintptr_t arg_datum, float& min_bound, float& max_bound, indigo::Array<char>& str1, indigo::Array<char>& str2);

    static void _errorHandler(const char* message, void* context);

    indigo::Array<char> _relName;
    indigo::Array<char> _shadowRelName;
    indigo::Array<char> _shadowHashRelName;

    int _searchType;
};
#endif /* MANGO_PG_SEARCH_ENGINE_H */
