#include <cfloat>

#include "bingo_pg_fix_pre.h"
extern "C"
{
#include <postgres.h>

#include <access/reloptions.h>
#include <catalog/pg_type_d.h>
#include <utils/builtins.h>
#include <utils/guc.h>
#include <utils/memutils.h>
#include <utils/rel.h>
}
#include "bingo_pg_fix_post.h"

#include "bingo_pg_common.h"
#include "pg_bingo_context.h"

#if PG_VERSION_NUM / 100 < 906
extern "C"
{
    BINGO_FUNCTION_EXPORT(bingo_options);
}
#endif

#define RELOPT_KIND_BINGO 1 << 8

static relopt_bool boolRelOpts[] = {{{"autovacuum_enabled", "Enables autovacuum in this relation", RELOPT_KIND_HEAP | RELOPT_KIND_TOAST}, true},
                                    /* list terminator */
                                    {{NULL}}};

static relopt_int intRelOpts[] = {
    {{"fillfactor", "Packs table pages only to this percentage", RELOPT_KIND_HEAP}, HEAP_DEFAULT_FILLFACTOR, HEAP_MIN_FILLFACTOR, 100},
    {{"autovacuum_vacuum_threshold", "Minimum number of tuple updates or deletes prior to vacuum", RELOPT_KIND_HEAP | RELOPT_KIND_TOAST}, -1, 0, INT_MAX},
    {{"autovacuum_analyze_threshold", "Minimum number of tuple inserts, updates or deletes prior to analyze", RELOPT_KIND_HEAP}, -1, 0, INT_MAX},
    {{"autovacuum_vacuum_cost_delay", "Vacuum cost delay in milliseconds, for autovacuum", RELOPT_KIND_HEAP | RELOPT_KIND_TOAST}, -1, 0, 100},
    {{"autovacuum_vacuum_cost_limit", "Vacuum cost amount available before napping, for autovacuum", RELOPT_KIND_HEAP | RELOPT_KIND_TOAST}, -1, 1, 10000},
    {{"autovacuum_freeze_min_age", "Minimum age at which VACUUM should freeze a table row, for autovacuum", RELOPT_KIND_HEAP | RELOPT_KIND_TOAST},
     -1,
     0,
     1000000000},
    {{"autovacuum_freeze_max_age", "Age at which to autovacuum a table to prevent transaction ID wraparound", RELOPT_KIND_HEAP | RELOPT_KIND_TOAST},
     -1,
     100000000,
     2000000000},
    {{"autovacuum_freeze_table_age", "Age at which VACUUM should perform a full table sweep to replace old Xid values with FrozenXID",
      RELOPT_KIND_HEAP | RELOPT_KIND_TOAST},
     -1,
     0,
     2000000000},
    {{"treat_x_as_pseudoatom", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"ignore_closing_bond_direction_mismatch", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"ignore_stereocenter_errors", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"stereochemistry_bidirectional_mode", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"stereochemistry_detect_haworth_projection", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"ignore_cistrans_errors", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"allow_non_unique_dearomatization", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"zero_unknown_aromatic_hydrogens", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"reject_invalid_structures", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"ignore_bad_valence", "", RELOPT_KIND_BINGO}, -1, 0, 1},
    {{"fp_ord_size", "", RELOPT_KIND_BINGO}, -1, 0, 2000000000},
    {{"fp_any_size", "", RELOPT_KIND_BINGO}, -1, 0, 2000000000},
    {{"fp_tau_size", "", RELOPT_KIND_BINGO}, -1, 0, 2000000000},
    {{"fp_sim_size", "", RELOPT_KIND_BINGO}, -1, 0, 2000000000},
    {{"sub_screening_max_bits", "", RELOPT_KIND_BINGO}, -1, 0, 2000000000},
    {{"sim_screening_pass_mark", "", RELOPT_KIND_BINGO}, -1, 0, 2000000000},
    {{"nthreads", "", RELOPT_KIND_BINGO}, -1, 0, 2000000000},
    /* list terminator */
    {{NULL}}

};

