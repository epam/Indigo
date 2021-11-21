#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"
#include "access/genam.h"
#include "access/relscan.h"
#include "fmgr.h"
#include "utils/typcache.h"
}

#ifdef qsort
#undef qsort
#endif

#include "mango_pg_search_engine.h"

#include "base_cpp/output.h"
#include "base_cpp/profiling.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "bingo_core_c.h"

#include "bingo_pg_common.h"
#include "bingo_pg_config.h"
#include "bingo_pg_index.h"
#include "bingo_pg_text.h"

using namespace indigo;

void MangoPgFpData::insertHash(dword hash, int c_cnt)
{
    int* comp_count = _hashes.at2(hash);
    if (comp_count)
    {
        (*comp_count) += c_cnt;
    }
    else
    {
        _hashes.insert(hash, c_cnt);
    }
}

void MangoPgFpData::setGrossStr(const char* gross_str, const char* counter_str)
{
    _gross.readString("'", true);
    _gross.appendString(gross_str, true);
    _gross.appendString("'", true);
    _gross.appendString(counter_str, true);
}

IMPL_ERROR(MangoPgSearchEngine, "molecule search engine");

MangoPgSearchEngine::MangoPgSearchEngine(const char* rel_name) : BingoPgSearchEngine(), _searchType(-1)
{
    // _setBingoContext();
    /*
     * Set up bingo configuration
     */
    bingoCore.bingoTautomerRulesReady(0, 0, 0);
    bingoCore.bingoIndexBegin();

    _relName.readString(rel_name, true);
    _shadowRelName.readString(rel_name, true);
    _shadowRelName.appendString("_shadow", true);
    _shadowHashRelName.readString(rel_name, true);
    _shadowHashRelName.appendString("_shadow_hash", true);
}

MangoPgSearchEngine::~MangoPgSearchEngine()
{
    // _setBingoContext();
    bingoCore.bingoIndexEnd();
}

bool MangoPgSearchEngine::matchTarget(int section_idx, int structure_idx)
{

    profTimerStart(t0, "mango_pg.match_target");

    if (_searchType == BingoPgCommon::MOL_SIM || _searchType == BingoPgCommon::MOL_MASS)
    {
        return true;
    }

    bool result = false;
    int bingo_res;
    QS_DEF(Array<char>, mol_buf);
    QS_DEF(Array<char>, xyz_buf);
    mol_buf.clear();
    xyz_buf.clear();
    // _setBingoContext();

    if (_searchType == BingoPgCommon::MOL_SUB || _searchType == BingoPgCommon::MOL_EXACT || _searchType == BingoPgCommon::MOL_SMARTS)
    {
        _bufferIndexPtr->readCmfItem(section_idx, structure_idx, mol_buf);
        bingo_res = bingoCore.mangoNeedCoords();
        CORE_HANDLE_ERROR(bingo_res, 0, "molecule search engine: error while getting coordinates flag", bingoGetError());

        if (bingo_res > 0)
        {
            _bufferIndexPtr->readXyzItem(section_idx, structure_idx, xyz_buf);
        }

        //      CORE_HANDLE_WARNING_TID(0, 1, "matching binary target", section_idx, structure_idx, " ");
        bingo_res = bingoCore.mangoMatchTargetBinary(mol_buf.ptr(), mol_buf.sizeInBytes(), xyz_buf.ptr(), xyz_buf.sizeInBytes());
        CORE_HANDLE_ERROR_TID(bingo_res, -1, "molecule search engine: error while matching binary target", section_idx, structure_idx, bingoGetError());
        CORE_RETURN_WARNING_TID(bingo_res, 0, "molecule search engine: error while matching binary target", section_idx, structure_idx, bingoGetWarning());

        result = (bingo_res > 0);
    }
    else if (_searchType == BingoPgCommon::MOL_GROSS)
    {
        BingoPgText gross_text;
        _searchCursor->getText(2, gross_text);

        int gross_len;
        gross_text.getText(gross_len);

        bingo_res = mangoMatchTarget(gross_text.getString(), gross_len);
        CORE_HANDLE_ERROR(bingo_res, -1, "molecule search engine: error while matching gross target", bingoGetError());
        CORE_RETURN_WARNING(bingo_res, 0, "molecule search engine: error while matching gross target", bingoGetWarning());

        result = (bingo_res > 0);
    }
    else
    {
        throw Error("internal error: undefined search type");
    }
    return result;
}

