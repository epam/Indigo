#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "fmgr.h"

#include "access/htup.h"
#include "catalog/index.h"
#include "catalog/pg_type.h"
#include "storage/bufmgr.h"
#include "utils/rel.h"

#if PG_VERSION_NUM / 100 >= 906
#include "access/amapi.h"
#endif

#if PG_VERSION_NUM / 100 >= 1200
#include "access/tableam.h"
#endif
}

#include "bingo_pg_fix_post.h"

#include "base_cpp/tlscont.h"

#include "bingo_pg_build.h"
#include "bingo_pg_common.h"
#include "bingo_postgres.h"
#include "pg_bingo_context.h"

enum
{
    BINGO_AM_STRATEGIES = 7,
    BINGO_AM_SUPPORT = 7
};

using namespace indigo;

extern "C"
{
#ifdef __MINGW32__
    EXPORT_SYMBOL PG_MODULE_MAGIC;
#else
    PG_MODULE_MAGIC;
#endif

#if PG_VERSION_NUM / 100 >= 906
    BINGO_FUNCTION_EXPORT(bingo_handler);

    CEXPORT IndexBuildResult* bingo_build(Relation, Relation, struct IndexInfo*);
    CEXPORT void bingo_buildempty(Relation);

#if PG_VERSION_NUM / 100 >= 1400
    CEXPORT bool bingo_insert(Relation, Datum*, bool*, ItemPointer, Relation, IndexUniqueCheck, bool indexUnchanged, struct IndexInfo*);
    CEXPORT void bingo_costestimate120(struct PlannerInfo*, struct IndexPath*, double, Cost*, Cost*, Selectivity*, double*, double*);
#elif PG_VERSION_NUM / 100 >= 1200
    CEXPORT bool bingo_insert(Relation, Datum*, bool*, ItemPointer, Relation, IndexUniqueCheck, struct IndexInfo*);
    CEXPORT void bingo_costestimate120(struct PlannerInfo*, struct IndexPath*, double, Cost*, Cost*, Selectivity*, double*, double*);
#elif PG_VERSION_NUM / 100 >= 1000
    CEXPORT bool bingo_insert(Relation, Datum*, bool*, ItemPointer, Relation, IndexUniqueCheck, struct IndexInfo*);
    CEXPORT void bingo_costestimate101(struct PlannerInfo*, struct IndexPath*, double, Cost*, Cost*, Selectivity*, double*, double*);
#else
    CEXPORT bool bingo_insert(Relation, Datum*, bool*, ItemPointer, Relation, IndexUniqueCheck);
    CEXPORT void bingo_costestimate96(struct PlannerInfo*, struct IndexPath*, double, Cost*, Cost*, Selectivity*, double*);
#endif
    CEXPORT IndexBulkDeleteResult* bingo_bulkdelete(IndexVacuumInfo*, IndexBulkDeleteResult*, IndexBulkDeleteCallback, void*);
    CEXPORT IndexBulkDeleteResult* bingo_vacuumcleanup(IndexVacuumInfo*, IndexBulkDeleteResult*);

    CEXPORT bytea* bingo_options(Datum, bool);
    CEXPORT bool bingo_validate(Oid);
    CEXPORT IndexScanDesc bingo_beginscan(Relation, int, int);
    CEXPORT void bingo_rescan(IndexScanDesc, ScanKey, int, ScanKey, int);
    CEXPORT void bingo_endscan(IndexScanDesc);
    CEXPORT bool bingo_gettuple(IndexScanDesc, ScanDirection);

#else
    BINGO_FUNCTION_EXPORT(bingo_build);
    BINGO_FUNCTION_EXPORT(bingo_buildempty);
#endif
}

#if PG_VERSION_NUM / 100 >= 906
/*
 * Bingo handler function: return IndexAmRoutine with access method parameters
 * and callbacks.
 */