static relopt_real realRelOpts[] = {
    {{"autovacuum_vacuum_scale_factor", "Number of tuple updates or deletes prior to vacuum as a fraction of reltuples", RELOPT_KIND_HEAP | RELOPT_KIND_TOAST},
     -1,
     0.0,
     100.0},
    {{"autovacuum_analyze_scale_factor", "Number of tuple inserts, updates or deletes prior to analyze as a fraction of reltuples", RELOPT_KIND_HEAP},
     -1,
     0.0,
     100.0},
    {{"seq_page_cost", "Sets the planner's estimate of the cost of a sequentially fetched disk page.", RELOPT_KIND_TABLESPACE}, -1, 0.0, DBL_MAX},
    {{"random_page_cost", "Sets the planner's estimate of the cost of a nonsequentially fetched disk page.", RELOPT_KIND_TABLESPACE}, -1, 0.0, DBL_MAX},
    {{"n_distinct", "Sets the planner's estimate of the number of distinct values appearing in a column (excluding child relations).", RELOPT_KIND_ATTRIBUTE},
     0,
     -1.0,
     DBL_MAX},
    {{"n_distinct_inherited", "Sets the planner's estimate of the number of distinct values appearing in a column (including child relations).",
      RELOPT_KIND_ATTRIBUTE},
     0,
     -1.0,
     DBL_MAX},
    /* list terminator */
    {{NULL}}};

// static relopt_string stringRelOpts[] =
//{
//	{
//		{
//				"similarity_type",
//				"",
//				RELOPT_KIND_BINGO,
//                                AccessExclusiveLock
//		}, 3, false, nullptr, "SIM"
//	},
//	/* list terminator */
//	{{NULL}}
//};

static relopt_gen** relOpts = NULL;

static int num_custom_options = 0;
static relopt_gen** custom_options = NULL;
static bool need_initialization = true;

static void initialize_reloptions(void);
static void parse_one_reloption(relopt_value* option, char* text_str, int text_len, bool validate);

static void parse_one_reloption(relopt_value* option, char* text_str, int text_len, bool validate)
{
    char* value;
    int value_len;
    bool parsed;
    bool nofree = false;

    if (option->isset && validate)
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("parameter \"%s\" specified more than once", option->gen->name)));

    value_len = text_len - option->gen->namelen - 1;
    value = (char*)palloc(value_len + 1);
    memcpy(value, text_str + option->gen->namelen + 1, value_len);
    value[value_len] = '\0';

    switch (option->gen->type)
    {
    case RELOPT_TYPE_BOOL: {
        parsed = parse_bool(value, &option->values.bool_val);
        if (validate && !parsed)
            ereport(ERROR, (errmsg("invalid value for boolean option \"%s\": %s", option->gen->name, value)));
    }
    break;
    case RELOPT_TYPE_INT: {
        relopt_int* optint = (relopt_int*)option->gen;

        parsed = parse_int(value, &option->values.int_val, 0, NULL);
        if (validate && !parsed)
            ereport(ERROR, (errmsg("invalid value for integer option \"%s\": %s", option->gen->name, value)));
        if (validate && (option->values.int_val < optint->min || option->values.int_val > optint->max))
            ereport(ERROR, (errmsg("value %s out of bounds for option \"%s\"", value, option->gen->name),
                            errdetail("Valid values are between \"%d\" and \"%d\".", optint->min, optint->max)));
    }
    break;
    case RELOPT_TYPE_REAL: {
        relopt_real* optreal = (relopt_real*)option->gen;
#if PG_VERSION_NUM / 100 >= 1200
        parsed = parse_real(value, &option->values.real_val, 0, NULL);
#else
        parsed = parse_real(value, &option->values.real_val);
#endif

        if (validate && !parsed)
            ereport(ERROR, (errmsg("invalid value for floating point option \"%s\": %s", option->gen->name, value)));
        if (validate && (option->values.real_val < optreal->min || option->values.real_val > optreal->max))
            ereport(ERROR, (errmsg("value %s out of bounds for option \"%s\"", value, option->gen->name),
                            errdetail("Valid values are between \"%f\" and \"%f\".", optreal->min, optreal->max)));
    }
    break;
    case RELOPT_TYPE_STRING: {
        relopt_string* optstring = (relopt_string*)option->gen;

        option->values.string_val = value;
        nofree = true;
        if (validate && optstring->validate_cb)
            (optstring->validate_cb)(value);
        parsed = true;
    }
    break;
    default:
        elog(ERROR, "unsupported reloption type %d", option->gen->type);
        parsed = true; /* quiet compiler */
        break;
    }

    if (parsed)
        option->isset = true;
    if (!nofree)
        pfree(value);
}