void MangoPgSearchEngine::prepareQuerySearch(BingoPgIndex& bingo_idx, PG_OBJECT scan_desc_ptr)
{

    profTimerStart(t0, "mango_pg.prepare_query_search");

    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;

    if (scan_desc->numberOfKeys >= 1 && scan_desc->numberOfKeys <= 2)
    {
        _searchType = scan_desc->keyData[0].sk_strategy;
    }
    else
    {
        throw Error("unsupported number of scan keys %d", scan_desc->numberOfKeys);
    }
    /*
     * Set pseudo types
     */
    if (_searchType == BingoPgCommon::MOL_MASS_GREAT || _searchType == BingoPgCommon::MOL_MASS_LESS)
        _searchType = BingoPgCommon::MOL_MASS;

    _queryFpData = std::make_unique<MangoPgFpData>();

    _setBingoContext();

    BingoPgSearchEngine::prepareQuerySearch(bingo_idx, scan_desc);

    switch (_searchType)
    {
    case BingoPgCommon::MOL_SUB:
        _prepareSubSearch(scan_desc);
        break;
    case BingoPgCommon::MOL_EXACT:
        _prepareExactSearch(scan_desc);
        break;
    case BingoPgCommon::MOL_GROSS:
        _prepareGrossSearch(scan_desc);
        break;
    case BingoPgCommon::MOL_SMARTS:
        _prepareSmartsSearch(scan_desc);
        break;
    case BingoPgCommon::MOL_MASS:
        _prepareMassSearch(scan_desc);
        break;
    case BingoPgCommon::MOL_SIM:
        _prepareSimSearch(scan_desc);
        break;
    default:
        throw Error("unsupported search type %d", _searchType);
        break;
    }
}

bool MangoPgSearchEngine::searchNext(PG_OBJECT result_ptr)
{

    bool result = false;
    _setBingoContext();
    if (_searchType == BingoPgCommon::MOL_EXACT || _searchType == BingoPgCommon::MOL_GROSS || _searchType == BingoPgCommon::MOL_MASS)
    {
        result = _searchNextCursor(result_ptr);
    }
    else if (_searchType == BingoPgCommon::MOL_SUB || _searchType == BingoPgCommon::MOL_SMARTS)
    {
        result = _searchNextSub(result_ptr);
    }
    else if (_searchType == BingoPgCommon::MOL_SIM)
    {
        result = _searchNextSim(result_ptr);
    }

    return result;
}

void MangoPgSearchEngine::_errorHandler(const char* message, void*)
{
    throw Error("Error while searching a molecule: %s", message);
}

