#ifndef _BINGO_POSTGRES_H__
#define _BINGO_POSTGRES_H__

#ifdef _WIN32
#define strcasestr strstr
#endif

#define BINGO_METAPAGE 0                    /* metapage is always block 0 */
#define BINGO_CONFIG_PAGE 1                 /* configuration page is always block 1 */
#define BINGO_SECTION_OFFSET_PER_BLOCK 2000 /* 2000*sizeof(int) < 8KB*/
#define BINGO_SECTION_OFFSET_BLOCKS_NUM 10
#define BINGO_METABLOCKS_NUM 2
#define BINGO_DICTIONARY_BLOCKS_NUM 100 /* 800KB for dictionary*/

#define BINGO_MOLS_PER_MAPBLOCK 440      /* sizeof(BingoTidData) = 18 * 440 < 8KB */
#define BINGO_MOLS_PER_FINGERBLOCK 64000 /* 64000 bits < 8KB */
#define BINGO_MOLS_PER_SECTION 64000
#define BINGO_TUPLE_OFFSET 1 /*INDEX tuple offset is always 1*/

#define BINGO_PG_NOLOCK 0
#define BINGO_PG_READ 1
#define BINGO_PG_WRITE 2

#define BINGO_INDEX_TYPE_MOLECULE 1
#define BINGO_INDEX_TYPE_REACTION 2

#if PG_VERSION_NUM / 100 >= 904
#ifdef __MINGW32__
#undef PGDLLEXPORT
#define PGDLLEXPORT EXPORT_SYMBOL
#define BINGO_FUNCTION_EXPORT(funcname) EXPORT_SYMBOL PG_FUNCTION_INFO_V1(funcname);
#else
#define BINGO_FUNCTION_EXPORT(funcname) PGDLLEXPORT PG_FUNCTION_INFO_V1(funcname);
#endif
#else

#define BINGO_FUNCTION_EXPORT(funcname)                                                                                                                        \
    PG_FUNCTION_INFO_V1(funcname);                                                                                                                             \
    PGDLLEXPORT Datum funcname(PG_FUNCTION_ARGS);

#endif

//#define PG_OBJECT void*
/*
 * Postgres internal conflicts with the indigo library
 * That is why there is a workaround with void pointer
 */
typedef void* PG_OBJECT;

#endif /* BINGO_POSTGRES_H */
