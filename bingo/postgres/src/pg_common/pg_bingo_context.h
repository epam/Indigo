#ifndef PG_BINGO_CONTEXT_H__
#define	PG_BINGO_CONTEXT_H__


extern "C" {
#include "c.h"
}

#ifdef qsort
#undef qsort
#endif
#ifdef printf
#undef printf
#endif

typedef struct BingoMetaPageData {
   int bingo_index_version;
   int n_molecules;
   int n_blocks_for_map;
   int n_blocks_for_fp;
   int n_blocks_for_dictionary;
   int offset_dictionary;
   int n_sections;
   int n_pages;
   int index_type;
} BingoMetaPageData;

typedef BingoMetaPageData *BingoMetaPage;

#define BingoPageGetMeta(page) \
	((BingoMetaPage) PageGetContents(page))

typedef struct BingoAutoVacOpts {
   bool enabled;
   int vacuum_threshold;
   int analyze_threshold;
   int vacuum_cost_delay;
   int vacuum_cost_limit;
   int freeze_min_age;
   int freeze_max_age;
   int freeze_table_age;
   double vacuum_scale_factor;
   double analyze_scale_factor;
} BingoAutoVacOpts;

typedef struct BingoIndexOptions {
   int treat_x_as_pseudoatom;
   int ignore_closing_bond_direction_mismatch;
   int ignore_stereocenter_errors;
   int stereochemistry_bidirectional_mode;
   int ignore_cistrans_errors;
   int allow_non_unique_dearomatization;
   int zero_unknown_aromatic_hydrogens;

   int fp_ord_size;
   int fp_any_size;
   int fp_tau_size;
   int fp_sim_size;
   int sub_screening_max_bits;
   int sim_screening_pass_mark;
   int nthreads;
} BingoIndexOptions;


typedef struct BingoStdRdOptions {
   int32 vl_len_; /* varlena header (do not touch directly!) */
   int fillfactor; /* page fill factor in percent (0..100) */
   BingoAutoVacOpts autovacuum; /* autovacuum-related options */
   BingoIndexOptions index_parameters;
} BingoStdRdOptions;

typedef struct BingoSectionInfoData {
   int n_structures;
   int n_blocks_for_map;
   int n_blocks_for_fp;
   int n_blocks_for_bin;
   int section_size;
   int last_cmf;
   int last_xyz;
   char has_removed;
} BingoSectionInfoData;

#endif	/* BINGO_PG_CONTEXT_H */