Datum bingo_handler(PG_FUNCTION_ARGS)
{
    IndexAmRoutine* amroutine = makeNode(IndexAmRoutine);

    amroutine->amstrategies = BINGO_AM_STRATEGIES;
    amroutine->amsupport = BINGO_AM_SUPPORT;
    amroutine->amcanorder = false;
    amroutine->amcanorderbyop = false;
    amroutine->amcanbackward = true;
    amroutine->amcanunique = false;
    amroutine->amcanmulticol = false;
    amroutine->amoptionalkey = false;
    amroutine->amsearcharray = false;
    amroutine->amsearchnulls = false;
    amroutine->amstorage = false;
    amroutine->amclusterable = false;
    amroutine->ampredlocks = false;
    amroutine->amkeytype = INT4OID;

    amroutine->ambuild = bingo_build;
    amroutine->ambuildempty = bingo_buildempty;
    amroutine->aminsert = bingo_insert;
    amroutine->ambulkdelete = bingo_bulkdelete;
    amroutine->amvacuumcleanup = bingo_vacuumcleanup;
    amroutine->amcanreturn = NULL;

#if PG_VERSION_NUM / 100 >= 1000
    amroutine->amcanparallel = false;
    amroutine->amestimateparallelscan = NULL;
    amroutine->aminitparallelscan = NULL;
    amroutine->amparallelrescan = NULL;
#endif

#if PG_VERSION_NUM / 100 >= 1200
    amroutine->amcostestimate = bingo_costestimate120;
#elif PG_VERSION_NUM / 100 >= 1000
    amroutine->amcostestimate = bingo_costestimate101;
#else
    amroutine->amcostestimate = bingo_costestimate96;
#endif

    amroutine->amoptions = bingo_options;
    amroutine->amproperty = NULL;
    amroutine->amvalidate = bingo_validate;
    amroutine->ambeginscan = bingo_beginscan;
    amroutine->amrescan = bingo_rescan;
    amroutine->amgettuple = bingo_gettuple;
    amroutine->amendscan = bingo_endscan;
    amroutine->amgetbitmap = NULL;
    amroutine->ammarkpos = NULL;
    amroutine->amrestrpos = NULL;

    PG_RETURN_POINTER(amroutine);
}
#endif

#if PG_VERSION_NUM / 100 > 1200
static void bingoIndexCallback(Relation index, ItemPointer item_ptr, Datum* values, bool* isnull, bool tupleIsAlive, void* state);
#else
static void bingoIndexCallback(Relation index, HeapTuple htup, Datum* values, bool* isnull, bool tupleIsAlive, void* state);
#endif

//#include <signal.h>
// void error_handler(int i) {
//   elog(ERROR, "query was cancelled");
//}

/*
 * Bingo build the index
 */
