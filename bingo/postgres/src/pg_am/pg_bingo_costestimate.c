#include <ctype.h>
#include <math.h>

#include "fmgr.h"
#include "postgres.h"
#if PG_VERSION_NUM / 100 >= 1200
#include "access/relation.h"
#include "optimizer/optimizer.h"
#else
#include "nodes/relation.h"
#include "optimizer/predtest.h"
#endif
#include "optimizer/cost.h"
#include "utils/selfuncs.h"
#include "utils/spccache.h"

/*
#include "access/sysattr.h"
#include "catalog/index.h"
#include "catalog/pg_opfamily.h"
#include "catalog/pg_statistic.h"
#include "catalog/pg_type.h"
#include "executor/executor.h"
#include "mb/pg_wchar.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "optimizer/clauses.h"
#include "optimizer/pathnode.h"
#include "optimizer/paths.h"
#include "optimizer/plancat.h"
#include "optimizer/predtest.h"
#include "optimizer/restrictinfo.h"
#include "optimizer/var.h"
#include "parser/parse_coerce.h"
#include "parser/parsetree.h"
#include "utils/builtins.h"
#include "utils/bytea.h"
#include "utils/date.h"
#include "utils/datum.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/nabstime.h"
#include "utils/pg_locale.h"
#include "utils/selfuncs.h"
#include "utils/spccache.h"
#include "utils/syscache.h"
#include "utils/tqual.h"
*/

/*-------------------------------------------------------------------------
 *
 * Index cost estimation functions
 *
 * genericcostestimate is a general-purpose estimator for use when we
 * don't have any better idea about how to estimate.  Index-type-specific
 * knowledge can be incorporated in the type-specific routines.
 *
 * One bit of index-type-specific knowledge we can relatively easily use
 * in genericcostestimate is the estimate of the number of index tuples
 * visited.  If numIndexTuples is not 0 then it is used as the estimate,
 * otherwise we compute a generic estimate.
 *
 *-------------------------------------------------------------------------
 */

// static void bingo_genericcostestimate(PlannerInfo* root, IndexOptInfo* index, List* indexQuals, RelOptInfo* outer_rel, double numIndexTuples,
//                                       Cost* indexStartupCost, Cost* indexTotalCost, Selectivity* indexSelectivity, double* indexCorrelation)
// {
//     double numIndexPages;
//     double num_sa_scans;
//     double num_outer_scans;
//     double num_scans;
//     QualCost index_qual_cost;
//     double qual_op_cost;
//     double qual_arg_cost;
//     double spc_random_page_cost;
//     List* selectivityQuals;
//     ListCell* l;

//     /*----------
//      * If the index is partial, AND the index predicate with the explicitly
//      * given indexquals to produce a more accurate idea of the index
//      * selectivity.  However, we need to be careful not to insert redundant
//      * clauses, because clauselist_selectivity() is easily fooled into
//      * computing a too-low selectivity estimate.  Our approach is to add
//      * only the index predicate clause(s) that cannot be proven to be implied
//      * by the given indexquals.  This successfully handles cases such as a
//      * qual "x = 42" used with a partial index "WHERE x >= 40 AND x < 50".
//      * There are many other cases where we won't detect redundancy, leading
//      * to a too-low selectivity estimate, which will bias the system in favor
//      * of using partial indexes where possible.  That is not necessarily bad
//      * though.
//      *
//      * Note that indexQuals contains RestrictInfo nodes while the indpred
//      * does not.  This is OK for both predicate_implied_by() and
//      * clauselist_selectivity().
//      *----------
//      */
//     if (index->indpred != NIL)
//     {
//         List* predExtraQuals = NIL;

//         foreach (l, index->indpred)
//         {
//             Node* predQual = (Node*)lfirst(l);
//             List* oneQual = list_make1(predQual);
// #if PG_VERSION_NUM / 100 >= 1000
//             if (!predicate_implied_by(oneQual, indexQuals, true))
// #else
//             if (!predicate_implied_by(oneQual, indexQuals))
// #endif
//                 predExtraQuals = list_concat(predExtraQuals, oneQual);
//         }
//         /* list_concat avoids modifying the passed-in indexQuals list */
//         selectivityQuals = list_concat(predExtraQuals, indexQuals);
//     }
//     else
//         selectivityQuals = indexQuals;