void MangoPgSearchEngine::_prepareExactQueryStrings(indigo::Array<char>& what_clause_str, indigo::Array<char>& from_clause_str,
                                                    indigo::Array<char>& where_clause_str)
{
    int hash_elements_count, count, bingo_res;
    dword hash;

    ArrayOutput what_clause(what_clause_str);
    ArrayOutput from_clause(from_clause_str);
    ArrayOutput where_clause(where_clause_str);

    what_clause.printf("sh.b_id");

    bingo_res = mangoGetHash(false, -1, &hash_elements_count, &hash);
    CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: error while getting hash", bingoGetError());

    if (hash_elements_count > MAX_HASH_ELEMENTS)
        hash_elements_count = MAX_HASH_ELEMENTS;

    from_clause.printf("%s sh", _shadowRelName.ptr());

    for (int i = 0; i < hash_elements_count; i++)
        from_clause.printf(", %s t%d", _shadowHashRelName.ptr(), i);

    /*
     * Create complex WHERE clause
     */

    bool where_was_added = false;
    if (hash_elements_count > 0)
    {
        where_was_added = true;
        /*
         * molecule ids must be same
         */
        where_clause.printf("sh.b_id = t0.b_id AND ");
        for (int i = 1; i < hash_elements_count; i++)
            where_clause.printf("t%d.b_id = t%d.b_id AND ", i - 1, i);
        /*
         * query components must match target components
         */
        for (int i = 0; i < hash_elements_count; i++)
        {
            bingo_res = mangoGetHash(false, i, &count, &hash);
            CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: error while getting hash", bingoGetError());

            where_clause.printf("t%d.ex_hash = %d AND ", i, hash);
        }

        /*
         * components count mast must target components count
         */
        Array<char> rel;
        bingo_res = mangoExactNeedComponentMatching();
        CORE_HANDLE_ERROR(bingo_res, 0, "molecule search engine: error while getting need matching", bingoGetError());

        if (bingo_res > 0)
            rel.readString(">=", true);
        else
            rel.readString("=", true);

        for (int i = 0; i < hash_elements_count; i++)
        {
            if (i != 0)
                where_clause.printf("AND ");
            bingo_res = mangoGetHash(false, i, &count, &hash);
            CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: error while getting hash", bingoGetError());

            where_clause.printf("t%d.f_count %s %d ", i, rel.ptr(), count);
        }
    }
    bingo_res = mangoExactNeedComponentMatching();
    CORE_HANDLE_ERROR(bingo_res, 0, "molecule search engine: error while getting need matching", bingoGetError());
    if (bingo_res == 0)
    {
        if (where_was_added)
            where_clause.printf("AND ");

        /*
         * There must be no other components in target
         */
        int query_fragments_count = 0;
        for (int i = 0; i < hash_elements_count; i++)
        {
            bingo_res = mangoGetHash(false, i, &count, &hash);
            CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: error while getting hash", bingoGetError());
            query_fragments_count += count;
        }
        where_clause.printf("sh.fragments = %d", query_fragments_count);
    }
    what_clause_str.push(0);
    from_clause_str.push(0);
    where_clause_str.push(0);
}

void MangoPgSearchEngine::_prepareExactTauStrings(indigo::Array<char>& what_clause_str, indigo::Array<char>& from_clause_str,
                                                  indigo::Array<char>& where_clause_str)
{
    ArrayOutput what_clause(what_clause_str);
    ArrayOutput from_clause(from_clause_str);
    ArrayOutput where_clause(where_clause_str);

    what_clause.printf("b_id");
    from_clause.printf("%s", _shadowRelName.ptr());

    const char* query_gross = mangoTauGetQueryGross();
    if (query_gross == 0)
        CORE_HANDLE_ERROR(0, 1, "molecule search engine: error while constructing gross string", bingoGetError());

    where_clause.printf("gross='%s' OR gross LIKE '%s H%%'", query_gross, query_gross);

    what_clause_str.push(0);
    from_clause_str.push(0);
    where_clause_str.push(0);
}

void MangoPgSearchEngine::_prepareSubSearch(PG_OBJECT scan_desc_ptr)
{
    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;
    Array<char> search_type;
    Array<char> search_query;
    Array<char> search_options;

    if (scan_desc->numberOfKeys != 1)
    {
        throw BingoPgError("molecule search engine: unsupported condition number '%d': "
                           "if you want to search several queries please use 'matchSub' or 'matchSmarts' functions for a secondary condition",
                           scan_desc->numberOfKeys);
    }

    int bingo_res;
    BingoPgFpData& data = *_queryFpData;

    BingoPgCommon::getSearchTypeString(_searchType, search_type, true);

    _getScanQueries(scan_desc->keyData[0].sk_argument, search_query, search_options);
    /*
     * Get block parameters and split search options
     */
    _getBlockParameters(search_options);

    /*
     * Set up matching parameters
     */
    bingo_res = mangoSetupMatch(search_type.ptr(), search_query.ptr(), search_options.ptr());
    CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: can not set sub search context", bingoGetError());

    const char* fingerprint_buf;
    int fp_len;

    bingo_res = mangoGetQueryFingerprint(&fingerprint_buf, &fp_len);
    CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: can not get sub query fingerprint", bingoGetError());

    int size_bits = fp_len * 8;
    data.setFingerPrints(fingerprint_buf, size_bits);
}

