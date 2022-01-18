#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "access/genam.h"
#include "access/relscan.h"
#include "fmgr.h"
#include "utils/typcache.h"
}

#include "bingo_pg_fix_post.h"

#include "ringo_pg_search_engine.h"

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

#include "bingo_core_c.h"

#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"
#include "bingo_pg_text.h"

using namespace indigo;

IMPL_ERROR(RingoPgSearchEngine, "reaction search engine");

RingoPgSearchEngine::RingoPgSearchEngine(const char* rel_name) : BingoPgSearchEngine(), _searchType(-1)
{
    // _setBingoContext();
    _relName.readString(rel_name, true);
    _shadowRelName.readString(rel_name, true);
    _shadowRelName.appendString("_shadow", true);
}

RingoPgSearchEngine::~RingoPgSearchEngine()
{
    bingoCore.bingoIndexEnd();
}

bool RingoPgSearchEngine::matchTarget(int section_idx, int structure_idx)
{
    bool result = false;
    int bingo_res = 1;
    QS_DEF(Array<char>, react_buf);
    react_buf.clear();

    _bufferIndexPtr->readCmfItem(section_idx, structure_idx, react_buf);
    try {
        bingo_res = bingoCore.ringoMatchTargetBinary(react_buf.ptr(), react_buf.sizeInBytes());
    } CORE_CATCH_ERROR_TID("reaction search engine: error while matching target", section_idx, structure_idx)

    CORE_RETURN_WARNING_TID(bingo_res, 0, "reaction search engine: error while matching target", section_idx, structure_idx, bingoCore.warning.ptr());

    result = (bingo_res == 1);

    return result;
}

void RingoPgSearchEngine::prepareQuerySearch(BingoPgIndex& bingo_idx, PG_OBJECT scan_desc_ptr)
{

    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;

    if (scan_desc->numberOfKeys >= 1 && scan_desc->numberOfKeys <= 2)
    {
        _searchType = scan_desc->keyData[0].sk_strategy;
    }
    else
    {
        throw Error("unsupported number of scan keys %d", scan_desc->numberOfKeys);
    }
    _queryFpData = std::make_unique<RingoPgFpData>();

    BingoPgSearchEngine::prepareQuerySearch(bingo_idx, scan_desc);

    switch (_searchType)
    {
    case BingoPgCommon::REACT_SUB:
        _prepareSubSearch(scan_desc);
        break;
    case BingoPgCommon::REACT_EXACT:
        _prepareExactSearch(scan_desc);
        break;
    case BingoPgCommon::REACT_SMARTS:
        _prepareSmartsSearch(scan_desc);
        break;
    default:
        throw Error("unsupported search type");
    }
}

bool RingoPgSearchEngine::searchNext(PG_OBJECT result_ptr)
{

    bool result = false;
    // _setBingoContext();

    if (_searchType == BingoPgCommon::REACT_EXACT)
    {
        result = _searchNextCursor(result_ptr);
    }
    else if (_searchType == BingoPgCommon::REACT_SUB || _searchType == BingoPgCommon::REACT_SMARTS)
    {
        result = _searchNextSub(result_ptr);
    }

    return result;
}

void RingoPgSearchEngine::_errorHandler(const char* message, void*)
{
    throw Error("Error while searching a reaction: %s", message);
}

void RingoPgSearchEngine::_prepareExactQueryStrings(indigo::Array<char>& what_clause_str, indigo::Array<char>& from_clause_str,
                                                    indigo::Array<char>& where_clause_str)
{
    ArrayOutput what_clause(what_clause_str);
    ArrayOutput from_clause(from_clause_str);
    ArrayOutput where_clause(where_clause_str);

    what_clause.printf("b_id");
    from_clause.printf("%s", _shadowRelName.ptr());

    dword ex_hash;
    try {
        bingoCore.ringoGetHash(0, &ex_hash);
    } CORE_CATCH_ERROR("reaction search engine: error while getting hash")

    where_clause.printf("ex_hash=%d", ex_hash);

    what_clause_str.push(0);
    from_clause_str.push(0);
    where_clause_str.push(0);
}