static void initialize_reloptions(void)
{
    int i;
    int j;

    j = 0;
    for (i = 0; boolRelOpts[i].gen.name; i++)
        j++;
    for (i = 0; intRelOpts[i].gen.name; i++)
        j++;
    for (i = 0; realRelOpts[i].gen.name; i++)
        j++;
    //   for (i = 0; stringRelOpts[i].gen.name; i++)
    //      j++;
    j += num_custom_options;

    if (relOpts)
        pfree(relOpts);
    relOpts = (relopt_gen**)MemoryContextAlloc(TopMemoryContext, (j + 1) * sizeof(relopt_gen*));

    j = 0;
    for (i = 0; boolRelOpts[i].gen.name; i++)
    {
        relOpts[j] = &boolRelOpts[i].gen;
        relOpts[j]->type = RELOPT_TYPE_BOOL;
        relOpts[j]->namelen = strlen(relOpts[j]->name);
        j++;
    }

    for (i = 0; intRelOpts[i].gen.name; i++)
    {
        relOpts[j] = &intRelOpts[i].gen;
        relOpts[j]->type = RELOPT_TYPE_INT;
        relOpts[j]->namelen = strlen(relOpts[j]->name);
        j++;
    }

    for (i = 0; realRelOpts[i].gen.name; i++)
    {
        relOpts[j] = &realRelOpts[i].gen;
        relOpts[j]->type = RELOPT_TYPE_REAL;
        relOpts[j]->namelen = strlen(relOpts[j]->name);
        j++;
    }

    //   for (i = 0; stringRelOpts[i].gen.name; i++) {
    //      relOpts[j] = &stringRelOpts[i].gen;
    //      relOpts[j]->type = RELOPT_TYPE_STRING;
    //      relOpts[j]->namelen = strlen(relOpts[j]->name);
    //      j++;
    //   }

    for (i = 0; i < num_custom_options; i++)
    {
        relOpts[j] = custom_options[i];
        j++;
    }

    /* add a list terminator */
    relOpts[j] = NULL;

    /* flag the work is complete */
    need_initialization = false;
}

relopt_value* bingoParseRelOptions(Datum options, bool validate, int kind, int* numrelopts)
{
    relopt_value* reloptions;
    int numoptions = 0;
    int i;
    int j;

    if (need_initialization)
        initialize_reloptions();

    /* Build a list of expected options, based on kind */

    for (i = 0; relOpts[i]; i++)
        if (relOpts[i]->kinds & kind)
            numoptions++;

    if (numoptions == 0)
    {
        *numrelopts = 0;
        return NULL;
    }

    reloptions = (relopt_value*)palloc(numoptions * sizeof(relopt_value));

    for (i = 0, j = 0; relOpts[i]; i++)
    {
        if (relOpts[i]->kinds & kind)
        {
            reloptions[j].gen = relOpts[i];
            reloptions[j].isset = false;
            j++;
        }
    }

    /* Done if no options */
    if (PointerIsValid(DatumGetPointer(options)))
    {
        ArrayType* array;
        Datum* optiondatums;
        int noptions;

        array = DatumGetArrayTypeP(options);

        Assert(ARR_ELEMTYPE(array) == TEXTOID);

        deconstruct_array(array, TEXTOID, -1, false, 'i', &optiondatums, NULL, &noptions);

        for (i = 0; i < noptions; i++)
        {
            text* optiontext = DatumGetTextP(optiondatums[i]);
            char* text_str = VARDATA(optiontext);
            int text_len = VARSIZE(optiontext) - VARHDRSZ;
            int j;

            /* Search for a match in reloptions */
            for (j = 0; j < numoptions; j++)
            {
                int kw_len = reloptions[j].gen->namelen;

                if (text_len > kw_len && text_str[kw_len] == '=' && pg_strncasecmp(text_str, reloptions[j].gen->name, kw_len) == 0)
                {
                    parse_one_reloption(&reloptions[j], text_str, text_len, validate);
                    break;
                }
            }

            if (j >= numoptions && validate)
            {
                char* s;
                char* p;

                s = TextDatumGetCString(optiondatums[i]);
                p = strchr(s, '=');
                if (p)
                    *p = '\0';
                ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("unrecognized parameter \"%s\"", s)));
            }
        }
    }

    *numrelopts = numoptions;
    return reloptions;
}

