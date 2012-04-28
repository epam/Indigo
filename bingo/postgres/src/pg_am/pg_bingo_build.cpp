extern "C"  {
#include "postgres.h"
#include "fmgr.h"
#include "access/htup.h"
#include "utils/relcache.h"
#include "storage/bufmgr.h"
#include "catalog/index.h"
}
#ifdef qsort
#undef qsort
#endif
#include "bingo_postgres.h"
#include "pg_bingo_context.h"
#include "bingo_pg_build.h"
#include "bingo_pg_common.h"
#include "bingo_pg_text.h"
#include "base_cpp/tlscont.h"




using namespace indigo;

extern "C" {
#ifdef PG_MODULE_MAGIC
   PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(bingo_build);
PGDLLEXPORT Datum bingo_build(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(bingo_buildempty);
PGDLLEXPORT Datum bingo_buildempty(PG_FUNCTION_ARGS);
}

static void bingoIndexCallback(Relation index,
        HeapTuple htup,
        Datum *values,
        bool *isnull,
        bool tupleIsAlive,
        void *state);

//#include <signal.h>
//void error_handler(int i) {
//   elog(ERROR, "query was cancelled");
//}

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

//   signal(SIGINT, &error_handler);
   elog(DEBUG1, "bingo: build: start building index");
   

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

   
   PG_BINGO_BEGIN
   {
     /*
      * Initialize the bingo index metadata page and initial blocks
      */
      BingoPgWrapper func_namespace;
      const char* schema_name = func_namespace.getFuncNameSpace(fcinfo->flinfo->fn_oid);
      BingoPgWrapper rel_namespace;
      const char* index_schema = rel_namespace.getRelNameSpace(index->rd_id);

      
      BingoPgBuild build_engine(index, schema_name, index_schema, true);
      /*
       * Do the heap scan and build index
       */
      BINGO_PG_TRY {
         reltuples = IndexBuildHeapScan(heap, index, indexInfo, true,
              bingoIndexCallback, (void *) &build_engine);
      } BINGO_PG_HANDLE(throw BingoPgError("Error while executing build index procedure %s", message));

//      build_engine.flush();
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
//   build_engine.insertStructureParallel(&htup->t_self, values[0]);
}

Datum
bingo_buildempty(PG_FUNCTION_ARGS) {
   Relation index = (Relation) PG_GETARG_POINTER(0);

   elog(NOTICE, "start bingo empty build ");


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


   PG_BINGO_BEGIN
   {
      /*
       * Initialize the bingo index metadata page and initial blocks
       */
      BingoPgWrapper func_namespace;
      const char* schema_name = func_namespace.getFuncNameSpace(fcinfo->flinfo->fn_oid);
      BingoPgWrapper rel_namespace;
      const char* index_schema = rel_namespace.getRelNameSpace(index->rd_id);

      BingoPgBuild build_engine(index, schema_name, index_schema, true);
   }
   PG_BINGO_END

   PG_RETURN_VOID();
}