//     /*
//      * Check for ScalarArrayOpExpr index quals, and estimate the number of
//      * index scans that will be performed.
//      */
//     num_sa_scans = 1;
//     foreach (l, indexQuals)
//     {
//         RestrictInfo* rinfo = (RestrictInfo*)lfirst(l);

//         if (IsA(rinfo->clause, ScalarArrayOpExpr))
//         {
//             ScalarArrayOpExpr* saop = (ScalarArrayOpExpr*)rinfo->clause;
//             int alength = estimate_array_length(lsecond(saop->args));

//             if (alength > 1)
//                 num_sa_scans *= alength;
//         }
//     }

//     /* Estimate the fraction of main-table tuples that will be visited */
//     *indexSelectivity = clauselist_selectivity(root, selectivityQuals, index->rel->relid, JOIN_INNER, NULL);

//     /*
//      * If caller didn't give us an estimate, estimate the number of index
//      * tuples that will be visited.  We do it in this rather peculiar-looking
//      * way in order to get the right answer for partial indexes.
//      */
//     if (numIndexTuples <= 0.0)
//     {
//         numIndexTuples = *indexSelectivity * index->rel->tuples;

//         /*
//          * The above calculation counts all the tuples visited across all
//          * scans induced by ScalarArrayOpExpr nodes.  We want to consider the
//          * average per-indexscan number, so adjust.  This is a handy place to
//          * round to integer, too.  (If caller supplied tuple estimate, it's
//          * responsible for handling these considerations.)
//          */
//         numIndexTuples = rint(numIndexTuples / num_sa_scans);
//     }

//     /*
//      * We can bound the number of tuples by the index size in any case. Also,
//      * always estimate at least one tuple is touched, even when
//      * indexSelectivity estimate is tiny.
//      */
//     /*
//         if (numIndexTuples > index->tuples)
//             numIndexTuples = index->tuples;a
//         if (numIndexTuples < 1.0)
//     */
//     numIndexTuples = 1.0;

//     /*
//      * Estimate the number of index pages that will be retrieved.
//      *
//      * We use the simplistic method of taking a pro-rata fraction of the total
//      * number of index pages.  In effect, this counts only leaf pages and not
//      * any overhead such as index metapage or upper tree levels. In practice
//      * this seems a better approximation than charging for access to the upper
//      * levels, perhaps because those tend to stay in cache under load.
//      */
//     /*
//         if (index->pages > 1 && index->tuples > 1)
//             numIndexPages = ceil(numIndexTuples * index->pages / index->tuples);
//         else
//     */
//     numIndexPages = 1.0;

//     /* fetch estimated page cost for schema containing index */
//     get_tablespace_page_costs(index->reltablespace, &spc_random_page_cost, NULL);

//     /*
//      * Now compute the disk access costs.
//      *
//      * The above calculations are all per-index-scan.  However, if we are in a
//      * nestloop inner scan, we can expect the scan to be repeated (with
//      * different search keys) for each row of the outer relation.  Likewise,
//      * ScalarArrayOpExpr quals result in multiple index scans.	This creates
//      * the potential for cache effects to reduce the number of disk page
//      * fetches needed.	We want to estimate the average per-scan I/O cost in
//      * the presence of caching.
//      *
//      * We use the Mackert-Lohman formula (see costsize.c for details) to
//      * estimate the total number of page fetches that occur.  While this
//      * wasn't what it was designed for, it seems a reasonable model anyway.
//      * Note that we are counting pages not tuples anymore, so we take N = T =
//      * index size, as if there were one "tuple" per page.
//      */
//     if (outer_rel != NULL && outer_rel->rows > 1)
//     {
//         num_outer_scans = outer_rel->rows;
//         num_scans = num_sa_scans * num_outer_scans;
//     }
//     else
//     {
//         num_outer_scans = 1;
//         num_scans = num_sa_scans;
//     }

//     if (num_scans > 1)
//     {
//         double pages_fetched;

//         /* total page fetches ignoring cache effects */
//         pages_fetched = numIndexPages * num_scans;

//         /* use Mackert and Lohman formula to adjust for cache effects */
//         pages_fetched = index_pages_fetched(pages_fetched, index->pages, (double)index->pages, root);