void MangoPgSearchEngine::_prepareExactSearch(PG_OBJECT scan_desc_ptr)
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
        throw BingoPgError("molecule search engine: unsupported condition number '%d': "
                           "if you want to search several queries please use 'matchExact'  for a secondary condition",
                           scan_desc->numberOfKeys);
    }

    profTimerStart(t0, "mango_pg.prepare_exact_search");

    BingoPgCommon::getSearchTypeString(_searchType, search_type, true);
    _getScanQueries(scan_desc->keyData[0].sk_argument, search_query, search_options);

    /*
     * Set up matching parameters
     */
    //   elog(WARNING, "processing query: %s", search_query.ptr());
    bingo_res = mangoSetupMatch(search_type.ptr(), search_query.ptr(), search_options.ptr());
    CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: can not set exact search context", bingoGetError());

    if (strcasestr(search_options.ptr(), "TAU") != 0)
    {
        _prepareExactTauStrings(what_clause, from_clause, where_clause);
    }
    else
    {
        _prepareExactQueryStrings(what_clause, from_clause, where_clause);
    }
    _searchCursor.reset(nullptr);
    profTimerStart(t4, "mango_pg.exact_search_cursor");
    _searchCursor = std::make_unique<BingoPgCursor>("SELECT %s FROM %s WHERE %s", what_clause.ptr(), from_clause.ptr(), where_clause.ptr());
    profTimerStop(t4);
    //   if(nanoHowManySeconds(profTimerGetTime(t4) )> 1)
    //      elog(WARNING, "select %s from %s where %s", what_clause.ptr(), from_clause.ptr(), where_clause.ptr());
}

void MangoPgSearchEngine::_prepareGrossSearch(PG_OBJECT scan_desc_ptr)
{
    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;

    QS_DEF(Array<char>, gross_query);
    QS_DEF(Array<char>, search_type);
    Array<char> search_sigh;
    Array<char> search_mol;
    int bingo_res;

    if (scan_desc->numberOfKeys != 1)
    {
        throw BingoPgError("molecule search engine: unsupported condition number '%d': "
                           "if you want to search several queries please use 'matchGross'  for a secondary condition",
                           scan_desc->numberOfKeys);
    }

    BingoPgCommon::getSearchTypeString(_searchType, search_type, true);

    _getScanQueries(scan_desc->keyData[0].sk_argument, search_sigh, search_mol);

    gross_query.readString(search_sigh.ptr(), true);
    gross_query.appendString(" ", true);
    gross_query.appendString(search_mol.ptr(), true);

    /*
     * Set up matching parameters
     */
    bingo_res = mangoSetupMatch(search_type.ptr(), gross_query.ptr(), 0);
    CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: can not set gross search context", bingoGetError());

    const char* gross_conditions = mangoGrossGetConditions();
    if (gross_conditions == 0)
        CORE_HANDLE_ERROR(0, 1, "molecule search engine: can not get gross conditions", bingoGetError());
    _searchCursor = std::make_unique<BingoPgCursor>("SELECT b_id, gross FROM %s WHERE %s", _shadowRelName.ptr(), gross_conditions);
}

void MangoPgSearchEngine::_prepareSmartsSearch(PG_OBJECT scan_desc_ptr)
{
    _prepareSubSearch(scan_desc_ptr);
}

