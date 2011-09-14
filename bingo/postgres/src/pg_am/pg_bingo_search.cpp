#include "bingo_pg_search.h"
#include "bingo_postgres.h"
#include "pg_bingo_context.h"
#include "bingo_pg_search_engine.h"
#include "bingo_pg_common.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "access/skey.h"
#include "access/relscan.h"
#include "utils/rel.h"
#include "utils/relcache.h"
#include "utils/lsyscache.h"
}

CEXPORT {
PG_FUNCTION_INFO_V1(bingo_beginscan);
Datum bingo_beginscan(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_gettuple);
Datum bingo_gettuple(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_getbitmap);
Datum bingo_getbitmap(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_rescan);
Datum bingo_rescan(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_endscan);
Datum bingo_endscan(PG_FUNCTION_ARGS);
}
/*
 * Bingo searching initialization
 */

Datum
bingo_beginscan(PG_FUNCTION_ARGS) {
   Relation rel = (Relation) PG_GETARG_POINTER(0);
   int keysz = PG_GETARG_INT32(1);
   ScanKey scankey = (ScanKey) PG_GETARG_POINTER(2);
   elog(INFO, "start bingo search");
   
   IndexScanDesc scan = RelationGetIndexScan(rel, keysz, scankey);

   scan->opaque = 0;
   
   PG_BINGO_BEGIN
   {
      /*
       * Prepare search context
       */
      BingoPgSearch* so = new BingoPgSearch(rel);

      /*
       * Setting bingo search context
       */
      scan->opaque = so;
   }
   PG_BINGO_END;

   PG_RETURN_POINTER(scan);
}

/*
 *	Rescan an index relation
 */
Datum
bingo_rescan(PG_FUNCTION_ARGS) {
   IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
   ScanKey scankey = (ScanKey) PG_GETARG_POINTER(1);

   /*
    * Update scan key, if a new one is given
    */
   if (scankey && scan->numberOfKeys > 0) {
      memmove(scan->keyData, scankey, scan->numberOfKeys * sizeof (ScanKeyData));
   }

   PG_RETURN_VOID();
}

/*
 * Close down a scan
 */
Datum
bingo_endscan(PG_FUNCTION_ARGS) {
   IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
   elog(INFO, "end scan");
   
   PG_BINGO_BEGIN
   {

      BingoPgSearch* so = (BingoPgSearch*) scan->opaque;
      /*
       * Delete bingo search context
       */
      delete so;
      scan->opaque = NULL;
   }
   PG_BINGO_END

   PG_RETURN_VOID();
}

/*
 * Get all tuples at once
 */
Datum
bingo_getbitmap(PG_FUNCTION_ARGS) {
   IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
   TIDBitmap *tbm = (TIDBitmap *) PG_GETARG_POINTER(1);

   qword item_size = 0;
   BingoPgSearch* search_engine = (BingoPgSearch*) scan->opaque;
   PG_BINGO_BEGIN
   {
      /*
       * Create search engine
       */
      /*
       * Fetch to the next item and add result to the bitmap
       */
      indigo::Array<ItemPointerData> found_items;
      ItemPointer item_ptr;
      do {
         item_ptr = &found_items.push();
      } while (search_engine->next(scan, item_ptr));
      /*
       * Pop the last element
       */
      found_items.pop();

      BINGO_PG_TRY {
         tbm_add_tuples(tbm, found_items.ptr(), found_items.size(), false);
      } BINGO_PG_HANDLE(throw BingoPgError("internal error: can not add bitmap solution: %s", message));

      item_size = found_items.size();
   }
   PG_BINGO_HANDLE(delete search_engine);

   PG_RETURN_INT64(item_size);
}
/*
 * Get a tuples by a chain
 */
Datum
bingo_gettuple(PG_FUNCTION_ARGS) {
   IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
   ScanDirection dir = (ScanDirection) PG_GETARG_INT32(1);
   bool result = false;

   BingoPgSearch* search_engine = (BingoPgSearch*) scan->opaque;
   PG_BINGO_BEGIN
   {
      scan->xs_recheck = false;
      /*
       * Get search engine
       */
      /*
       * Fetch to the next item
       */
      result = search_engine->next(scan, &scan->xs_ctup.t_self);

   }
   PG_BINGO_HANDLE(delete search_engine);
   /*
    * If true then searching was successfull
    */
   PG_RETURN_BOOL(result);
}