//         /*
//          * Now compute the total disk access cost, and then report a pro-rated
//          * share for each outer scan.  (Don't pro-rate for ScalarArrayOpExpr,
//          * since that's internal to the indexscan.)
//          */
//         *indexTotalCost = (pages_fetched * spc_random_page_cost) / num_outer_scans;
//     }
//     else
//     {
//         /*
//          * For a single index scan, we just charge spc_random_page_cost per
//          * page touched.
//          */
//         *indexTotalCost = numIndexPages * spc_random_page_cost;
//     }

//     /*
//      * A difficulty with the leaf-pages-only cost approach is that for small
//      * selectivities (eg, single index tuple fetched) all indexes will look
//      * equally attractive because we will estimate exactly 1 leaf page to be
//      * fetched.  All else being equal, we should prefer physically smaller
//      * indexes over larger ones.  (An index might be smaller because it is
//      * partial or because it contains fewer columns; presumably the other
//      * columns in the larger index aren't useful to the query, or the larger
//      * index would have better selectivity.)
//      *
//      * We can deal with this by adding a very small "fudge factor" that
//      * depends on the index size.  The fudge factor used here is one
//      * spc_random_page_cost per 100000 index pages, which should be small
//      * enough to not alter index-vs-seqscan decisions, but will prevent
//      * indexes of different sizes from looking exactly equally attractive.
//      */
//     *indexTotalCost += index->pages * spc_random_page_cost / 100000.0;
//     //*indexTotalCost += 1000;

//     /*
//      * CPU cost: any complex expressions in the indexquals will need to be
//      * evaluated once at the start of the scan to reduce them to runtime keys
//      * to pass to the index AM (see nodeIndexscan.c).  We model the per-tuple
//      * CPU costs as cpu_index_tuple_cost plus one cpu_operator_cost per
//      * indexqual operator.	Because we have numIndexTuples as a per-scan
//      * number, we have to multiply by num_sa_scans to get the correct result
//      * for ScalarArrayOpExpr cases.
//      *
//      * Note: this neglects the possible costs of rechecking lossy operators
//      * and OR-clause expressions.  Detecting that that might be needed seems
//      * more expensive than it's worth, though, considering all the other
//      * inaccuracies here ...
//      */
//     cost_qual_eval(&index_qual_cost, indexQuals, root);
//     qual_op_cost = cpu_operator_cost * list_length(indexQuals);
//     qual_arg_cost = index_qual_cost.startup + index_qual_cost.per_tuple - qual_op_cost;
//     if (qual_arg_cost < 0) /* just in case... */
//         qual_arg_cost = 0;
//     if (indexStartupCost)
//         *indexStartupCost = qual_arg_cost;
//     *indexTotalCost += qual_arg_cost;
//     *indexTotalCost += numIndexTuples * num_sa_scans * (cpu_index_tuple_cost + qual_op_cost);

//     /*
//      * We also add a CPU-cost component to represent the general costs of
//      * starting an indexscan, such as analysis of btree index keys and initial
//      * tree descent.  This is estimated at 100x cpu_operator_cost, which is a
//      * bit arbitrary but seems the right order of magnitude. (As noted above,
//      * we don't charge any I/O for touching upper tree levels, but charging
//      * nothing at all has been found too optimistic.)
//      *
//      * Although this is startup cost with respect to any one scan, we add it
//      * to the "total" cost component because it's only very interesting in the
//      * many-ScalarArrayOpExpr-scan case, and there it will be paid over the
//      * life of the scan node.
//      */
//     *indexTotalCost += num_sa_scans * 100.0 * cpu_operator_cost;

//     /*
//      * Generic assumption about index correlation: there isn't any.
//      */
//     *indexCorrelation = -1.0;
// }

#if PG_VERSION_NUM / 100 >= 1200

