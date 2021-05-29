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

#include "base_c/bitarray.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "bingo_core_c.h"

#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"
#include "bingo_pg_text.h"

using namespace indigo;

IMPL_ERROR(RingoPgSearchEngine, "reaction search engine");

RingoPgSearchEngine::RingoPgSearchEngine(BingoPgConfig& bingo_config, const char* rel_name) : BingoPgSearchEngine(), _searchType(-1)
{
    _setBingoContext();
    /*
     * Set up bingo configuration
     */
    bingo_config.setUpBingoConfiguration();
    bingoTautomerRulesReady(0, 0, 0);
    bingoIndexBegin();

    _relName.readString(rel_name, true);
    _shadowRelName.readString(rel_name, true);
    _shadowRelName.appendString("_shadow", true);
}

RingoPgSearchEngine::~RingoPgSearchEngine()
{
    bingoIndexEnd();
}

bool RingoPgSearchEngine::matchTarget(int section_idx, int structure_idx)
{
    bool result = false;
    int bingo_res;
    QS_DEF(std::string, react_buf);
    react_buf.clear();

    _bufferIndexPtr->readCmfItem(section_idx, structure_idx, react_buf);
    bingo_res = ringoMatchTargetBinary(react_buf.ptr(), react_buf.sizeInBytes());
    CORE_HANDLE_ERROR_TID(bingo_res, -1, "reaction search engine: error while matching target", section_idx, structure_idx, bingoGetError());
    CORE_RETURN_WARNING_TID(bingo_res, 0, "reaction search engine: error while matching target", section_idx, structure_idx, bingoGetWarning());

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
    _queryFpData.reset(new RingoPgFpData());

    _setBingoContext();
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
    _setBingoContext();

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

void RingoPgSearchEngine::_prepareExactQueryStrings(indigo::std::string& what_clause_str, indigo::std::string& from_clause_str,
                                                    indigo::std::string& where_clause_str)
{
    StringOutput what_clause(what_clause_str);
    StringOutput from_clause(from_clause_str);
    StringOutput where_clause(where_clause_str);

    what_clause.printf("b_id");
    from_clause.printf("%s", _shadowRelName.ptr());

    dword ex_hash;
    int bingo_res = ringoGetHash(0, &ex_hash);
    CORE_HANDLE_ERROR(bingo_res, 1, "reaction search engine: error while getting hash", bingoGetError());

    where_clause.printf("ex_hash=%d", ex_hash);

    what_clause_str.push(0);
    from_clause_str.push(0);
    where_clause_str.push(0);
}

void RingoPgSearchEngine::_prepareSubSearch(PG_OBJECT scan_desc_ptr)
{
    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;
    QS_DEF(std::string, search_type);
    int bingo_res;
    std::string search_query;
    std::string search_options;
    BingoPgFpData& data = _queryFpData.ref();

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
    bingo_res = ringoSetupMatch(search_type.ptr(), search_query.ptr(), search_options.ptr());
    CORE_HANDLE_ERROR(bingo_res, 1, "reaction search engine: can not set rsub search context", bingoGetError());

    const char* fingerprint_buf;
    int fp_len;

    bingo_res = ringoGetQueryFingerprint(&fingerprint_buf, &fp_len);
    CORE_HANDLE_ERROR(bingo_res, 1, "reaction search engine: can not get query fingerprint", bingoGetError());

    int size_bits = fp_len * 8;
    data.setFingerPrints(fingerprint_buf, size_bits);
}

void RingoPgSearchEngine::_prepareExactSearch(PG_OBJECT scan_desc_ptr)
{
    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;
    QS_DEF(std::string, what_clause);
    QS_DEF(std::string, from_clause);
    QS_DEF(std::string, where_clause);
    QS_DEF(std::string, search_type);
    std::string search_query;
    std::string search_options;
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
    bingo_res = ringoSetupMatch(search_type.ptr(), search_query.ptr(), search_options.ptr());
    CORE_HANDLE_ERROR(bingo_res, 1, "reaction search engine: can not set rexact search context", bingoGetError());

    _prepareExactQueryStrings(what_clause, from_clause, where_clause);

    _searchCursor.reset(new BingoPgCursor("SELECT %s FROM %s WHERE %s", what_clause.ptr(), from_clause.ptr(), where_clause.ptr()));
}

void RingoPgSearchEngine::_prepareSmartsSearch(PG_OBJECT scan_desc_ptr)
{
    _prepareSubSearch(scan_desc_ptr);
}

void RingoPgSearchEngine::_getScanQueries(uintptr_t arg_datum, indigo::std::string& str1_out, indigo::std::string& str2_out)
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
