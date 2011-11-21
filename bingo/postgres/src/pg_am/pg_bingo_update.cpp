#include "bingo_pg_build.h"
#include "bingo_pg_common.h"
#include "bingo_postgres.h"
#include "bingo_pg_text.h"

extern "C" {
#include "postgres.h"
#include "fmgr.h"
#include "utils/relcache.h"
#include "utils/rel.h"
#include "catalog/index.h"
#include "storage/bufmgr.h"
}

extern "C" {
PG_FUNCTION_INFO_V1(bingo_insert);
PGDLLEXPORT Datum bingo_insert(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(bingo_bulkdelete);
PGDLLEXPORT Datum bingo_bulkdelete(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(bingo_vacuumcleanup);
PGDLLEXPORT Datum bingo_vacuumcleanup(PG_FUNCTION_ARGS);
}


/*
 *	Insert an index tuple into a hash table.
 *
 */
Datum
bingo_insert(PG_FUNCTION_ARGS) {
   Relation index = (Relation) PG_GETARG_POINTER(0);
   Datum *values = (Datum *) PG_GETARG_POINTER(1);
   bool isnull = *((bool*) PG_GETARG_POINTER(2));
   ItemPointer ht_ctid = (ItemPointer) PG_GETARG_POINTER(3);

   /*
    * Skip inserting null tuples
    */
   if(isnull)
      PG_RETURN_BOOL(false);

   bool result = false;

   PG_BINGO_BEGIN
   {
      BingoPgWrapper rel_namespace;
      const char* index_schema = rel_namespace.getRelNameSpace(index->rd_id);
      
      BingoPgBuild build_engine(index, 0, index_schema, false);
      /*
       * Molecule structure is a text
       */
      BingoPgText struct_text(values[0]);
      /*
       * Insert a new structure
       */
      result = build_engine.insertStructure(ht_ctid, struct_text);
   }
   PG_BINGO_END

   //#ifdef NOT_USED
   //	Relation	heapRel = (Relation) PG_GETARG_POINTER(4);
   //	IndexUniqueCheck checkUnique = (IndexUniqueCheck) PG_GETARG_INT32(5);
   //#endif
   //	IndexTuple	itup;
   //
   //	/* generate an index tuple */
   //	itup = _hash_form_tuple(rel, values, isnull);
   //	itup->t_tid = *ht_ctid;
   //
   //	/*
   //	 * If the single index key is null, we don't insert it into the index.
   //	 * Hash tables support scans on '='. Relational algebra says that A = B
   //	 * returns null if either A or B is null.  This means that no
   //	 * qualification used in an index scan could ever return true on a null
   //	 * attribute.  It also means that indices can't be used by ISNULL or
   //	 * NOTNULL scans, but that's an artifact of the strategy map architecture
   //	 * chosen in 1986, not of the way nulls are handled here.
   //	 */
   //	if (IndexTupleHasNulls(itup))
   //	{
   //		pfree(itup);
   //		PG_RETURN_BOOL(false);
   //	}
   //
   //	_hash_doinsert(rel, itup);
   //
   //	pfree(itup);

   PG_RETURN_BOOL(result);
}


/*
 * Bulk deletion of all index entries pointing to a set of heap tuples.
 * The set of target tuples is specified via a callback routine that tells
 * whether any given heap tuple (identified by ItemPointer) is being deleted.
 *
 * Result: a palloc'd struct containing statistical info for VACUUM displays.
 */
Datum
bingo_bulkdelete(PG_FUNCTION_ARGS) {
   IndexVacuumInfo *info = (IndexVacuumInfo *) PG_GETARG_POINTER(0);
   IndexBulkDeleteResult *stats = (IndexBulkDeleteResult *) PG_GETARG_POINTER(1);
   IndexBulkDeleteCallback bulk_del_cb = (IndexBulkDeleteCallback) PG_GETARG_POINTER(2);
   void *cb_state = (void *) PG_GETARG_POINTER(3);

   elog(NOTICE, "start test bulk delete");
   PG_BINGO_BEGIN
   {
      /*
       * Initialize local variables
       */
      Relation index_rel = info->index;
      double tuples_removed = 0;
      ItemPointerData item_data;
      ItemPointer item_ptr = &item_data;
      BingoPgExternalBitset section_bitset(BINGO_MOLS_PER_SECTION);

      /*
       * Create index manager
       */
      BingoPgIndex bingo_index(index_rel);

      /*
       * Iterate through all the sections and search for removed tuples
       */
      int section_idx = bingo_index.readBegin();
      for (; section_idx != bingo_index.readEnd(); section_idx = bingo_index.readNext(section_idx)) {
         bingo_index.getSectionBitset(section_idx, section_bitset);
         /*
          * Iterate through section structures
          */
         for (int mol_idx = section_bitset.begin(); mol_idx != section_bitset.end(); mol_idx = section_bitset.next(mol_idx)) {
            bingo_index.readTidItem(section_idx, mol_idx, item_ptr);
            if (bulk_del_cb(item_ptr, cb_state)) {
               /*
                * Remove the structure from the bingo index
                */
               bingo_index.removeStructure(section_idx, mol_idx);
               tuples_removed += 1;
            }
         }
      }
      /*
       * Write new structures number
       */
      bingo_index.writeMetaInfo();

      if (stats == NULL)
         stats = (IndexBulkDeleteResult *) palloc0(sizeof (IndexBulkDeleteResult));

      stats->estimated_count = false;
      stats->tuples_removed = tuples_removed;

   }
   PG_BINGO_END

   PG_RETURN_POINTER(stats);
}

/*
 * Post-VACUUM cleanup.
 *
 * Result: a palloc'd struct containing statistical info for VACUUM displays.
 */
Datum bingo_vacuumcleanup(PG_FUNCTION_ARGS) {
   IndexVacuumInfo *info = (IndexVacuumInfo *) PG_GETARG_POINTER(0);
   IndexBulkDeleteResult *stats = (IndexBulkDeleteResult *) PG_GETARG_POINTER(1);
   Relation rel = info->index;
   BlockNumber num_pages = 0;

   elog(NOTICE, "start test vacuum");
   /* 
    * If bulkdelete wasn't called, return NULL signifying no change
    * Note: this covers the analyze_only case too
    */
   if (stats == NULL) {
      PG_RETURN_POINTER(NULL);
   }
   /*
    * update statistics
    */
   num_pages = RelationGetNumberOfBlocks(rel);
   stats->num_pages = num_pages;
   stats->num_index_tuples = 1;
   stats->estimated_count = false;

   PG_RETURN_POINTER(stats);
}

