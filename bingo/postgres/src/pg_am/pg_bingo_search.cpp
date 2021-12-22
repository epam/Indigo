#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "access/relscan.h"
#include "access/skey.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "utils/rel.h"
#include "utils/relcache.h"
#if PG_VERSION_NUM / 100 >= 1200
#include "access/genam.h"
#endif
}

#include "bingo_pg_fix_post.h"

#include "base_cpp/tlscont.h"
#include "bingo_pg_common.h"
#include "bingo_pg_search.h"
#include "bingo_pg_search_engine.h"
#include "pg_bingo_context.h"

#if PG_VERSION_NUM / 100 < 906

extern "C"
{
    BINGO_FUNCTION_EXPORT(bingo_beginscan);

    BINGO_FUNCTION_EXPORT(bingo_gettuple);
    /*
     * Turn off bitmap scan
     */
    // BINGO_FUNCTION_EXPORT(bingo_getbitmap);

    BINGO_FUNCTION_EXPORT(bingo_rescan);

    BINGO_FUNCTION_EXPORT(bingo_endscan);
}
#endif

//#include <signal.h>
//__sighandler_t old_handler = 0;

// static void error_handler(int i) {
////   signal(SIGINT, old_handler);
////   elog(WARNING, "aaa");
////   elog(ERROR, "query was cancelled");
////   throw CancelException();
//}

/*
 * Bingo searching initialization
 */

#if PG_VERSION_NUM / 100 >= 906
CEXPORT IndexScanDesc bingo_beginscan(Relation rel, int keysz, int norderbys)
{

#else
Datum bingo_beginscan(PG_FUNCTION_ARGS)
{
    Relation rel = (Relation)PG_GETARG_POINTER(0);
    int keysz = PG_GETARG_INT32(1);
#if PG_VERSION_NUM / 100 == 900
    ScanKey norderbys = (ScanKey)PG_GETARG_POINTER(2);
#elif PG_VERSION_NUM / 100 >= 901
    int norderbys = PG_GETARG_INT32(2);
#else
    elog(ERROR, "unsupported version %s", PG_VERSION)
#endif
#endif

    IndexScanDesc scan = RelationGetIndexScan(rel, keysz, norderbys);

    scan->opaque = 0;
    BingoPgSearch* so = 0;

    PG_BINGO_BEGIN
    {
        /*
         * Prepare search context
         */
        so = new BingoPgSearch(rel);

        /*
         * Setting bingo search context
         */
        scan->opaque = so;

        BingoPgWrapper rel_namespace;
        const char* index_schema = rel_namespace.getRelNameSpace(rel->rd_id);

        BingoPgCommon::appendPath(index_schema);
    }
    PG_BINGO_HANDLE(delete so; scan->opaque = NULL);

#if PG_VERSION_NUM / 100 >= 906
    return scan;
#else
    PG_RETURN_POINTER(scan);
#endif
}

/*
 * Rescan an index relation
 */
#if PG_VERSION_NUM / 100 >= 906
CEXPORT void bingo_rescan(IndexScanDesc scan, ScanKey scankey, int nkeys, ScanKey orderbys, int norderbys)
{
#else
Datum bingo_rescan(PG_FUNCTION_ARGS)
{
    IndexScanDesc scan = (IndexScanDesc)PG_GETARG_POINTER(0);
    ScanKey scankey = (ScanKey)PG_GETARG_POINTER(1);
#endif

    BingoPgSearch* so = 0;
    PG_BINGO_BEGIN
    {
        /*
         * Update scan key, if a new one is given
         */
        if (scankey && scan->numberOfKeys > 0)
        {
            memmove(scan->keyData, scankey, scan->numberOfKeys * sizeof(ScanKeyData));
        }

        so = (BingoPgSearch*)scan->opaque;
        if (so != NULL)
        {
            so->prepareRescan(scan);
        }
    }
    PG_BINGO_HANDLE(delete so; scan->opaque = NULL);

#if PG_VERSION_NUM / 100 < 906
    PG_RETURN_VOID();
#endif
}
/*
 * Close down a scan
 */

#if PG_VERSION_NUM / 100 >= 906
CEXPORT void bingo_endscan(IndexScanDesc scan)
{
#else
Datum bingo_endscan(PG_FUNCTION_ARGS)
{
    IndexScanDesc scan = (IndexScanDesc)PG_GETARG_POINTER(0);
#endif
    elog(DEBUG1, "bingo: search: finish searching");

    PG_BINGO_BEGIN
    {

        BingoPgSearch* so = (BingoPgSearch*)scan->opaque;
        /*
         * Delete bingo search context
         */
        if (so != NULL)
            delete so;
        scan->opaque = NULL;
    }
    PG_BINGO_END

#if PG_VERSION_NUM / 100 < 906
    PG_RETURN_VOID();
#endif
}
/*
 * Get all tuples at once
 */
// Datum
// bingo_getbitmap(PG_FUNCTION_ARGS) {
//   IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
//   TIDBitmap *tbm = (TIDBitmap *) PG_GETARG_POINTER(1);
//
//   qword item_size = 0;
//   BingoPgSearch* search_engine = (BingoPgSearch*) scan->opaque;
//   PG_BINGO_BEGIN
//   {
//      /*
//       * Create search engine
//       */
//      /*
//       * Fetch to the next item and add result to the bitmap
//       */
//      indigo::Array<ItemPointerData> found_items;
//      ItemPointer item_ptr;
//      do {
//         item_ptr = &found_items.push();
//      } while (search_engine->next(scan, item_ptr));
//      /*
//       * Pop the last element
//       */
//      found_items.pop();
//      item_size = found_items.size();
//      BINGO_PG_TRY {
//         tbm_add_tuples(tbm, found_items.ptr(), found_items.size(), false);
//      } BINGO_PG_HANDLE(throw BingoPgError("internal error: can not add bitmap solution: %s", message));
//   }
//   PG_BINGO_HANDLE(delete search_engine);
//
//   PG_RETURN_INT64(item_size);
//}
/*
 * Get a tuples by a chain
 */
#if PG_VERSION_NUM / 100 >= 906
CEXPORT bool bingo_gettuple(IndexScanDesc scan, ScanDirection dir)
{
#else
Datum bingo_gettuple(PG_FUNCTION_ARGS)
{
    IndexScanDesc scan = (IndexScanDesc)PG_GETARG_POINTER(0);
    ScanDirection dir = (ScanDirection)PG_GETARG_INT32(1);
#endif

    bool result = false;

    BingoPgSearch* search_engine = (BingoPgSearch*)scan->opaque;
    if (search_engine == NULL)
        elog(ERROR, "bingo: search error: search context was deleted");

    PG_BINGO_BEGIN
    {
        scan->xs_recheck = false;
/*
 * Fetch to the next item
 */
#if PG_VERSION_NUM / 100 >= 1200
        result = search_engine->next(scan, &scan->xs_heaptid);
#else
        result = search_engine->next(scan, &scan->xs_ctup.t_self);
#endif
    }
    PG_BINGO_HANDLE(delete search_engine; scan->opaque = NULL);
    /*
     * If true then searching was successfull
     */
#if PG_VERSION_NUM / 100 >= 906
    return result;
#else
    PG_RETURN_BOOL(result);
#endif
}