void bingo_costestimate120(struct PlannerInfo* root, struct IndexPath* path, double loop_count, Cost* indexStartupCost, Cost* indexTotalCost,
                           Selectivity* indexSelectivity, double* indexCorrelation, double* indexPages)
{

    GenericCosts costs;
    MemSet(&costs, 0, sizeof(costs));
    costs.numIndexTuples = 1;
    costs.numIndexPages = 1;
    genericcostestimate(root, path, loop_count, &costs);

    costs.indexTotalCost = 1;
    costs.indexCorrelation = -1;
    costs.numIndexPages = 1;

    *indexStartupCost = costs.indexStartupCost;
    *indexTotalCost = costs.indexTotalCost;
    *indexSelectivity = costs.indexSelectivity;
    *indexCorrelation = costs.indexCorrelation;
    *indexPages = costs.numIndexPages;
}

#else

static List* add_predicate_to_quals(IndexOptInfo* index, List* indexQuals)
{
    List* predExtraQuals = NIL;
    ListCell* lc;

    if (index->indpred == NIL)
        return indexQuals;

    foreach (lc, index->indpred)
    {
        Node* predQual = (Node*)lfirst(lc);
        List* oneQual = list_make1(predQual);
#if PG_VERSION_NUM / 100 >= 1000
        if (!predicate_implied_by(oneQual, indexQuals, true))
#else
        if (!predicate_implied_by(oneQual, indexQuals))
#endif
            predExtraQuals = list_concat(predExtraQuals, oneQual);
    }
    /* list_concat avoids modifying the passed-in indexQuals list */
    return list_concat(predExtraQuals, indexQuals);
}

