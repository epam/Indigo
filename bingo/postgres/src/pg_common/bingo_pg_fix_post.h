#ifndef __bingo_pg_fix_post_h__
#define __bingo_pg_fix_post_h__

#ifdef qsort
#undef qsort
#endif

#ifdef printf
#undef printf
#endif

#ifdef vprintf
#undef vprintf
#endif

#ifdef snprintf
#undef snprintf
#endif

#ifdef open
#undef open
#endif

#ifdef bind
#undef bind
#endif

#ifdef islower_l
#undef islower_l
#endif

#ifdef isupper_l
#undef isupper_l
#endif

#ifdef locale_t
#undef locale_t
#endif

#endif //__bingo_pg_fix_post_h__
