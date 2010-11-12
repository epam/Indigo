/*
 * $Header: cdemodp.h 03-apr-2001.11:46:06 cmlim Exp $
 */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*          Copyright (c) Oracle Corporation 20001           .             */
/*          All Rights Reserved.                                           */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
**  NAME:
**   cdemodp.h - C Demo prog for Direct Path api
**
**  DESCRIPTION:
**   - Common header file for cdemodp driver and client progs.
**
**  NOTES:
**
**
**  MODIFIED   (MM/DD/YY)
**     cmlim    04/03/01 - remove flag_tbl field
**
**  eegolf      02/21/01 - added new 9i items 
**  cmlim       09/16/98 - Creation
**
*/


#ifndef cdemodp_ORACLE
# define cdemodp_ORACLE

# include <oratypes.h>

# ifndef externdef
#  define externdef
# endif

/* External column attributes */
struct col
{
  text *name_col;                                             /* column name */
  ub2   id_col;                                            /* column load id */
  ub2   exttyp_col;                                         /* external type */
  text *datemask_col;                             /* datemask, if applicable */
  ub1   prec_col;                                /* precision, if applicable */
  sb1   scale_col;                                   /* scale, if applicable */
  ub2   csid_col;                                        /* character set id */
  ub1   date_col;            /* is column a chrdate or date? 1=TRUE. 0=FALSE */
  struct obj * obj_col;          /* description of object, if applicable */
#define COL_OID 0x1                                         /* col is an OID */
  ub4   flag_col;
};

/* Input field descriptor
 * For this example (and simplicity),
 * fields are strictly positional.
 */
struct fld
{
  ub4  begpos_fld;                             /* 1-based beginning position */
  ub4  endpos_fld;                             /* 1-based ending    position */
  ub4  maxlen_fld;                       /* max length for out of line field */
  ub4    flag_fld;
#define FLD_INLINE            0x1
#define FLD_OUTOFLINE         0x2
#define FLD_STRIP_LEAD_BLANK  0x4
#define FLD_STRIP_TRAIL_BLANK 0x8
};

struct obj
{
  text               *name_obj;                                /* type  name*/
  ub2                 ncol_obj;              /* number of columns in col_obj*/
  struct col         *col_obj;                          /* column attributes*/
  struct fld         *fld_obj;                           /* field descriptor*/
  ub4                 rowoff_obj;  /* current row offset in the column array*/
  ub4                 nrows_obj;              /* number of rows in col array*/
  OCIDirPathFuncCtx  *ctx_obj;       /* Function context for this obj column*/
  OCIDirPathColArray *ca_obj;           /* column array  for this obj column*/
  ub4                 flag_obj;                              /* type of obj */
#define OBJ_OBJ  0x1                                             /* obj col */
#define OBJ_OPQ  0x2                                  /* opaque/sql str col */
#define OBJ_REF  0x4                                             /* ref col */
};

struct tbl
{
  text        *owner_tbl;                                     /* table owner */
  text        *name_tbl;                                       /* table name */
  text        *subname_tbl;                        /* subname, if applicable */
  ub2          ncol_tbl;                     /* number of columns in col_tbl */
  text        *dfltdatemask_tbl;            /* table level default date mask */
  struct col  *col_tbl;                                 /* column attributes */
  struct fld  *fld_tbl;                                  /* field descriptor */
  ub1          parallel_tbl;                         /* parallel: 1 for true */
  ub1          nolog_tbl;                          /* no logging: 1 for true */
  ub4          xfrsz_tbl;                   /* transfer buffer size in bytes */
  text         *objconstr_tbl;   /* obj constr/type if loading a derived obj */
};

struct sess                        /* options for a direct path load session */
{
  text        *username_sess;                                        /* user */
  text        *password_sess;                                    /* password */
  text        *inst_sess;                            /* remote instance name */
  text        *outfn_sess;                                /* output filename */
  ub4          maxreclen_sess;          /* max size of input record in bytes */
};


#endif                                              /* cdemodp_ORACLE */