static void genericcostestimate92(PlannerInfo* root, IndexPath* path, double loop_count, double numIndexTuples, Cost* indexStartupCost, Cost* indexTotalCost,
                                  Selectivity* indexSelectivity, double* indexCorrelation)
{
    IndexOptInfo* index = path->indexinfo;
    List* indexQuals = path->indexquals;
    List* indexOrderBys = path->indexorderbys;
    double numIndexPages;
    double num_sa_scans;
    double num_outer_scans;
    double num_scans;
    QualCost index_qual_cost;
    double qual_op_cost;
    double qual_arg_cost;
    double spc_random_page_cost;
    List* selectivityQuals;
    ListCell* l;

    /*
     * If the index is partial, AND the index predicate with the explicitly
     * given indexquals to produce a more accurate idea of the index
     * selectivity.
     */

    selectivityQuals = add_predicate_to_quals(index, indexQuals);

    /*
     * Check for ScalarArrayOpExpr index quals, and estimate the number of
     * index scans that will be performed.
     */
    num_sa_scans = 1;
    foreach (l, indexQuals)
    {
        RestrictInfo* rinfo = (RestrictInfo*)lfirst(l);

        if (IsA(rinfo->clause, ScalarArrayOpExpr))
        {
            ScalarArrayOpExpr* saop = (ScalarArrayOpExpr*)rinfo->clause;
            int alength = estimate_array_length(lsecond(saop->args));

            if (alength > 1)
                num_sa_scans *= alength;
        }
    }

    /* Estimate the fraction of main-table tuples that will be visited */
    *indexSelectivity = clauselist_selectivity(root, selectivityQuals, index->rel->relid, JOIN_INNER, NULL);

    /*
     * If caller didn't give us an estimate, estimate the number of index
     * tuples that will be visited.  We do it in this rather peculiar-looking
     * way in order to get the right answer for partial indexes.
     */
    if (numIndexTuples <= 0.0)
    {
        numIndexTuples = *indexSelectivity * index->rel->tuples;

        /*
         * The above calculation counts all the tuples visited across all
         * scans induced by ScalarArrayOpExpr nodes.  We want to consider the
         * average per-indexscan number, so adjust.  This is a handy place to
         * round to integer, too.  (If caller supplied tuple estimate, it's
         * responsible for handling these considerations.)
         */
        numIndexTuples = rint(numIndexTuples / num_sa_scans);
    }
    numIndexTuples = 1.0;

    /*
     * We can bound the number of tuples by the index size in any case. Also,
     * always estimate at least one tuple is touched, even when
     * indexSelectivity estimate is tiny.
     */
    if (numIndexTuples > index->tuples)
        numIndexTuples = index->tuples;
    if (numIndexTuples < 1.0)
        numIndexTuples = 1.0;

    /*
     * Estimate the number of index pages that will be retrieved.
     *
     * We use the simplistic method of taking a pro-rata fraction of the total
     * number of index pages.  In effect, this counts only leaf pages and not
     * any overhead such as index metapage or upper tree levels. In practice
     * this seems a better approximation than charging for access to the upper
     * levels, perhaps because those tend to stay in cache under load.
     */
    /*
        if (index->pages > 1 && index->tuples > 1)
            numIndexPages = ceil(numIndexTuples * index->pages / index->tuples);
        else
    */
    numIndexPages = 1.0;

    /* fetch estimated page cost for schema containing index */
    get_tablespace_page_costs(index->reltablespace, &spc_random_page_cost, NULL);

    /*
     * Now compute the disk access costs.
     *
     * The above calculations are all per-index-scan.  However, if we are in a
     * nestloop inner scan, we can expect the scan to be repeated (with
     * different search keys) for each row of the outer relation.  Likewise,
     * ScalarArrayOpExpr quals result in multiple index scans.	This creates
     * the potential for cache effects to reduce the number of disk page
     * fetches needed.	We want to estimate the average per-scan I/O cost in
     * the presence of caching.
     *
     * We use the Mackert-Lohman formula (see costsize.c for details) to
     * estimate the total number of page fetches that occur.  While this
     * wasn't what it was designed for, it seems a reasonable model anyway.
     * Note that we are counting pages not tuples anymore, so we take N = T =
     * index size, as if there were one "tuple" per page.
     */
    num_outer_scans = loop_count;
    num_scans = num_sa_scans * num_outer_scans;

    if (num_scans > 1)
    {
        double pages_fetched;

        /* total page fetches ignoring cache effects */
        pages_fetched = numIndexPages * num_scans;

        /* use Mackert and Lohman formula to adjust for cache effects */
        pages_fetched = index_pages_fetched(pages_fetched, index->pages, (double)index->pages, root);

        /*
         * Now compute the total disk access cost, and then report a pro-rated
         * share for each outer scan.  (Don't pro-rate for ScalarArrayOpExpr,
         * since that's internal to the indexscan.)
         */
        *indexTotalCost = (pages_fetched * spc_random_page_cost) / num_outer_scans;
    }
    else
    {
        /*
         * For a single index scan, we just charge spc_random_page_cost per
         * page touched.
         */
        *indexTotalCost = numIndexPages * spc_random_page_cost;
    }

    /*
     * A difficulty with the leaf-pages-only cost approach is that for small
     * selectivities (eg, single index tuple fetched) all indexes will look
     * equally attractive because we will estimate exactly 1 leaf page to be
     * fetched.  All else being equal, we should prefer physically smaller
     * indexes over larger ones.  (An index might be smaller because it is
     * partial or because it contains fewer columns; presumably the other
     * columns in the larger index aren't useful to the query, or the larger
     * index would have better selectivity.)
     *
     * We can deal with this by adding a very small "fudge factor" that
     * depends on the index size.  The fudge factor used here is one
     * spc_random_page_cost per 10000 index pages, which should be small
     * enough to not alter index-vs-seqscan decisions, but will prevent
     * indexes of different sizes from looking exactly equally attractive.
     */
    *indexTotalCost += index->pages * spc_random_page_cost / 100000.0;

    /*
     * CPU cost: any complex expressions in the indexquals will need to be
     * evaluated once at the start of the scan to reduce them to runtime keys
     * to pass to the index AM (see nodeIndexscan.c).  We model the per-tuple
     * CPU costs as cpu_index_tuple_cost plus one cpu_operator_cost per
     * indexqual operator.	Because we have numIndexTuples as a per-scan
     * number, we have to multiply by num_sa_scans to get the correct result
     * for ScalarArrayOpExpr cases.  Similarly add in costs for any index
     * ORDER BY expressions.
     *
     * Note: this neglects the possible costs of rechecking lossy operators
     * and OR-clause expressions.  Detecting that that might be needed seems
     * more expensive than it's worth, though, considering all the other
     * inaccuracies here ...
     */
    cost_qual_eval(&index_qual_cost, indexQuals, root);
    qual_arg_cost = index_qual_cost.startup + index_qual_cost.per_tuple;
    cost_qual_eval(&index_qual_cost, indexOrderBys, root);
    qual_arg_cost += index_qual_cost.startup + index_qual_cost.per_tuple;
    qual_op_cost = cpu_operator_cost * (list_length(indexQuals) + list_length(indexOrderBys));
    qual_arg_cost -= qual_op_cost;
    if (qual_arg_cost < 0) /* just in case... */
        qual_arg_cost = 0;

    *indexStartupCost = qual_arg_cost;
    *indexTotalCost += qual_arg_cost;
    *indexTotalCost += numIndexTuples * num_sa_scans * (cpu_index_tuple_cost + qual_op_cost);

    /*
     * We also add a CPU-cost component to represent the general costs of
     * starting an indexscan, such as analysis of btree index keys and initial
     * tree descent.  This is estimated at 100x cpu_operator_cost, which is a
     * bit arbitrary but seems the right order of magnitude. (As noted above,
     * we don't charge any I/O for touching upper tree levels, but charging
     * nothing at all has been found too optimistic.)
     *
     * Although this is startup cost with respect to any one scan, we add it
     * to the "total" cost component because it's only very interesting in the
     * many-ScalarArrayOpExpr-scan case, and there it will be paid over the
     * life of the scan node.
     */
    *indexTotalCost += num_sa_scans * 100.0 * cpu_operator_cost;
    *indexTotalCost = 1.0;

    /*
     * Generic assumption about index correlation: there isn't any.
     */
    *indexCorrelation = -1.0;
}

