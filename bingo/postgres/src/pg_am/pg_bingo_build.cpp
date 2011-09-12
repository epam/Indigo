#include "bingo_postgres.h"
#include "pg_bingo_context.h"
#include "bingo_pg_build.h"
#include "bingo_pg_common.h"
#include "bingo_pg_text.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "access/htup.h"
#include "access/itup.h"
#include "utils/relcache.h"
#include "nodes/execnodes.h"
#include "storage/bufmgr.h"
#include "catalog/index.h"
#include "catalog/namespace.h"
#include "utils/lsyscache.h"
}


CEXPORT {
   PG_FUNCTION_INFO_V1(bingo_build);
   Datum bingo_build(PG_FUNCTION_ARGS);
}

static void bingoIndexCallback(Relation index,
        HeapTuple htup,
        Datum *values,
        bool *isnull,
        bool tupleIsAlive,
        void *state);

/*
 * Bingo build the index
 */
Datum
bingo_build(PG_FUNCTION_ARGS) {
   Relation heap = (Relation) PG_GETARG_POINTER(0);
   Relation index = (Relation) PG_GETARG_POINTER(1);
   IndexInfo *indexInfo = (IndexInfo *) PG_GETARG_POINTER(2);
//   BlockNumber relpages;
   IndexBuildResult *result = 0;
   double reltuples = 0;

   elog(INFO, "start bingo build");


   /*
    * We expect to be called exactly once for any index relation. If that's
    * not the case, big trouble's what we have.
    */
   if (RelationGetNumberOfBlocks(index) != 0)
      elog(ERROR, "index \"%s\" already contains data",
           RelationGetRelationName(index));

//   /* 
//    * Estimate the number of rows currently present in the table
//    */
//   estimate_rel_size(heap, NULL, &relpages, &reltuples);

   /* 
    * Initialize the bingo index metadata page and initial blocks
    */
   const char* schema_name = get_namespace_name(get_func_namespace(fcinfo->flinfo->fn_oid));

   PG_BINGO_BEGIN
   {
      BingoPgBuild build_engine(index, schema_name, true);
      /*
       * Do the heap scan and build index
       */
      BINGO_PG_TRY {
         reltuples = IndexBuildHeapScan(heap, index, indexInfo, true,
              bingoIndexCallback, (void *) &build_engine);
      } BINGO_PG_HANDLE(throw BingoPgError("Error while executing build index procedure %s", err->message));

      /*
       * Return statistics
       */
      result = (IndexBuildResult *) palloc(sizeof (IndexBuildResult));

      result->heap_tuples = reltuples;
      /*
       * Index is always cost cheaper so set tuples number 1
       */
      result->index_tuples = 1;
   }
   PG_BINGO_END

   PG_RETURN_POINTER(result);
}
/*
 * Bingo build callback. Accepts heap relation.
 */
static void bingoIndexCallback(Relation index,
        HeapTuple htup,
        Datum *values,
        bool *isnull,
        bool tupleIsAlive,
        void *state) {
   /*
    * Skip inserting null tuples
    */
   if(*isnull)
      return;


   /*
    * Get bingo state
    */
   BingoPgBuild &build_engine = *(BingoPgBuild *) state;

   /*
    * Molecule structure is a text
    */
   
   BingoPgText struct_text(values[0]);
   
   /*
    * Insert a new structure
    */
   build_engine.insertStructure(&htup->t_self, struct_text);
}


