/*
 * $Header: cdemolb.h 14-jul-99.12:45:58 mjaeger Exp $
 */

/* Copyright (c) 1996, 1999,, 2000 Oracle Corporation.  All rights reserved.
*/

/* NOTE:  See 'header_template.doc' in the 'doc' dve under the 'forms'
      directory for the header file template that includes instructions.
*/

/*
   NAME
     cdemolb.h -  header file for cdemolb.c

   DESCRIPTION
     see cdemolb.c

   RELATED DOCUMENTS

   INSPECTION STATUS
     Inspection date:
     Inspection status:
     Estimated increasing cost defects per page:
     Rule sets:

   ACCEPTANCE REVIEW STATUS
     Review date:
     Review status:
     Reviewers:

   PUBLIC FUNCTION(S)
     <list of external functions declared/defined - with one-line descriptions>

   PRIVATE FUNCTION(S)
     <list of static functions defined in .c file - with one-line descriptions>

   EXAMPLES

   NOTES
     see cdemolb.c

   MODIFIED   (MM/DD/YY)
   dchatter    05/09/00 - lob read as char
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   svedala     09/09/98 - lines longer than 79 chars reformatted - bug 722491
   azhao       01/31/97 - don't include orastd.h
   azhao       01/30/97 - fix lint error
   echen       01/03/97 - OCI beautification
   pshah       10/11/96 -
   aroy        07/22/96 - header file for cdemolb.c
   aroy        07/22/96 - Creation

*/


#ifndef cdemolb
# define cdemolb

#ifndef OCI_ORACLE
#include <oci.h>
#endif

/*---------------------------------------------------------------------------
                           TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/
#define LFCOUNT          2
#define STMTLEN          100

#define LONGTEXTLENGTH   1024
#define BUFSIZE          1024

#define EX_FAILURE       1
#define EX_SUCCESS       0

#define CDEMOLB_TEXT_FILE "cdemolb.dat"

static CONST text insstmt[LFCOUNT][STMTLEN] =
                {
                  "INSERT INTO CLBTAB VALUES ( 'Jack', EMPTY_CLOB())",
	          ""
                };

static CONST text selstmt[LFCOUNT][STMTLEN] =
                {
                  "SELECT essay FROM CLBTAB WHERE name = 'Jack' for update",
                  "SELECT essay FROM CLBTAB WHERE name = 'Jack'"
                };

typedef struct { /* statement bind/define handles */
  OCIStmt  *stmhp;
  OCIBind  *bndhp[2];
  OCIDefine  *dfnhp[2];
} stmtdef;

typedef struct {                 /* GLOBAL STRUCTURE                  */
  OCIEnv *envhp;                 /* Environment handle               */
  OCISvcCtx *svchp;                 /* Service handle                   */
  OCIServer *srvhp;                 /* Server handles                   */
  OCIError *errhp;                 /* Error handle                     */
  OCISession *authp;                 /* Authentication handle            */
  stmtdef *s1;             /*Statement handle 1 - for inserting*/
  stmtdef *s2;             /* Handle for -select update */
} ldemodef;

/* define macros to be used in tests */
#define COMMENT(x) (void) fprintf(stdout,"\nCOMMENT: %s\n", x)

/*---------------------------------------------------------------------------
                               FUNCTIONS
  ---------------------------------------------------------------------------*/
static void alloc_lob_desc(/*_ ldemodef *ctx, OCILobLocator **lobsrc _*/);
static ldemodef *alloc_handles(/*_ void _*/);
static void cleanup(/*_ ldemodef *ctx _*/);
static void errrpt(/*_ ldemodef *ctx, const text *op _*/);
static void authenticate_user(/*_ ldemodef *ctx _*/);
static void alloc_stmt_handles(/*_ ldemodef *ctx, stmtdef *sptr,
                                                  sb2 nbnd, sb2 ndfn _*/);
static void insert_select_loc (/*_ ldemodef *ctx, dvoid *lobsrc _*/);

static void select_loc_data (/*_ ldemodef *ctx _*/);

static void Write_to_loc(/*_ ldemodef *ctx, OCILobLocator *lobp _*/);
static void Read_from_loc(/*_ ldemodef *ctx, OCILobLocator *lobp _*/);
static void deauthenticate(/*_ ldemodef *ctx _*/);


#endif                                              /* cdemolb */

