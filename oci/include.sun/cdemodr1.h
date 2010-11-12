/*
 * $Header: cdemodr1.h 14-jul-99.12:43:57 mjaeger Exp $
 */

/* Copyright (c) 1997, 1999, Oracle Corporation.  All rights reserved.
*/

/* NOTE:  See 'header_template.doc' in the 'doc' dve under the 'forms'
      directory for the header file template that includes instructions.
*/

/*
   NAME
     cdemodr1.h - DML RETURNING demo program.

   DESCRIPTION
     Demonstrate INSERT/UPDATE/DELETE statements with RETURNING clause

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
     <other useful comments, qualifications, etc.>

   MODIFIED   (MM/DD/YY)
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   svedala     10/01/98 - include stdlib.h - bug 714175
   svedala     09/09/98 - lines longer than 79 chars reformatted - bug 722491
   echen       07/30/97 - fix bug 516406
   azhao       05/30/97 - Creation

*/

/*------------------------------------------------------------------------
 * Include Files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>
/*
#include <orastd.h>
*/

/*------------------------------------------------------------------------
 * Define Constants
 */

#define MAXBINDS       25
#define MAXROWS         5           /* max no of rows returned per iter */
#define MAXCOLS        10
#define MAXITER        10           /* max no of iters in execute */
#define MAXCOLLEN      40           /* if changed, update cdemodr1.sql */
#define DATBUFLEN       7

int main(/*_ int argc, char *argv[] _*/);
static sword init_handles(/*_ OCIEnv **envhp, OCISvcCtx **svchp,
                               OCIError **errhp, OCIServer **svrhp,
                               OCISession **authp, ub4 mode _*/);

static sword attach_server(/*_ ub4 mode, OCIServer *srvhp,
                        OCIError *errhp, OCISvcCtx *svchp _*/);
static sword log_on(/*_ OCISession *authp, OCIError *errhp, OCISvcCtx *svchp,
                 text *uid, text *pwd, ub4 credt, ub4 mode _*/);
static sword init_bind_handle(/*_ OCIStmt *stmthp, OCIBind *bndhp[],
                                                             int nbinds _*/);
static void print_raw(/*_ ub1 *raw, ub4 rawlen _*/);

static void free_handles(/*_ OCIEnv *envhp, OCISvcCtx *svchp, OCIServer *srvhp,
                      OCIError *errhp, OCISession *authp, OCIStmt *stmthp _*/);
void report_error(/*_ OCIError *errhp _*/);
void logout_detach_server(/*_ OCISvcCtx *svchp, OCIServer *srvhp,
                              OCIError *errhp, OCISession *authp,
                              text *userid _*/);
sword finish_demo(/*_ boolean loggedon, OCIEnv *envhp, OCISvcCtx *svchp,
                      OCIServer *srvhp, OCIError *errhp, OCISession *authp,
                      OCIStmt *stmthp, text *userid _*/);
static sword demo_insert(/*_ OCISvcCtx *svchp, OCIStmt *stmthp,
                              OCIBind *bndhp[], OCIError *errhp _*/);
static sword demo_update(/*_ OCISvcCtx *svchp, OCIStmt *stmthp,
                              OCIBind *bndhp[], OCIError *errhp _*/);
static sword demo_delete(/*_ OCISvcCtx *svchp, OCIStmt *stmthp,
                              OCIBind *bndhp[], OCIError *errhp _*/);
static sword bind_name(/*_ OCIStmt *stmthp, OCIBind *bndhp[],
                            OCIError *errhp _*/);
static sword bind_pos(/*_ OCIStmt *stmthp, OCIBind *bndhp[],
                           OCIError *errhp _*/);
static sword bind_input(/*_ OCIStmt *stmthp, OCIBind *bndhp[],
                             OCIError *errhp _*/);
static sword bind_output(/*_ OCIStmt *stmthp, OCIBind *bndhp[],
                              OCIError *errhp _*/);
static sword bind_array(/*_ OCIBind *bndhp[], OCIError *errhp _*/);
static sword bind_dynamic(/*_ OCIBind *bndhp[], OCIError *errhp _*/);
static sb4 cbf_no_data(/*_ dvoid *ctxp, OCIBind *bindp, ub4 iter, ub4 index,
                 dvoid **bufpp, ub4 *alenpp, ub1 *piecep, dvoid **indpp _*/);
static sb4 cbf_get_data(/*_ dvoid *ctxp, OCIBind *bindp, ub4 iter, ub4 index,
                             dvoid **bufpp, ub4 **alenpp, ub1 *piecep,
                             dvoid **indpp, ub2 **rcodepp _*/);
static sword alloc_buffer(/*_ ub4 pos, ub4 iter, ub4 rows _*/);
static sword print_return_data(/*_ int iter _*/);