bytea* bingo_reloptions(Datum reloptions, bool validate)
{
    relopt_value* options;
    void* rdopts;
    int numoptions;
    static const relopt_parse_elt tab[] = {
        {"fillfactor", RELOPT_TYPE_INT, offsetof(StdRdOptions, fillfactor)},
        {"autovacuum_enabled", RELOPT_TYPE_BOOL, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, enabled)},
        {"autovacuum_vacuum_threshold", RELOPT_TYPE_INT, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, vacuum_threshold)},
        {"autovacuum_analyze_threshold", RELOPT_TYPE_INT, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, analyze_threshold)},
        {"autovacuum_vacuum_cost_delay", RELOPT_TYPE_INT, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, vacuum_cost_delay)},
        {"autovacuum_vacuum_cost_limit", RELOPT_TYPE_INT, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, vacuum_cost_limit)},
        {"autovacuum_freeze_min_age", RELOPT_TYPE_INT, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, freeze_min_age)},
        {"autovacuum_freeze_max_age", RELOPT_TYPE_INT, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, freeze_max_age)},
        {"autovacuum_freeze_table_age", RELOPT_TYPE_INT, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, freeze_table_age)},
        {"autovacuum_vacuum_scale_factor", RELOPT_TYPE_REAL, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, vacuum_scale_factor)},
        {"autovacuum_analyze_scale_factor", RELOPT_TYPE_REAL, offsetof(StdRdOptions, autovacuum) + offsetof(AutoVacOpts, analyze_scale_factor)},
        {"treat_x_as_pseudoatom", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, treat_x_as_pseudoatom)},
        {"ignore_closing_bond_direction_mismatch", RELOPT_TYPE_INT,
         offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, ignore_closing_bond_direction_mismatch)},
        {"ignore_stereocenter_errors", RELOPT_TYPE_INT,
         offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, ignore_stereocenter_errors)},
        {"stereochemistry_bidirectional_mode", RELOPT_TYPE_INT,
         offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, stereochemistry_bidirectional_mode)},
        {"stereochemistry_detect_haworth_projection", RELOPT_TYPE_INT,
         offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, stereochemistry_detect_haworth_projection)},
        {"ignore_cistrans_errors", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, ignore_cistrans_errors)},
        {"allow_non_unique_dearomatization", RELOPT_TYPE_INT,
         offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, allow_non_unique_dearomatization)},
        {"zero_unknown_aromatic_hydrogens", RELOPT_TYPE_INT,
         offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, zero_unknown_aromatic_hydrogens)},
        {"reject_invalid_structures", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, reject_invalid_structures)},
        {"ignore_bad_valence", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, ignore_bad_valence)},
        {"fp_ord_size", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, fp_ord_size)},
        {"fp_any_size", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, fp_any_size)},
        {"fp_tau_size", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, fp_tau_size)},
        {"fp_sim_size", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, fp_sim_size)},
        {"sub_screening_max_bits", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, sub_screening_max_bits)},
        {"sim_screening_pass_mark", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, sim_screening_pass_mark)},
        {"nthreads", RELOPT_TYPE_INT, offsetof(BingoStdRdOptions, index_parameters) + offsetof(BingoIndexOptions, nthreads)}};

    options = bingoParseRelOptions(reloptions, validate, RELOPT_KIND_BINGO, &numoptions);

    /* if none set, we're done */
    if (numoptions == 0)
        return NULL;

    rdopts = allocateReloptStruct(sizeof(BingoStdRdOptions), options, numoptions);

    fillRelOptions(rdopts, sizeof(BingoStdRdOptions), options, numoptions, validate, tab, lengthof(tab));

    pfree(options);

    return (bytea*)rdopts;
}