void RingoPgSearchEngine::_prepareSubSearch(PG_OBJECT scan_desc_ptr)
{
    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;
    QS_DEF(Array<char>, search_type);
    Array<char> search_query;
    Array<char> search_options;
    BingoPgFpData& data = *_queryFpData;

    if (scan_desc->numberOfKeys != 1)
    {
        throw BingoPgError("reaction search engine: unsupported condition number '%d': "
                           "if you want to search several queries please use 'matchRSub' or 'matchRSmarts' functions for a secondary condition",
                           scan_desc->numberOfKeys);
    }

    BingoPgCommon::getSearchTypeString(_searchType, search_type, false);

    _getScanQueries(scan_desc->keyData[0].sk_argument, search_query, search_options);

    /*
     * Get block parameters and split search options
     */
    _getBlockParameters(search_options);

    /*
     * Set up matching parameters
     */
    try {
        bingoCore.ringoSetupMatch(search_type.ptr(), search_query.ptr(), search_options.ptr());
    } CORE_CATCH_ERROR("reaction search engine: can not set rsub search context")

    const char* fingerprint_buf;
    int fp_len;

    try {
        bingoCore.ringoGetQueryFingerprint(&fingerprint_buf, &fp_len);
    } CORE_CATCH_ERROR("reaction search engine: can not get query fingerprint")

    int size_bits = fp_len * 8;
    data.setFingerPrints(fingerprint_buf, size_bits);
}

void RingoPgSearchEngine::_prepareExactSearch(PG_OBJECT scan_desc_ptr)
{
    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;
    QS_DEF(Array<char>, what_clause);
    QS_DEF(Array<char>, from_clause);
    QS_DEF(Array<char>, where_clause);
    QS_DEF(Array<char>, search_type);
    Array<char> search_query;
    Array<char> search_options;
    int bingo_res;

    if (scan_desc->numberOfKeys != 1)
    {
        throw BingoPgError("reaction search engine: unsupported condition number '%d': "
                           "if you want to search several queries please use 'matchRExact' function for a secondary condition",
                           scan_desc->numberOfKeys);
    }

    BingoPgCommon::getSearchTypeString(_searchType, search_type, false);

    _getScanQueries(scan_desc->keyData[0].sk_argument, search_query, search_options);

    /*
     * Set up matching parameters
     */
    try {
        bingoCore.ringoSetupMatch(search_type.ptr(), search_query.ptr(), search_options.ptr());
    } CORE_CATCH_ERROR("reaction search engine: can not set rexact search context")

    _prepareExactQueryStrings(what_clause, from_clause, where_clause);

    _searchCursor = std::make_unique<BingoPgCursor>("SELECT %s FROM %s WHERE %s", what_clause.ptr(), from_clause.ptr(), where_clause.ptr());
}

void RingoPgSearchEngine::_prepareSmartsSearch(PG_OBJECT scan_desc_ptr)
{
    _prepareSubSearch(scan_desc_ptr);
}

void RingoPgSearchEngine::_getScanQueries(uintptr_t arg_datum, indigo::Array<char>& str1_out, indigo::Array<char>& str2_out)
{
    /*
     * Get query info
     */
    BINGO_PG_TRY
    {
        HeapTupleHeader query_data = DatumGetHeapTupleHeader(arg_datum);
        Oid tupType = HeapTupleHeaderGetTypeId(query_data);
        int32 tupTypmod = HeapTupleHeaderGetTypMod(query_data);
        TupleDesc tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
        int ncolumns = tupdesc->natts;

        if (ncolumns != 2)
            elog(ERROR, "internal error: expecting two columns in query but was %d", ncolumns);

        HeapTupleData tuple;
        /*
         * Build a temporary HeapTuple control structure
         */
        tuple.t_len = HeapTupleHeaderGetDatumLength(query_data);
        ItemPointerSetInvalid(&(tuple.t_self));
        tuple.t_tableOid = InvalidOid;
        tuple.t_data = query_data;

        Datum* values = (Datum*)palloc(ncolumns * sizeof(Datum));
        bool* nulls = (bool*)palloc(ncolumns * sizeof(bool));

        /*
         *  Break down the tuple into fields
         */
        heap_deform_tuple(&tuple, tupdesc, values, nulls);

        /*
         * Query tuple consist of query and options
         */
        BingoPgText str1, str2;
        str1.init(values[0]);
        str2.init(values[1]);

        str1_out.readString(str1.getString(), true);
        str2_out.readString(str2.getString(), true);

        pfree(values);
        pfree(nulls);
        ReleaseTupleDesc(tupdesc);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not get scan query: %s", message));
}