void MangoPgSearchEngine::_prepareMassSearch(PG_OBJECT scan_desc_ptr)
{
    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;
    bool min_mass_flag = false;
    bool max_mass_flag = false;
    float min_mass = 0;
    float max_mass = FLT_MAX;
    QS_DEF(Array<char>, where_clause_str);
    ArrayOutput where_clause(where_clause_str);

    for (int arg_idx = 0; arg_idx < scan_desc->numberOfKeys; ++arg_idx)
    {
        ScanKeyData& key_data = scan_desc->keyData[arg_idx];
        if (key_data.sk_strategy == BingoPgCommon::MOL_MASS_LESS)
        {
            char* mass_string = DatumGetCString(key_data.sk_argument);
            BufferScanner mass_scanner(mass_string);
            max_mass = mass_scanner.readFloat();
            max_mass_flag = true;
        }
        else if (key_data.sk_strategy == BingoPgCommon::MOL_MASS_GREAT)
        {
            char* mass_string = DatumGetCString(key_data.sk_argument);
            BufferScanner mass_scanner(mass_string);
            min_mass = mass_scanner.readFloat();
            min_mass_flag = true;
        }
        else
        {
            throw Error("unsupported search mass and other types");
        }
    }

    if (min_mass_flag)
        where_clause.printf("mass > %f", min_mass);
    if (max_mass_flag)
        where_clause.printf("AND mass < %f", max_mass);
    where_clause.writeChar(0);
    _searchCursor = std::make_unique<BingoPgCursor>("SELECT b_id FROM %s WHERE %s", _shadowRelName.ptr(), where_clause_str.ptr());
}

void MangoPgSearchEngine::_prepareSimSearch(PG_OBJECT scan_desc_ptr)
{
    IndexScanDesc scan_desc = (IndexScanDesc)scan_desc_ptr;
    QS_DEF(Array<char>, search_type);
    Array<char> search_query;
    Array<char> search_options;
    int bingo_res;
    float min_bound = 0, max_bound = 1;
    BingoPgFpData& data = *_queryFpData;

    BingoPgCommon::getSearchTypeString(_searchType, search_type, true);

    if (scan_desc->numberOfKeys != 1)
    {
        throw BingoPgError("molecule search engine: unsupported condition number '%d': "
                           "if you want to search several queries please use 'matchSim'  for a secondary condition",
                           scan_desc->numberOfKeys);
    }

    _getScanQueries(scan_desc->keyData[0].sk_argument, min_bound, max_bound, search_query, search_options);
    /*
     * Get block parameters and split search options
     */
    _getBlockParameters(search_options);
    /*
     * Set up matching parameters
     */
    bingo_res = mangoSetupMatch(search_type.ptr(), search_query.ptr(), search_options.ptr());
    CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: can not set sim search context", bingoGetError());

    if (min_bound > max_bound)
        throw Error("min bound %f can not be greater then max bound %f", min_bound, max_bound);

    bingo_res = mangoSimilaritySetMinMaxBounds(min_bound, max_bound);
    CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: can not get similarity min max bounds", bingoGetError());

    const char* fingerprint_buf;
    int fp_len;

    bingo_res = mangoGetQueryFingerprint(&fingerprint_buf, &fp_len);
    CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: can not get query fingerprint", bingoGetError());

    int size_bits = fp_len * 8;
    data.setFingerPrints(fingerprint_buf, size_bits);
}

void MangoPgSearchEngine::_getScanQueries(uintptr_t arg_datum, Array<char>& str1_out, Array<char>& str2_out)
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

void MangoPgSearchEngine::_getScanQueries(uintptr_t arg_datum, float& min_bound, float& max_bound, indigo::Array<char>& str1_out, indigo::Array<char>& str2_out)
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

        if (ncolumns != 4)
            elog(ERROR, "internal error: expecting four columns in query but was %d", ncolumns);

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
        min_bound = DatumGetFloat4(values[0]);
        max_bound = DatumGetFloat4(values[1]);
        BingoPgText str1, str2;
        str1.init(values[2]);
        str2.init(values[3]);

        str1_out.readString(str1.getString(), true);
        str2_out.readString(str2.getString(), true);

        pfree(values);
        pfree(nulls);
        ReleaseTupleDesc(tupdesc);
    }
    BINGO_PG_HANDLE(throw Error("internal error: can not get scan query: %s", message));
}