#if PG_VERSION_NUM / 100 >= 906
CEXPORT bytea* bingo_options(Datum reloptions, bool validate)
{
#else
Datum bingo_options(PG_FUNCTION_ARGS)
{
    Datum reloptions = PG_GETARG_DATUM(0);
    bool validate = PG_GETARG_BOOL(1);
#endif

    bytea* result;

    result = bingo_reloptions(reloptions, validate);
#if PG_VERSION_NUM / 100 >= 906
    return result;
#else
    if (result)
        PG_RETURN_BYTEA_P(result);
    PG_RETURN_NULL();
#endif
}
// Datum
// bingo_options(PG_FUNCTION_ARGS) {
//   Datum reloptions = PG_GETARG_DATUM(0);
//   bool validate = PG_GETARG_BOOL(1);
//   elog(INFO, "bingo options");
//
//   bytea *result;
//    elog(INFO, "bingo options");
//
//   result = default_reloptions(reloptions, validate, RELOPT_KIND_HASH);
//
//  if (result)
//      PG_RETURN_BYTEA_P(result);
//   PG_RETURN_NULL();
//}

// Datum
// bingo_options(PG_FUNCTION_ARGS) {
//   Datum reloptions = PG_GETARG_DATUM(0);
//   bool validate = PG_GETARG_BOOL(1);
//   elog(INFO, "bingo options");
//
//   int noptions = 0;
//   BingoStdRdOptions *rdopts = 0;
//
//   try {
//      RedBlackStringObjMap< Array<char> > attributes;
//
//   /*
//    * Done if no options
//    */
//      if (PointerIsValid(DatumGetPointer(reloptions))) {
//         ArrayType *array;
//         Datum *optiondatums;
//
//         array = DatumGetArrayTypeP(reloptions);
//
//         Assert(ARR_ELEMTYPE(array) == TEXTOID);
//
//         deconstruct_array(array, TEXTOID, -1, false, 'i',
//                 &optiondatums, NULL, &noptions);
//
//
//         attributes.clear();
//         for (int i = 0; i < noptions; i++) {
//            int text_len;
//            char* text_data = BingoPgCommon::getTextData(&optiondatums[i], text_len);
//            Helpers::addAttributeAndValue(text_data, text_len, attributes);
//         }
//      }
//
//      rdopts = (BingoStdRdOptions*)palloc0(sizeof(BingoStdRdOptions));
//
//      Helpers::mapParameter(attributes, "treat_x_as_pseudoatom", rdopts->index_parameters.treat_x_as_pseudoatom);
//      Helpers::mapParameter(attributes, "ignore_closing_bond_direction_mismatch", rdopts->index_parameters.ignore_closing_bond_direction_mismatch);
//      Helpers::mapParameter(attributes, "fp_ord_size", rdopts->index_parameters.fp_ord_size);
//      Helpers::mapParameter(attributes, "fp_any_size", rdopts->index_parameters.fp_any_size);
//      Helpers::mapParameter(attributes, "fp_tau_size", rdopts->index_parameters.fp_tau_size);
//      Helpers::mapParameter(attributes, "fp_sim_size", rdopts->index_parameters.fp_sim_size);
//      Helpers::mapParameter(attributes, "sub_screening_max_bits", rdopts->index_parameters.sub_screening_max_bits);
//      Helpers::mapParameter(attributes, "sim_screening_pass_mark", rdopts->index_parameters.sim_screening_pass_mark);
//
//      if(validate && attributes.size() > 0)
//         elog(ERROR, "Error while loading options: unknown option %s\n", attributes.key(attributes.begin()));
//
//   } catch (Exception& e) {
//      elog(ERROR, "Error while loading options: %s\n", e.message());
//   }
//
//   /* if none set, we're done */
//   if (noptions == 0 || rdopts == 0)
//      PG_RETURN_NULL();
//
//   int xx = sizeof(BingoStdRdOptions);
//   SET_VARSIZE(rdopts, xx);
//
//   PG_RETURN_BYTEA_P((bytea*)rdopts);
//}