void bingo_costestimate96(struct PlannerInfo* root, struct IndexPath* path, double loop_count, Cost* indexStartupCost, Cost* indexTotalCost,
                          Selectivity* indexSelectivity, double* indexCorrelation)
{
    genericcostestimate92(root, path, loop_count, 1.0, indexStartupCost, indexTotalCost, indexSelectivity, indexCorrelation);
}

void bingo_costestimate101(struct PlannerInfo* root, struct IndexPath* path, double loop_count, Cost* indexStartupCost, Cost* indexTotalCost,
                           Selectivity* indexSelectivity, double* indexCorrelation, double* indexPages)
{

    genericcostestimate92(root, path, loop_count, 1.0, indexStartupCost, indexTotalCost, indexSelectivity, indexCorrelation);

    *indexPages = 1;
}
#endif

// #if PG_VERSION_NUM / 100 >= 904
// PGDLLEXPORT PG_FUNCTION_INFO_V1(bingo_costestimate);
// Datum bingo_costestimate(PG_FUNCTION_ARGS)
// {
//     PlannerInfo* root = (PlannerInfo*)PG_GETARG_POINTER(0);
//     IndexPath* path = (IndexPath*)PG_GETARG_POINTER(1);
//     double loop_count = PG_GETARG_FLOAT8(2);
//     Cost* indexStartupCost = (Cost*)PG_GETARG_POINTER(3);
//     Cost* indexTotalCost = (Cost*)PG_GETARG_POINTER(4);
//     Selectivity* indexSelectivity = (Selectivity*)PG_GETARG_POINTER(5);
//     double* indexCorrelation = (double*)PG_GETARG_POINTER(6);

//     genericcostestimate92(root, path, loop_count, 1.0, indexStartupCost, indexTotalCost, indexSelectivity, indexCorrelation);

//     PG_RETURN_VOID();
// }

// Datum bingo_costestimate(PG_FUNCTION_ARGS)
// {

// #if PG_VERSION_NUM / 100 >= 902
//     PlannerInfo* root = (PlannerInfo*)PG_GETARG_POINTER(0);
//     IndexPath* path = (IndexPath*)PG_GETARG_POINTER(1);
//     double loop_count = PG_GETARG_FLOAT8(2);
//     Cost* indexStartupCost = (Cost*)PG_GETARG_POINTER(3);
//     Cost* indexTotalCost = (Cost*)PG_GETARG_POINTER(4);
//     Selectivity* indexSelectivity = (Selectivity*)PG_GETARG_POINTER(5);
//     double* indexCorrelation = (double*)PG_GETARG_POINTER(6);

//     genericcostestimate92(root, path, loop_count, 1.0, indexStartupCost, indexTotalCost, indexSelectivity, indexCorrelation);
// #else

