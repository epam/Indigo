/*
 * $Header: cdemodp0.h 14-jul-99.12:43:24 mjaeger Exp $
 */

/* Copyright (c) 1998, 1999, Oracle Corporation.  All rights reserved.
*/

/*
   NAME
     cdemodp0.h - C Demo program for Direct Path api

   DESCRIPTION
     - Internal data structs, macros, & defines for cdemodp driver.

   NOTES
     Structures, macros, constants used only by cdemodp.c.

   MODIFIED   (MM/DD/YY)
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   abrumm      12/22/98 - lint
   cmlim       11/17/98 - take away hardcoded MAX_RECLEN; now a session parm
   cmlim       10/06/98 - correct typo
   cmlim       10/02/98 - added externref
   cmlim       09/16/98 - internal data structs, macros, & defines for cdemodp
   cmlim       09/16/98 - Creation (abrumm 04/07/98)

*/


#ifndef cdemodp0_ORACLE
# define cdemodp0_ORACLE

# include <oratypes.h>
# include <oci.h>

/* partial field context structure, maintained by field setting function */
struct pctx
{
  ub1   valid_pctx;                              /* partial context is valid */
  ub4   pieceCnt_pctx;                            /* count of partial pieces */
  ub4   row_pctx;                               /* which row in column array */
  ub4   col_pctx;                            /* which column in column array */
  ub4   len_pctx;                            /* length of this column so far */
  int   fd_pctx;                 /* open file descriptor data is coming from */
  char *fnm_pctx;                                /* filename for data source */
};

/* CLEAR_PCTX(struct pctx pctx)
 *   Macro to clear the partial context state
 */
#define CLEAR_PCTX(pctx) \
  ((pctx).valid_pctx = FALSE,   (pctx).pieceCnt_pctx = 0,    \
   (pctx).row_pctx = UB4MAXVAL, (pctx).col_pctx = UB4MAXVAL, \
   (pctx).len_pctx = 0,         (pctx).fd_pctx  = -1,        \
   (pctx).fnm_pctx = (char *)0)

#define SET_PCTX(pctx, rowoff, coloff, clen, fd, fnm) \
  ((pctx).valid_pctx = TRUE,   (pctx).pieceCnt_pctx++,     \
   (pctx).row_pctx = (rowoff), (pctx).col_pctx = (coloff), \
   (pctx).len_pctx += (ub4)(clen), (pctx).fd_pctx = (fd),  \
   (pctx).fnm_pctx = (fnm))

#define LEN_PCTX(pctx) ((pctx).len_pctx)

/* Does the input record correspond to the first piece of a row?
 * Note that a row which is not pieced is a first piece too.
 */
#define FIRST_PIECE(pctx)       \
( (pctx).valid_pctx == FALSE || \
 ((pctx).valid_pctx == TRUE  && ((pctx).pieceCnt_pctx == 1)))

/* return values from field_set() */
#define FIELD_SET_COMPLETE   0
#define FIELD_SET_ERROR      1
#define FIELD_SET_BUF        2
#define FIELD_SET_PARTIAL    3

/* return values from do_convert() */
#define CONVERT_SUCCESS      0
#define CONVERT_ERROR        1
#define CONVERT_NEED_DATA    2
#define CONVERT_CONTINUE     3

/* return values from do_load() */
#define LOAD_SUCCESS     0
#define LOAD_ERROR       1
#define LOAD_NEED_DATA   2
#define LOAD_NO_DATA     3

/* state values for simple_load() */
#define RESET             1 /* initial state, reset data structures to empty */
#define GET_RECORD        2                             /* get input records */
#define FIELD_SET         3     /* assign fields of input records to columns */
#define DO_CONVERT        4   /* convert column array input to stream format */
#define DO_LOAD           5                        /* load the direct stream */
#define END_OF_INPUT      6                            /* no more input data */

/* Secondary buffer sizes for OUTOFLINE fields; no science here, just a WAG */
#define SECONDARY_BUF_SIZE (1024*1024)           /* size of secondary buffer */
#define SECONDARY_BUF_SLOP (8*1024)  /* get another field if this much avail */

#define STATICF

#ifndef externref
# define externref extern
#endif


#endif                                              /* cdemodp0_ORACLE */
