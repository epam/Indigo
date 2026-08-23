#include "bingo_pg_fix_pre.h"

extern "C"
{
#include "postgres.h"

#include "catalog/index.h"
#include "fmgr.h"
#include "storage/bufmgr.h"
#include "storage/lmgr.h"
#include "utils/rel.h"
#include "utils/relcache.h"
#if PG_VERSION_NUM / 100 >= 1200
#include "access/genam.h"
#endif
}

#include "bingo_pg_fix_post.h"

#include "bingo_pg_build.h"
#include "bingo_pg_common.h"
#include "bingo_pg_text.h"
#include "bingo_postgres.h"

#if PG_VERSION_NUM / 100 < 906
extern "C"
{

    BINGO_FUNCTION_EXPORT(bingo_insert);

    BINGO_FUNCTION_EXPORT(bingo_bulkdelete);

    BINGO_FUNCTION_EXPORT(bingo_vacuumcleanup);
}
#endif

/*
 * Bingo updates several index pages and metadata structures as one logical
 * operation. Those read/modify/write sequences are not safe when two backends
 * mutate the same Bingo index concurrently. Use PostgreSQL's per-relation
 * extension lock as a dedicated mutation mutex for the Bingo index.
 */
static void bingoLockIndexMutation(Relation index)
{
    LockRelationForExtension(index, ExclusiveLock);
}

static void bingoUnlockIndexMutation(Relation index)
{
    UnlockRelationForExtension(index, ExclusiveLock);
}

/*
 * Insert an index tuple into a bingo table.
 */
#if PG_VERSION_NUM / 100 >= 1400
CEXPORT bool bingo_insert(Relation index, Datum* values, bool* isnull, ItemPointer ht_ctid, Relation heapRelation, IndexUniqueCheck checkUnique,
                          bool indexUnchanged, struct IndexInfo* indexInfo)
{
#elif PG_VERSION_NUM / 100 >= 1000
CEXPORT bool bingo_insert(Relation index, Datum* values, bool* isnull, ItemPointer ht_ctid, Relation heapRelation, IndexUniqueCheck checkUnique,
                          struct IndexInfo* indexInfo)
{
#elif PG_VERSION_NUM / 100 >= 906
CEXPORT bool bingo_insert(Relation index, Datum* values, bool* isnull, ItemPointer ht_ctid, Relation heapRelation, IndexUniqueCheck checkUnique)
{
#else
Datum bingo_insert(PG_FUNCTION_ARGS)
{
    Relation index = (Relation)PG_GETARG_POINTER(0);
    Datum* values = (Datum*)PG_GETARG_POINTER(1);
    bool* isnull = ((bool*)PG_GETARG_POINTER(2));
    ItemPointer ht_ctid = (ItemPointer)PG_GETARG_POINTER(3);
#endif

    if (*isnull)
#if PG_VERSION_NUM / 100 >= 906
        return false;
#else
        PG_RETURN_BOOL(false);
#endif

    bool result = false;

    bingoLockIndexMutation(index);
    PG_TRY();
    {
        PG_BINGO_BEGIN
        {
            BingoPgWrapper rel_namespace;
            const char* index_schema = rel_namespace.getRelNameSpace(index->rd_id);

            BingoPgBuild build_engine(index, 0, index_schema, false);
            result = build_engine.insertStructureSingle(ht_ctid, values[0]);
        }
        PG_BINGO_END
    }
    PG_CATCH();
    {
        bingoUnlockIndexMutation(index);
        PG_RE_THROW();
    }
    PG_END_TRY();
    bingoUnlockIndexMutation(index);

#if PG_VERSION_NUM / 100 >= 906
    return result;
#else
    PG_RETURN_BOOL(result);
#endif
}

/*
 * Bulk deletion of all index entries pointing to a set of heap tuples.
 * The set of target tuples is specified via a callback routine that tells
 * whether any given heap tuple (identified by ItemPointer) is being deleted.
 */
#if PG_VERSION_NUM / 100 >= 906
CEXPORT IndexBulkDeleteResult* bingo_bulkdelete(IndexVacuumInfo* info, IndexBulkDeleteResult* stats, IndexBulkDeleteCallback bulk_del_cb, void* cb_state)
{
#else
Datum bingo_bulkdelete(PG_FUNCTION_ARGS)
{
    IndexVacuumInfo* info = (IndexVacuumInfo*)PG_GETARG_POINTER(0);
    IndexBulkDeleteResult* stats = (IndexBulkDeleteResult*)PG_GETARG_POINTER(1);
    IndexBulkDeleteCallback bulk_del_cb = (IndexBulkDeleteCallback)PG_GETARG_POINTER(2);
    void* cb_state = (void*)PG_GETARG_POINTER(3);
#endif

    elog(NOTICE, "bingo.index: start bulk delete");

    Relation index_rel = info->index;
    bingoLockIndexMutation(index_rel);
    PG_TRY();
    {
        PG_BINGO_BEGIN
        {
            ItemPointerData item_data;
            ItemPointer item_ptr = &item_data;
            BingoPgExternalBitset section_bitset(BINGO_MOLS_PER_SECTION);

            /*
             * VACUUM mutates the section existence bitsets and metapage count,
             * so it must use the same WAL-enabled update strategy as INSERT.
             */
            BingoPgIndex bingo_index(index_rel);
            bingo_index.updateBegin();

            for (int section_idx = 0; section_idx != bingo_index.readEnd(); section_idx = bingo_index.readNext(section_idx))
            {
                bingo_index.getSectionBitset(section_idx, section_bitset);
                for (int mol_idx = section_bitset.begin(); mol_idx != section_bitset.end(); mol_idx = section_bitset.next(mol_idx))
                {
                    bingo_index.readTidItem(section_idx, mol_idx, item_ptr);
                    if (bulk_del_cb(item_ptr, cb_state))
                    {
                        bingo_index.removeStructure(section_idx, mol_idx);
                    }
                }
            }
        }
        PG_BINGO_END
    }
    PG_CATCH();
    {
        bingoUnlockIndexMutation(index_rel);
        PG_RE_THROW();
    }
    PG_END_TRY();
    bingoUnlockIndexMutation(index_rel);

#if PG_VERSION_NUM / 100 >= 906
    return NULL;
#else
    PG_RETURN_POINTER(NULL);
#endif
}

/*
 * Post-VACUUM cleanup.
 */
#if PG_VERSION_NUM / 100 >= 906
CEXPORT IndexBulkDeleteResult* bingo_vacuumcleanup(IndexVacuumInfo* info, IndexBulkDeleteResult* stats)
{
#else
Datum bingo_vacuumcleanup(PG_FUNCTION_ARGS)
{
    IndexVacuumInfo* info = (IndexVacuumInfo*)PG_GETARG_POINTER(0);
    IndexBulkDeleteResult* stats = (IndexBulkDeleteResult*)PG_GETARG_POINTER(1);
#endif

    elog(NOTICE, "bingo.index: start post-vacuum");

#if PG_VERSION_NUM / 100 >= 906
    return NULL;
#else
    PG_RETURN_POINTER(NULL);
#endif
}