//     struct PlannerInfo* root;
//     struct IndexOptInfo* index;
//     struct List* indexQuals;
//     struct RelOptInfo* outer_rel;
//     Cost* indexStartupCost;
//     Cost* indexTotalCost;
//     Selectivity* indexSelectivity;
//     double* indexCorrelation;

//     root = (PlannerInfo*)PG_GETARG_POINTER(0);
//     index = (IndexOptInfo*)PG_GETARG_POINTER(1);
//     indexQuals = (List*)PG_GETARG_POINTER(2);
//     outer_rel = (RelOptInfo*)PG_GETARG_POINTER(3);
//     indexStartupCost = (Cost*)PG_GETARG_POINTER(4);
//     indexTotalCost = (Cost*)PG_GETARG_POINTER(5);
//     indexSelectivity = (Selectivity*)PG_GETARG_POINTER(6);
//     indexCorrelation = (double*)PG_GETARG_POINTER(7);

//     /*
//        elog(INFO, "bingo cost estimate %f", root->);
//     */

//     bingo_genericcostestimate(root, index, indexQuals, outer_rel, 1.0, indexStartupCost, indexTotalCost, indexSelectivity, indexCorrelation);
// #endif
//     /*
//        elog(INFO, "start up cost %f", *indexStartupCost);
//        elog(INFO, "start total cost %f", *indexTotalCost);
//        elog(INFO, "correlation %f", *indexCorrelation);
//        elog(INFO, "selectivity %f", *indexSelectivity);
//        *indexStartupCost = 0.0;
//        *indexTotalCost = 1.0;
//        *indexSelectivity = 0.0;
//        *indexCorrelation = 0.0;
//     */

//     //*indexStartupCost = (Cost)1.0;
//     // indexTotalCost = 1.0;
//     /*
//        elog(INFO, "pointer = %d", root);
//     */
//     /*
//         *indexSelectivity = clauselist_selectivity(root, indexQuals,
//                                                   index->rel->relid,
//                                                   JOIN_INNER, NULL);
//     */
//     /*
//                QualCost index_qual_cost;
//           int numIndexPages = 0, numIndexTuples = 0;
//           cost_qual_eval(&index_qual_cost, indexQuals, root);
//         *indexStartupCost = index_qual_cost.startup;
//         *indexTotalCost = seq_page_cost * numIndexPages +
//                   (cpu_index_tuple_cost + index_qual_cost.per_tuple) * numIndexTuples;
//     */

//     /*
//      *indexStartupCost = 0.0;
//      *indexTotalCost = 0.1;
//      *indexCorrelation = 0.0;
//      *indexSelectivity = 0.0;
//      */

//     PG_RETURN_VOID();
// }

/*
Datum
bingo_costestimate(PG_FUNCTION_ARGS) {
   struct PlannerInfo *root;
   struct IndexOptInfo *index;
   struct List *indexQuals;
   struct RelOptInfo *outer_rel;
   struct Cost *indexStartupCost;
   struct Cost *indexTotalCost;
   struct Selectivity *indexSelectivity;
   double *indexCorrelation;

   QualCost index_qual_cost;
   int numIndexPages;
   int numIndexTuples;

   root = (PlannerInfo *) PG_GETARG_POINTER(0);
   index = (IndexOptInfo *) PG_GETARG_POINTER(1);
   indexQuals = (List *) PG_GETARG_POINTER(2);
   outer_rel = (RelOptInfo *) PG_GETARG_POINTER(3);
   indexStartupCost = (Cost *) PG_GETARG_POINTER(4);
   indexTotalCost = (Cost *) PG_GETARG_POINTER(5);
   indexSelectivity = (Selectivity *) PG_GETARG_POINTER(6);
   indexCorrelation = (double *) PG_GETARG_POINTER(7);

   *indexSelectivity = clauselist_selectivity(root, indexQuals, index->rel->relid, JOIN_INNER, NULL);

   numIndexPages = 10;
   numIndexPages = 100;
   cost_qual_eval(&index_qual_cost, indexQuals, root);
   *indexStartupCost = index_qual_cost.startup;
   *indexTotalCost = seq_page_cost * numIndexPages +
           (cpu_index_tuple_cost + index_qual_cost.per_tuple) * numIndexTuples;

   PG_RETURN_VOID();
}
*/

// Node	   *newNodeMacroHolder;
// char	   *BufferBlocks;