bool MangoPgSearchEngine::_searchNextSim(PG_OBJECT result_ptr)
{

    profTimerStart(t0, "mango_pg.search_sim");
    /*
     * If there are mathces found on the previous steps
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

    BingoPgFpData& query_data = *_queryFpData;
    BingoPgIndex& bingo_index = *_bufferIndexPtr;
    QS_DEF(Array<int>, bits_count);
    QS_DEF(Array<int>, common_ones);
    BingoPgExternalBitset screening_bitset(BINGO_MOLS_PER_SECTION);

    int *min_bounds, *max_bounds, bingo_res;
    /*
     * Read first section
     */
    if (_currentSection < 0)
        _currentSection = _blockBegin;
    /*
     * Iterate through the sections
     */
    for (; _currentSection < _blockEnd; ++_currentSection)
    {
        _currentIdx = -1;
        /*
         * Get section existing structures
         */
        bingo_index.getSectionBitset(_currentSection, _sectionBitset);
        int possible_str_count = _sectionBitset.bitsNumber();
        /*
         * If there is no bits then screen whole the structures
         */
        if (query_data.bitEnd() != 0 && possible_str_count > 0)
        {
            /*
             * Read structures bits count
             */
            //         profTimerStart(t5, "mango_pg.get_section_bits");
            bingo_index.getSectionBitsCount(_currentSection, bits_count);
            //         profTimerStop(t5);
            /*
             * Prepare min max bounds
             */
            //         profTimerStart(t3, "mango_pg.get_min_max");
            bingo_res = mangoSimilarityGetBitMinMaxBoundsArray(bits_count.size(), bits_count.ptr(), &min_bounds, &max_bounds);
            //         profTimerStop(t3);

            CORE_HANDLE_ERROR(bingo_res, 1, "molecule search engine: error while getting similarity bounds array", bingoGetError());

            /*
             * Prepare common bits array
             */
            common_ones.resize(bits_count.size());
            common_ones.zerofill();
            /*
             * Iterate through the query bits
             */
            int iteration_idx = 0;
            int fp_count = query_data.bitEnd();
            for (int fp_idx = query_data.bitBegin(); fp_idx != query_data.bitEnd() && possible_str_count > 0; fp_idx = query_data.bitNext(fp_idx))
            {
                int fp_block = query_data.getBit(fp_idx);
                /*
                 * Copy passed structures on each iteration step
                 */
                //            profTimerStart(t7, "mango_pg.copy");
                screening_bitset.copy(_sectionBitset);
                //            profTimerStop(t7);

                /*
                 * Get commons in fingerprint buffer
                 */

                //            profTimerStart(t2, "mango_pg.andwith");
                bingo_index.andWithBitset(_currentSection, fp_block, screening_bitset);
                //            profTimerStop(t2);

                //            profTimerStart(t4, "mango_pg.new_common_ones");
                int screen_idx = screening_bitset.begin();
                for (; screen_idx != screening_bitset.end() && possible_str_count > 0; screen_idx = screening_bitset.next(screen_idx))
                {
                    /*
                     * Calculate new common ones
                     */
                    int& one_counter = common_ones[screen_idx];
                    ++one_counter;
                    /*
                     * If common ones is out of bounds then it is not passed the screening
                     */
                    if ((one_counter > max_bounds[screen_idx]) || ((one_counter + fp_count - iteration_idx) < min_bounds[screen_idx]))
                    {
                        _sectionBitset.set(screen_idx, false);
                        --possible_str_count;
                    }
                }
                //            profTimerStop(t4);
                ++iteration_idx;
            }

            /*
             * Screen the last time for all the possible structures
             */
            //         profTimerStart(t6, "mango_pg.lst_screen");
            int screen_idx = _sectionBitset.begin();
            for (; screen_idx != _sectionBitset.end() && possible_str_count > 0; screen_idx = _sectionBitset.next(screen_idx))
            {
                int& one_counter = common_ones[screen_idx];
                if ((one_counter > max_bounds[screen_idx]) || (one_counter < min_bounds[screen_idx]))
                {
                    /*
                     * Not passed screening
                     */
                    _sectionBitset.set(screen_idx, false);
                    --possible_str_count;
                }
            }
            //         profTimerStop(t6);
        }
        /*
         * Return false on empty fingerprint
         */
        if (query_data.bitEnd() == 0)
            return false;

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
                profTimerStop(t0);
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