#if PG_VERSION_NUM / 100 >= 906
CEXPORT IndexBuildResult* bingo_build(Relation heap, Relation index, struct IndexInfo* indexInfo)
{
#else
Datum bingo_build(PG_FUNCTION_ARGS)
{
    Relation heap = (Relation)PG_GETARG_POINTER(0);
    Relation index = (Relation)PG_GETARG_POINTER(1);
    IndexInfo* indexInfo = (IndexInfo*)PG_GETARG_POINTER(2);
#endif

    //   BlockNumber relpages;
    IndexBuildResult* result = 0;
    double reltuples = 0;

    //   signal(SIGINT, &error_handler);
    elog(DEBUG1, "bingo: build: start building index");

    /*
     * We expect to be called exactly once for any index relation. If that's
     * not the case, big trouble's what we have.
     */
    if (RelationGetNumberOfBlocks(index) != 0)
        elog(ERROR, "index \"%s\" already contains data", RelationGetRelationName(index));

    //   /*
    //    * Estimate the number of rows currently present in the table
    //    */
    //   estimate_rel_size(heap, NULL, &relpages, &reltuples);

    PG_BINGO_BEGIN
    {
        /*
         * Initialize the bingo index metadata page and initial blocks
         */
        BingoPgWrapper func_namespace;
#if PG_VERSION_NUM / 100 >= 906
        const char* schema_name = "bingo";
#else
        const char* schema_name = func_namespace.getFuncNameSpace(fcinfo->flinfo->fn_oid);
#endif

        BingoPgWrapper rel_namespace;
        const char* index_schema = rel_namespace.getRelNameSpace(index->rd_id);

        BingoPgBuild build_engine(index, schema_name, index_schema, true);
        /*
         * Do the heap scan and build index
         */
        BINGO_PG_TRY
        {
#if PG_VERSION_NUM / 100 >= 1200
            reltuples = table_index_build_scan(heap, index, indexInfo, true, true, bingoIndexCallback, (void*)&build_engine, NULL);
#elif PG_VERSION_NUM / 100 >= 1100
            reltuples = IndexBuildHeapScan(heap, index, indexInfo, true, bingoIndexCallback, (void*)&build_engine, NULL);
#else
        reltuples = IndexBuildHeapScan(heap, index, indexInfo, true, bingoIndexCallback, (void*)&build_engine);
#endif
        }
        BINGO_PG_HANDLE(throw BingoPgError("Error while executing build index procedure %s", message));

        build_engine.flush();
        /*
         * Return statistics
         */
        result = (IndexBuildResult*)palloc(sizeof(IndexBuildResult));

        result->heap_tuples = reltuples;
        /*
         * Index is always cost cheaper so set tuples number 1
         */
        result->index_tuples = 1;
    }
    PG_BINGO_END
#if PG_VERSION_NUM / 100 >= 906
    return result;
#else
    PG_RETURN_POINTER(result);
#endif
}
/*
 * Bingo build callback. Accepts heap relation.
 */
static void bingoIndexCallbackImpl(Relation index, PG_OBJECT item_ptr, Datum* values, bool* isnull, bool tupleIsAlive, void* state)
{
    /*
     * Skip inserting null tuples
     */
    if (*isnull)
        return;

    /*
     * Get bingo state
     */
    BingoPgBuild& build_engine = *(BingoPgBuild*)state;

    /*
     * Insert a new structure (single or parallel)
     */
    PG_BINGO_BEGIN
    {
        build_engine.insertStructure(item_ptr, values[0]);
    }
    PG_BINGO_END
}

#if PG_VERSION_NUM / 100 > 1200
static void bingoIndexCallback(Relation index, ItemPointer item_ptr, Datum* values, bool* isnull, bool tupleIsAlive, void* state)
{
    bingoIndexCallbackImpl(index, item_ptr, values, isnull, tupleIsAlive, state);
}
#else
static void bingoIndexCallback(Relation index, HeapTuple htup, Datum* values, bool* isnull, bool tupleIsAlive, void* state)
{
    bingoIndexCallbackImpl(index, &htup->t_self, values, isnull, tupleIsAlive, state);
}
#endif

#if PG_VERSION_NUM / 100 >= 906
CEXPORT void bingo_buildempty(Relation index)
{
#else
Datum bingo_buildempty(PG_FUNCTION_ARGS)
{
    Relation index = (Relation)PG_GETARG_POINTER(0);
#endif

    elog(NOTICE, "start bingo empty build ");

    /*
     * We expect to be called exactly once for any index relation. If that's
     * not the case, big trouble's what we have.
     */
    if (RelationGetNumberOfBlocks(index) != 0)
        elog(ERROR, "index \"%s\" already contains data", RelationGetRelationName(index));

    //   /*
    //    * Estimate the number of rows currently present in the table
    //    */
    //   estimate_rel_size(heap, NULL, &relpages, &reltuples);

    PG_BINGO_BEGIN
    {
        /*
         * Initialize the bingo index metadata page and initial blocks
         */
        BingoPgWrapper func_namespace;
#if PG_VERSION_NUM / 100 >= 906
        const char* schema_name = "bingo";
#else
        const char* schema_name = func_namespace.getFuncNameSpace(fcinfo->flinfo->fn_oid);
#endif
        BingoPgWrapper rel_namespace;
        const char* index_schema = rel_namespace.getRelNameSpace(index->rd_id);

        BingoPgBuild build_engine(index, schema_name, index_schema, true);
    }
    PG_BINGO_END
#if PG_VERSION_NUM / 100 < 906
    PG_RETURN_VOID();
#endif
}

CEXPORT bool bingo_validate(Oid opclassoid)
{
    return true;
}
